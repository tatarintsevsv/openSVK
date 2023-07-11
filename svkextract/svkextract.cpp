#include "svkextract.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <sstream>
#include <vector>
#include "../svkcompose/base64.h"

using namespace std;

svkExtract::svkExtract()
{

}
int svkExtract::__execute(std::string cmd, string *reply)
{    
    printf("        __execute %s\n",cmd.c_str());
    *reply = "";
    FILE* f=popen(cmd.c_str(),"r");
    if(errno!=0||f==NULL){
        syslog(LOG_ERR, "Ошибка открытия процесса %s: %s",cmd.c_str(), strerror(errno));
        return WEXITSTATUS(pclose(f));
    }
    int fd = fileno(f);
    fd_set set;
    struct timeval t;
    t.tv_sec=60;
    char buf[BUFSIZ];
    while(1){        
        FD_ZERO(&set);
        FD_SET(fd,&set);
        select(fd+1,&set,NULL,NULL, &t);
        if(FD_ISSET(fd,&set)){
            int len = read(fd,buf,BUFSIZ);
            if(len){
                *reply+=std::string((char*)buf);
            }else{
                return WEXITSTATUS(pclose(f));
            };
        }
        if(errno!=0){
            syslog(LOG_ERR,"POPEN");
            return WEXITSTATUS(pclose(f));
        }
    }
}
int readline(char* buf, size_t bufsize, int f){
    size_t strlen=0;
    char c;
    size_t readed;
    buf[0]='\0';
    while((readed=read(f,&c,1))==1&&strlen<bufsize-1){
        buf[strlen+1]='\0';
        if(c=='\r')
            continue;
        if(c=='\n')
            return strlen;
        buf[strlen++]=c!='\t'?c:' ';

    }
    return (readed==1)?strlen:-1;
}
void svkExtract::parseHeader(dict* headers){
    //lseek(fd,0,SEEK_SET);
    char* line=(char*)malloc(1024);
    int len;
    string lastHeader="";
    while (readline(line, 1024, fd) != -1) {
        string s = line;
        if(s.empty())
            break;
        int space=s.find(" ");
        string header = s.substr(0,space-1);
        string value = s.substr(space+1,s.length());
        if(space!=0){
            (*headers)[header] = value;
            lastHeader=header;
        }else{
            (*headers)[lastHeader] = (*headers)[lastHeader]+'\n'+value;
        }
    }
    free(line);
}

void svkExtract::readPart(string boundary, mimepart *part)
{
    part->headers.clear();
    parseHeader(&part->headers);
    part->start = lseek(fd, 0, SEEK_CUR);
    part->end = part->start;
    string prevline=boundary;
    string line;
    char* s=(char*)malloc(1024);s[0]='-';s[1]='\0';
    do{
        prevline = s;
        int len=readline(s, 1024, fd);
        line = s;
        if(len==-1)
            break;
    }while((!prevline.empty())&&(line!=string("--"+boundary))&&(line!=string("--"+boundary+"--")));
    part->end = lseek(fd, 0, SEEK_CUR) - line.length() - 4;
    free(s);
}

int svkExtract::run(char* configRoot, char* filename)
{
    openlog("SVKextract",LOG_CONS|LOG_PID,LOG_MAIL);
    configInit(CONFIG_XML,"SVKextract");
    configSetRoot(configRoot);

    if((fd=open(filename,O_RDONLY))<=0){
        syslog(LOG_ERR,"Ошибка открытия файла %s (%s)",filename,strerror(errno));
        return -2;
    }    
    //fd=open(filename,O_RDONLY);
    parseHeader(&headers);
    string xp = "rule[contains('"+headers["From"]+"',@from)]/";
    string extractto = xmlReadString(xp+"@extractto");
    string out = xmlReadString(xp+"@out");
    // --> //config/stage[3]/source[1]/rule[contains('fns440test@ext-gate.svk.mskgtu.cbr.ru',@from)]/@extractto
    if(extractto.length()==0){
        close(fd);
        closelog();
        return 0;
    }
    syslog(LOG_INFO,"Обработка письма %s",filename);
    // Debug headers output

    syslog(LOG_INFO,"[От]: %s",headers["From"].c_str());
    syslog(LOG_INFO,"[Кому]: %s",headers["To"].c_str());
    syslog(LOG_INFO,"[Тема]: %s",headers["Subject"].c_str());
    cout<<"================================\n"<<"from: "<<headers["From"]<<endl<<"to: "<<headers["To"]<<endl<<"subj: "<<headers["Subject"]<<endl<<"================================\n";

    int p=headers["Content-Type"].find("boundary=")+10;
    string boundary=headers["Content-Type"].substr(p);
    boundary=boundary.substr(0,boundary.find("\""));
    char line[1024];
    readline((char*)&line,1024,fd);
    if(string(line)!=("--"+boundary)){
        syslog(LOG_ERR,"Структура сообщения нарушена (отсутствует или неверный boundary)");
        close(fd);
        return -3;
    }
    mimepart part;
    do{

        readPart(boundary,&part);
        if(part.headers.find("Content-Disposition")!=part.headers.end()){
            int p=part.headers["Content-Disposition"].find("filename=")+10;
            string attachment=part.headers["Content-Disposition"].substr(p);
            attachment=attachment.substr(0,attachment.find("\""));
            cout<<"Найдена MIME часть с вложением "<<attachment<<endl;
            int c=0;
            string dest = extractto+attachment;
            while(access( dest.c_str(), F_OK ) != -1)
                dest=extractto+to_string(++c)+"_"+attachment;
            int f;
            if((f=open(dest.c_str(),O_CREAT|O_WRONLY))<=0){
                syslog(LOG_ERR,"Ошибка открытия файла для записи %s (%s)",dest.c_str(),strerror(errno));
                return -2;
            }
            int sz=part.end-part.start;
            char* content=(char*)malloc(sz);
            off_t curr_p=lseek(fd, 0, SEEK_CUR);
            lseek(fd, part.start, SEEK_SET);
            read(fd,content,sz);
            //Content-Transfer-Encoding: base64
            string cte = part.headers["Content-Transfer-Encoding"];
            for (auto & c: cte) c = toupper(c);
            if(cte=="BASE64"){
                vector<BYTE> c = base64_decode(content);
                copy(c.begin(), c.end(), content);
                write(f,content,c.size());
            }else{
                write(f,content,sz);
            }
            close(f);
            free(content);
            lseek(fd, curr_p, SEEK_SET);
            //dest = extractto+
            //( access( name.c_str(), F_OK ) != -1 )
        }


        //cout<<"-----------------------------\n"<<part.end-part.start<<" bytes\n";
    }while(part.end+4!=part.start);

    if(out.length()){
        out+="cur"+string(filename).substr(string(filename).find_last_of("/"));
        if(rename(filename,out.c_str())!=0){
                syslog(LOG_ERR,"Ошибка переноса письма в %s (%s)",out.c_str(),strerror(errno));
                return errno;
            };
    }


    close(fd);
    closelog();
    return 0;
}
