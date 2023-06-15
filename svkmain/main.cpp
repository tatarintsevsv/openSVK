#include <iostream>
#include <string>
#include "svkmain.h"
#include <unistd.h>
#include <linux/kernel.h>


using namespace std;



int main(int argc,char* argv[])
{    
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
