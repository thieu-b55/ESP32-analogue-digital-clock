/*
     SPI
     MOSI            23
     MISO            19
     SCLK            18
     CS KLOK         26
     CS DATUM/TEMP   27

     I2C
     SDA             21
     SCL             22

     Serial2
     RX              16            
     TX              17

    Libraries
    *** USE THESE LIBRARIES ***
    * Also use the latest ESP32 arduino core by Espressif Systems  >>> see Board manager 
    * At this moment 3.0.3
    AsyncTCP            https://github.com/me-no-dev/AsyncTCP
    ESPAAsyncWebServer  https://github.com/me-no-dev/ESPAsyncWebServer
*/

#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include <TinyGPSPlus.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#include <AsyncTCP.h>  
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#define LED_PIN     25
#define NUM_LEDS    60
#define HELDERHEID  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

#define DS3231SN      0x68
#define CS_KLOK       26
#define CS_DATUM      27
#define ONEWIREBUS    4
#define SECONDEN_INT  39

#define RX2           16
#define TX2           17

SPISettings spiSettings = SPISettings(15000, SPI_MSBFIRST, SPI_MODE2);
OneWire oneWire(ONEWIREBUS);
DallasTemperature sensors(&oneWire);
TinyGPSPlus gps;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_RGB + NEO_KHZ800);
AsyncWebServer server(80);
Preferences pref;

bool seconde_interrupt_bool = false;
bool DS3231SN_gezet = false;

/*
   uuraanduiding ledring gebeurt door 3 blauwe leds en dat zijn deze per uur van 0 tot 11 uur
*/
char led[12][3] = {{59, 0, 1}, {4, 5, 6}, {9, 10, 11}, {14, 15, 16}, {19, 20, 21}, {24, 25, 26}, {29, 30, 31}, {34, 35, 36},
                   {39, 40, 41}, {44, 45, 46}, {49, 50, 51}, {54, 55, 56}};

char tijd_html_char[6];  
char datum_html_char[6];
char maand_html_char[6];
char jaar_html_char[6]; 
char zomeruur_html_char[6];
char zomermin_html_char[6];   
char utcuur_html_char[6];
char utcmin_html_char[6];   
 
const char* APSSID = "ESP32Klok";
const char* APPSWD = "ESP32pswd";   

const char* TIJD_HTML_INPUT = "tijd_html_input";
const char* TIJD_BEVESTIG = "tijd_bevestig";
const char* DATUM_HTML_INPUT = "datum_html_input";
const char* MAAND_HTML_INPUT = "maand_html_input";
const char* JAAR_HTML_INPUT = "jaar_html_input";
const char* DATUM_BEVESTIG = "datum_bevestig";
const char* ZOMERUUR_HTML_INPUT = "zomeruur_html_input";
const char* ZOMERMIN_HTML_INPUT = "zomermin_html_input";
const char* ZOMERTIJD_BEVESTIG = "zomertijd_bevestig";
const char* UTCUUR_HTML_INPUT = "utcuur_html_input";
const char* UTCMIN_HTML_INPUT = "utcmin_html_input";
const char* UTCTIJD_BEVESTIG = "utctijd_bevestig";

float temperatuurC_float;

int seconde_int;
int minuut_int;
int uur_int;
int dag_int;
int datum_int;
int maand_int;
int jaar_int;
int temperatuur_int;
int temp_int;
int teller_int = 0;
int gps_seconde;
int gps_minuut;
int gps_uur;
int gps_dag;
int gps_maand;
int gps_jaar;
int seconde;
int minuut;
int uur;
int dag;
int maand;
int jaar;
int html_uur_int;
int html_minuut_int;
int plus;
int tmp;
int adres;
int zomeruur = 1;
int zomermin = 0;
int zomeruur_int;
int zomermin_int;
int tijdzone_uur = 1;
int tijdzone_minuut = 0;
int datum_html_int;
int maand_html_int;
int jaar_html_int;
int utcuur_int;
int utcmin_int;

int maand_array[13] = {31, 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

String tijd_html_string = "          ";
String datum_html_string = "    ";
String maand_html_string = "    ";
String jaar_html_string = "      ";
String zomeruur_html_string = "     ";
String zomermin_html_string = "     ";
String utcuur_html_string = "      ";
String utcmin_html_string = "      ";

unsigned long nu_ul;

void seconde_voorbij();
byte dec_naar_bcd(byte waarde);
byte bcd_naar_dec(byte waarde);
void schrijf_DS3231SN();
void lees_DS3231SN();
void display_klok(uint8_t adres, uint8_t waarde);
void display_datum_temp(uint8_t adres, uint8_t waarde);
void toon_tijd();
void toon_temperatuur();
void toon_datum();
void gps_opvragen();
void gps_tijd(TinyGPSTime &t, TinyGPSDate &d);


const char index_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>ESP32 Klok</title>
    <meta name="viewport" content="width=device-width, initial-scale=.80">
    <style>
      input {
        text-align: center;
      }
      div.kader {
          position: relative;
          left: 0px;
          width: 400px;
      }
      div.vak_1 {
        position: absolute;
        left : 133px;
        width: 66px;
      }
      div.vak_2 {
        position: absolute;
        left : 199px;
        width: 66px;
      }
      div.blanco_10{
        width: auto;
        height: 10px;
      }
      div.blanco_20{
        width: auto;
        height: 20px;
      }
      div.blanco_30{
        width: auto;
        height: 30px;
      }
      div.blanco_40{
        width: auto;
        height: 40px;
      }
      div.blanco_60{
        width: auto;
        height: 60px;
      }
    </style>
  </head>
  <body>
    <center>
      <h3><center> ESP32 Klok Temperatuur </center></h3>
      <small>
        <div class="kader">
          <b><center> Tijd </center></b>
          <div class="blanco_20">&nbsp;</div>
          <form action="/get" target="hidden-form">
            <center><input type="text" name="tijd_html_input" value=%tijd_html% size=1></center>
            <div class="blanco_20">&nbsp;</div>
            <center><input type="submit" name="tijd_bevestig" value=" OK " onclick="bevestig()"></center>
          </form>
          <div class="blanco_30">&nbsp;</div>
          <b><center> Datum </center></b>
          <div class="blanco_20">&nbsp;</div>
          <form action="/get" target="hidden-form">
          <center>
            <input type="text" name="datum_html_input" value=%datum_html% size=1>
            <input type="text" name="maand_html_input" value=%maand_html% size=1>
            <input type="text" name="jaar_html_input" value=%jaar_html% size=1>
          </center>
          <div class="blanco_20">&nbsp;</div>
          <center><input type="submit" name="datum_bevestig" value=" OK " onclick="bevestig()"></center>
          </form>
          <div class="blanco_30">&nbsp;</div>
          <b><center> Zomertijd </center></b>
          <div class="blanco_10">&nbsp;</div>
          <b>
            <div class="vak_1">
              <center> Uur </center>
            </div>
            <div class="vak_2">
              <center> Minuten </center>
            </div>
          </b>
          &nbsp;
          <div class="blanco_20">&nbsp;</div>
          <form action="/get" target="hidden-form">
            <div class="vak_1">
              <center><input type="text" name="zomeruur_html_input" value=%zomeruur_html% size=1</center>
            </div>
            <div class="vak_2">
              <center><input type="text" name="zomermin_html_input" value=%zomermin_html% size=1</center>
            </div>
            &nbsp;
            <div class="blanco_30">&nbsp;</div>
            <center><input type="submit" name="zomertijd_bevestig" value=" OK " onclick="bevestig()"></center>
          </form>
          <div class="blanco_30">&nbsp;</div>
          <b><center> UTC Verschil </center></b>
          <div class="blanco_10">&nbsp;</div>
          <b>
            <div class="vak_1">
              <center> Uur </center>
            </div>
            <div class="vak_2">
              <center> Minuten </center>
            </div>
          </b>
          &nbsp;
          <div class="blanco_20">&nbsp;</div>
          <form action="/get" target="hidden-form">
            <div class="vak_1">
              <center><input type="text" name="utcuur_html_input" value=%utcuur_html% size=1</center>
            </div>
            <div class="vak_2">
              <center><input type="text" name="utcmin_html_input" value=%utcmin_html% size=1</center>
            </div>
            &nbsp;
            <div class="blanco_30">&nbsp;</div>
            <center><input type="submit" name="utctijd_bevestig" value=" OK " onclick="bevestig()"></center>
          </form>
          <div class="blanco_30">&nbsp;</div>
          <b>
          <center> GPS signaal </center>
          </b>
          <div class="blanco_2">&nbsp;</div>
          <center> %gps_status% </center>
        </div>
      </small>
    </center>
    <div class="blanco_20">&nbsp;</div>
    <h6>thieu-b55 augustus 2024</h6>
    <script>
      function bevestig(){
        setTimeout(function(){document.location.reload();},250);
      }
    </script>
  </body>  
</html>
)rawliteral";

String processor(const String& var){
  if(var == "tijd_html"){
    lees_DS3231SN();
    sprintf(tijd_html_char, "%02d:%02d", uur_int, minuut_int);
    return(tijd_html_char);
  }
  if(var == "datum_html"){
    sprintf(datum_html_char, "%02d", datum_int);
    return(datum_html_char);
  }
  if(var == "maand_html"){
    sprintf(maand_html_char, "%02d", maand_int);
    return(maand_html_char);
  }
  if(var == "jaar_html"){
    sprintf(jaar_html_char, "%04d", jaar_int);
    return(jaar_html_char);
  }
  if(var == "zomeruur_html"){
    sprintf(zomeruur_html_char, "%02d%", zomeruur);
    return(zomeruur_html_char);
  }
  if(var == "zomermin_html"){
    sprintf(zomermin_html_char, "%02d%", zomermin);
    return(zomermin_html_char);
  }
  if(var == "utcuur_html"){
    sprintf(utcuur_html_char, "%02d%", tijdzone_uur);
    return(utcuur_html_char);
  }
  if(var == "utcmin_html"){
    sprintf(utcmin_html_char, "%02d%", tijdzone_minuut);
    return(utcmin_html_char);
  }
  if(var == "gps_status"){
    if(DS3231SN_gezet){
      return("gps ontvangst");
    }
    else{
      return("geen ontvangst");
    }
  }
  return("");
}
  
void html_input(){
  server.begin();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
    char terminator = char(0x0a);
    if(request->hasParam(TIJD_HTML_INPUT)){
      tijd_html_string = "";
      tijd_html_string = (request->getParam(TIJD_HTML_INPUT) -> value()) + String(terminator);
    }
    if(request->hasParam(TIJD_BEVESTIG)){
      if(!DS3231SN_gezet){
        int pos = tijd_html_string.indexOf(":");
        if(pos != -1){
          html_uur_int = tijd_html_string.substring(0, pos).toInt();
          html_minuut_int = tijd_html_string.substring(pos + 1).toInt();
          if((html_uur_int > -1) && (html_uur_int < 24) && (html_minuut_int > -1) && (html_minuut_int < 60)){
             Wire.beginTransmission(DS3231SN);
             Wire.write(0x01);
             Wire.write(dec_naar_bcd(html_minuut_int));
             Wire.write(dec_naar_bcd(html_uur_int & 0x3f));
             Wire.endTransmission();
          }
        }
      }
    }
    if(request->hasParam(DATUM_HTML_INPUT)){
      datum_html_string = "";
      datum_html_string = (request->getParam(DATUM_HTML_INPUT) -> value()) + String(terminator);
    }
    if(request->hasParam(MAAND_HTML_INPUT)){
      maand_html_string = "";
      maand_html_string = (request->getParam(MAAND_HTML_INPUT) -> value()) + String(terminator);      
    }
    if(request->hasParam(JAAR_HTML_INPUT)){
      jaar_html_string = "";
      jaar_html_string = (request->getParam(JAAR_HTML_INPUT) -> value()) + String(terminator);      
    }
    if(request->hasParam(DATUM_BEVESTIG)){
      if(!DS3231SN_gezet){
        datum_html_int = datum_html_string.toInt();
        maand_html_int = maand_html_string.toInt();
        jaar_html_int = jaar_html_string.toInt();
        if((datum_html_int > 0) && (datum_html_int < 32) && (maand_html_int > 0) && (maand_html_int < 13) && (jaar_html_int > 2023) && (jaar_html_int < 2200)){
          Wire.beginTransmission(DS3231SN);
          Wire.write(0x04);
          Wire.write(dec_naar_bcd(datum_html_int));
          Wire.write(dec_naar_bcd(maand_html_int));
          Wire.write(dec_naar_bcd(jaar_html_int - 2000));
          Wire.endTransmission();
          lees_DS3231SN();
          toon_datum();
        }
      }
    }
    if(request->hasParam(ZOMERUUR_HTML_INPUT)){
      zomeruur_html_string = "";
      zomeruur_html_string = (request->getParam(ZOMERUUR_HTML_INPUT) -> value()) + String(terminator); 
    }
    if(request->hasParam(ZOMERMIN_HTML_INPUT)){
      zomermin_html_string = "";
      zomermin_html_string = (request->getParam(ZOMERMIN_HTML_INPUT) -> value()) + String(terminator); 
    }
    if(request->hasParam(ZOMERTIJD_BEVESTIG)){
      zomeruur_int = zomeruur_html_string.toInt();
      zomermin_int = zomermin_html_string.toInt();
      if((((zomermin_int == 0) && (zomeruur_int == 0)) || ((zomermin_int == 0) && (zomeruur_int == 1))) || ((zomermin_int == 30) && (zomeruur_int == 0))){
        pref.putShort("zomeruur", zomeruur_int);
        pref.putShort("zomermin", zomermin_int);
        zomeruur = pref.getShort("zomeruur");
        zomermin = pref.getShort("zomermin");
        if(DS3231SN_gezet){
          gps_opvragen();
          schrijf_DS3231SN();
        }
      }
    }
    if(request->hasParam(UTCUUR_HTML_INPUT)){
      utcuur_html_string = "";
      utcuur_html_string = (request->getParam(UTCUUR_HTML_INPUT) -> value()) + String(terminator);
    }
    if(request->hasParam(UTCMIN_HTML_INPUT)){
      utcmin_html_string = "";
      utcmin_html_string = (request->getParam(UTCMIN_HTML_INPUT) -> value()) + String(terminator);
    }
    if(request->hasParam(UTCTIJD_BEVESTIG)){
      utcuur_int = utcuur_html_string.toInt();
      utcmin_int = utcmin_html_string.toInt();
      if((utcuur_int > -13) && (utcuur_int < 15) && ((utcmin_int == 0) || (utcmin_int == 30))){
        pref.putShort("tzuur", utcuur_int);
        pref.putShort("tzmin", utcmin_int);
        tijdzone_uur = pref.getShort("tzuur");
        tijdzone_minuut = pref.getShort("tzmin");
        if(DS3231SN_gezet){
          gps_opvragen();
          schrijf_DS3231SN();
        }
      }
    }
  });
}
  
void setup() {
  delay(2500);
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(APSSID, APPSWD);
  html_input();
  sensors.begin();
  pref.begin("instelling", false); 
  //pref.clear();
  if(pref.getString("controle") != "dummy geladen"){
    pref.putShort("zomeruur", 1);
    pref.putShort("zomermin", 0);
    pref.putShort("tz_uur", 1);
    pref.putShort("tz_minuut", 0);
    pref.putString("controle", "dummy geladen");
  }
  zomeruur = pref.getShort("zomeruur");
  zomermin = pref.getShort("zomermin");
  tijdzone_uur = pref.getShort("tz_uur");
  tijdzone_minuut = pref.getShort("tz_minuut");
  pinMode(CS_DATUM, OUTPUT);
  pinMode(CS_KLOK, OUTPUT);
  pinMode(SECONDEN_INT, INPUT);
  attachInterrupt(digitalPinToInterrupt(SECONDEN_INT), seconde_voorbij, FALLING);
  digitalWrite(CS_DATUM, HIGH);
  digitalWrite(CS_KLOK, HIGH);
  delay(100);
  Wire.begin();                       // SDA >> GPIO21    SCL >> GPIO22
  Wire.setClock(40000);               // slow chinese DS3231 modules
  Wire.beginTransmission(DS3231SN);
  Wire.write(0x07);
  Wire.write(0x80);
  Wire.write(0x80);
  Wire.write(0x80);
  Wire.write(0x80);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x45);
  Wire.write(0x00);
  Wire.endTransmission();
  pixels.begin();
  pixels.show();
  SPI.begin();
  display_klok(0x0F, 0x00);
  delay(10);
  display_klok(0x09, 0xFF);
  display_klok(0x0A, 0x01);
  display_klok(0x0B, 0x07);
  display_klok(0x0C, 0x01);
  display_datum_temp(0x0F, 0x00);
  delay(10);
  display_datum_temp(0x09, 0xFF);
  display_datum_temp(0x0A, 0x01);
  display_datum_temp(0x0B, 0x07);
  display_datum_temp(0x0C, 0x01);
  toon_temperatuur();
}

void loop() {
  if (seconde_interrupt_bool) {
    seconde_interrupt_bool = false;
    Wire.beginTransmission(DS3231SN);
    Wire.write(0x0F);
    Wire.write(0x00);
    Wire.endTransmission();
    lees_DS3231SN();
    if ((minuut_int == 0) && (seconde_int == 0) && (DS3231SN_gezet)) {
      gps_opvragen();
      schrijf_DS3231SN();
      lees_DS3231SN();
    }
    toon_tijd();
    if (seconde_int == 31) {
      toon_temperatuur();
    }
    if (seconde_int == 1) {
      toon_datum();
    }
    if (!DS3231SN_gezet) {
      teller_int ++;
      if (teller_int == 60) {
        teller_int = 0;
        gps_opvragen();
        if ((gps_dag != 0) && (gps_maand != 0)) {
          DS3231SN_gezet = true;
          schrijf_DS3231SN();
        }
      }
    }
  }
}

void IRAM_ATTR seconde_voorbij() {
  seconde_interrupt_bool = true;
}

byte dec_naar_bcd(byte waarde) {
  return (((waarde / 10) << 4) + (waarde % 10));
}

byte bcd_naar_dec(byte waarde) {
  return (((waarde >> 4) * 10) + (waarde % 16));
}

void schrijf_DS3231SN() {
  minuut = gps_minuut;
  uur = gps_uur;
  dag = gps_dag;
  maand = gps_maand;
  jaar = gps_jaar - 2000;
  if (tijdzone_uur >= 0) {
    minuut = minuut + tijdzone_minuut + zomermin;
    if (minuut > 59) {
      minuut -= 60;
      uur += 1;
      if (uur > 23) {
        uur -= 24;
        dag += 1;
        if (maand != 2) {
          if (dag > maand_array[maand]) {
            dag -= maand_array[maand];
            maand += 1;
            if (maand > 12) {
              maand -= 12;
              jaar += 1;
            }
          }
        }
        if (maand == 2) {
          if (((jaar % 4) == 0)) {
            if (dag > 29) {
              dag -= 29;
              maand += 1;
            }
          }
          if (((jaar % 4) != 0)) {
            if (dag > 28) {
              dag -= 28;
              maand += 1;
            }
          }
        }
      }
    }
    uur = uur + tijdzone_uur + zomeruur;
    if (uur > 23) {
      uur -= 24;
      dag += 1;
      if (maand != 2) {
        if (dag > maand_array[maand]) {
          dag -= maand_array[maand];
          maand += 1;
          if (maand > 12) {
            maand -= 12;
            jaar += 1;
          }
        }
      }
      else {
        if (((jaar % 4) == 0)) {
          if (dag > 29) {
            dag -= 29;
          }
        }
        else {
          if (dag > 28) {
            dag -= 28;
          }
        }
        maand += 1;
      }
    }
  }
  if(tijdzone_uur < 0){
    minuut = minuut - tijdzone_minuut;
    if (minuut < 0) {
      minuut += 60;
      uur -= 1;
      if (uur < 0) {
        uur += 24;
        dag -= 1;
        if (dag == 0) {
          if (maand != 3) {
            dag = maand_array[maand - 1];
            maand -= 1;
            if (maand == 0) {
              maand = 12;
              jaar -= 1;
            }
          }
          else {
            if (((jaar % 4) == 0)) {
              dag = 29;
            }
            else {
              dag = 28;
            }
          }
          maand -= 1;
        }
      }
    }
    uur = uur + tijdzone_uur + zomeruur;
    if (uur < 0) {
      uur += 24;
      dag -= 1;
      if (dag == 0) {
        if (maand != 3) {
          dag = maand_array[maand - 1];
          maand -= 1;
          if (maand == 0) {
            maand = 12;
            jaar -= 1;
          }
        }
        else {
          if (((jaar % 4) == 0)) {
            dag = 29;
          }
          else {
            dag = 28;
          }
        }
        maand -= 1;
      }
    }
  }
  Wire.beginTransmission(DS3231SN);
  Wire.write(0x00);
  Wire.write(dec_naar_bcd(gps_seconde));
  Wire.write(dec_naar_bcd(minuut));
  Wire.write(dec_naar_bcd(uur & 0x3f));
  Wire.write(1);
  Wire.write(dec_naar_bcd(dag));
  Wire.write(dec_naar_bcd(maand));
  Wire.write(dec_naar_bcd(jaar));
  Wire.endTransmission();
}

void lees_DS3231SN() {
  Wire.beginTransmission(DS3231SN);
  Wire.write(0x0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231SN, 7);
  seconde_int = (bcd_naar_dec(Wire.read()));
  minuut_int = (bcd_naar_dec(Wire.read()));
  uur_int = (bcd_naar_dec(Wire.read()));
  dag_int = Wire.read();
  datum_int = (bcd_naar_dec(Wire.read()));
  maand_int = (bcd_naar_dec(Wire.read()));
  jaar_int = (bcd_naar_dec(Wire.read())) + 2000;
}

void display_klok(uint8_t adres, uint8_t waarde) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_KLOK, LOW);
  SPI.transfer(adres);
  SPI.transfer(waarde);
  digitalWrite(CS_KLOK, HIGH);
  SPI.endTransaction();
}

void display_datum_temp(uint8_t adres, uint8_t waarde) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS_DATUM, LOW);
  SPI.transfer(adres);
  SPI.transfer(waarde);
  digitalWrite(CS_DATUM, HIGH);
  SPI.endTransaction();
}

void toon_tijd() {
  display_klok(0x01, 0x0F);
  display_klok(0x02, (seconde_int % 10));
  display_klok(0x03, (seconde_int / 10));
  display_klok(0x04, (minuut_int % 10) | 0x80);
  display_klok(0x05, (minuut_int / 10));
  display_klok(0x06, (uur_int % 10) | 0x80);
  display_klok(0x07, (uur_int / 10));
  display_klok(0x08, 0x0F);

  /*
    uitsturen ledring zwak rood/groen voor de uren op de wijzerplaat
                      helder groen zijn de seconden
                      rood de minuten
                      1 heldere en 2 zwakke blauwe leds de uren
  */
  pixels.clear();
  pixels.setPixelColor(seconde_int, HELDERHEID / 2, 0, 0);
  tmp = pixels.getPixelColor(minuut_int);
  tmp = tmp + (HELDERHEID << 8);
  pixels.setPixelColor(minuut_int, tmp);
  /*
     variable plus zorgt ervoor dat de uur aanduiding elke 12 minuten 1 led verder springt
  */
  plus = minuut_int / 12;
  /*
     12 uur analoge klok
  */
  if (uur_int > 11) {
    uur_int -= 12;
  }
  adres = led[uur_int][0] + plus;
  if (adres > 59) {
    adres = adres - 60;
  }
  tmp = pixels.getPixelColor(adres);
  tmp = tmp + 5;
  pixels.setPixelColor(adres, tmp);
  adres = led[uur_int][1] + plus;
  tmp = pixels.getPixelColor(adres);
  tmp = tmp + HELDERHEID;
  pixels.setPixelColor(adres, tmp);
  adres = led[uur_int][2] + plus;
  if (adres > 59) {
    adres = adres - 60;
  }
  tmp = pixels.getPixelColor(adres);
  tmp = tmp + 5;
  pixels.setPixelColor(adres, tmp);
  /*
     uur aanduiding
  */
  for (int i = 0; i < NUM_LEDS; i = i + 5) {
    tmp = pixels.getPixelColor(i);
    if (tmp == 0) {
      pixels.setPixelColor(i, 1, 1, 0);
    }
  }
  pixels.show();
}

void toon_temperatuur() {
  sensors.requestTemperatures();
  temperatuurC_float = sensors.getTempCByIndex(0);
  temperatuur_int = temperatuurC_float * 100;
  display_datum_temp(0x09, 0XF8);
  if (temperatuur_int < 0) {
    display_datum_temp(0x08, 0x0E);
  }
  else {
    display_datum_temp(0x08, 0x0F);
  }
  temp_int = temperatuur_int / 1000;
  display_datum_temp(0x07, temp_int);
  temp_int = temperatuur_int % 1000;
  temperatuur_int = temp_int / 100;
  display_datum_temp(0x06, (temperatuur_int | 0x80));
  temperatuur_int = temp_int % 100;
  temp_int = temperatuur_int / 10;
  display_datum_temp(0x05, temp_int);
  temp_int = temperatuur_int % 10;
  display_datum_temp(0x04, temp_int);
  display_datum_temp(0x03, 0x63);
  display_datum_temp(0x02, 0x4E);
  display_datum_temp(0x01, 0x00);
}

void toon_datum() {
  display_datum_temp(0x09, 0xFF);
  display_datum_temp(0x01, (jaar_int % 10));
  temp_int = jaar_int / 10;
  display_datum_temp(0x02, (temp_int % 10));
  temp_int = temp_int / 10;
  display_datum_temp(0x03, (temp_int % 10));
  display_datum_temp(0x04, (temp_int / 10));
  display_datum_temp(0x05, (maand_int % 10) | 0x80);
  display_datum_temp(0x06, (maand_int / 10));
  display_datum_temp(0x07, (datum_int % 10) | 0x80);
  display_datum_temp(0x08, (datum_int / 10));
}

void gps_opvragen() {
  //Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  //Serial2.setRxBufferSize(2048);
  nu_ul = millis();
  while ((millis() - nu_ul) < 1000) {
    while (Serial2.available()) {
      gps.encode(Serial2.read());
    }
  }
  //Serial2.end();
  gps_tijd(gps.time, gps.date);
  
}

void gps_tijd(TinyGPSTime &t, TinyGPSDate &d) {
  gps_seconde = t.second();
  gps_minuut = t. minute();
  gps_uur = t.hour();
  gps_dag = d.day();
  gps_maand = d.month();
  gps_jaar = d.year();
}
