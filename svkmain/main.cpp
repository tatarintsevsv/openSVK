#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include "svkmain.h"
#include <unistd.h>
#include <linux/kernel.h>


using namespace std;



int main(int argc,char* argv[])
{
    string d = string(argv[0]).substr(0,string(argv[0]).find_last_of('/'));
    if(chdir(d.c_str())==-1){
        printf("Ошибка смены каталога. %s",strerror(errno));
        return -1;
    }
    while(1){
        printf("Запуск обработки\n");
        svkMain* svk = new svkMain();
        svk->run();
        int interval=svk->get_pollinterval();
        delete svk;
        if(interval==0)
            return 0;
        printf("Завершено. Следующая обработка через %i сек.\n",interval);
        sleep(interval);
    }
    return 0;
}
