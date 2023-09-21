#include "svkmain.h"

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


FILE* svkMain::__executeasync(const string &cmd){

    printf("        __executeasync %s\n",cmd.c_str());
    FILE* f=popen(cmd.c_str(),"r");
    if(f==NULL){
        syslog(LOG_ERR, "Ошибка открытия процесса: %s", strerror(errno));
        return NULL;
    }
    return f;
}

int svkMain::__execute(const string &cmd, string *reply)
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
    struct timeval t = {0};
    t.tv_sec=60;
    char* buf=(char*)malloc(8196);
    while(1){        
        FD_ZERO(&set);
        FD_SET(fd,&set);
        select(fd+1,&set,NULL,NULL, &t);
        if(FD_ISSET(fd,&set)){            
            if(read(fd,buf,8196)){
                *reply+=std::string((char*)buf);
            }else{
                free(buf);
                return WEXITSTATUS(pclose(f));
            };
        }
    }
    free(buf);
    return WEXITSTATUS(pclose(f));
}

void svkMain::poolProcessing(vector<string> &lines,const string &sem_wait, const string &sem_count,int instances){
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

svkMain::svkMain()
{
    openlog("SVKMain",LOG_CONS|LOG_PID,LOG_MAIL);
    configInit(CONFIG_XML,"SVKMain");
    configSetRoot("");
    pollinterval = xmlReadInt("//config/global/@pollinterval");
}

svkMain::~svkMain()
{
    configClose();
    syslog(LOG_INFO,"обработка завершена");
    closelog();
}

int svkMain::stage_pop3(const string &configRoot){
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
    poolProcessing(lines,"/dev/shm/svkpop3sem_wait","/dev/shm/svkpop3sem_count",instances);

    return 0;
}

int svkMain::stage_execute(const string &configRoot)
{
    string cmd=xmlReadString("execute/@command");
    string reply;
    syslog(LOG_INFO,"Запуск внешней программы %s",cmd.c_str());
    int res = __execute(cmd,&reply);
    if(xmlReadInt("execute/@verbose",0)==1){
        for(const auto &s:splitString(reply))
            syslog(LOG_INFO,"%s",s.c_str());
    };
    syslog(LOG_INFO,"Внешняя программа завершена с кодом %i (%s)",res,res?strerror(errno):"Ok");
    return res;
}

int svkMain::stage_smtp(const string &configRoot)
{    
    string source=xmlReadPath("@source");
    int instances=xmlReadInt("@instances",1);
    std::ostringstream oss;
    vector<string> lines;
    if(source.empty())
        return 0;

    const linuxdir dir{source+"cur/"};
    for(const auto& entry: dir){
        const auto filenameStr = entry;
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
    int fl=open(lockfile.c_str(),O_RDWR|O_CREAT,0644);
    write(fl,"LOCKED",6);
    close(fl);
    poolProcessing(lines,"/dev/shm/svksmtpsem_wait","/dev/shm/svksmtpsem_count",instances);

    unlink(lockfile.c_str());
    return 0;
}

int svkMain::stage_telnet(const string &configRoot)
{
    std::ostringstream oss;
    oss<<BINPATH<<"svktelnet "<<configRoot;
    std::string cmd = oss.str();
    std::string list="";
    int r = __execute(cmd,&list);
    return r;
}

int svkMain::stage_compose(const string &configRoot)
{
    std::ostringstream oss;
    oss<<BINPATH<<"svkcompose "<<configRoot;
    std::string cmd = oss.str();
    std::string list="";
    int r = __execute(cmd,&list);
    return r;
}

int svkMain::stage_extract(const string &configRoot)
{
    string source=xmlReadPath("@in");
    if(source.empty())
        return -1;

    source+="tmp/";
    const linuxdir dir{source};
    for(const auto& entry: dir){
        const auto filenameStr = entry;
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
    source=xmlReadPath("@in");    
    const linuxdir dir2{source+"tmp/"};
    for(const auto& entry: dir2){
        const auto filepath = source+"tmp/"+entry;
        string out=source+"cur/"+entry;
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
    syslog(LOG_INFO,"=================================================");
    syslog(LOG_INFO,"Запуск обработки");
    char* nset = configGetNodeSet("count(//config/stage)");
    int stagesCount = stoi(nset);
    if(nset != NULL)
        free(nset);
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
            case stageTypes::execute:
                res = stage_execute(path); break;
            case stageTypes::pop3:
                res = stage_pop3(path); break;
            case stageTypes::telnet:
                res = stage_telnet(path); break;
            case stageTypes::compose:
                res = stage_compose(path); break;
            case stageTypes::smtp:
                {
                    int smtpCount = 0;
                    try{
                        char * nset = configGetNodeSet(string("count("+path+"smtp"+")").c_str());
                        smtpCount = stoi(nset);
                        free(nset);
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
                        char * nset = configGetNodeSet(string("count("+path+"extract"+")").c_str());
                        mdaCount = stoi(nset);
                        free(nset);
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        syslog(LOG_ERR,"Не указаны правила обработки писем");
                        break;
                    }
                    for(int rule=0;rule<mdaCount;rule++){
                        string xp = path+"extract["+to_string(rule+1)+"]/";
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
    return 0;
}
