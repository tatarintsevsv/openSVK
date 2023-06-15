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
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

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
    //chdir(BINPATH);
}
void svkMain::forkWork(string cmd, string param1, string param2, string param3){
    int pipefd[2];
    if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
    }
    pid_t cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) {    /* Child writes to pipe */
        while ((dup2(pipefd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        close(pipefd[1]);
        close(pipefd[0]);
        int rc = execl(cmd.c_str(),cmd.c_str(),param1.empty()?NULL:param1.c_str(),param2.empty()?NULL:param2.c_str(),param3.empty()?NULL:param3.c_str(),NULL);
        _exit(EXIT_FAILURE);
    }
}
int svkMain::__execute(std::string cmd, string *reply)
{    
    printf("__execute %s\n",cmd.c_str());
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
    int r = __execute(cmd,&list);
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
        r = __execute(cmd,&list);
        if(r!=0){
            // TODO analize return code
        }
    }
    return r;
}

int svkMain::stage_smtp(string configRoot)
{
    // TODO: lock maildir
    string source=xmlReadString("smtp/@source");
    string sent=xmlReadString("smtp/@sent");
    if(source.empty())
        return 0;
    const fs::path dir{source};
    for(const auto& entry: fs::directory_iterator(dir)){
        if (!entry.is_regular_file())
            continue;
        const auto filenameStr = entry.path().filename().string();
        std::ostringstream oss;
        oss.str("");        
        string filepath=source+filenameStr;
        oss<<BINPATH<<"svksmtp "<<configRoot<<" "<< filepath;
        string cmd = oss.str();
        string list="";
        int r = __execute(cmd,&list);
        if(r!=0){
            // TODO analize return code
        }else{
            if(!sent.empty())
                if(rename(filepath.c_str(),string(sent+filenameStr).c_str())!=0)
                    syslog(LOG_ERR,"Ошибка переноса письма %s в %s (%s)",filepath.c_str(),string(sent+filenameStr).c_str(),strerror(errno));
        }
    }
    return 0;
}

int svkMain::stage_telnet(string configRoot)
{
    std::ostringstream oss;
    oss<<BINPATH<<"svktelnet "<<configRoot;
    std::string cmd = oss.str();
    std::string list="";
    int r = __execute(cmd,&list);
    return r;
}

int svkMain::stage_compose(string configRoot)
{
    std::ostringstream oss;
    oss<<BINPATH<<"svkcompose "<<configRoot;
    std::string cmd = oss.str();
    std::string list="";
    int r = __execute(cmd,&list);
    return r;
};

int svkMain::run()
{
    openlog("SVKMain",LOG_CONS|LOG_PID,LOG_MAIL);
    configInit(CONFIG_XML,"SVKMain");
    configSetRoot("");
    pollinterval = xmlReadInt("//config/global/@pollinterval");

    syslog(LOG_INFO,"=================================================");
    syslog(LOG_INFO,"Запуск обработки");
    int stagesCount = stoi(configGetNodeSet("count(//config/stage)"));
    for(int stage=1;stage<stagesCount+1;stage++){
        string path="//config/stage["+to_string(stage)+"]/";
        configSetRoot(path.c_str());
        if(xmlReadInt("@enabled")==0)
            continue;
        printf("Обработка %s\n",path.c_str());
        string desc = xmlReadString("description/text()");
        syslog(LOG_INFO,"Этап %i. %s",stage,desc.c_str());
        if(xmlReadString("@type")=="pop3"){
            stage_pop3(path);
        }
        else if(xmlReadString("@type")=="telnet"){
            if(stage_telnet(path)!=0)
                    return -1;
        }
        else if(xmlReadString("@type")=="compose"){
            if(stage_compose(path)!=0)
                    return -1;
        }
        else if(xmlReadString("@type")=="smtp"){
            if(stage_smtp(path)!=0)
                    return -1;
        }
    }
    configClose();    
    syslog(LOG_INFO,"=================================================");
    syslog(LOG_INFO,"обработка завершена");
    closelog();
    return 0;
}
