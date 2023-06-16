#include <ArduinoOTA.h>
#include <xlogger.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
//#include "SIM800L.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
//#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
#include <base64.h>


#define SIM800_RX_PIN D2
#define SIM800_TX_PIN D3
//#define SIM800_RST_PIN D4 


#define               PROGRAM_VERSION   "1.00"

#define DEBUG

#ifdef DEBUG
  // log level: info
  #define             DEBUG_PRINT(...)    logger.print(__VA_ARGS__)
  #define             DEBUG_PRINTLN(...)  logger.println(__VA_ARGS__)
  // log level: warning
  #define             DEBUG_WPRINT(...)   logger.print(llWarning, __VA_ARGS__)
  #define             DEBUG_WPRINTLN(...) logger.println(llWarning, __VA_ARGS__)
  // log level: error
  #define             DEBUG_EPRINT(...)   logger.print(llError, __VA_ARGS__)
  #define             DEBUG_EPRINTLN(...) logger.println(llError, __VA_ARGS__)
#else
  #define             DEBUG_PRINT(...)
  #define             DEBUG_PRINTLN(...)
  #define             DEBUG_WPRINT(...)
  #define             DEBUG_WPRINTLN(...)
  #define             DEBUG_EPRINT(...)
  #define             DEBUG_EPRINTLN(...)
#endif

xLogger               logger;
char                  HARDWARE_ID[7] = {0};


TinyGPSPlus gps;
SoftwareSerial SerialGPS(D5, D0); 
SoftwareSerial SerialGSM(D2, D3); 
ESP8266WebServer server(80);

//char* ssid1 = "Dom";
//char* pass1 = "Rozov0732";
//char* ssid = "Zenfone Max Pro";
//char* pass = "123456777";
//const char APN[] = "Internet.tele2.ru";
//const char URL[] = "https://postman-echo.com/get?foo1=bar1&foo2=bar2";
//const char URL[]="http://217.25.227.150:8265/gps.php?latitude=51.2568&longitude=39.5896&speed=25&altitude=18.2&battlevel=71&tm=1628589452101&deviceid=Granta";
const char URL[]="https://api.telegram.org/bot583018390:AAE8qo6M_D-kPv8ICKffeQxXdneXWs5ivO8/sendmessage?chat_id=407427903&text=test";

//#define UDP_PORT 555;

float Latitude , Longitude, Speed, Altitude, Tochnost, Latitude_raz , Longitude_raz;
//int year1 , month1 , date1, hour1 , minute1 , second1;
String  LatitudeString , LongitudeString, SpeedString, AltitudeString, TochnostString, gsm_serString,serString,GPSdata,not_encoded_in_base64,GSMdata;
//, gsm_ser
//gps_serString, GSMdata,DateString,TimeString;
//String ansverGSM;

int deley=1000,deley1=0,deley2=0,serInt;
bool wificon=false, sotryas_check=false;
int wificount=0;
int looper=0,qtde=0;
bool serial_lis=0;
float Latitudeold=0,Longitudeold=0;
float lat_gsm,lat_gsm_old,lat_gsm_raz;
float lon_gsm,lon_gsm_old,lon_gsm_raz;
const uint32_t connectTimeoutMs = 5000;
char* vishki_arr="";

//WiFiServer server(80);
//WiFiUDP UDP;
//SIM800L* sim800l;
WiFiClient wclient; 
ESP8266WiFiMulti wifiMulti;
void ICACHE_RAM_ATTR sotryas();

void setup()
{
  attachInterrupt(A0, sotryas, CHANGE);
  Serial.begin(115200);
  SerialGPS.begin(9600);
  SerialGSM.begin(9600);
  SerialGSM.setTimeout(2000);
  Serial.println("");
  Serial.println("Initializing...");    // Печать текста
  delay(5000); 
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("Dom", "Rozov0732");
  wifiMulti.addAP("Zenfone Max Pro", "123456777");
  ArduinoOTA.setHostname("GPS_Moyak");
  ArduinoOTA.begin();
  WiFi.hostname("GSM_Moyak");  
  sprintf(HARDWARE_ID, "%06X", ESP.getChipId());
  logger.begin(HARDWARE_ID, &Serial, false);
  logger.setProgramVersion(PROGRAM_VERSION);


  DEBUG_PRINTLN(F("Starting..."));
//  updateSerial();
  //delay(2000);
  SerialGSM.println("AT");                 // Отправка команды AT
  //updateSerial();
  SerialGSM.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");         // Проверка регистрации в сети
  //updateSerial();
  SerialGSM.println("AT+SAPBR=3,1,\"APN\",\"internet.tele2.ru\"");         
  //updateSerial();
  SerialGSM.println("AT+SAPBR=3,1,\"USER\",\"tele2\"");         
  //updateSerial();
  SerialGSM.println("AT+SAPBR=3,1,\"PWD\",\"tele2\"");         
  //updateSerial();
  SerialGSM.println("at+ceng=4,1");         
  SerialGSM.println("AT+CNETSCAN=1"); //Включение подробного отображения вышек
  //updateSerial();
  //SerialGSM.println("AT+SAPBR=1,1");         
  //updateSerial();
  //SerialGSM.println("at+ceng=1");         
  //updateSerial();
  /*SerialGSM.println("AT+CIPSTART=\"UDP\",\"217.25.227.150\",\"555\"");
  updateSerial();
  SerialGSM.println("AT+CIPSEND=11");
  updateSerial();
  SerialGSM.println("phone/check");
  updateSerial();
  SerialGSM.println("AT+CIPCLOSE");
  updateSerial();
  SerialGSM.println("AT+SAPBR=0,1");         
  updateSerial();*/
  delay(5000);
  /*if (WiFi.status() != WL_CONNECTED) 
  {
    if (wificon)
    {
      ssid = "Zenfone Max Pro";
      pass = "123456777";
    }
    else  
    {
      ssid = "Dom";
      pass = "Rozov0732";
    }
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);
    wificon=!wificon;
  }*/
  server.on("/reset", []()
  {
    server.send(200, "text/plain", "restart");
    delay(200);
    ESP.restart();
  });
  server.begin();
  updateSerial();
  SerialGPS.listen();
  SerialGPS.flush();
}

void sotryas()
{
  //if (analogRead(A0) < 200)
  //{
    sotryas_check=true;
  //}
}

void loop()
{
  ArduinoOTA.handle();
  if (sotryas_check)
  {
    logger.println("Sotryasenie");
    sotryas_check=false;
  }
  /*if ( millis()-looper < 1000 )
  {
    if (SerialGSM.isListening())
    {
      SerialGSM.listen();
    }
    
    updateSerial();
  }
  else
  {
    SerialGPS.listen();
    //SerialGPS.flush();
  }*/
  if (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED)
  {
    if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED)
    { 
      Serial.print("WiFi connected: ");
      Serial.println(WiFi.SSID());
      ArduinoOTA.setHostname("GPS_Moyak");
      ArduinoOTA.begin();
    }
  }
  

  /*if (wificount > 2000)
  {
    if (WiFi.status() != WL_CONNECTED) 
    {
      if (wificon)
      {
        ssid = "Zenfone Max Pro";
        pass = "123456777";
      }
      else  
      {
        ssid = "Dom";
        pass = "Rozov0732";
      }  
      Serial.print("Connecting to ");
      Serial.print(ssid);
      Serial.println("...");
      //logger.print("Connecting to ");
      //logger.println(ssid);
      WiFi.begin(ssid, pass);
      //ArduinoOTA.setHostname("GPS_Moyak");
      //ArduinoOTA.begin();
      wificon=!wificon;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      ArduinoOTA.setHostname("GPS_Moyak");
      ArduinoOTA.begin();
    }
    wificount=0;
  }
  else
  {
    wificount+=1;
  }*/
  //DEBUG_WPRINTLN(F("*****************************************"));

  // show var
  //logger.print('c');                              // char
  //logger.println("-char");                        // *char
  //logger.println(F("flash char"));                // *char from flash
  //logger.println(String("String class"));         // String
  //logger.println(SF("String class from flash"));  // String from flash
  //logger.print("DEC:");
  //logger.println(10);
  //logger.print("HEX:");
  //logger.println(10, HEX);
  //logger.print("float:");
  //logger.println(10.101);
  //logger.printf(llError, "printf dec=%d hex=%02x char=%c str=%s \r\n", 10, 10, 'c', "strline");

  // debug macro
  //DEBUG_PRINTLN(F("Debug info level"));
  //DEBUG_WPRINTLN(F("Debug warning level"));
  //DEBUG_EPRINTLN(F("Debug error level"));
  //updateSerial();
  for(int i = 0; i < 10; i ++) {
    logger.handle();
    yield();
    delay(10);
  }
  //SerialGSM.listen();
  //updateSerial();
  //delay(500);
  
  while (SerialGPS.available() > 0)
  {
    if (gps.encode(SerialGPS.read()))
    {    
      //Latitude+=0.005;
      //Longitude+=0.005;
       
      Latitude = gps.location.lat();
      LatitudeString = String(Latitude , 6);
      Longitude = gps.location.lng();
      LongitudeString = String(Longitude , 6);
      Speed = gps.speed.kmph();
      SpeedString = String(Speed , 2);
      Altitude = gps.altitude.meters();
      AltitudeString = String(Altitude , 2);
      Tochnost=gps.hdop.hdop();
      TochnostString=String(Tochnost , 2);
    }
  }

      /*if (gps.date.isValid())
      {
        DateString = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();

        if (date < 10)
        DateString = '0';
        DateString += String(date);

        DateString += " / ";

        if (month < 10)
        DateString += '0';
        DateString += String(month);
        DateString += " / ";

        if (year < 10)
        DateString += '0';
        DateString += String(year);
      }

      if (gps.time.isValid())
      {
        TimeString = "";
        hour = gps.time.hour()+ 5; //adjust UTC
        minute = gps.time.minute();
        second = gps.time.second();
    
        if (hour < 10)
        TimeString = '0';
        TimeString += String(hour);
        TimeString += " : ";

        if (minute < 10)
        TimeString += '0';
        TimeString += String(minute);
        TimeString += " : ";

        if (second < 10)
        TimeString += '0';
        TimeString += String(second);
      }*/

  if ( millis() - deley > 60000 )//and gps.location.isValid() )
  { 
    Serial.println(gps.location.isValid());
    if (!gps.location.isValid() || gps.satellites.value()==0)
    {
      char* serStr;
      looper=millis();
      logger.println("Vishki start");
      if (!SerialGSM.isListening())
      {
        SerialGSM.listen();
      }
      //updateSerial();
      //delay(2000);
      //updateSerial();
      //delay(2000);
      //SerialGSM.println("at+ceng?");         // Запрос вышек
      //SerialGSM.println("AT+CLBSCFG=0,3");
      //updateSerial();
      //SerialGSM.println("AT+SAPBR=1,1");
      //updateSerial();
      updateSerial();
      serString="";
      Serial.println("Начало прослушивания");
      while (millis()-looper<5000)
      {
        //Serial.println("Считывание ");
        if (SerialGSM.available() > 0)
        {
          serInt=0;
          while (SerialGSM.available() > 0)
          {
            //serStr+=new char(SerialGSM.read());
            serInt=SerialGSM.read();
            serStr+=char(serInt);
            //Serial.println(serInt);
            //Serial.println(serString);
            //serString=SerialGSM.readStringUntil('\n');
            
          }
          //Serial.println(serString);
          /*Serial.println(serStr);
          if (serString!="")
          {
            if (serStr=="OK")
            {
              Serial.println("Интернет подключен");
              looper-=5001;
            }
            else if (serStr=="ERROR")
            {
              Serial.println("Интернет НЕ подключен");
              looper-=5001;
            }
            else
            { 
              Serial.println("Ошибка");
              looper-=5001;
            }
          }*/
          //Serial.println("После присвоения");
        }
      }
      serString="";
      //SerialGSM.println("AT+CLBS=1,1");
      SerialGSM.println("AT+CNETSCAN");
      looper=millis();
      Serial.println("Начало координат");
      while (millis()-looper<10000)
      {
        int del=millis();
        while (SerialGSM.available() == 0)
        {
          looper+=10;
          //Serial.println("Плюсуем");
          if (millis()-del>5000)
          {
            break;
          }
        }
        while (SerialGSM.available() > 0)
        {
          Serial.println("начало цикла");
          looper=millis();
          Serial.println("looper");
          String gsm_ser = SerialGSM.readStringUntil('\n');
          Serial.println(gsm_ser);
          String* fields = split(gsm_ser, ',', qtde);
          
          //if (gsm_ser.startsWith("+CLBS:") and !gsm_ser.startsWith("+CLBS: 3"))
          Serial.print("длина ");
          Serial.println(gsm_ser.length());
          if (gsm_ser.startsWith("Operator") && gsm_ser.length()>60)
          {
            //Serial.print("Длина fields ");
            //Serial.println(sizeof(fields) / sizeof(fields[0]));
            //Для AT+CLBS=1,1
            Serial.println("Перед извлечением");
            /*Serial.println("Перед извлечением");
            lat_gsm = fields[2].toFloat();  // извлекаем широту
            lon_gsm = fields[1].toFloat();  // извлекаем долготу
            Serial.println("После извлечения");
            Serial.print("lat :");
            Serial.println(lat_gsm,6);
            Serial.print("lon :");
            Serial.println(lon_gsm,6);
            lat_gsm_raz=lat_gsm_old - lat_gsm;
            lon_gsm_raz=lon_gsm_old - lon_gsm;
            if (lat_gsm_raz < 0)
            {
              lat_gsm_raz*= -1;
            }
            if (lon_gsm_raz < 0)
            {
              lon_gsm_raz*= -1;
            }
            Serial.println("Перед условием");
            if (((lat_gsm_raz > 0.0005 || lon_gsm_raz > 0.0005) && (lat_gsm_raz < 1 || lon_gsm_raz < 1)) || ((lat_gsm_old==0 || lon_gsm_old==0)) && (lat_gsm!=0 and lon_gsm!=0))
            {
              logger.println("Send GSM Location");
              Serial.println("Send GSM Location");
              logger.print("lat :");
              logger.println(lat_gsm,6);
              logger.print("lon :");
              logger.println(lon_gsm,6);
              senddata(String(lat_gsm,6),String(lon_gsm,6),"0","200");
              lat_gsm_old=lat_gsm;
              lon_gsm_old=lon_gsm;
            }
            Serial.println("После условия");
            looper-=10001;
          }*/
          //Для at+ceng?
          /*if (gsm_ser.startsWith("CENG: 0"))
          {
            Serial.println("Первая вышка");
            String* fields = split(gsm_ser, ',', qtde);
            int mcc;//=fields[7];
            int mcn;//=fields[10];
            char lac[5];
            char cellid[5];
            int dbg;
            fields[10].toCharArray(lac, 5);
            fields[7].toCharArray(cellid, 5);
            mcc=fields[4].toInt();
            mcn=fields[5].toInt();
            dbg=fields[2].toInt();
            //uint32_t mcc = strtoul(String(fields[7]), NULL, 16);
            //uint32_t mcn = strtoul(String(fields[10]), NULL, 16);
            //Serial.println(strtoul(mcc, NULL, 16));
            //Serial.println(strtoul(mcn, NULL, 16));
            Serial.println(mcc);
            Serial.println(mcn);
            Serial.println(StrToHex(lac));
            Serial.println(StrToHex(cellid));
            Serial.println(dbg);
            not_encoded_in_base64+=mcc;
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=mcn;
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=StrToHex(lac);
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=StrToHex(cellid);
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=-113+dbg*2;
            not_encoded_in_base64+=";";
            
          } 
          if (gsm_ser.startsWith("CENG: 1") && !gsm_ser.startsWith("CENG: 1,1"))
          {
            Serial.println("Вышка 1");
            String* fields = split(gsm_ser, ',', qtde);
            int mcc;//=fields[7];
            int mcn;//=fields[10];
            char lac[5];
            char cellid[5];
            int dbg;
            fields[7].toCharArray(lac, 5);
            fields[4].toCharArray(cellid, 5);
            mcc=fields[5].toInt();
            mcn=fields[6].toInt();
            dbg=fields[3].toInt();
            //uint32_t mcc = strtoul(String(fields[7]), NULL, 16);
            //uint32_t mcn = strtoul(String(fields[10]), NULL, 16);
            //Serial.println(strtoul(mcc, NULL, 16));
            //Serial.println(strtoul(mcn, NULL, 16));
            Serial.println(mcc);
            Serial.println(mcn);
            Serial.println(StrToHex(lac));
            Serial.println(StrToHex(cellid));
            Serial.println(dbg);
            not_encoded_in_base64+=mcc;
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=mcn;
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=StrToHex(lac);
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=StrToHex(cellid);
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=-113+dbg*2;
            not_encoded_in_base64+=";";
          }*/
           /*String toEncode = "Hello World";
            String encoded = base64::encode(toEncode);
            Serial.println(encoded);*/
          //if (gsm_ser.startsWith("Operator"))
         // {
            Serial.println("вышка");
            String* fields = split(gsm_ser, ',', qtde);
            String mcc;//=fields[7];
            String mcn;//=fields[10];
            String lac;
            String cellid;
            String dbg;
            int dbg_int;
            char cellid_char[5];
            char lac_char[5];
           
            //fields[6].toCharArray(lac, 20);
            Serial.println(fields[6]);
            String* lac1 = split(fields[6], ':', qtde);
            lac=lac1[1];
            //lac1[1].toCharArray(lac, 20);

            //fields[4].toCharArray(cellid, 20);
            Serial.println(fields[4]);
            String* cellid1 = split(fields[4], ':', qtde);
            cellid=cellid1[1];
            //cellid1[1].toCharArray(cellid, 20);

            //fields[1].toCharArray(mcc, 20);
            Serial.println(fields[1]);
            String* mcc1 = split(fields[1], ':', qtde);
            mcc=mcc1[1];
            //mcc1[1].toCharArray(mcc, 20);

            //fields[2].toCharArray(mcn, 20);
            Serial.println(fields[2]);
            String* mcn1 = split(fields[2], ':', qtde);
            mcn=mcn1[1];
            //mcn1[1].toCharArray(mcn, 20);

            //fields[3].toCharArray(dbg, 20);
            Serial.println(fields[3]);
            String* dbg1 = split(fields[3], ':', qtde);
            dbg=dbg1[1];
            dbg_int=dbg.toInt();
            //dbg1[1].toCharArray(dbg, 20);

            //for (int i=0;i<8;i++)
            //{
            //  Serial.print("Номер ");
            //  Serial.println(i);
            //  Serial.println(fields[i]);
            //}
            //uint32_t mcc = strtoul(String(fields[7]), NULL, 16);
            //uint32_t mcn = strtoul(String(fields[10]), NULL, 16);
            //Serial.println(strtoul(mcc, NULL, 16));
            //Serial.println(strtoul(mcn, NULL, 16));
            //Serial.println("fields[6]");
            cellid.toCharArray(cellid_char,5);
            //Serial.println("fields[7]");
            lac.toCharArray(lac_char,5);
            Serial.println(gsm_ser);
            Serial.println(mcc);
            Serial.println(mcn);
            Serial.println(StrToHex(lac_char));
            Serial.println(StrToHex(cellid_char));
            Serial.println(dbg);
            not_encoded_in_base64+=mcc;
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=mcn;
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=StrToHex(lac_char);
            not_encoded_in_base64+=",";
            not_encoded_in_base64+=StrToHex(cellid_char);
            not_encoded_in_base64+=",";
            //not_encoded_in_base64+=dbg;
            not_encoded_in_base64+=-113+dbg_int*2;
            not_encoded_in_base64+=";";
            looper=millis();
          //} 
        }
        else
        {
          looper-=5000;
        }
        Serial.println("конец цикла");
      }
      }
      Serial.println("после looper 10000");
      //SerialGSM.println("AT+SAPBR=0,1");
      gsm_serString="";
      Serial.println(not_encoded_in_base64);
      String encoded = base64::encode(not_encoded_in_base64);
      Serial.println(encoded);
      encoded.replace("\n","");
      Serial.println(encoded);
      if (encoded!="")
      {
        Serial.println("SendDataGSM");
        senddatagsm(encoded,"0");
      }
      not_encoded_in_base64="";
      SerialGPS.listen();
    }
    else
    {
      long rssi_zenfone = WiFi.RSSI();
      logger.print("RSSI_");
      logger.print(WiFi.SSID());
      logger.print(": ");
      logger.println(rssi_zenfone);
      logger.print("Location valid ");logger.println(gps.location.isValid());
      Serial.print("Latitude   = ");Serial.println(LatitudeString);
      Serial.print("Longitude  = ");Serial.println(LongitudeString);
      Serial.print("Спутники "); Serial.print(gps.satellites.value()); Serial.print(" valid "); Serial.println(gps.satellites.isValid());
      Serial.print(gps.time.hour()+ 3);Serial.print(":");Serial.print(gps.time.minute());Serial.print(":");Serial.println(gps.time.second()); //adjust UTC
      Latitude_raz=Latitudeold - Latitude;
      Longitude_raz=Longitudeold - Longitude;
      if (Latitude_raz < 0)
      {
        Latitude_raz*= -1;
      } 
      if (Longitudeold - Longitude < 0)
      {
        Longitude_raz*= -1;
      }
      Serial.print("Raznica Lat= ");
      Serial.println(Latitudeold - LatitudeString.toFloat());
      Serial.print("Raznica Lng= ");
      Serial.println(Longitudeold - LongitudeString.toFloat());
  
      logger.print(gps.time.hour()+ 3);logger.print(":");logger.print(gps.time.minute());logger.print(":");logger.println(gps.time.second()); //adjust UTC
      logger.print("Latitude= ");
      logger.println(Latitude,4);
      logger.print("Longtitude= ");
      logger.println(Longitude,4);
      logger.print("Raznica Lat= ");
      logger.println(Latitude_raz,4);
      logger.print("Raznica Lng= ");
      logger.println(Longitude_raz,4);
      logger.print("Sputniki "); logger.print(gps.satellites.value()); logger.print(" valid "); logger.println(gps.satellites.isValid());
      if ((((Latitude_raz > 0.0005 || Longitude_raz > 0.0005) && (Latitude_raz < 1 || Longitude_raz < 1)) || ((Latitudeold==0 || Longitudeold==0)) && (Latitude!=0 and Longitude!=0))&&(gps.location.isValid()))
      {
        senddata(LatitudeString,LongitudeString,SpeedString,TochnostString);
        logger.print("Latitude send   = ");logger.println(LatitudeString);
        logger.print("Longitude send = ");logger.println(LongitudeString);
        Latitudeold=LatitudeString.toFloat();
        Longitudeold=LongitudeString.toFloat();
        deley=millis();
        SerialGPS.listen();
        delay(1000);
      } 
      else
      {
        Latitudeold=LatitudeString.toFloat();
        Longitudeold=LongitudeString.toFloat();
      } 
      deley=millis();
      delay(2000);
    }
  } 
  /*else if (millis() - deley > 60000)
  {
    deley=millis();
    logger.println("NOT Valid");
    logger.print("Latitude = ");logger.println(LatitudeString);
    logger.print("Longitude = ");logger.println(LongitudeString);

    //logger.print("Raznica Lat = ");logger.println(String(abs(Latitudeold - LatitudeString.toFloat())));
    //logger.print("Raznica Lng = ");logger.println(String(abs(Longitudeold - LongitudeString.toFloat())));
    logger.print("Sputniki "); logger.print(gps.satellites.value()); logger.print(" valid "); logger.println(gps.satellites.isValid());
    logger.print(String(gps.time.hour()+ 3));logger.print(":");logger.print(String(gps.time.minute()));logger.print(":");logger.println(String(gps.time.second())); //adjust UTC
    
    Serial.print("Latitude   = ");Serial.println(LatitudeString);
    Serial.print("Longitude  = ");Serial.println(LongitudeString);

    Serial.print("Raznica Lat= ");
    Serial.println(String(abs(Latitudeold - LatitudeString.toFloat())));
    Serial.print("Raznica Lng= ");
    Serial.println(String(abs(Longitudeold - LongitudeString.toFloat())));
    SerialGSM.listen();
    SerialGSM.println("AT+CIPGSMLOC=1,1");
    updateSerial();
    delay(1500);
    SerialGSM.println("at+ceng?");
    updateSerial();
    delay(1500);
    SerialGPS.listen();
    long rssi_zenfone = WiFi.RSSI();
    Serial.print("RSSI_");
    Serial.print(WiFi.SSID());
    Serial.print(": ");
    Serial.println(rssi_zenfone);
  }*/
}
//}
  //SerialGSM.listen();
  //updateSerial();
//}

void updateSerial()
{
  if (Serial.available() > 0 )
  {
    while (Serial.available()) 
    {
      int ser=Serial.read();
      serString+=char(ser);
      SerialGSM.write(ser);  // Переадресация с последовательного порта SIM800L на последовательный порт Arduino IDE
      logger.write(ser);
      delay(10); 
    }
    //logger.println();
  }
  if (SerialGSM.available() > 0)
  {
    while(SerialGSM.available()) 
    {
      int gsm_ser=SerialGSM.read();
      //GSMdata = SerialGSM.readStringUntil('\n');
      //gsm_serString+=char(gsm_ser);
      //Serial.println(GSMdata);      // Переадресация c Arduino IDE на последовательный порт SIM800L
      Serial.write(gsm_ser);
      logger.write(gsm_ser);
      //logger.println(GSMdata);
      looper=millis();
      delay(10); 
    }
    //logger.println();
    //delay(10);                           // Пауза 500 мс
  }
  /*if (serString!="")
  {
    logger.println("From Serial");
    logger.println(serString);
    serString="";
  }
  if (gsm_serString!="")
  {
    logger.println("From GSM");
    logger.println(gsm_serString);
    gsm_serString="";
  } */
}

void senddata(String lat, String lon, String speed, String tochnost)
{
  //SerialGSM.listen();
  delay(1000);
  logger.println("!!!!!!!Sent data!!!!!!!!");
  logger.print("Location valid ");logger.println(gps.location.isValid());
  SerialGSM.println("AT+SAPBR=1,1");         
  //updateSerial();
  delay(1000);
  SerialGSM.println("AT+CIPSTART=\"UDP\",\"217.25.227.150\",\"555\"");
  updateSerial();
  delay(1000);
  SerialGSM.println("AT+CIPSEND=11");
  updateSerial();
  delay(1000);
  SerialGSM.println("phone/check");
  updateSerial();
  delay(1000);
  SerialGSM.println("AT+CIPCLOSE");
  updateSerial();
  delay(1000);
  SerialGSM.println("AT+CIPSTART=\"TCP\",\"217.25.227.150\",\"8265\"");
  updateSerial();
  while (SerialGSM.available()==0)
  {
    delay(5000); 
  }
  //SerialGSM.println("AT+CIPSEND="+String (94+LatitudeString.length()+LongitudeString.length()+SpeedString.length()));
  SerialGSM.println("AT+CIPSEND");
  updateSerial();
  SerialGSM.print("GET /gps.php?latitude=");
  updateSerial();
  SerialGSM.print(lat);
  updateSerial();
  SerialGSM.print("&longitude=");
  updateSerial();
  SerialGSM.print(lon);
  //updateSerial();
  SerialGSM.print("&speed=");  
  //updateSerial();
  SerialGSM.print(speed);
  //updateSerial();
  SerialGSM.print("&deviceid=granta&battlevel=100");
  //updateSerial();
  SerialGSM.print("&accuracy=");  
  //updateSerial();
  SerialGSM.print(tochnost);
  //updateSerial();
  SerialGSM.print(" HTTP/1.1\r\nHost: 217.25.227.150:8265\r\n\r\n");
  updateSerial();
  delay(1500);
  SerialGSM.println((char)26);
  while (SerialGSM.available()==0)
  {
    delay(5000); 
  }
  updateSerial();
  delay(5000);
  SerialGSM.println("AT+CIPCLOSE");
  updateSerial();
  delay(1500);
  SerialGSM.println("AT+SAPBR=0,1");     
  updateSerial();
  delay(1500);
  deley=millis();
  //SerialGPS.listen();
  //delay(1000);
}

void senddatagsm(String data1, String speed1)
{
  //SerialGSM.listen();
  delay(5000);
  SerialGSM.flush();
  while (SerialGSM.available() > 0)
  {
    serInt=SerialGSM.read();
    //Serial.println("SendDataGSM");
    delay(50);
  }
  delay(1000);
  Serial.println("SendDataGSM");
  updateSerial();
  logger.println("!!!!!!!Sent dataGSM!!!!!!!!");
  Serial.println("!!!!!!!Sent dataGSM!!!!!!!!");
  SerialGSM.println("AT"); 
  updateSerial();
  SerialGSM.println("AT+SAPBR=1,1");         
  delay(5000);
  updateSerial();
  delay(1000);
  SerialGSM.println("AT+CIPSTART=\"UDP\",\"217.25.227.150\",\"555\"");
  updateSerial();
  delay(2000);
  updateSerial();
  delay(1000);
  SerialGSM.println("AT+CIPSEND=11");
  updateSerial();
  delay(1000);
  SerialGSM.println("phone/check");
  updateSerial();
  delay(1000);
  SerialGSM.println("AT+CIPCLOSE");
  updateSerial();

  /*SerialGSM.println( "AT+SAPBR=2,1" );
  SerialGSM.println( "AT+HTTPINIT" );
  SerialGSM.println( "AT+HTTPPARA=CID,1" );
  SerialGSM.println( "AT+HTTPPARA=URL,http://217.25.227.150:8265/gpsgsm.php?gsmdata="  + data + "&speed=" + speed + "&deviceid=granta&battlevel=100");
  updateSerial();
  SerialGSM.println( "AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded" );
  updateSerial();
  int part=data.length();
  SerialGSM.println( "AT+HTTPDATA="+String(part+20)+",5000" );
  updateSerial();
  SerialGSM.println( "AT+HTTPACTION=1" );
  updateSerial();
  SerialGSM.println( "AT+HTTPREAD" );
  updateSerial();
  SerialGSM.println( "AT+HTTPTERM" );
  updateSerial();
  SerialGSM.println( "AT+SAPBR=0,1" );
  updateSerial();*/
int part=data1.length()/7;
  Serial.println(data1.substring(0,part));
  Serial.println(data1.substring(part,part*2));
  Serial.println(data1.substring(part*2,part*3));
  Serial.println(data1.substring(part*3,part*4));
  Serial.println(data1.substring(part*4,part*5));
  Serial.println(data1.substring(part*5,part*6));
  Serial.println(data1.substring(part*6));
  delay(1000);
  SerialGSM.println("AT+CIPSTART=\"TCP\",\"217.25.227.150\",\"8265\"");
  //updateSerial();
  while (SerialGSM.available()==0)
  {
  delay(200); 
  }
  //while (SerialGSM.available()>0)
  //{
  //  Serial.print("Цикл ");
  //  GSMdata = SerialGSM.readStringUntil('\n');
  //  Serial.println(GSMdata);
  //  if (GSMdata.startsWith("OK"))
  //  {
  //    Serial.println("Подтверждение");
  //  }
  //}
  while (SerialGSM.available()>0)
  {
    GSMdata = SerialGSM.readStringUntil('\n');
    if (GSMdata.startsWith("CONNECT OK"))
    {
      Serial.println("Подключение ОК");
    }
    else if (GSMdata.startsWith("ERROR"))
    {
      Serial.println("Ошибка");
      SerialGSM.println("AT+CIPSTART=\"TCP\",\"217.25.227.150\",\"8265\"");
    }
  }
  updateSerial();
  //while (SerialGSM.available()==0)
  //{
  //  delay(5000); 
  //}
  //SerialGSM.println("AT+CIPSEND="+String (94+LatitudeString.length()+LongitudeString.length()+SpeedString.length()));
  SerialGSM.println("AT+CIPSEND");
  updateSerial();
  delay(1000);
  updateSerial();
  delay(1000);
  updateSerial();
  SerialGSM.print("GET /gpsgsm.php?gsmdata=");
  updateSerial();
  //Serial.println(data1.length());
  
  
  SerialGSM.print(data1.substring(0,part));
  delay(100);
  updateSerial();
  //Serial.println(data.substring(0,part));
  SerialGSM.print(data1.substring(part,part*2));
  delay(100);
  updateSerial();
  //Serial.println(data.substring(part,part*2));
  SerialGSM.print(data1.substring(part*2,part*3));
  delay(100);
  updateSerial();
  //Serial.println(data.substring(part*2,part*3));
  SerialGSM.print(data1.substring(part*3,part*4));
  delay(100);
  updateSerial();
  //Serial.println(data.substring(part*3,part*4));
  SerialGSM.print(data1.substring(part*4,part*5));
  delay(100);
  updateSerial();
  //Serial.println(data.substring(part*4,part*5));
  SerialGSM.print(data1.substring(part*5,part*6));
  delay(100);
  updateSerial();
  //Serial.println(data.substring(part*5,part*6));
  SerialGSM.print(data1.substring(part*6));
  delay(100);
  updateSerial();
  //Serial.println(data.substring(part*6));
  //updateSerial();
  SerialGSM.print("&speed="); 
  delay(100); 
  updateSerial();
  SerialGSM.print(speed1);
  delay(100);
  //updateSerial();
  SerialGSM.print("&deviceid=granta&battlevel=100");
  delay(100);
  updateSerial();
  SerialGSM.print(" HTTP/1.1\r\nHost: 217.25.227.150:8265\r\n\r\n");
  delay(100);
  updateSerial();
  delay(1500);
  SerialGSM.println((char)26);
  while (SerialGSM.available()==0)
  {
  delay(200); 
  }
  while (SerialGSM.available()>0)
  {
    Serial.print("Цикл ");
    GSMdata = SerialGSM.readStringUntil('\n');
    Serial.println(GSMdata);
    if (GSMdata.startsWith("SEND OK"))
    {
      Serial.println("Подключение ОК");
    }
    else if (GSMdata.startsWith("ERROR"))
    {
      Serial.println("Ошибка");
      SerialGSM.println("AT+CIPSTART=\"TCP\",\"217.25.227.150\",\"8265\"");
    }
  }
  //delay(1000);
  updateSerial();
  //while (SerialGSM.available()==0)
  //{
    delay(2000); 
    updateSerial();
 // }
  SerialGSM.println("AT+CIPCLOSE");
  updateSerial();
  delay(1500);
  SerialGSM.println("AT+SAPBR=0,1"); 
  while (SerialGSM.available()==0)
  {
    delay(5000); 
  }
  updateSerial();
  deley=millis();
  //SerialGPS.listen();
  //delay(1000);
}

int StrToHex(char str[])
{
 return (int) strtol(str, 0, 16);
}

void updateSerialGPS()
{
  /*if (Serial.available() > 0 )
  {
    while (Serial.available()) 
    {
      int ser=Serial.read();
      serString+=char(ser);
      SerialGPS.write(ser);  // Переадресация с последовательного порта SIM800L на последовательный порт Arduino IDE
      //logger.write(ser);
      //delay(10); 
    }
    //logger.println();
  }*/
  if (SerialGPS.available() > 0)
  {
    while(SerialGPS.available()) 
    {
      logger.println("-------------------");
      GPSdata = SerialGPS.readStringUntil('\n');
      logger.println(GPSdata);
      logger.println("-------------------");
      //int gps_ser=SerialGPS.read();
      //gps_serString+=char(gps_ser);
      //Serial.write(gps_ser);      // Переадресация c Arduino IDE на последовательный порт SIM800L
      //logger.write(gsm_ser);
      //delay(10); 
    }
    //logger.println();
    //delay(10);                           // Пауза 500 мс
  }
  /*if (serString!="")
  {
    logger.println("From Serial");
    logger.println(serString);
    serString="";
  }
  if (gps_serString!="")
  {
    logger.println("From GPS");
    logger.println(gps_serString);
    gps_serString="";
  } */ 
}

String* split(String& v, char delimiter, int& length) 
{
  length = 1;
  bool found = false;
  // Figure out how many itens the array should have
  for (int i = 0; i < v.length(); i++) {
    if (v[i] == delimiter) {
      length++;
      found = true;
    }
  }
  // If the delimiter is found than create the array
  // and split the String
  if (found) {
    // Create array
    String* valores = new String[length];
    // Split the string into array
    int i = 0;
    for (int itemIndex = 0; itemIndex < length; itemIndex++) {
      for (; i < v.length(); i++) {
        if (v[i] == delimiter) {
          i++;
          break;
        }
        valores[itemIndex] += v[i];
      }
    }
    // Done, return the values
    return valores;
  }
  // No delimiter found
  return nullptr;
}
