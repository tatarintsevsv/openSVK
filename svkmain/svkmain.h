#ifndef SVKMAIN_H
#define SVKMAIN_H

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <syslog.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <semaphore.h>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../common/linuxdir.h"
#include "../config/configcxx.h"

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
    int stage_pop3(const string &configRoot);
    int stage_execute(const string &configRoot);
    int stage_smtp(const string &configRoot);
    int stage_telnet(const string &configRoot);
    int stage_compose(const string &configRoot);
    int stage_extract(const string &configRoot);
private:
    int pollinterval;
    int __execute(const string &cmd, string *reply);
    FILE *__executeasync(const string &cmd);
    void poolProcessing(vector<string> &lines,const string &sem_wait, const string &sem_count,int instances);


public:
    svkMain() = default;
    int get_pollinterval(){return pollinterval;};
    int run();
};


#endif // SVKMAIN_H
