#ifndef SVKMAIN_H
#define SVKMAIN_H
#include <string>
#include <map>
#include <syslog.h>
#include "../config/configcxx.h"

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
    void parseHeader(dict* headers);
    void readPart(string boundary, mimepart* part);
    dict headers;
public:
    svkExtract();
    int run(char* configRoot, char* filename);
};


#endif // SVKMAIN_H
