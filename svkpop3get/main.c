#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>
#include "../config/config.h"

#define CONFIG_XML "/home/arise/qt/CBR_SVK/bin/config.xml"
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

char *trim(char *str)
{
  char *end;
  char* res = malloc(1);res[0]='\0';
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0) return res;

  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  res = realloc(res,strlen(str)+1);
  memset(res,'\0',strlen(str)+1);
  memcpy(res,str, end-str+1);
  return res;
}

void getparam(char* dest,char* val,int start){
    char* v = trim(&val[start]);
    memccpy(dest,v,0,strlen(v));
    free(v);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
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
            syslog(LOG_INFO,"Проверка ID сообщения %s %s",argv[2],argv[3]);
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
        sprintf (filename, "%s/%lu_%s_%i",maildir,(unsigned long)time(NULL),argv[2],rand());
        f = fopen(filename,"wb");
        if(f==NULL){
            syslog(LOG_ERR,"Ошибка открытия файла для записи: %s",filename);                        
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
                remove(filename);
            }else{
                fclose(f);
                syslog(LOG_INFO,"Получено сообщение %s. Размер %lu байт. Сохранено в %s",argv[2],ftell(f),filename);
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
    printf("%s\n", res==CURLE_OK?filename:"-");
    return (int)res;
}
