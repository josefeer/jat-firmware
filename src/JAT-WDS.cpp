#include <Arduino.h>

/*RF Libraries*/
#include <DigitalIO.h>
#include <SPI.h>
#include "RF24.h"

/*Temp Sensor Libraries*/
#include <OneWire.h>
#include <DallasTemperature.h>

/*Bluetooth Library*/
#include <SoftwareSerial.h>

/*ILI9341 Library*/
#include <Adafruit_ILI9341.h>

/*Constants Temp Sensor*/
const int ONE_WIRE_BUS = 6;

/*Constants RF*/
const int radioID = 1;
const int Group1 = 76;
const int Group2 = 115;

/*Constants ILI9341*/
#define TFT_DC 9
#define TFT_CS 7
#define TFT_RST 8
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_CLK 13

/*RF CONFIG*/
RF24 Radio(4,5);
byte addresses[][6] = {"1Node","2Node", "3Node", "4Node", "5Node", "6Node", "7Node", "8Node", "9Node"};

/*Temp Sensor CONFIG*/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/*Bluetooth CONFIG*/
SoftwareSerial BTserial(18, 19); // RX | TX

/*ILI9341 CONFIG*/
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

/*Global Variables*/
struct sensors{
    float s1 = -127.00;
    float s2 = -127.00;
    float s3 = -127.00;
    float s4 = -127.00;
    float s5 = -127.00;
    float s6 = -127.00;
    float s7 = -127.00;
    float s8 = -127.00;
    float s9 = -127.00;
} values;

struct mainjson{
    String s1;
    String s2;
    String s3;
    String s4;
    String s5;
    String s6;
    String s7;
    String s8;
    String s9;
} values2BT;

struct NodeResponse{
    int nodeID;
    float value;
} RFnode, package;

float sensortemp;
float previoustemp = -150.00;
float maxtemp = -2000.00;
float mintemp = 2000.00;
float errortemp = -127.00;


String BTjson = "{\"S1\":\""+values2BT.s1+
"\",\"S2\":\""+values2BT.s2+
"\",\"S3\":\""+values2BT.s3+
"\",\"S4\":\""+values2BT.s4+
"\",\"S5\":\""+values2BT.s5+
"\",\"S6\":\""+values2BT.s6+
"\",\"S7\":\""+values2BT.s7+
"\",\"S8\":\""+values2BT.s8+
"\",\"S9\":\""+values2BT.s9+
"\"}";


/*Functions*/
void PingPongG1(){
    Radio.setChannel(Group1);
    int currentRadioID;
    bool ping = true;

    for(currentRadioID = 1; currentRadioID < 4; currentRadioID++){
        Radio.openWritingPipe(addresses[currentRadioID]);
        Radio.openReadingPipe(currentRadioID, addresses[0]);
        Radio.stopListening();
        delay(50);

        if(!Radio.write(&ping, sizeof(ping))){
            Serial.println("failed reach radioID: "+String(currentRadioID+1));
        }

        Radio.txStandBy();
        Radio.startListening();
        unsigned long started_waiting_at = micros();
        bool timeout = false;

        while(!Radio.available()){
            if(micros() - started_waiting_at > 200000){
                timeout = true;
                break;
            }
        }

        if(timeout){
            Serial.println("failed, request timeout");
        }
        else{
            Radio.read(&RFnode, sizeof(RFnode));
            switch(RFnode.nodeID){
                case 2:
                    values.s2 = RFnode.value;
                    break;
                case 3:
                    values.s3 = RFnode.value;
                    break;
                case 4:
                    values.s4 = RFnode.value;
                    break;
                case 5:
                    values.s5 = RFnode.value;
                    break;
                default:
                    Serial.println("RFnode value error");
                    break;
            }
        }

    }

}

void PongPingG1(NodeResponse package){
    Radio.setChannel(Group1);

    switch(radioID){
        case 2:
            Radio.openReadingPipe(1,addresses[1]);
            Radio.openWritingPipe(addresses[0]);
            Serial.println("radioID 2, pipes open!");
            break;

        case 3:
            Radio.openReadingPipe(2, addresses[2]);
            Radio.openWritingPipe(addresses[0]);
            Serial.println("radioID 3, pipes open!");
            break;

        case 4:
            Radio.openReadingPipe(3,addresses[3]);
            Radio.openWritingPipe(addresses[0]);
            Serial.println("radioID 4, pipes open!");
            break;

        case 5:
            Radio.openReadingPipe(4,addresses[4]);
            Radio.openWritingPipe(addresses[0]);
            Serial.println("radioID 5, pipes open!");
            break;

        default:
            Serial.println("radioID error");
            break;
    }

    if(Radio.available()){
        bool ping = false;

        while(Radio.available()){
            Radio.read(&ping, sizeof(ping));

        }
        if(ping == true){
            Radio.stopListening();
            Radio.write(&package, sizeof(package));
            Radio.txStandBy();
            Radio.startListening();
        }
        else{
            Serial.println("ping failed!");
        }

    }

}

void PingPongG2(){
    Radio.setChannel(Group2);
    int currentRadioID;
    bool ping = true;

    for(currentRadioID = 5; currentRadioID < 9; currentRadioID++){
        Radio.openWritingPipe(addresses[currentRadioID]);
        Radio.openReadingPipe(currentRadioID-4, addresses[0]);
        Radio.stopListening();
        delay(50);

        if(!Radio.write(&ping, sizeof(ping))){
            Serial.println("failed reach radioID: "+String(currentRadioID+1));
        }

        Radio.txStandBy();
        Radio.startListening();
        unsigned long started_waiting_at = micros();
        bool timeout = false;

        while(!Radio.available()){
            if(micros() - started_waiting_at > 200000){
                timeout = true;
                break;
            }
        }

        if(timeout){
            Serial.println("failed, request timeout");
        }
        else{
            Radio.read(&RFnode, sizeof(RFnode));
            switch(RFnode.nodeID){
                case 6:
                    values.s6 = RFnode.value;
                    break;
                case 7:
                    values.s7 = RFnode.value;
                    break;
                case 8:
                    values.s8 = RFnode.value;
                    break;
                case 9:
                    values.s9 = RFnode.value;
                    break;
                default:
                    Serial.println("RFnode value error");
                    break;
            }
        }

    }

}

void PongPingG2(NodeResponse package){
    Radio.setChannel(Group2);

    switch(radioID){
        case 6:
            Radio.openReadingPipe(1,addresses[5]);
            Radio.openWritingPipe(addresses[0]);
            Serial.println("radioID 6, pipes open!");
            break;

        case 7:
            Radio.openReadingPipe(2, addresses[6]);
            Radio.openWritingPipe(addresses[0]);
            Serial.println("radioID 7, pipes open!");
            break;

        case 8:
            Radio.openReadingPipe(3,addresses[7]);
            Radio.openWritingPipe(addresses[0]);
            Serial.println("radioID 8, pipes open!");
            break;

        case 9:
            Radio.openReadingPipe(4,addresses[8]);
            Radio.openWritingPipe(addresses[0]);
            Serial.println("radioID 9, pipes open!");
            break;

        default:
            Serial.println("radioID error");
            break;
    }

    if(Radio.available()){
        bool ping = false;

        while(Radio.available()){
            Radio.read(&ping, sizeof(ping));

        }
        if(ping == true){
            Radio.stopListening();
            Radio.write(&package, sizeof(package));
            Radio.txStandBy();
            Radio.startListening();
        }
        else{
            Serial.println("ping failed!");
        }

    }

}

NodeResponse makepackage(float value){
    package.nodeID = radioID;
    package.value = value;

    return package;
}

void updateBTjson(){
    values2BT.s1 = String(values.s1);
    values2BT.s2 = String(values.s2);
    values2BT.s3 = String(values.s3);
    values2BT.s4 = String(values.s4);
    values2BT.s5 = String(values.s5);
    values2BT.s6 = String(values.s6);
    values2BT.s7 = String(values.s7);
    values2BT.s8 = String(values.s8);
    values2BT.s9 = String(values.s9);

    BTjson = "{\"S1\":\""+values2BT.s1+
    "\",\"S2\":\""+values2BT.s2+
    "\",\"S3\":\""+values2BT.s3+
    "\",\"S4\":\""+values2BT.s4+
    "\",\"S5\":\""+values2BT.s5+
    "\",\"S6\":\""+values2BT.s6+
    "\",\"S7\":\""+values2BT.s7+
    "\",\"S8\":\""+values2BT.s8+
    "\",\"S9\":\""+values2BT.s9+
    "\"}";

}

void screenprint(String text, int color, int x, int y, int size){
    tft.setCursor(x,y);
    tft.setTextSize(size);
    tft.setTextColor(color);
    tft.println(text);
}

void startscreen(){
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(ILI9341_BLACK);

    /*Base GUI*/
    tft.drawFastHLine(0,75, 240, ILI9341_WHITE);
    screenprint("Unit:", ILI9341_DARKGREY, 10, 90,2);
    tft.drawCircle(80,90,3, ILI9341_CYAN);
    screenprint("C", ILI9341_CYAN, 90, 90, 2);
    tft.drawFastVLine(120, 75, 40, ILI9341_WHITE);
    screenprint("ID:", ILI9341_DARKGREY ,130, 90, 2);
    screenprint("S"+String(radioID), ILI9341_CYAN, 175, 90, 2);
    tft.drawFastHLine(0,115, 240, ILI9341_WHITE);
    screenprint("TEMPERATURE", ILI9341_DARKGREY, 20, 130, 3);
    screenprint("--.--", ILI9341_WHITE, 50, 180, 4);
    tft.drawFastHLine(0,240, 240, ILI9341_WHITE);
    screenprint("MAX", ILI9341_RED, 37, 260, 2);
    screenprint("--.--", ILI9341_WHITE, 22, 290, 2);
    tft.drawFastVLine(120,240,80, ILI9341_WHITE);
    screenprint("MIN", ILI9341_BLUE, 173, 260, 2);
    screenprint("--.--", ILI9341_WHITE, 158, 290, 2);

}


/*MCU Functions*/
void setup() {
    Serial.begin(9600);
    BTserial.begin(9600);
    sensors.begin();
    Radio.begin();
    Radio.setPALevel(RF24_PA_HIGH);
    Radio.setDataRate(RF24_250KBPS);
    Radio.startListening();
    startscreen();
}

void loop() {
    sensors.requestTemperatures();
    sensortemp = sensors.getTempCByIndex(0);
    values.s1 = sensortemp;
    Serial.println("CURRENT TEMP:"+String(sensortemp));

    if(radioID == 1){
        PingPongG1();
        delay(100);
        PingPongG2();
        updateBTjson();
        Serial.println(BTjson);
        BTserial.println(BTjson);
        delay(400);
    }
    else{
        if(radioID > 1 && radioID < 6){
            RFnode = makepackage(sensortemp);
            PongPingG1(RFnode);
        }
        else if(radioID > 5 && radioID < 10){
            RFnode = makepackage(sensortemp);
            PongPingG2(RFnode);
        }
        else{
            Serial.println("Sent Error");
        }
        delay(200);
    }

    if (sensortemp == errortemp) {
      tft.fillRect(50,175,150,40,ILI9341_BLACK);
      screenprint("ERROR", ILI9341_WHITE,57,180,4);
    }
    else{

      if (sensortemp != previoustemp) {
        previoustemp = sensortemp;

        tft.fillRect(50,175,150,40,ILI9341_BLACK);
        screenprint(String(sensortemp), ILI9341_WHITE,57,180,4);
      }

      if(sensortemp > maxtemp){
        maxtemp = sensortemp;

        tft.fillRect(8,280,90,28,ILI9341_BLACK);
        screenprint(String(sensortemp), ILI9341_WHITE,15,290,2);
      }

      if(sensortemp < mintemp){
        mintemp = sensortemp;

        tft.fillRect(145,280,90,28,ILI9341_BLACK);
        screenprint(String(sensortemp), ILI9341_WHITE,152,290,2);
      }

    }

}
