# sim800l_gprs_gps_aprs
sim800l_gprs_gps_aprs tracker

Трекер для отправки данных о местоположении на сервер APRS-IS 
состоит из нескольких вещей с Алиэкспресса: 
GPS модуля
радиомодуля SIM800L 
Arduino
двух DC-DC модулей
внешней антенны для радиомодуля
внешней антенны для GPS
корпуса

Для работы с GPS модулем используется библиотека TinyGPS++, она позволяет
из последовательности NMEA "вырезать" нужные для отправки на апрс сервер элементы (координаты,
количество спутников, время и так далее).
По последовательностям NMEA дополнительная информация  http://aprs.gids.nl/nmea/
Модули бывают с разными заводскими настройками скорости обмена, проверяйте отделным скетчем или родным
софтом, я столнулся с тем, что в некоторых модулях uBlox neo6m не сохраняется скорость, выставляемая при помощи родного софта
uBlox
Отправляемый на APRS-IS сервер пакет представляет из себя строку определенного формата, смотрите в тексте
скетча. Окончание передачи текста обязательно завершается ascii  кодом 0x1A (26) (Ctrl-z). Насколько я понимаю, при отправке окончания
текста, по идее, не нужно указывать количество отправляемых байт.
Базовые настройки для подключения смотрим тут http://aprs.cqham.ru:14501/

Все команды по подключению GSM модуля и отправки данных на сервер APRS-IS при отладке (или для самообразования)  можно выполнить
через putty или другой терминал (учтите, что не все они поддерживают передачу специальных символов типа 0x1A (26))
Монитор порта в среде arduino IDE не поддерживает, я пользовался программой CoolTerm.
В скетче используется 2 виртуальных сериал порта и один реальный (подключив к компу можно посмотреть лог при отладке)
Столкнулся с неприятной особенность библиотеки SoftwareSerial, что при 2х и более софтовых сериал портах
при работе с ними надо явно переключаться на тот, с которого получаем данные (например: neo6m.listen();)

GPRS модуль использовал SIM800L, работает неплохо, есть особенности по питанию (я запитываю от отдельного
преобразователя DC-DC напряжением 4В)
Данные о позиции отправляются при изменении местоположения. Следующий пакет отправляется с некоторой задержкой, которая определяется в зависимости от скорости движения (тут надо еще посмотреть, предполагаю что скорость не очень точно определяется в данном случае)

Данный трекер работает тут https://aprs.fi/#!call=a%2FUB3FBR-9

---------------------------
Tracker for sending location data to the APRS-IS server
consists of several things with Aliexpress:
GPS module
radio module SIM800L
Arduino
two DC-DC modules
external antenna for radio module
external antenna for GPS
housing

To work with the GPS module, the TinyGPS ++ library is used, it allows
from the sequence NMEA "cut" the necessary elements for sending to the server server (coordinates,
number of satellites, time, and so on).
By NMEA sequences additional information http://aprs.gids.nl/nmea/
Modules come with different factory settings for the exchange rate, check with a separate sketch or native
software, I came up with the fact that in some modules uBlox neo6m does not save the speed, set with the help of native software
uBlox
The packet sent to the APRS-IS server is a string of a certain format, see in the text
sketch. The end of the text transfer necessarily ends with ascii code 0x1A (26) (Ctrl-z). As I understand it, when sending an end
text, in theory, you do not need to specify the number of bytes to send.
The basic settings for the connection are here http://aprs.cqham.ru:14501/

All commands for connecting the GSM module and sending data to the APRS-IS server during debugging (or for self-learning) can be performed
via putty or another terminal (note that not all of them support the transmission of special characters like 0x1A (26))
The port monitor in arduino IDE does not support, I used the program CoolTerm.
The sketch uses 2 virtual serials of the port and one real (connecting to the computer you can see the log when debugging)
Faced an unpleasant feature of the SoftwareSerial library that with 2 or more software serials ports
when working with them you must explicitly switch to the one from which we get the data (for example: neo6m.listen ();)

GPRS module used SIM800L, it works well, there are special features on power supply (I feed from a separate
converter DC-DC voltage 4V)
Position data is sent when the location changes. The next packet is sent with some delay, which is determined depending on the speed (here it is necessary to look, I assume that the speed is not very accurately determined in this case)

This tracker works here https://aprs.fi/#!call=a%2FUB3FBR-9


