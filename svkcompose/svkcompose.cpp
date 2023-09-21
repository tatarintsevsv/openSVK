#include "svkcompose.h"
#include "base64.h"
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
#include <uuid/uuid.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iomanip>

using namespace std;

void svkCompose::prepareFile(const string &filename){
    char c_hostname[BUFSIZ]={0};
    gethostname(&c_hostname[0], BUFSIZ);
    string hostname = string(c_hostname);
    for(int i=0;i<hostname.length();i++)
        if(hostname[i]=='\\'||hostname[i]==':')
            hostname[i]='_';
    struct stat stat_buf;
    stat(string(source+filename).c_str(), &stat_buf);
    syslog(LOG_INFO,"Подготовка файла к отправке %s (%lu байт)",string(source+filename).c_str(),stat_buf.st_size);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long millisecondsSinceEpoch =
        (unsigned long long)(tv.tv_sec) * 1000 +
        (unsigned long long)(tv.tv_usec) / 1000;

    string mailname =std::to_string(rand())+"."+std::to_string(millisecondsSinceEpoch)+"."+hostname;

    string filepath = result+"tmp/"+mailname;
    FILE* fr = fopen(string(source+filename).c_str(),"r");
    if(fr==NULL){
        syslog(LOG_ERR,"Ошибка открытия файла для чтения: %s",filename.c_str());
        return;
    }
    FILE* f = fopen(filepath.c_str(),"w");
    if(f==NULL){
        syslog(LOG_ERR,"Ошибка открытия файла для записи: %s",filepath.c_str());
        return;
    }


    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
    fprintf(f,"Reply-To: %s\nFrom: %s\nTo: %s\nSubject:%s\nDate: %s\nMIME-Version: 1.0\n",
            from.c_str(),from.c_str(),recipients.c_str(),subject.c_str(),string(buffer).c_str());
    uuid_t tguid;
    uuid_generate(tguid);
    std::ostringstream ss;
    ss<<"part_";
    for(int i=0;i<sizeof(tguid);i++)
        ss<<std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(tguid[i]);
    string boundary = ss.str();

    string((char*)tguid,sizeof(tguid));
    fprintf(f,"Content-Type: multipart/mixed; boundary=\"%s\"\n\n--%s\n" \
            "Content-Type: text/plain; charset=\"UTF-8\"\n" \
            "Content-Transfer-Encoding: quoted-printable\n\n" \
            "=\n\n" \
            "--%s\n" \
            "Content-Type: application/octet-stream\n" \
            "Content-Disposition: attachment; filename=\"%s\"\n"
            "Content-Transfer-Encoding: base64\n\n",
            boundary.c_str(),boundary.c_str(),boundary.c_str(),filename.c_str());
    BYTE* buf = (BYTE*)malloc(stat_buf.st_size);
    if(buf==NULL){
        fclose(f);
        syslog(LOG_ALERT,"Ошибка выделения памяти");
    }
    size_t readed;
    readed = fread(buf,1,stat_buf.st_size,fr);
    if(readed!=stat_buf.st_size){
        syslog(LOG_ERR,"Считано меньше чем размер файла!");
    }
    string b64 = base64_encode(buf,stat_buf.st_size);
    fclose(fr);
    free(buf);
    string line="";
    for(auto it=b64.begin();it!=b64.end();it++){
        if(line.length()<64){
            line+=*it;
        }else{
            fprintf(f,"%s\n",line.c_str());
            line=*it;
        }
    }
    fprintf(f,"%s\n\n--%s--\n",line.c_str(),boundary.c_str());    
    fclose(f);
    // Maildir
    if(rename(filepath.c_str(),(result+"cur/"+mailname).c_str())!=0)
            syslog(LOG_ERR,"ошибка переноса в cur: %s",strerror(errno));
    //**************
    syslog(LOG_INFO,"Для файла %s подготовлено письмо %s",string(source+filename).c_str(),(result+"cur/"+mailname).c_str());
    if(remove(string(source+filename).c_str())!=0){
        syslog(LOG_ERR,"Ошибка удаления исходного файла %s",string(source+filename).c_str());
    };
}

void svkCompose::processDir(const string &node)
{
    configSetRoot(node.c_str());
    source=xmlReadPath("@in");
    result=xmlReadPath("@out");
    from=xmlReadString("@from");
    recipients=xmlReadString("@recipients");
    subject=xmlReadString("@subject");

    if(source.empty()||result.empty())
        return;
    //****************
    const linuxdir dir{source};
    for(const auto& entry: dir)
        prepareFile(entry);
}

svkCompose::svkCompose(string root)
{
    configInit(CONFIG_XML,"SVKCompose");
    this->root = root;
    configSetRoot(root.c_str());
    facility=xmlReadString("@facility","SVKCompose");
    openlog(facility.c_str(),LOG_CONS|LOG_PID,LOG_MAIL);
    configSetRoot(root.c_str());    
    srand(getuid());

}

int svkCompose::run()
{
    string xpath = "count("+root+"compose"+")";
    char* nset = configGetNodeSet(xpath.c_str());
    int count = stoi(nset);
    free(nset);
    for(int i=1;i<count+1;i++){
        processDir(root+"compose["+std::to_string(i)+"]/");
    }
    configClose();
    return 0;
}
