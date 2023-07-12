#ifndef SVKMAIN_H
#define SVKMAIN_H
#include <string>
#include <map>
#include <syslog.h>

#include "../config/config.h"

#define CONFIG_XML "./config.xml"
#define BINPATH "./"

using  namespace std;
namespace std{
class svkmain;
}
typedef map<string,string> dict;
struct mimepart{
    dict headers;
    off_t start;
    off_t end;
};

class svkExtract{
private:
    int fd;
    std::string xmlReadString(string path,string def=""){
        char* r = configReadString(path.c_str(),def.c_str());
        std::string res = std::string(r);
        free(r);
        return res;
    };
    int xmlReadInt(std::string path, int def=0){
         return configReadInt(path.c_str(), def);
    };
    void parseHeader(dict* headers);
    void readPart(string boundary, mimepart* part);
    dict headers;
public:
    svkExtract();
    int run(char* configRoot, char* filename);
};


#endif // SVKMAIN_H
