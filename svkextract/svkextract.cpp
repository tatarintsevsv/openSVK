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
    string facility=xmlReadString("@facility","SVKextract");
    if(facility.length())
        openlog(facility.c_str(),LOG_CONS|LOG_PID,LOG_MAIL);

    if((fd=open(filename,O_RDONLY))<=0){
        syslog(LOG_ERR,"Ошибка открытия файла %s (%s)",filename,strerror(errno));
        return -2;
    }    
    //fd=open(filename,O_RDONLY);
    parseHeader(&headers);
    string xp = "rule[contains('"+headers["From"]+"',@from)]/";
    string extractto = xmlReadPath(xp+"@extractto");
    string out = xmlReadPath(xp+"@out");

    // Debug headers output
    syslog(LOG_INFO,"Обработка письма %s",filename);
    syslog(LOG_INFO,"[От]: %s",headers["From"].c_str());
    syslog(LOG_INFO,"[Кому]: %s",headers["To"].c_str());
    syslog(LOG_INFO,"[Тема]: %s",headers["Subject"].c_str());
    if(extractto.length()==0){
        close(fd);
        closelog();
        return 0;
    }

    int p=headers["Content-Type"].find("boundary=")+10;
    string boundary=headers["Content-Type"].substr(p);
    boundary=boundary.substr(0,boundary.find("\""));
    char line[1024];
    int len;
    while((len=readline((char*)&line,1024,fd))!=0)
        if(string(line)==("--"+boundary))
            break;
    if(len==0){
        syslog(LOG_ERR,"Структура сообщения нарушена (отсутствует или неверный boundary)");
        close(fd);
        return -3;
    }

    try {
        mimepart part;
        do{

            readPart(boundary,&part);

            if(part.headers.find("Content-Disposition")!=part.headers.end()){
                int p=part.headers["Content-Disposition"].find("filename=")+10;
                string attachment=part.headers["Content-Disposition"].substr(p);
                attachment=attachment.substr(0,attachment.find("\""));
                int c=0;
                string dest = extractto+attachment;
                while(access( dest.c_str(), F_OK ) != -1)
                    dest=extractto+to_string(++c)+"_"+attachment;
                int f;
                if((f=open(dest.c_str(),O_CREAT|O_WRONLY,0644))<=0){
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
                syslog(LOG_INFO,"Вложение сохранено в %s",dest.c_str());
                lseek(fd, curr_p, SEEK_SET);
            }

        }while(part.end+4!=part.start);
    }  catch (...) {
        syslog(LOG_CRIT,"!!! We got an exception!");
    }
    close(fd);

    if(extractto.length()>0||out.length()>0){
        out+="cur"+string(filename).substr(string(filename).find_last_of("/"));
        if(rename(filename,out.c_str())!=0){
                syslog(LOG_ERR,"Ошибка переноса письма в %s (%s)",out.c_str(),strerror(errno));
                return errno;
        }else {
            syslog(LOG_INFO,"Обработано. Перенесено в %s",out.c_str());
        }
    }else{
        unlink(filename);
        syslog(LOG_INFO,"Обработано. Удалено из входящих");
    }

    closelog();
    return 0;
}
