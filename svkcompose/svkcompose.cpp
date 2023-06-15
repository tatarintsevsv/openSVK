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
#include <filesystem>
#include <uuid/uuid.h>
#include <sys/stat.h>

namespace fs = std::filesystem;
using namespace std;

void svkCompose::prepareFile(string filename){
    char c_hostname[BUFSIZ]={0};
    gethostname(&c_hostname[0], BUFSIZ);
    string hostname = string(c_hostname);
    for(int i=0;i<hostname.length();i++)
        if(hostname[i]=='\\'||hostname[i]==':')
            hostname[i]='_';
    struct stat stat_buf;
    stat(string(source+filename).c_str(), &stat_buf);
    syslog(LOG_INFO,"Подготовка файла к отправке %s (%lu байт)",string(source+filename).c_str(),stat_buf.st_size);

    string filepath = result+std::to_string((unsigned long)time(NULL))+"."+std::to_string((unsigned long)rnd++)+"."+hostname;
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
    fread(buf,1,stat_buf.st_size,fr);
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
    fprintf(f,"\n\n\n%s\n",b64.c_str());
    fclose(f);
    if(remove(string(source+filename).c_str())!=0){
        syslog(LOG_ERR,"Ошибка удаления исходного файла %s",string(source+filename).c_str());
    };
}
void svkCompose::processDir(string node)
{
    configSetRoot(node.c_str());
    source=xmlReadString("@in");
    result=xmlReadString("@out");
    from=xmlReadString("@from");
    recipients=xmlReadString("@recipients");
    subject=xmlReadString("@subject");

    if(source.empty()||result.empty())
        return;
    const fs::path dir{source};
    for(const auto& entry: fs::directory_iterator(dir)){
        if (!entry.is_regular_file())
            continue;
        const auto filenameStr = entry.path().filename().string();
        prepareFile(filenameStr);
    }
}

svkCompose::svkCompose(string root)
{
    configInit(CONFIG_XML,"SVKCompose");
    this->root = root;
    configSetRoot(root.c_str());
    string facility=xmlReadString("@facility","SVKCompose");
    openlog(facility.c_str(),LOG_CONS|LOG_PID,LOG_MAIL);
    configSetRoot(root.c_str());
    syslog(LOG_INFO,"Подготовка писем");
    rnd = rand();

}

int svkCompose::run()
{
    string xpath = "count("+root+"source"+")";
    int count = stoi(configGetNodeSet(xpath.c_str()));
    for(int i=1;i<count+1;i++){
        processDir(root+"source["+std::to_string(i)+"]/");
    }
    configClose();
    return 0;
}
