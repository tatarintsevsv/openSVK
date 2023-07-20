# openSVK #

Пакет предназначен для использования в качестве транспорта при взаимодействии с СВК ТУ Банка России по протоколам SMTP/POP3.

ПО может функционировать исключительно под управлением ОС **Linux.**

**Основные возможности** 
1. Формирование писем из файлов в исходных каталогов (компонент svkcompose)
1. Аутентификация на Telnet сервере (компонент svktelnet)
1. Отправка писем по протоколу SMTP с возможностью одновременной отправки нескольких писем (компонент svksmtp)
1. Прием писем по протоколу POP3 с возможностью получения одновременно нескольких писем (компонент svkpop3list, svkpop3get)
1. Сохранение вложений из писем в заданные каталоги (компонент svkextract)
1. Возможность интеграции с сервером dovecot для мониторинга принятых/отправленных писем
1. ПО может функционировать в двух вариантах — в виде демона и виде консольного приложения

   **Общее описание** 

1. ПО функционирует в автономном режиме
1. Цикл работы программы включает следующие этапы:
   - Подготовка файлов к отправке
   - Аутентификация на сервере Telnet
   - Отправка писем по протоколу SMTP
   - Прием писем по протоколу POP3
   - Извлечение вложений из поступивших писем
1. Цикл работы программы автоматически запускается с определенным интервалом времени
1. ` `Подготовка файлов к отправке: В задаваемых конфигурацией исходных каталогах, ПО ищет файлы и формирует на их основе письма:
   - Для каждого исходного файла формируется отдельное письмо
   - Исходный файл помещается в письмо в виде вложения
   - Для каждого исходного каталога имеется возможность настроить параметры формирования писем – тема письма, получатели
   - После формирования, письма помещаются в каталоги подготовленных к отправке писем
1. Аутентификация на сервере Telnet:
   - При аутентификации используются имя и пароль, задаваемые конфигурацией ПО
   - Компонент самостоятельно определяет тип запроса (LOGIN/LOGOUT) и в случае необходимости проходит аутентификацию
1. Отправка писем по протоколу SMTP: 
   - Для аутентификации используются имя и пароль, задаваемые конфигурацией ПО
   - Защита соединения (SSL) не используется
   - В случае ошибки отправки, письмо откладывается. И при следующем цикле работы, будет произведена повторная попытка его отправить
   - Конфигурация допускает настройку одновременной отправки нескольких писем 
1. Прием писем по протоколу POP3:
   - Для аутентификации используются имя и пароль, задаваемые конфигурацией ПО
   - Защита соединения (SSL) не используется
   - В случае ошибки отправки, письмо откладывается. И при следующем цикле работы, будет произведена повторная попытка его скачать
   - После успешного получения, письма с сервера удаляются
   - Конфигурация допускает настройку получения одновременно нескольких писем 
1. Извлечение вложений из поступивших писем:
   - Каталог для сохранения вложений зависит от отправителя письма. Для каждого отправителя должен быть сопоставлен каталог для сохранения.
   - Сохраняются все вложения из писем, имеющие имя файла
   - Если в каталоге для сохранения уже имеется файл с заданным именем, вложение автоматически переименовывается
   - Если для отправителя письма, конфигурацией не предусмотрен каталог для сохранения вложений, такое письмо перемещается в каталог для дальнейшей ручной обработки
   - Обработанные письма сохраняются в определенный каталог maildir (резервное копирование)
1. Работа ПО журнализируется посредством syslog (файлы журналов по умолчанию /var/log/mail.log и /var/log/mail.err). При фиксируются следующие события:
   - Начало и окончание цикла обработки
   - Начало и окончание каждого из этапов
   - При Подготовке файлов к отправке:
     - Факт формирования письма с указанием имени файла-вложения и получателей сформированного письма
     - Ошибки обработки – ошибки доступа к файлу, ошибки сохранения сформированных писем
   - При Аутентификации на сервере Telnet:
     - Начало этапа аутентификации
     - Факт запроса сервером имени/пароля
     - Ошибки – ошибки аутентификации, ошибки сетевого взаимодействия
   - При Отправке писем:
     - Факт начала и завершения отправки каждого письма
     - Ошибки – ошибки аутентификации, ошибки доступа к файлам, ошибки сетевого взаимодействия
   - При Приеме писем:
     - Количество писем на сервере
     - Факт начала и завершения получения для каждого письма
     - Ошибки – ошибки аутентификации, ошибки доступа к файлам, ошибки сетевого взаимодействия
   - На этапе извлечения вложений:
     - Факт обработки каждого письма
     - Общую информацию о письме (Отправитель, получатель, тема)
     - Факт сохранения вложения с указанием имени сохраняемого файла
     - Факта невозможности обработки в случае если для отправителя не задан каталог для сохранения

   **Установка ПО**

Для автоматической установки скачайте и запустите скрипт install.sh.

Скрипт проверит систему, установит недостающие пакеты, скачает репозиторий, скомпилирует и установит файлы ПО в /opt

**Конфигурация**

Конфигурация ПО содержится в файле config.xml

Параметры конфигурации могут быть загружены из переменных окружения. Для указания переменной окружения в качестве значения, необходимо в начале добавить $$$ (т.е. для того чтобы параметр param был равен значению переменной окружения VAR, необходимо указывать param="$$$VAR")

Корневой узел файла <config> параметров не имеет. 

Первый потомок корневого узла <global> содержит параметр, pollinterval, задающий периодичность запуска обработки

Цикл работы задается последовательностью тэгов <stage>,  описывающих этапы работы

|Имя параметра|Описание|Комментарий|
| :- | :- | :- |
|type|Тип этапа. |<p>Возможные значения:</p><p>telnet</p><p>pop3</p><p>smtp</p><p>compose</p><p>extract</p><p>execute</p><p></p>|
|enabled|Запускать ли данный этап|<p>Возможные значения: </p><p>0</p><p>1 (По умолчанию)</p>|
|facility|Syslog facility|Необязательный (у компонентов имеются значения по умолчанию)|

Внутри тэга <stage> может находиться тэг <description> с текстовым описанием этапа, а так же тэги, описывающие правила этапа.

**Этап telnet**

Этап прохождения telnet аутентификации.

Должен содержать (помимо тэга <description>) единственный тэг <telnet> со следующими параметрами

|Имя параметра|Описание|Комментарий|
| :- | :- | :- |
|host|Адрес telnet сервера|Например 192.168.19.20|
|username|Имя пользователя||
|password|Пароль||
|timeout|Таймаут работы, сек|необязательный|
|rules|Путь к файлу с правилами «общения» с сервером формата «вопрос-ответ»||

**Этап pop3**

Этап приема писем

Должен содержать (помимо тэга <description>) один или несколько тэгов <pop3> со следующими параметрами

|Имя параметра|Описание|Комментарий|
| :- | :- | :- |
|maildir|Каталог, для входящих писем. (файлы писем сохраняются в подкаталоге «tmp»)|<p>Maildir. </p><p>Каталог должен содержать подкаталоги «cur» и «tmp»</p>|
|host|Адрес pop3 сервера|Для Москвы 192.168.19.4|
|username|Имя пользователя||
|password|Пароль||
|keep|Не удалять письма с сервера|<p>Возможные значения: </p><p>0 (По умолчанию)</p><p>1 </p><p>Включайте только для тестирования!</p>|
|uidl|Использовать команду протокола POP3 «UIDL» вместо «LIST»|<p>Возможные значения: </p><p>0 (По умолчанию)</p><p>1 </p>|
|instances|Количество одновременно скачиваемых писем|<p>По умолчанию 1</p><p>Используйте с осторожностью. Обычно на серверах ограничивается количество одновременных подключений</p>|

**Этап smtp**

Этап отправки писем

Должен содержать (помимо тэга <description>) один или несколько тэгов <pop3> со следующими параметрами

|Имя параметра|Описание|Комментарий|
| :- | :- | :- |
|source|Каталог, c письмами на отправку. (файлы писем ищутся в подкаталоге «cur»)|<p>Maildir. </p><p>Каталог должен содержать подкаталоги «cur» и «tmp»</p>|
|sent|Каталог, для сохраненияотправленных писем (файлы сохраняются в подкаталог «cur»)|<p>Maildir. </p><p>Каталог должен содержать подкаталоги «cur» и «tmp»</p>|
|host|Адрес smtp сервера|Для Москвы 192.168.19.3|
|username|Имя пользователя||
|password|Пароль||
|recipients|Получатели письма|Через запятую|
|from|Отправитель письма||
|instances|Количество одновременно скачиваемых писем|<p>По умолчанию 1</p><p>Используйте с осторожностью. Обычно на серверах ограничивается количество одновременных подключений</p>|


**Этап compose**

Этап подготовки писем к отправке

Должен содержать (помимо тэга <description>) один или несколько тэгов <compose> со следующими параметрами

|Имя параметра|Описание|Комментарий|
| :- | :- | :- |
|in|Каталог исходных файлов, на основе которых формируются письма|Все пути к каталогам в конфигурации должны заканчиваться символом «/»|
|out|Каталог, где будет сформировано письмо. После формирования помещается в подкаталог «cur»|<p>Maildir. </p><p>Каталог должен содержать подкаталоги «cur» и «tmp»</p>|
|subject|Тема письма||
|recipients|Получатели письма|Через запятую|
|from|Отправитель письма||

**Этап extract**

Этап извлечения вложений из писем

Должен содержать (помимо тэга <description>) один или несколько тэгов <extract>. Единственный параметр тэга in, указывающий каталог maildir с принятыми письмами, которые необходимо обработать.

Внутри тэга in должны содержаться один или несколько тэгов rule с правилами извлечения вложений, со следующими параметрами

|Имя параметра|Описание|Комментарий|
| :- | :- | :- |
|from|Отправитель письма|Данное правило обработает письма от указанного отправителя|
|out|Каталог, куда будет перенесено письмо. После обработки помещается в подкаталог «cur»|<p>Maildir. </p><p>Каталог должен содержать подкаталоги «cur» и «tmp»</p>|
|extractto|Каталог, куда будут извлекаться вложения.|Если в каталоге уже имеется такой файл, файл переименовывается в виде «№\_filename.ext»|

**Этап execute**

Этап запуска произвольной команды

Должен содержать (помимо тэга <description>) единственный тэг <command>, со следующими параметрами

|Имя параметра|Описание|Комментарий|
| :- | :- | :- |
|command|Команда для выполнения|<p>Например </p><p>ping -c 2 192.168.19.20</p>|
|verbose|Писать ли в журнал вывод программы|<p>Возможные значения: </p><p>0 (По умолчанию)</p><p>1 </p>|

