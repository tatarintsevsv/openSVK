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
/**
 * @brief Типы этапов
 */
enum class stageTypes{telnet,  /// Telnet аутентификация
                      pop3,    /// Получение почты
                      smtp,    /// Отправка подготовленных писем
                      compose, /// Формирование писем из файлов
                      extract, /// Обработка входящих (Извлечение вложений)
                      execute  /// Выполнение внешней программы
                     };
/**
 *  @brief Мапа сопоставления названия этапа числовому значению
 */
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
private:

    /**
     * @brief Функция запуска процесса получения почты
     * @param configRoot - путь к узлу конфигурации этапа
     */

    int stage_pop3(const string &configRoot);

    /**
     * @brief Функция запуска внешней программы
     * @param configRoot - путь к узлу конфигурации этапа
     */
    int stage_execute(const string &configRoot);

    /**
     * @brief Функция запуска процесса отправки почты
     * @param configRoot - путь к узлу конфигурации этапа
     */
    int stage_smtp(const string &configRoot);

    /**
     * @brief Функция запуска процесса Telnet аутентификации
     * @param configRoot - путь к узлу конфигурации этапа
     */
    int stage_telnet(const string &configRoot);

    /**
     * @brief Функция запуска процесса подготовки писем (Формирование писем из файлов)
     * @param configRoot - путь к узлу конфигурации этапа
     */
    int stage_compose(const string &configRoot);

    /**
     * @brief Функция запуска процесса обработки входящих (Извлечение вложений)
     * @param configRoot - путь к узлу конфигурации этапа
     */
    int stage_extract(const string &configRoot);

    /**
     * @brief интервал проведения сессии обмена с почтовым сервером
     */
    int pollinterval;
    /**
     * @brief Запуск программы с ожиданием завершения
     * @param cmd - командная строка
     * @param reply - указатель на строку для записи вывода внешней программы
     * @return код возврата программы
     */
    int __execute(const string &cmd, string *reply);
    /**
     * @brief Запуск программы без ожидания её окончания
     * @param cmd - командная строка
     * @return
     */
    FILE *__executeasync(const string &cmd);
    /**
     * @brief Запуск пула команд с синхронизацией семафорами
     * @param lines - команды для выполнения
     * @param sem_wait - имя семафора ожидания
     * @param sem_count - имя семафора-счетчика запущенных
     * @param instances - количество одновременно запущенных процессов
     */
    void poolProcessing(vector<string> &lines,const string &sem_wait, const string &sem_count,int instances);


public:
    svkMain();
    ~svkMain();
    /**
     * @brief Получение интервала опроса сервера
     * @return интервал, сек
     */
    int get_pollinterval(){return pollinterval;};
    /**
     * @brief Запуск обработки
     */
    int run();
};


#endif // SVKMAIN_H
