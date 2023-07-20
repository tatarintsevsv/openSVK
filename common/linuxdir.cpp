#include "linuxdir.h"
linuxdir::linuxdir(string dir){
    struct dirent **namelist;
    int filescount = scandir(dir.c_str(), &namelist, 0, alphasort);
    if (filescount < 0)
        syslog(LOG_ERR,"Ошибка сканирования каталога %s (%s)",dir.c_str(),strerror(errno));
    else {
        for(int i=0;i<filescount;++i){
            string fn=string(namelist[i]->d_name);
            struct stat s;
            if(lstat((dir+fn).c_str(), &s)==0){
                if(S_ISREG(s.st_mode))
                    this->push_back(fn);
                else if(S_ISLNK(s.st_mode))
                    syslog(LOG_WARNING,"%s является ссылкой. Обрабатываться не будет",fn.c_str());
            }
            free(namelist[i]);
        }
        free(namelist);
    }
};
