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

#define CONFIG_XML "/home/arise/qt/CBR_SVK/bin/config.xml"
#define BUFSIZE 8192

char* user;
char* password;
CURL *curl;
int timeout;
static size_t curl_read(char *ptr, size_t size, size_t nmemb, char *userdata){
//    size_t len = fread(ptr,size,nmemb,fd);
    return CURL_READFUNC_PAUSE;

}
static size_t curl_write(char *ptr, size_t size, size_t nmemb, char *userdata){
    char* buf=malloc(size * nmemb+1);
    memcpy(buf,ptr,size * nmemb);
    buf[size * nmemb]='\0';

    syslog(LOG_DEBUG,"W %s",buf);
    free(buf);
    return size * nmemb;
}


int negotiate(){
    time_t starttime=time(NULL);
    CURLcode res=CURLE_OK;
    size_t iolen;
    char buf[BUFSIZE];

    while(time(NULL)<starttime+timeout){
        usleep(300);
        res = curl_easy_recv(curl, buf, BUFSIZE, &iolen);
        if(res!=CURLE_OK&&res!=CURLE_AGAIN)
            syslog(LOG_ERR,"Ошибка транспорта %i: %s",res,curl_easy_strerror(res));
        //    break;
        //char bufx[100]={0xff, 0xfe, 0x18,
        //              0xff, 0xfe, 0x20,
        //              0xff, 0xfe, 0x23,
        //              0xff, 0xfe, 0x27,
        //              0xff, 0xfb, 0x01,
        //              0xff, 0xfc, 0x01,
        //              0xff, 0xfb, 0x03,
        //              0xff, 0xfc, 0x03,
        //               };
        //* SENT WILL BINARY
        //* SENT DO BINARY
        //* SENT WILL SUPPRESS GO AHEAD
        //* SENT DO SUPPRESS GO AHEAD

        //WILL 	251 FB	The sender wants to enable an option
        //WON’T 	252 FC	The sender do not wants to enable an option.
        //DO      253 FD	Sender asks receiver to enable an option.
        //DONT 	254 FE	Sender asks receiver not to enable an option.

        //int s;
        //res = curl_easy_send(curl,bufx,24,&s);

        if(iolen!=0){
            for(int i=0;i<iolen;i++){
                switch ((char)buf[i]){
                case((char)249): printf(" GA");break;
                case((char)250): printf(" SB");break;
                case((char)251): printf(" WILL");break;
                case((char)252): printf(" WON’T");break;
                case((char)253): printf(" DO");break;
                case((char)254): printf(" DONT");break;
                case((char)255): printf("   ==IAC==");break;
                case(1 ): printf(" Echo");break;
                case(3 ): printf(" Suppress Go Ahead");break;
                case(5 ): printf(" Status");break;
                case(6 ): printf(" Timing Mark");break;
                case(8 ): printf(" Output Line Width");break;
                case(9 ): printf(" Output Page Size");break;
                case(10): printf(" Output Carriage Return Disposition");break;
                case(11): printf(" Output Horizontal Tabstops");break;
                case(12): printf(" Output Horizontal Tab Disposition");break;
                case(14): printf(" Output Horizontal Tabstops");break;
                case(15): printf(" Output Vertical Tab Disposition");break;
                case(18): printf(" Logout");break;
                case(24): printf(" Terminal Type");break;
                case(25): printf(" End of Record");break;
                case(26): printf(" User Identification");break;
                case(31): printf(" Window Size");break;
                case(32): printf(" Terminal Speed");break;
                case(33): printf(" Remote Flow Control");break;
                case(34): printf(" Linemode");break;
                case(35): printf(" X Display Location");break;
                case(36): printf(" Environment Variables");break;
                case(39): printf(" Telnet Environment Option");break;
                default: printf(" ?%i",buf[i]);
                };
                syslog(LOG_DEBUG," \\ ");
            }
        };
    }
    return res;
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
    char* rules;

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
    rules = configReadString("telnet/@rules","");
    timeout = configReadInt("telnet/@timeout",60);
    buf = NULL;


    CURLcode res = CURLE_OK;
    curl = curl_easy_init();

    if(curl) {
        //syslog(LOG_INFO,"Получение списка сообщений на сервере");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_URL, host);
        curl_easy_setopt(curl, CURLOPT_NOBODY,1);
        //curl_easy_setopt(curl,CURLOPT_CONNECT_ONLY,2);
        //curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);
        //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_response_headers);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, curl_read);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
        struct curl_slist *options=NULL;
                 //options = curl_slist_append(NULL, "TTYPE=vt100");

                 curl_easy_setopt(curl, CURLOPT_TELNETOPTIONS, options);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK){
            syslog(LOG_ERR,"Ошибка транспорта %i: %s",res,curl_easy_strerror(res));
        }
        int n;        
        //negotiate();
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
