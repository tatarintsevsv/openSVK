#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdlib.h>

#include <stdbool.h>
#include <curl/curl.h>
#include <time.h>
#include <unistd.h>
#include "../config/config.h"

#define CONFIG_XML "./config.xml"
#define BUFSIZE 8192
#define TELNET_RESULT_OK 1
#define TELNET_RESULT_FAIL 2

char* user;
char* password;
CURL *curl;
int timeout;
char* rules;
char* readbuf;
int bufsize=0;
int bufptr=0;
int global_result=0;

static size_t curl_read(char *ptr, size_t size, size_t nmemb, char *userdata){
//    size_t len = fread(ptr,size,nmemb,fd);

    FILE* f = fopen(rules,"r");
    if(f==NULL){
        syslog(LOG_ERR,"Ошибка открытия файла правил %s",rules);
        return CURL_READFUNC_ABORT;
    }
    size_t len=0;
    int readed=0;
    int cmdlen=0;
    char* await=NULL;
    char* cmd=NULL;
    while ((readed = getline(&await, &len, f)) != -1) {
        cmdlen = getline(&cmd, &len, f);
        if(cmdlen>1&&readed>1&&strlen(readbuf)>0){
            cmd[cmdlen-1]='\0';
            await[readed-1]='\0';
            if(strstr(readbuf,await)!=NULL){
                if((char)cmd[0]!='>'){
                    memcpy(ptr,cmd,size*nmemb);
                }else{
                    if(strstr(cmd,">USERNAME")){
                        syslog(LOG_INFO,"Отправка имени пользователя");
                        cmdlen = strlen(user);
                        memcpy(ptr,user,cmdlen);
                    }
                    else if(strstr(cmd,">PASSWORD")){
                        syslog(LOG_INFO,"Отправка пароля");
                        cmdlen = strlen(password);
                        memcpy(ptr,password,cmdlen);
                    }
                    else if(strstr(cmd,">QUIT_OK ")){
                        global_result = TELNET_RESULT_OK;
                        syslog(LOG_INFO,"Завершение работы. %s",&cmd[9]);
                        cmdlen = -1;
                    }
                    else if(strstr(cmd,">QUIT_FAIL ")){
                        global_result = TELNET_RESULT_FAIL;
                        syslog(LOG_INFO,"Завершение работы. %s",&cmd[11]);
                        cmdlen = -1;
                    }
                    else if(strstr(cmd,">ECHO ")){
                        syslog(LOG_INFO,"%s",&cmd[6]);
                        cmdlen = CURL_READFUNC_PAUSE;
                    }
                }
                if(cmdlen>0&&cmdlen!=CURL_READFUNC_PAUSE){
                    memcpy(ptr+cmdlen,"\n\0",2);
                    cmdlen+=2;
                }
                free(readbuf);
                readbuf = malloc(BUFSIZE);
                bufsize = BUFSIZE;
                bufptr = 0;
                free(await); free(cmd);
                fclose(f);
                return cmdlen;
            }
        }
        free(await); await=NULL; free(cmd); cmd=NULL;
    }
    fclose(f);
    return CURL_READFUNC_PAUSE;

}
static size_t curl_write(char *ptr, size_t size, size_t nmemb, char *userdata){
    if(size*nmemb>0){
        char* buf=malloc(BUFSIZE);
        memcpy(buf,ptr,size * nmemb);
        buf[size * nmemb]='\0';
        readbuf=realloc(readbuf,bufsize+(size * nmemb)+1);
        memcpy(&readbuf[bufptr],ptr,size * nmemb);
        bufptr+=size * nmemb;
        bufsize+=(size * nmemb)+1;
        readbuf[bufptr]='\0';
        syslog(LOG_DEBUG,"%s",buf);
        //printf("%s",readbuf);
        free(buf);
    }
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

    /*
     * https://www.omnisecu.com/tcpip/telnet-commands-and-options.php
    */

    if(argc<2){
        return -1;
    }    
    char *buf = NULL;
    size_t len;
    char* facility;
    char* host;


    if(!configInit(CONFIG_XML,"telnet"))
        return 1;

    configSetRoot(argv[1]);
    facility=configReadString("@facility","SVK_TELNET");
    openlog(facility,LOG_CONS|LOG_PID,LOG_MAIL);
    syslog(LOG_DEBUG,"Используем конфигурацию %s",argv[1]);
    buf = configReadString("telnet/@host","");
    host = malloc(strlen(buf)+10);
    sprintf(host,"telnet://%s",buf);

    user=configReadString("telnet/@username","");
    password = configReadString("telnet/@password","");
    rules = configReadPath("telnet/@rules","");
    timeout = configReadInt("telnet/@timeout",60);
    buf = NULL;


    CURLcode res = CURLE_OK;
    curl = curl_easy_init();
    if(curl) {
        readbuf = malloc(BUFSIZE);
        bufsize = BUFSIZE;
        bufptr = 0;
        syslog(LOG_INFO,"Обработка сценария %s",rules);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_URL, host);
        curl_easy_setopt(curl, CURLOPT_NOBODY,1);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_response_headers);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, curl_read);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
        //struct curl_slist *options=NULL;
        //         //options = curl_slist_append(NULL, "TTYPE=vt100");
        //
        //         curl_easy_setopt(curl, CURLOPT_TELNETOPTIONS, options);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK){
            syslog(LOG_ERR,"Ошибка транспорта %i: %s",res,curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }
    free(facility);
    free(host);
    free(user);
    free(password);
    free(rules);
    configClose();
    return (int)res;
}
