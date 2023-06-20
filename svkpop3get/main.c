#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <curl/curl.h>
#include "../config/config.h"

#define CONFIG_XML "./config.xml"
#define BUFSIZE 1024

char* buffer = NULL;
size_t bufferSize = 0;
FILE* f = NULL;

static size_t curl_read(void *ptr, size_t size, size_t nmemb, void* userdata){
    if(userdata!=NULL){
        char* buf=malloc(size * nmemb+1);
        buf[size * nmemb]='\0';
        memcpy(buf,ptr,size * nmemb);        
        free(buf);
    }
    return fwrite(ptr,1,size * nmemb,(FILE*)userdata);
}

static size_t curl_response_headers(char *ptr, size_t size, size_t nmemb, char *userdata){
    char* buf=malloc(size * nmemb+1);
    buf[size * nmemb]='\0';
    memcpy(buf,ptr,size * nmemb);
    syslog(LOG_DEBUG,"%s",buf);
    free(buf);
    return size * nmemb;
}

int main(int argc, char *argv[])
{
    int r = open("/dev/random",O_RDONLY);
    int seed = getuid()+time(0);
    read(r,&seed,sizeof(int));
    close(r);
    srand(seed);
    if(argc<4){
        return -1;
    }    
    char *buf = NULL;
    char* facility;
    char* host;
    char* user;
    char* password;
    char* maildir;
    char filename[BUFSIZE]={0};
    char filepath[BUFSIZE]={0};
    bool uidl;
    bool keep;
    configSetRoot(argv[1]);
    facility=configReadString("@facility","SVK_POP3");
    if(!configInit(CONFIG_XML,"pop3"))
        return 1;
    openlog(facility,LOG_CONS|LOG_PID,LOG_MAIL);
    syslog(LOG_DEBUG,"Используем конфигурацию %s",argv[1]);

    buf = configReadString("pop3/@host","");
    host = malloc(strlen(buf)+8);
    sprintf(host,"pop3://%s",buf);
    free(buf);
    user=configReadString("pop3/@username","");
    password = configReadString("pop3/@password","");
    maildir = configReadString("pop3/@maildir","");
    uidl = configReadInt("pop3/@uidl",0)==1;
    keep = configReadInt("pop3/@keep",0)==1;
    buf = NULL;

    CURL *curl;
    CURLcode res = CURLE_OK;
    curl = curl_easy_init();
    if(curl) {
        if(uidl){
            // check message id
            //syslog(LOG_INFO,"Проверка ID сообщения %s %s",argv[2],argv[3]);
            f = open_memstream(&buffer, &bufferSize);
            char cmd[BUFSIZE]={0};
            sprintf (cmd, "UIDL %s", argv[2]);
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_USERNAME, user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
            curl_easy_setopt(curl, CURLOPT_URL, host);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, cmd);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_read);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA,f);
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            res = curl_easy_perform(curl);
            fclose(f);
            buffer[bufferSize]='\0';
            if(res != CURLE_OK){
                syslog(LOG_ERR,"Ошибка транспорта %i: %s",res,curl_easy_strerror(res));
                curl_easy_cleanup(curl);
                free(buffer);
                return (int)res;
            }
            if(strstr(buffer,argv[2])==NULL){
                syslog(LOG_ERR,"обнаружено несоответствие ID сообщения. %s. Ожидается %s",buffer,argv[2]);
            }
            free(buffer);

        }        
        // Receive message
        syslog(LOG_INFO,"Получение сообщения %s",argv[2]);
        char cmd[BUFSIZE]={0};
        sprintf (cmd, "%s/%s",host, argv[2]);        
        char hostname[512]={0};
        gethostname(&hostname[0], 512);
        for(int i=0;hostname[i]!='\0'&&i<512;i++){
            if(hostname[i]=='/')hostname[i]='_';
            if(hostname[i]==':')hostname[i]='_';
        }
        struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long long millisecondsSinceEpoch =
            (unsigned long long)(tv.tv_sec) * 1000 +
            (unsigned long long)(tv.tv_usec) / 1000;

        sprintf (filename, "%010d.%llu.%s",rand(),millisecondsSinceEpoch,(char*)hostname);
        sprintf (filepath, "%stmp/%s",maildir,filename);
        f = fopen(filepath,"wb");
        if(f==NULL){
            syslog(LOG_ERR,"Ошибка открытия файла для записи: %s",filepath);
        }else{
            curl_easy_setopt(curl, CURLOPT_USERNAME, user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
            curl_easy_setopt(curl, CURLOPT_URL, cmd);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "");
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_response_headers);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA,0L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_read);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,f);
            curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                syslog(LOG_ERR,"Ошибка транспорта %i: %s",res,curl_easy_strerror(res));
                fclose(f);
                remove(filepath);
            }else{
                long size = ftell(f);
                fclose(f);

                syslog(LOG_INFO,"Получено сообщение %s. Размер %lu байт. Сохранено в %s",argv[2],size,filepath);
                if(!keep){
                    syslog(LOG_INFO,"Удаление сообщения %s.",argv[2]);
                    curl_easy_setopt(curl, CURLOPT_USERNAME, user);
                    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
                    curl_easy_setopt(curl, CURLOPT_URL, cmd);
                    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELE");
                    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
                    res = curl_easy_perform(curl);
                    if(res != CURLE_OK)
                        syslog(LOG_ERR,"Ошибка транспорта %i: %s",res,curl_easy_strerror(res));
                }
            }
        }
        curl_easy_cleanup(curl);
    }
    char newfn[1024];
    sprintf (newfn, "%scur/%s",maildir,filename);
    free(facility);
    free(host);
    free(user);
    free(password);
    free(maildir);
    if(rename(filepath,newfn)!=0){
        syslog(LOG_ERR,"Ошибка переноса письма в %s (%s)",newfn,strerror(errno));
        return errno;
    };
    printf("%s\n", res==CURLE_OK?newfn:"-");
    return (int)res;
}
