/*Ардуино GPRS GPS APRS трекер
 * разработал Юрий UB3FBR  ub3fbr@yandex.ru 2017
 * Используйте, модифицируйте код без ограничений 
 * исходный код https://github.com/UB3FBR/sim800l_gprs_gps_aprs.git
 * тема на форуме http://infotex58.ru/forum/index.php?topic=986.msg8605#msg8605
 */
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
SoftwareSerial myGSM(8,9); //подключаем GSM модуль SIM800L пин 9 - RX, 8 - TX !!! внимательно!!! лучше проверять все отдельными тестовыми скетчами - пины тх рх
SoftwareSerial neo6m(6,7); //подключаем GPS модуль uBlox neo6m  пин 6 - TXмодуля, т.е. у gps модуля используем только один пин выхода
TinyGPSPlus gps; // объект TinyGPS++ для работы с данными от модуля GPS

/* Используются последовательности NMEA дополнительная информация  http://aprs.gids.nl/nmea/
 * из последовательности "вырезаем" нужные для отправки на апрс сервер элементы
 * координаты  широту и долготу
 */
TinyGPSCustom la(gps, "GPGGA", 2); // $GPGSA 2 элемент
TinyGPSCustom n_s(gps, "GPGGA", 3); // $GPGSA 3 элемент
TinyGPSCustom lon(gps, "GPGGA", 4); // $GPGSA 4 элемент
TinyGPSCustom e_w(gps, "GPGGA", 5); // $GPGSA 5 элемент
TinyGPSCustom sat(gps, "GPGGA", 7); // $GPGSA 7 элемент
TinyGPSCustom spd(gps, "GPVTG", 7); // $GPVTG 7 элемент - для "похожего на умного" маяка анализируем скорость в км/ч в последовательности 
/*Строковые переменные для вывода в консоль 
 * и формирования пакета отправляемого на APRS-IS
 */
String lla;
String llon;
String ns;
String ew;
String sats;
String spds;
int intsats; //преобразованные в число кол-во спутников
int intspds; //и скорость

int send_error = 0;  //определение факта ошибки при отправке данных на сервер апрс

void setup()
{
 Serial.begin(9600);
 delay(500); 
 myGSM.begin(9600);
 delay(500);   
 neo6m.begin(4800); //предварительно проверяем скорость обмена с GPS модулем, может быть 4800 и 9600
 delay(500);
 pinMode(12, OUTPUT);
 myGSMsetup(); //устанавливаем параметры модуля SIM800L и данные apn оператора, подключаемся к gprs
}

void loop()
{
  
// Если данные обновились - анализируем их, отправляем и ждем 
  if (la.isUpdated() || lon.isUpdated())
  {
    lla=la.value();
    lla = lla.substring(0,7); //string.substring(from, to) вырезаем часть строки уменьшая точность
    ns=n_s.value();
    llon=lon.value();
    llon = llon.substring(0,8); //string.substring(from, to)
    ew=e_w.value();
    sats=sat.value(); intsats = sats.toInt();
    spds=spd.value(); intspds = spds.toInt();
    
    Serial.println();
    Serial.print(F("LAT=")); Serial.println(lla);
    Serial.print(F("ns=")); Serial.println(ns); 
    Serial.print(F("LON=")); Serial.println(llon);
    Serial.print(F("ew=")); Serial.println(ew);
    Serial.print(F("SATs=")); Serial.println(sats);
    Serial.print(F("Speeds=")); Serial.println(spds);
    Serial.println();
/*Проверяем количество захваченных спутников intsats, если меньше 4, ничего не отправляем
 * Если больше или равно 4 - отравляем данные на сервер APRS
 */
  if (intsats > 3){
    Serial.println(F("=SATs > 3 ="));
    Serial.println(F("=Open port and Send data="));
    myGSMopenport(); 
    
    sendTCPpacket(); 
    delay(3000);
    Serial.println(F("=Close port and Delay="));
    myGSMcloseport();
    neo6m.listen();
    Serial.println (F("========================="));
/*Добавляем похожую на "умную" задержку
 * чтобы трек был не "рубленный", а более гладкий на карте
 */
 Serial.print(F("=IntSpds="));Serial.println(intspds); //выводим текущую скорость для контроля 
 if (intspds > 90) {Serial.println(F("=Delay=20s ="));delay(20000);}
 else if (intspds > 60){Serial.println(F("=Delay=30s ="));delay(30000);}
 else if (intspds > 30) {Serial.println(F("=Delay=40s ="));delay(40000);}
 else {Serial.println(F("=Delay=60s ="));delay(60000);} //если предыдущие условия не сработали - задержка по умолчанию 60 сек (минимальная скорость движения)
 } 
 
  
 }
  while (neo6m.available() > 0)
    gps.encode(neo6m.read()); 
}


void printSerialData()
{
 
 while(myGSM.available() > 0)
 Serial.write(myGSM.read());
 
}

void sendTCPpacket()
{
  myGSM.listen();
  /* Фомируем пакет для отправки на сервер согласно правилам APRS
   *  test в начале пакета - для первичного подключения
   *  используем свои учетные данные на APRS-IS
   *  таблицу символов для нужного значка на карте в aprs.fi
   */
  String aprs_data = "test ";
  aprs_data+="\r";
  aprs_data+="user UB3FBR-9 pass 12345 vers arduino-gprs-SIM800L"; //примените свой пароль для APRS-IS  вместо 12345
  aprs_data+="\r";
  
  aprs_data+="UB3FBR-9>APRS,TCPIP*:="; // сначала эта строка была такой:  aprs_data+="UB3FBR-9>APRS:=";
  aprs_data+=lla; //"5509.85"; //Latitude
  aprs_data+=ns; //"N"; N for latitude North and S for latitude South
  aprs_data+="/"; //Символ TABLE выбирает одну из двух таблиц символов, тут основная таблица
  aprs_data+=llon; //"03726.17"; //Longitude
  aprs_data+=ew; //"E"; W for longitude West and E for longitude East
  aprs_data+=">"; // выбор символа APRS, тут символ машины с ssid-9 такой: ">" ; Символ домашнего QTH такой: "-" ,  
  aprs_data+="APRS/GPRS tracker(SIM800L)";
  aprs_data+=" SATview=";
  aprs_data+=sats; //количество спутников
  //add U батареи, количество спутников, температура салона
  //local time 
  aprs_data+="\r";
  
 myGSM.println(aprs_data);
 //myGSM.println("UB3FBR-9>APRS,TCPIP*:>op.Yuri ub3fbr@yandex.ru"); // статусная строка
 
 delay(4000);
 printSerialData();
 myGSM.write(0x1A);
 delay(3000);
 printSerialData();
 
 Serial.println(F("=blink_led="));
 digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
 delay(1000);                       // wait for a second
 digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
  
 neo6m.listen();  
   
}

/* полезные ссылки по камандам АТ
 * http://www.raviyp.com/embedded/169-sim300-gprs-commands
 * https://habrahabr.ru/post/119030/
 * http://arduino.ru/forum/apparatnye-voprosy/vopros-po-modulyu-sim900
 * goo.gl/cs5X5r  (http://forum.cxem.net)
 * http://m2msupport.net/m2msupport/atcipshut-deactivate-gprs-pdp-context/
 * http://radiolaba.ru/microcotrollers/gsm-modul-neoway-m590-opisanie-i-komandyi-upravleniya.html
 */
void myGSMsetup()
{
 Serial.println (F("=Setup...=")); 
 myGSM.listen(); //Перед тем как считывать данные с софтового порта, его необходимо перевести в режим ожидания данных 
 delay (10000);
/* Проверяем доступность модуля модема 
 * к дальнейшей работе - готовность к работе и регистрацию в сети  
 */
 do { 
  Serial.println(F("=Check module status...="));
  myGSM.println(F("AT+CPAS")); // Информация о состояние модуля модема (0 – готов к работе)
  delay (500);
} while (!myGSM.find("0"));
  
do { 
  Serial.println(F("=Check registration in the network...="));
  myGSM.println(F("AT+CREG?")); // Информация о регистрации в сети  0,1  0 – нет кода регистрации сети,1 – зарегистрирован, домашняя сеть; роуминг -  код 5
  delay (500);
} while (!(myGSM.find("+CREG: 0,1"))or(myGSM.find("+CREG: 0,5"))); // добавил тут  логический оператор ИЛИ (or) результат Истина, если хотя бы один операнд истина

//=======================================================================================
Serial.println(F("=GPRS initialization=")); 
 myGSM.println(F("AT+CIPSHUT")); //разрыв всех соединений и деактивирование gprs
 delay(1000);
 printSerialData();
 
 myGSM.println(F("AT+CIPMUX=0")); //установка режима одиночного подключения
 delay(2000);
 printSerialData();

do { 
  Serial.println(F("=GPRS Attaching...="));
  myGSM.println(F("AT+CGATT=1")); //Заставляет модуль подключиться к GPRS.  Проверить, подключен ли он, можно командой "AT+CGATT?"
  delay (500);
} while (!myGSM.find("OK"));

 
 myGSM.println(F("AT+CSTT=\"internet.mts.ru\",\"mts\",\"mts\""));
 delay(5000);
 printSerialData();

do { 
  Serial.println(F("=Start wireless...="));
  myGSM.println(F("AT+CIICR"));  //Устанавливает беспроводное подключение GPRS
  delay (1000);
} while (!myGSM.find("OK"));

do { 
  Serial.println(F("=Check IP address="));
  myGSM.println(F("AT+CIFSR")); //Возвращает IP-адрес модуля
  delay (2500);
} while (!myGSM.find(".")); 

send_error = 0; //сбрасываем состояние переменной ошибки
 
 neo6m.listen(); //снова слушаем GPS модуль
 Serial.println(F("=Setup complited="));
 Serial.println(F("=========================")); 
}

void myGSMopenport()
{
 /*----------открываем порт сервера aprs --------------- 
  *  базовые настройки для подключения смотрим тут http://aprs.cqham.ru:14501/ 
  *  подключившись через putty или процедурой myGSMopenport на указанном сайте 
  *  должны увидеть своё соединение с сервером в списке (после обновления)
  *  до отключения клиента putty или выполнения процедуры myGSMcloseport
  */
 myGSM.listen(); 

 myGSM.println(F("AT+CIPSTART=\"TCP\",\"194.186.45.251\",\"14580\""));
 delay(3000);
 printSerialData();

 myGSM.println(F("AT+CIPSEND"));
 delay(2000);
 printSerialData();


if (myGSM.find("ERROR"))
{
send_error = 1;
Serial.println(F("=Aprs server connect error="));
//возможно перезагружать?
}

 neo6m.listen();

}

void myGSMcloseport()
{
  //----------закрываем порт сервера aprs --------------- 
  myGSM.listen();
  
myGSM.println(F("AT+CIPCLOSE")); //разрыв подключения с сервером aprs
delay(1000); 


if (myGSM.find("OK"))
 {
send_error = 0;
Serial.println(F("=Aprs server connection close="));
 }

//myGSM.println(F("AT+CIPSHUT")); разрыв всех соединений и деактивирование gprs
neo6m.listen();
}





