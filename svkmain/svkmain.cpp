#include "svkmain.h"
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


using namespace std;

std::vector<std::string> splitString(const std::string& str)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, '\n')) {
        if(token.length()){
            if(token.at(token.length()-1)=='\r')
                token.pop_back();
            tokens.push_back(token);
        }
    }
    return tokens;
}

svkMain::svkMain()
{
    chdir(BINPATH);
}

int svkMain::__execute(std::string cmd,std::string configRoot, string *reply)
{    
    printf("running %s\n",cmd.c_str());
    *reply = "";
    FILE* f=popen(cmd.c_str(),"r");
    if(errno!=0||f==NULL){
        syslog(LOG_ERR, "Ошибка открытия процесса pop3getи: %s", strerror(errno));
        return WEXITSTATUS(pclose(f));
    }
    int fd = fileno(f);
    fd_set set;
    struct timeval t;
    t.tv_sec=60;
    char buf[BUFSIZ];
    while(1){        
        FD_ZERO(&set);
        FD_SET(fd,&set);
        select(fd+1,&set,NULL,NULL, &t);
        if(FD_ISSET(fd,&set)){
            int len = read(fd,buf,BUFSIZ);
            if(len){
                *reply+=std::string((char*)buf);
            }else{
                return WEXITSTATUS(pclose(f));
            };
        }
        if(errno!=0){
            syslog(LOG_ERR,"POPEN");
            return WEXITSTATUS(pclose(f));
        }
    }
}

int svkMain::stage_pop3(string configRoot){
    std::ostringstream oss;
    oss<<BINPATH<<"svkpop3list "<<configRoot;
    std::string cmd = oss.str();
    std::string list="";
    int r = __execute(cmd,configRoot,&list);
    if(r!=0){
        return r;
    }else{
        //printf("%s",list.c_str());
    }
    vector<string> lines = splitString(list);
    //printf("%s\n",list.c_str());
    for(auto it = lines.rbegin();it!=lines.rend(); ++it){
        oss.str("");
        oss<<BINPATH<<"svkpop3get "<<configRoot<<" "<< (*it).c_str();
        cmd = oss.str();
        //printf("%s\n",cmd.c_str());
        r = __execute(cmd,configRoot,&list);
        if(r!=0){
            // TODO analize return code
        }
    }
    return r;
}

int svkMain::stage_telnet(string configRoot)
{
    std::ostringstream oss;
    oss<<BINPATH<<"svktelnet "<<configRoot;
    std::string cmd = oss.str();
    std::string list="";
    int r = __execute(cmd,configRoot,&list);
    return r;
};

int svkMain::run()
{
    openlog("SVKMain",LOG_CONS|LOG_PID,LOG_MAIL);
    configInit(CONFIG_XML,"SVKMain");
    configSetRoot("");
    string lockfile = xmlReadString("//config/global/@lockfile");

    struct flock lock;
    int fd;

    if((fd = open(lockfile.c_str(), O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR))==0){
        syslog(LOG_ALERT,"Ошибка создания блокировки %s",lockfile.c_str());
        return -1;
    };

    memset(&lock, 0, sizeof(struct flock));
    fcntl(fd, F_GETLK, &lock);
    if(lock.l_type==F_WRLCK || lock.l_type==F_RDLCK){
        syslog(LOG_ALERT,"Обнаружена блокировка %s (один экземпляр уже работает?)",lockfile.c_str());
        close(fd);
        _exit(-1);
    }
    memset(&lock, 0, sizeof(struct flock));
    lock.l_type = F_WRLCK;
    lock.l_start=0;
    lock.l_whence = SEEK_SET;
    lock.l_len=0;
    lock.l_pid = getpid();
    if (fcntl(fd, F_SETLK, &lock) < 0)
        perror("Ошибка блокировки\n");

    syslog(LOG_INFO,"Запуск обработки");
    int stagesCount = stoi(configGetNodeSet("count(//config/stage)"));
    for(int stage=1;stage<stagesCount+1;stage++){
        string path="//config/stage["+to_string(stage)+"]/";
        configSetRoot(path.c_str());        
        if(xmlReadInt("@enabled")==0)
            continue;
        string desc = xmlReadString("description/text()");
        syslog(LOG_INFO,"Этап %i. %s",stage,desc.c_str());
        if(xmlReadString("@type")=="pop3"){
            stage_pop3(path);
        }
        else if(xmlReadString("@type")=="telnet"){
            if(stage_telnet(path)!=0)
                    return -1;
        }
    }
    close(fd);
    remove(lockfile.c_str());
    configClose();
    return 0;
}
