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
#include <semaphore.h>
#include <time.h>
#include <sstream>
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
FILE* svkMain::__executeasync(string cmd){

    printf("        __executeasync %s\n",cmd.c_str());
    FILE* f=popen(cmd.c_str(),"r");
    if(f==NULL){
        syslog(LOG_ERR, "Ошибка открытия процесса: %s", strerror(errno));
        return NULL;
    }
    return f;
}

int svkMain::__execute(std::string cmd, string *reply)
{    
    printf("        __execute %s\n",cmd.c_str());
    *reply = "";
    FILE* f=popen(cmd.c_str(),"r");
    if(f==NULL){
        syslog(LOG_ERR, "Ошибка открытия процесса: %s", strerror(errno));
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
            //syslog(LOG_ERR,"POPEN error %s",strerror(errno));
            return WEXITSTATUS(pclose(f));
        }
    }
    return 0;
}

void svkMain::poolProcessing(vector<string> lines,string sem_wait,string sem_count,int instances){
    FILE* fi[instances];
    string replies[instances];
    while(lines.size()){
        sem_t *sem;
        sem_t *sem_c;
        sem_unlink(sem_wait.c_str());
        sem_unlink(sem_count.c_str());
        sem = sem_open(sem_wait.c_str(), O_CREAT, S_IRWXU,0);
        sem_c = sem_open(sem_count.c_str(), O_CREAT, S_IRWXU,0);
        for(int i=0;i<instances;++i)
            fi[i]=NULL;
        int fd=0;
        int activeinstances=0;
        for(int i=0;i<instances&&lines.size();++i,++activeinstances){
            fi[i] = __executeasync(lines.back());
            lines.pop_back();
            if(fileno(fi[i])>fd)
                fd=fileno(fi[i]);
        }
        int val=0;
        while(val<activeinstances)
            sem_getvalue(sem_c, &val);
        for(int i=0;i<activeinstances;++i)
            sem_post(sem);
        char* buf[BUFSIZ];
        sem_close(sem_c);
        sem_close(sem);
        sem_unlink(sem_wait.c_str());
        sem_unlink(sem_count.c_str());

        fd_set set;
        struct timeval t;
        t.tv_sec=10;
        while(1){
            FD_ZERO(&set);
            for(int i=0;i<instances;++i)
                if(fi[i]!=NULL)
                    FD_SET(fileno(fi[i]),&set);
            select(fd+1,&set,NULL,NULL, &t);
            bool isactive=false;
            for(int i=0;i<instances;++i){
                if(fi[i]!=NULL&&FD_ISSET(fileno(fi[i]),&set)){
                    int len = read(fileno(fi[i]),buf,BUFSIZ);
                    if(len){
                        replies[i]+=std::string((char*)buf);
                        isactive=true;
                    }
                    else fi[i]=NULL;
                }
            }
            if(!isactive)
                break;
        }// wait for finished
    }// whole messages
}
int svkMain::stage_pop3(string configRoot){
    std::ostringstream oss;
    oss<<BINPATH<<"svkpop3list "<<configRoot;
    std::string cmd = oss.str();
    std::string list="";    
    int r = __execute(cmd,&list);
    if(r!=0)
        return r;
    int instances=xmlReadInt("pop3/@instances",1);
    FILE* fi[instances];
    string replies[instances];    
    vector<string> lines;// = splitString(list);
    for(const auto &l: splitString(list)){
        oss.str("");
        oss<<BINPATH<<"svkpop3get "<<configRoot<<" "<< l;
        lines.push_back(oss.str());
    }
    poolProcessing(lines,"/svkpop3sem_wait","/svkpop3sem_count",instances);

    return 0;
}

int svkMain::stage_smtp(string configRoot)
{    
    string source=xmlReadString("@source");
    int instances=xmlReadInt("@instances",1);
    std::ostringstream oss;
    vector<string> lines;
    if(source.empty())
        return 0;

    const fs::path dir{source+"cur/"};
    for(const auto& entry: fs::directory_iterator(dir)){
        if (!entry.is_regular_file())
            continue;
        const auto filenameStr = entry.path().filename().string();
        string filepath=source+"cur/"+filenameStr;
        oss.str("");
        oss<<BINPATH<<"svksmtp "<<configRoot<<" "<<filenameStr;
        lines.push_back(oss.str());
    }
    string lockfile=source+"dovecot-uidlist.lock";
    struct timespec ts_start;
    clock_gettime(CLOCK_REALTIME, &ts_start);
    struct timespec ts_now;
    while(access(lockfile.c_str(),F_OK)==0){
        clock_gettime(CLOCK_REALTIME, &ts_now);
        if(ts_now.tv_sec>ts_start.tv_sec+60){
            syslog(LOG_ERR,"Блокировка не снята в течение 60сек. %s",lockfile.c_str());
            return 1;
        }
    };
    int fl=open(lockfile.c_str(),O_RDWR|O_CREAT);
    write(fl,"LOCKED",6);
    close(fl);
    poolProcessing(lines,"/svksmtpsem_wait","/svksmtpsem_count",instances);

    unlink(lockfile.c_str());
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
}

int svkMain::stage_extract(string configRoot)
{
    string source=xmlReadString("@in");
    if(source.empty())
        return -1;
    source+="tmp/";
    const fs::path dir{source};
    for(const auto& entry: fs::directory_iterator(dir)){
        if (!entry.is_regular_file())
            continue;
        const auto filenameStr = entry.path().filename().string();
        string filepath=source+filenameStr;
        std::ostringstream oss;
        oss.str("");
        oss<<BINPATH<<"svkextract "<<configRoot<<" "<< filepath;
        string cmd = oss.str();
        string list="";
        int r = __execute(cmd,&list);
        if(r!=0)
            ;// TODO analize return code
    }
    source=xmlReadString("@in");
    const fs::path dir2{source+"tmp/"};
    for(const auto& entry: fs::directory_iterator(dir2)){
        if (!entry.is_regular_file())
            continue;
        const auto filepath = source+"tmp/"+entry.path().filename().string();
        string out=source+"cur/"+entry.path().filename().string();
        if(rename(filepath.c_str(),out.c_str())!=0){
                syslog(LOG_ERR,"Ошибка переноса письма в %s (%s)",out.c_str(),strerror(errno));
                return errno;
        }else {
            syslog(LOG_ERR,"Не найдено правила для обработки письма. Перенесено в %s",out.c_str());
        }
    }
    return 0;
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
        string t = xmlReadString("@type");
        int res=0;
        switch (stages[t]) {
            case stageTypes::pop3:
                res = stage_pop3(path);
                break;
            case stageTypes::telnet:
                res = stage_telnet(path);
                break;
            case stageTypes::compose:
                res = stage_compose(path);
                break;
            case stageTypes::smtp:
                {
                    int smtpCount = 0;
                    try{
                        smtpCount = stoi(configGetNodeSet(string("count("+path+"smtp"+")").c_str()));
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        syslog(LOG_ERR,"Не указаны узлы отправки smtp");
                    }
                    for(int smtp=0;smtp<smtpCount;smtp++){
                        string xp = path+"smtp["+to_string(smtp+1)+"]/";
                        configSetRoot(xp.c_str());
                        printf("    Обработка %s\n",xp.c_str());
                        if(stage_smtp(xp)!=0)
                            ;// TODO: check errors
                    }
                }
                break;
            case stageTypes::extract:
                {
                    int mdaCount = 0;
                    try{
                        mdaCount = stoi(configGetNodeSet(string("count("+path+"source"+")").c_str()));
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        syslog(LOG_ERR,"Не указаны узлы отправки smtp");
                    }
                    for(int rule=0;rule<mdaCount;rule++){
                        string xp = path+"source["+to_string(rule+1)+"]/";
                        configSetRoot(xp.c_str());
                        res |= stage_extract(xp);
                    }
                }
                break;
            default:
                syslog(LOG_ERR, "Ошибка кофигурации. Неизвестный тип шага '%s'",t.c_str());
                break;
        }
        if(res!=0)
            ;//TODO: check result

    }
    configClose();
    syslog(LOG_INFO,"обработка завершена");
    closelog();
    return 0;
}
