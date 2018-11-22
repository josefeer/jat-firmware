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
const int radioID = 3;
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
struct temperatureSensors{
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

struct NodeResponse{
    int nodeID;
    float value;
} RFnode, package;

float sensortemp;
float previoustemp = -150.00;
float maxtemp = -2000.00;
float mintemp = 2000.00;
float errortemp = -127.00;

String BTjson;

/*Functions*/
void pipesSetup(){

    Radio.openWritingPipe(addresses[0]);
    Serial.println("JAT-"+String(radioID)+": pipes open");

}

void ReceiveRFData(){
    
    int currentRadioID;

    Radio.setChannel(Group1);
    delay(50);

    for(currentRadioID = 1; currentRadioID < 4; currentRadioID++){

        Radio.openReadingPipe(currentRadioID, addresses[0]);
        delay(100);
        
        if(Radio.available()){
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

void SendRFData(NodeResponse package){

    Radio.write(&package, sizeof(package));
    Radio.txStandBy();
    Serial.println("JAT-"+String(radioID)+": Package Send");

}

NodeResponse makepackage(float value){
    package.nodeID = radioID;
    package.value = value;

    return package;
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

void updatescreen(){

    if (sensortemp == errortemp) {

        if (previoustemp != errortemp){
            previoustemp = sensortemp;

            tft.fillRect(50,175,150,40,ILI9341_BLACK);
            screenprint("ERROR", ILI9341_WHITE,57,180,4);

        }

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

void updateBTjson(){

    BTjson = "{\"S1\":\""+String(values.s1)+
    "\",\"S2\":\""+String(values.s2)+
    "\",\"S3\":\""+String(values.s3)+
    "\",\"S4\":\""+String(values.s4)+
    "\",\"S5\":\""+String(values.s5)+
    "\",\"S6\":\""+String(values.s6)+
    "\",\"S7\":\""+String(values.s7)+
    "\",\"S8\":\""+String(values.s8)+
    "\",\"S9\":\""+String(values.s9)+
    "\"}";

}


/*MCU Functions*/
void setup() {

    Serial.begin(9600);
    sensors.begin();
    Radio.begin();
    Radio.setPALevel(RF24_PA_HIGH);
    Radio.setDataRate(RF24_250KBPS);

    if(radioID == 1){

        BTserial.begin(9600);
        Radio.startListening();

    }
    else if(radioID >1 && radioID < 6){

        Radio.setChannel(Group1);
        Radio.stopListening();
        pipesSetup();

    }
    else {

        Serial.println("********** radioID error **********");

    }

    startscreen();

}

void loop() {

    sensors.requestTemperatures();
    sensortemp = sensors.getTempCByIndex(0);
    Serial.println("JAT-"+String(radioID)+": "+String(sensortemp));

    if(radioID == 1){

        values.s1 = sensortemp;
        ReceiveRFData();
        updateBTjson();
        Serial.println(BTjson);
        BTserial.println(BTjson);

    }
    else if(radioID > 1 && radioID < 6){

        RFnode = makepackage(sensortemp);
        SendRFData(RFnode);

    }
    else{

        Serial.println("********** radioID error **********");

    }

    updatescreen();

}
