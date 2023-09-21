#ifndef SVKMAIN_H
#define SVKMAIN_H
#include <stdio.h>
#include <string>
#include <syslog.h>

#include "../common/linuxdir.h"
#include "../config/config.h"
#include "../config/configcxx.h"

#define CONFIG_XML "./config.xml"
#define BINPATH "./"

using  namespace std;
namespace std{
class svkmain;
}

class svkCompose{
protected:
    /**
     * @brief Функция формирования письма из файла
     * @param filename - исходный файл
     */
    void prepareFile(const string& filename);

    /**
     * @brief Функция обхода каталога для обработки файлов
     * @param node - узел конфигурации
     */
    void processDir(const string& node);
private:
    /**
     * @brief Корневой узел конфигурации для работы
     */
    std::string root;
    /**
     * @brief syslog facility
     */
    string facility;
    /**
     * @brief Текущий исходный каталог
     */
    string source;
    /**
     * @brief Каталог, в котором будут формироваться письма
     */
    string result;
    /**
     * @brief имя отправителя для письма
     */
    string from;
    /**
     * @brief Получатели письма
     */
    string recipients;
    /**
     * @brief Тема письма
     */
    string subject;

public:
    /**
     * @brief Конструктор. Инициализирует конфигурацию и считывает параметры
     * @param root - корневой узел для работы
     */
    svkCompose(string root);

    /**
     * @brief Запуск обработки
     * @return результат работы (Всегда 0)
     */
    int run();

};


#endif // SVKMAIN_H
