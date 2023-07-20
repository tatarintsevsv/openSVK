#ifndef SVKMAIN_H
#define SVKMAIN_H
#include <stdio.h>
#include <string>
#include <syslog.h>

#include "../common/linuxdir.h"
#include "../config/config.h"
#include "../config/configcxx.h"

#define CONFIG_XML "./config.xml"
#define BINPATH "./"

using  namespace std;
namespace std{
class svkmain;
}

class svkCompose{
protected:
    void prepareFile(string filename);
    void processDir(string node);
private:
    int seed;
    std::string root;    
    string facility;
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
