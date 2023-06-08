#include <iostream>
#include <string>
#include "../config/config.h"

#define CONFIG_XML "/home/arise/qt/CBR_SVK/bin/config.xml"

using namespace std;

int main(int argc,char* argv[])
{
    configSetRoot(argv[1]);
    string facility=string(configReadString("@facility","SVK_POP3"));
    /*
    if(!configInit(CONFIG_XML,"pop3"))
        return 1;
    openlog(facility,LOG_CONS|LOG_PID,LOG_MAIL);
    syslog(LOG_DEBUG,"Используем конфигурацию %s",argv[1]);
*/
    cout << "Hello World!" << endl;

    return 0;
}
