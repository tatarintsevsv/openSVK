#ifndef SVKMAIN_H
#define SVKMAIN_H
#include <string>
#include <map>
#include <syslog.h>
#include "../config/configcxx.h"

#define CONFIG_XML "./config.xml"
#define BINPATH "./"

using  namespace std;
namespace std{
class svkmain;
}
/**
 * @brief Тип словарь для хранения пар Заголовок-Значение для MIME
 */
typedef map<string,string> dict;

/**
 * @brief Структура MIME-части сообщения
 * @param headers - Заголовки
 * @param start - смещение начала MIME-части в файле
 * @param end - смещение конца MIME-части в файле
 */
struct mimepart{
    dict headers;
    off_t start;
    off_t end;
};

class svkExtract{
private:
    /**
     * @brief Файловый дескриптор открытого файла с сообщением
     */
    int fd;
    /**
     * @brief Функция чтения заголовков MIME-части
     * @param headers - указатель на словарь для сохранения заголовков
     */
    void parseHeader(dict* headers);
    /**
     * @brief Функция чтения MIME-части
     * @param boundary - MIME-разделитель
     * @param part - указатель на структуру mimepart
     */
    void readPart(string boundary, mimepart* part);
    /**
     * @brief Заголовки письма
     */
    dict headers;
public:
    svkExtract();
    /**
     * @brief Функция обработки файла
     * @param configRoot
     * @param filename
     * @return
     */
    int run(char* configRoot, char* filename);
};


#endif // SVKMAIN_H
