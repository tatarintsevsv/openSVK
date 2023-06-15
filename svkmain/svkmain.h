#ifndef SVKMAIN_H
#define SVKMAIN_H
#include <stdio.h>
#include <string>
#include <syslog.h>

#include "../config/config.h"

#define CONFIG_XML "/home/arise/qt/CBR_SVK/bin/config.xml"
#define BINPATH "/home/arise/qt/CBR_SVK/bin/"

using  namespace std;
namespace std{
class svkmain;
}

class svkMain{
protected:
    int stage_pop3(std::string configRoot);
    int stage_smtp(std::string configRoot);
    int stage_telnet(std::string configRoot);
    int stage_compose(std::string configRoot);
private:
    int pollinterval;
    int __execute(std::string cmd, string *reply);
    void forkWork(string cmd, string param1="", string param2="", string param3="");
    std::string xmlReadString(string path,string def=""){
        char* r = configReadString(path.c_str(),def.c_str());
        std::string res = std::string(r);
        free(r);
        return res;
    };
    int xmlReadInt(std::string path, int def=0){
         return configReadInt(path.c_str(), def);
    };
public:
    svkMain();
    int get_pollinterval(){return pollinterval;};
    int run();
};


#endif // SVKMAIN_H
