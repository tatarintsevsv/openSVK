#ifndef CONFIGCXX_H
#define CONFIGCXX_H
#include <stdbool.h>
#include <string>
#include "config.h"
/**
 * @brief (std::string версия) Чтение строки из конфигурации
 * @param path - путь
 * @param def - значение по умолчанию
 * @return строка.
 */
std::string xmlReadString(std::string path,std::string def="");

/**
 * @brief (std::string версия) Чтение строки из конфигурации, представляющей из себя путь к файлу/каталогу (с разворачиванием "~/" и т.п.)
 * @param path - путь
 * @param def - значение по умолчанию
 * @return строка.
 */
int xmlReadInt(std::string path, int def=0);

/**
 * @brief Чтение целого числа из конфигурации
 * @param path - путь
 * @param def - значение по умолчанию
 * @return целое число
 */
std::string xmlReadPath(std::string path,std::string def="");

#endif // CONFIGCXX_H
