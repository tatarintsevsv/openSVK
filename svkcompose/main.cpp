#include <iostream>
#include <string>
#include "svkcompose.h"

using namespace std;


/**
 * @brief Точка входа
 * @param argc - Количество параметров. Обязательно = 2
 * @param argv - Параметры командной строки: argv[0] - имя исполняемого файла, argv[1] - узел с конфигурацией этапа (stage type="compose")
 * @return
 */
int main(int argc,char* argv[])
{
    if(argc<2)
        return -1;
    svkCompose* compose = new svkCompose(argv[1]);
    compose->run();
    delete compose;
    return 0;
}
