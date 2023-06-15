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

class svkCompose{
protected:
    void prepareFile(string filename);
    void processDir(string node);
private:
    int rnd;
    std::string root;
    std::string xmlReadString(string path,string def=""){
        char* r = configReadString(path.c_str(),def.c_str());
        std::string res = std::string(r);
        free(r);
        return res;
    };
    int xmlReadInt(std::string path, int def=0){
         return configReadInt(path.c_str(), def);
    };
    string source;
    string result;
    string from;
    string recipients;
    string subject;

public:
    svkCompose(string root);
    int run();

};


#endif // SVKMAIN_H
