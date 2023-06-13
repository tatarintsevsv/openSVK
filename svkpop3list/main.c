#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>
#include "../config/config.h"

#define CONFIG_XML "/home/arise/qt/CBR_SVK/bin/config.xml"
#define BUFSIZE 1024

int msgcount=0;


static size_t curl_read(void *ptr, size_t size, size_t nmemb, void* userdata){
    char* buf=malloc(size * nmemb+1);
    buf[size * nmemb]='\0';
    memcpy(buf,ptr,size * nmemb);
    printf("%s",buf);
    syslog(LOG_DEBUG,"> %s",buf);
    for(int i=0;i<size * nmemb;i++)
        if(buf[i]=='\n')
            msgcount++;
    free(buf);
    return size * nmemb;
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
    if(argc<2){
        return -1;
    }    
    char *buf = NULL;
    size_t len;
    char* facility;
    char* host;
    char* user;
    char* password;
    bool uidl;
    configSetRoot(argv[1]);
    facility=configReadString("@facility","SVK_POP3");
    if(!configInit(CONFIG_XML,"pop3")){
        printf("error configInit\n");
        return 1;
    }
    openlog(facility,LOG_CONS|LOG_PID,LOG_MAIL);
    syslog(LOG_DEBUG,"Используем конфигурацию %s",argv[1]);

    buf = configReadString("pop3/@host","");
    host = malloc(strlen(buf)+8);
    sprintf(host,"pop3://%s",buf);
    free(buf);
    user=configReadString("pop3/@username","");
    password = configReadString("pop3/@password","");    
    uidl = configReadInt("pop3/@uidl",0)==1;
    buf = NULL;

    CURL *curl;
    CURLcode res = CURLE_OK;
    curl = curl_easy_init();

    if(curl) {
        syslog(LOG_INFO,"Получение списка сообщений на сервере");
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, host);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, uidl?"UIDL":"LIST");
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_response_headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_read);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            syslog(LOG_ERR,"Ошибка транспорта %i: %s",res,curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    printf("\n");
    syslog(LOG_INFO,"На сервере %i сообщений",msgcount);


    return (int)res;
}
