#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdbool.h>
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
        syslog(LOG_DEBUG,"%s",buf);
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
    srand(time(NULL));
    if(argc<3){
        return -2;
    }
    f=fopen(argv[2],"r");
    if(f==NULL){
        return -3;
    }
    char *buf = NULL;
    char* facility;
    char* host;
    char* user;
    char* password;
    char* source;
    char* from;
    char* recipients;

    configSetRoot(argv[1]);
    facility=configReadString("@facility","SVK_SMTP");
    if(!configInit(CONFIG_XML,"smtp"))
        return -4;
    openlog(facility,LOG_CONS|LOG_PID,LOG_MAIL);
    syslog(LOG_DEBUG,"Используем конфигурацию %s",argv[1]);
    buf = configReadString("smtp/@host","");
    host = malloc(strlen(buf)+8);
    sprintf(host,"smtp://%s",buf);
    free(buf);
    user=configReadString("smtp/@username","");
    password = configReadString("smtp/@password","");
    source = configReadString("smtp/@source","");
    from = configReadString("smtp/@from","");
    recipients = configReadString("smtp/@recipients","");
    buf = NULL;
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    syslog(LOG_INFO,"Отправка '%s' (%i байт) от %s. Корреспонденты %s",argv[2],size,from,recipients);
    rewind(f);

    CURL *curl;
    CURLcode res = CURLE_OK;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, host);
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_response_headers);
        curl_easy_setopt(curl, CURLOPT_READDATA,f);
        curl_easy_setopt(curl, CURLOPT_HEADER,1L);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);
        struct curl_slist* rcpt_list = NULL;
        for(int i=strlen(recipients)-1;i>=0;i--){
            if(recipients[i]==','){
                recipients[i]='\0';
                rcpt_list = curl_slist_append(rcpt_list, &recipients[i+1]);
            }
        }
        rcpt_list = curl_slist_append(rcpt_list, recipients);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, rcpt_list);
        res = curl_easy_perform(curl);
        curl_slist_free_all(rcpt_list);
        if(res != CURLE_OK){
            syslog(LOG_ERR,"Отправка '%s'. Ошибка транспорта %i: %s",argv[2],res,curl_easy_strerror(res));
        }else{
            double speed=0;
            double time=0;
            int code=0;
            curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed);
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &time);
            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &code);
            syslog(LOG_INFO,"Отправка '%s' завершена. Скорость %.0f байт/сек в течение %.2f сек. Результат: %i", argv[2],speed,time,code);
        }
        curl_easy_cleanup(curl);
    };
    free(facility);
    free(host);
    free(user);
    free(password);
    free(source);
    free(from);
    free(recipients);
    return (int)res;
}
