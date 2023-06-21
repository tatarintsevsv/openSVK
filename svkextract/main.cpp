#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include "svkextract.h"
#include <unistd.h>
#include <linux/kernel.h>


using namespace std;



int main(int argc,char* argv[])
{
    if(argc<3)
        return -1;
    string d = string(argv[0]).substr(0,string(argv[0]).find_last_of('/'));
    if(chdir(d.c_str())==-1){
        printf("Ошибка смены каталога. %s",strerror(errno));
        return -1;
    };
    printf("Запуск обработки\n");
    svkExtract* svk = new svkExtract();
    int res=svk->run(argv[1],argv[2]);
    delete svk;

    return res;
}
