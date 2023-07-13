#ifndef SVKMAIN_H
#define SVKMAIN_H
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <syslog.h>
#include "../config/config.h"

#define CONFIG_XML "./config.xml"
#define BINPATH "./"
using  namespace std;

enum class stageTypes{telnet,pop3,smtp,compose,extract,execute};
static map<string,stageTypes> stages {{"telnet",stageTypes::telnet},
                                      {"pop3",stageTypes::pop3},
                                      {"smtp",stageTypes::smtp},
                                      {"compose",stageTypes::compose},
                                      {"extract",stageTypes::extract},
                                      {"execute",stageTypes::execute}};


namespace std{
class svkmain;
}

class svkMain{
protected:
    int stage_pop3(string configRoot);
    int stage_execute(string configRoot);
    int stage_smtp(string configRoot);
    int stage_telnet(string configRoot);
    int stage_compose(string configRoot);
    int stage_extract(string configRoot);
private:
    int pollinterval;
    int __execute(string cmd, string *reply);
    FILE *__executeasync(string cmd);
    void forkWork(string cmd, string param1="", string param2="", string param3="");    
    void poolProcessing(vector<string> lines,string sem_wait,string sem_count,int instances);

    string xmlReadString(string path,string def=""){
        char* r = configReadString(path.c_str(),def.c_str());
        string res = string(r);
        free(r);
        return res;
    };
    string xmlReadPath(string path,string def=""){
        char* r = configReadPath(path.c_str(),def.c_str());
        string res = string(r);
        free(r);
        return res;
    };
    int xmlReadInt(string path, int def=0){
         return configReadInt(path.c_str(), def);
    };


public:
    svkMain();
    int get_pollinterval(){return pollinterval;};
    int run();
};


#endif // SVKMAIN_H
