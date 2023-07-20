#ifndef LINUXDIR_H
#define LINUXDIR_H

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

using  namespace std;
class linuxdir: public vector<string>{
public:
    linuxdir(string dir);
private:
  linuxdir(){};
};

#endif