#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include "../config/config.h"

#define CONFIG_XML "./config.xml"
#define BUFSIZE 1024

/**
 * @brief Буфер чтения
 */
char* buffer = NULL;
/**
 * @brief Количество данных в буфере
 */
size_t bufferSize = 0;
/**
 * @brief Файловый дескриптор для чтения файла
 */
int f = 0;

/**
 * @brief Счетчик синхронизации семафора
 */
int sem_counter_val;

/**
 * @brief callback для данных libcurl
 * @details https://curl.se/libcurl/c/CURLOPT_READFUNCTION.html
*/
static size_t curl_read(void *ptr, size_t size, size_t nmemb, void* userdata){
    if(size * nmemb!=0){
        char* buf=malloc(size * nmemb+1);
        buf[size * nmemb]='\0';
        memcpy(buf,ptr,size * nmemb);        
        syslog(LOG_DEBUG,"%s",buf);
        free(buf);
    }    
    return read(f,ptr,size * nmemb);
}

/**
 * @brief callback для данных libcurl при получении заголовков
 * @details https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
*/
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
    openlog("SVK_SMTP",LOG_CONS|LOG_PID,LOG_MAIL);
    if(argc<3){
        return -2;
    }
    char *buf = NULL;
    char* facility;
    char* host;
    char* user;
    char* password;    
    char* from;
    char* recipients;
    char* sent;
    // sem sync
#ifndef disablesync
    sem_t *sem;
    sem_t *sem_c;
    sem_c = sem_open("/dev/shm/svksmtpsem_count", O_RDWR);
    if(sem_c==SEM_FAILED){
        syslog(LOG_ERR,"Ошибка открытия семафора svksmtpsem_count %s",strerror(errno));
        return -1;
    }
    sem_post(sem_c);
    sem = sem_open("/dev/shm/svksmtpsem_wait", O_RDWR);
    if(sem==SEM_FAILED){
        syslog(LOG_ERR,"Ошибка открытия семафора svksmtpsem_wait %s",strerror(errno));
        return -1;
    }
    sem_getvalue(sem_c, &sem_counter_val);
    syslog(LOG_INFO,"Ожидание синхронизации потоков #%i",sem_counter_val);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec+=60;
    if(sem_timedwait(sem, &ts)==-1){
        syslog(LOG_INFO,"Ошибка синхронизации %s",strerror(errno));
        return -1;
    }
    sem_close(sem);
    sem_close(sem_c);
    syslog(LOG_INFO,"Синхронизация завершена");
#endif

    if(!configInit(CONFIG_XML,"smtp"))
        return -4;
    configSetRoot(argv[1]);
    facility=configReadString("../@facility","SVK_SMTP");    
    if(strlen(facility)){
        closelog();
        openlog(facility,LOG_CONS|LOG_PID,LOG_MAIL);
    }
    sent=configReadPath("@sent","");
    char* filename=malloc(BUFSIZE);    
    buf = configReadPath("@source","./");
    sprintf(filename,"%scur/%s",buf,argv[2]);
    free(buf);
    f=open(filename,O_RDONLY);
    if(f<=0){
        syslog(LOG_ERR,"Ошибка открытия файла '%s' (%s)",filename,strerror(errno));
        free(filename);
        return -3;
    }

    syslog(LOG_DEBUG,"Используем конфигурацию %s",argv[1]);
    buf = configReadString("@host","");
    host = malloc(strlen(buf)+8);
    sprintf(host,"smtp://%s",buf);
    free(buf);
    user=configReadString("@username","");
    password = configReadString("@password","");    
    from = configReadString("@from","");
    recipients = configReadString("@recipients","");
    buf = NULL;
    struct stat stat_buf;
        stat(filename, &stat_buf);
    int size = stat_buf.st_size;
    syslog(LOG_INFO,"Отправка '%s' (%i байт) от %s. Корреспонденты %s",argv[2],size,from,recipients);

    CURL *curl;
    CURLcode res = CURLE_OK;
    curl = curl_easy_init();
    if(curl) {
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, host);
        curl_easy_setopt(curl, CURLOPT_USERNAME, user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_response_headers);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION,curl_read);
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
        close(f);
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

            if(sent!=NULL&&strlen(sent)>0){
                char* s=malloc(BUFSIZE);
                sprintf(s,"%scur/%s",sent,argv[2]);
                if(rename(filename,s)!=0){
                    syslog(LOG_ERR,"Ошибка переноса письма %s в %s (%s)",filename,s,strerror(errno));
                }else{
                    syslog(LOG_INFO,"Письмо перенесено в %s",s);
                }
                free(s);
            }else{
                syslog(LOG_INFO,"Письмо удалено");
                unlink(filename);
            };
            free(buf);
        }
        curl_easy_cleanup(curl);
    };
    free(facility);
    free(sent);

    free(host);
    free(user);
    free(password);

    free(from);
    free(recipients);
    free(filename);
    return (int)res;
}
