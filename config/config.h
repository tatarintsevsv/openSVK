#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

#ifdef __cplusplus
#define ETYPE extern "C"
#else
#define ETYPE extern
#endif

/**
 * @brief Закрытие конфигурации, освобождение ресурсов
 */
ETYPE void configClose();

/**
 * \brief Инициализация конфигурации
 * \param filename - имя файла конфигурации (./config.xml)
 * \param facility - syslog facility
 * \return true - если инициализация прошла успешно
 */
ETYPE bool configInit(const char *filename, const char *facility);

/**
 * @brief Чтение строки из конфигурации
 * @param path - путь
 * @param def - значение по умолчанию
 * @return строка. (!!!) Вызывающая сторона должна самостоятельно освобождать память
 */
ETYPE char* configReadString(const char* path,const char* def);

/**
 * @brief Чтение строки из конфигурации, представляющей из себя путь к файлу/каталогу (с разворачиванием "~/" и т.п.)
 * @param path - путь
 * @param def - значение по умолчанию
 * @return строка. (!!!) Вызывающая сторона должна самостоятельно освобождать память
 */
ETYPE char* configReadPath(const char* path,const char* def);

/**
 * @brief Чтение целого числа из конфигурации
 * @param path - путь
 * @param def - значение по умолчанию
 * @return целое число
 */
ETYPE int configReadInt(const char* path, int def);

/**
 * @brief Получение результата обработки XPath
 * @param XPath - выражение XPath
 * @return строка - результат обработки XPath
 */
ETYPE char* configGetNodeSet(const char* XPath);

/**
 * @brief Установка корневого узла
 * @param path - xml путь/xpath
 */
ETYPE void configSetRoot(const char* path);

#endif // CONFIG_H
