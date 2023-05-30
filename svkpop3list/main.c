#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>


#define BUFSIZE 1024
char facility[BUFSIZE]={0};
char host[BUFSIZE]={0};
char user[BUFSIZE]={0};
char password[BUFSIZE]={0};
char maildir[BUFSIZE]={0};


static size_t curl_read(void *ptr, size_t size, size_t nmemb, void* userdata){
    char* buf=malloc(size * nmemb+1);
    buf[size * nmemb]='\0';
    memcpy(buf,ptr,size * nmemb);
    printf("%s",buf);
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
    openlog("SVK_POP3",LOG_CONS|LOG_PID,LOG_MAIL);
    syslog(LOG_DEBUG,"using config %s",argv[1]);    
    if(argc<2){
        return -1;
    }

    char *buf = NULL;
    size_t len;
    FILE* f = fopen(argv[1],"r");
    if(f==NULL){
        syslog(LOG_CRIT, "%s", "can't read config file");
        return -1;
    }
    while (getline(&buf, &len, f) != -1) {
        char* trimmed = trim(buf);
        if(strlen(trimmed)==0){
            // empty string
        }else
        if(trimmed[0]=='#'){
            // comment
        }else
        if(strncmp(trimmed,"facility",8)==0){
            getparam(facility,trimmed,8);
            closelog();
            openlog(facility,LOG_CONS|LOG_PID,LOG_MAIL);
        }else
        if(strncmp(trimmed,"host",4)==0){
            getparam(host,trimmed,4);
        }else
        if(strncmp(trimmed,"user",4)==0){
            getparam(user,trimmed,4);
        }else
        if(strncmp(trimmed,"password",8)==0){
            getparam(password,trimmed,8);
        }else
        if(strncmp(trimmed,"maildir",7)==0){
            getparam(maildir,trimmed,7);
        }
        free(trimmed);
        free(buf);
        buf=NULL;
    }

    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();

    if(curl) {
      curl_easy_setopt(curl, CURLOPT_USERNAME, user);
      curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
      //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
      curl_easy_setopt(curl, CURLOPT_URL, host);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "UIDL");
      curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_response_headers);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_read);


      /* Perform the list */
      res = curl_easy_perform(curl);

      /* Check for errors */
      if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

      /* Always cleanup */
      curl_easy_cleanup(curl);
    }
    printf("\n");

    return (int)res;
}
