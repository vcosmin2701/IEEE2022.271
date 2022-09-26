#include <Arduino.h>
#include <ModbusRTU.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <SoftwareSerial.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

// receivePin, transmitPin, inverse_logic, bufSize, isrBufSize
SoftwareSerial S(D1, D4);
ModbusRTU mb;

AsyncWebServer server(80);
AsyncEventSource events("/events");

const int analogInPin = A0; // ESP8266 Analog Pin ADC0 = A0

// parameteri declarati ca variabile globale
unsigned long lastTime = 0;
unsigned long timerDelay = 120000; // send readings timer

uint16_t databits, stopBits; // for future development
uint16_t functionCode;       // for future development
uint16_t coilCount;          // for future development
char parity;                 // for future development

uint16_t baud_rate = 9600;
uint16_t serverAdress = 1;
uint16_t startAdress = 1;
uint16_t bitCount = 1;
uint16_t firstReg = 0;
uint16_t regCount = 1;

float voltage = 0; // value read

uint16_t mb_response;      // store the modbus server response
uint16_t transaction_code; // store the code that indicates the transaction status

// // wireless connection
const char *ssid = "Controller ModbusRS485";
const char *password = "";


#endif

void wifi_start();   // start the wifi access point
void server_start(); // start the web server ierface

void server_settings(); // process the Settings tab requests

void send_modbus_readCoil(uint16_t startAdress, uint16_t coilCount); // lisen to modbus server response and process response
void send_modbus_readHolding(uint16_t firstReg, uint16_t regCount);

void notFound(AsyncWebServerRequest *request); // in case the web page requested is not found

void adc_read(); // read the battery voltage

void modbus_start();                                                   // start the modbus connection
bool cb(Modbus::ResultCode event, uint16_t transactionId, void *data); // print in serial command the modbus errors events,
                                                                       // to be future defined

String processor(const String &var); // convert values from int to string to be sent to web interface
                                     // where only strings are accepted

void server_readCoils();   // process the readCoils function tab request
void server_readHolding(); // process the readHolding function tab request

void wifi_start()
{
    WiFi.softAP(ssid,password);
    IPAddress IP = WiFi.softAPIP();

    Serial.print("AP IP address: ");
    Serial.println(IP);
}

void server_start()
{
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/interfata_grafica.html",
                              String(), false, processor); });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/style.css"); });

    server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/scripts.js"); });
}

// settings?serverAdress=1&baud_rate=9600&databits=8&parity=even&stopBits=1
void server_settings()
{
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
              {   
                // atribuire valorile parametrilor 
                serverAdress = request->getParam(0)->value().toInt();
                baud_rate = request->getParam(1)->value().toInt();
                databits = request->getParam(2)->value().toInt();
                stopBits = request->getParam(4)->value().toInt();

                Serial.println("serverAdress:"); // print in serial command for debug reasons
                Serial.println(serverAdress);
                Serial.println("baudrate:");
                Serial.println(baud_rate);
                                             
                // reincarcare interfata web
                request->send(LittleFS, "/interfata_grafica.html", String(), false, processor); });
}

void server_readCoils()
{
    // readCoils?functionCode=1&startAdress=0&coilCount=1
    server.on("/readCoils", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              functionCode = request->getParam(0)->value().toInt();
              startAdress = request->getParam(1)->value().toInt();
              coilCount = request->getParam(2)->value().toInt();
            
              Serial.println("startAdress"); // print in serial command for debug reasons
              Serial.println(startAdress);
              Serial.println("coilCount");
              Serial.println(coilCount);

              send_modbus_readCoil(startAdress,coilCount);

              request->send(LittleFS, "/interfata_grafica.html", 
              String(), false, processor); });
}

void server_readHolding()
{
    // readHolding?functionCode=3&firstReg=2&regCount=4
    server.on("/readHolding", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              functionCode = request->getParam(0)->value().toInt();
              firstReg = request->getParam(1)->value().toInt();
              regCount = request->getParam(2)->value().toInt();

              send_modbus_readHolding(firstReg, regCount);

              request->send(LittleFS, "/interfata_grafica.html", String(), false, processor); });
}

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void adc_read()
{
    // read the analog in value * voltage divider constant
    voltage = ((analogRead(analogInPin) * 3.3) / 1023.0) / 0.232325581395348;
}

void modbus_start()
{
    S.setTransmitEnablePin(D0);       // set D0 as Drive enable Receive Enable
    S.begin(baud_rate, SWSERIAL_8E1); // set baudrate and serial paramteres
    mb.begin(&S);                     // start modbus conextion

    mb.master(); // set uC as modbus master
}

bool cb(Modbus::ResultCode event, uint16_t transactionId, void *data)
{ // Callback to monitor errors
    transaction_code = event;
    switch (event)
    {
    case Modbus::EX_SUCCESS: // no modbus error
        Serial.println("\n");
        Serial.print("Transaction code: 0x");
        Serial.print(event, HEX);
        Serial.print(" -> Transaction successful");
        break;
    case Modbus::EX_TIMEOUT: // response not arrived in the expected time
        Serial.println("\n");
        Serial.print("Transaction code: 0x");
        Serial.print(event, HEX);
        Serial.print(" -> Response timeout expired");
        break;
    case Modbus::EX_ILLEGAL_ADDRESS: // requested address not in accessible range
        Serial.println("\n");
        Serial.print("Transaction code: 0x");
        Serial.print(event, HEX);
        Serial.print(" -> Output Address not in Range");
        break;
    case Modbus::EX_ILLEGAL_VALUE:
        Serial.println("\n");
        Serial.print("Transaction code: 0x");
        Serial.print(event, HEX);
        Serial.print(" -> Output Value not in Range");
        break;
    case Modbus::EX_SLAVE_FAILURE:
        Serial.println("\n");
        Serial.print("Transaction code: 0x");
        Serial.print(event, HEX);
        Serial.print(" -> Slave or Master Device Fails to process request");
        break;
    default:
        break;
    }
    return true;
}

void send_modbus_readCoil(uint16_t startAdress, uint16_t coilCount)
{
    bool coils[coilCount];

    if (!mb.slave())
    { // send modbus function code $01
        mb.readCoil(serverAdress, firstReg, coils, coilCount, cb);

        while (mb.slave())
        { // Check if transaction is active
            mb.task();
            delay(10);
        }
        Serial.println("\n");
        Serial.printf("Request response = %.2d \n", coils[0]);
        Serial.println();

        mb_response = coils[0]; // store modbus response value

        if (transaction_code != 0)
        { // in case of error set ansver as invalid
            events.send("NaN", "mb_response", millis());
        }
        else
        { // sent to frontend the modbus response value
            events.send(String(mb_response).c_str(), "mb_response", millis());
            // resert the value
            coils[0] = 0;
        }
    }
}

void send_modbus_readHolding(uint16_t firstReg, uint16_t regCount)
{
    uint16_t res[regCount];
    if (!mb.slave())
    {                                                           // send modbus function code $03
        mb.readHreg(serverAdress, firstReg, res, regCount, cb); // Send Read Hreg from Modbus Server
        // Check if no transaction in progress
        while (mb.slave())
        { // Check if transaction is active
            mb.task();
            delay(10);
        }
        Serial.println("\n");
        Serial.printf("Request response = %.2d \n", res[0]);
        Serial.println();

        mb_response = res[0];

        if (transaction_code != 0)
        { // sent to frontend the modbus response value
            events.send("NaN", "mb_response", millis());
        }
        else
        {
            events.send(String(mb_response).c_str(), "mb_response", millis());
            res[0] = 0;
        }
    }
}

String processor(const String &var)
{
    adc_read();

    if (var == "VOLTAGE")
    {
        return String(voltage);
    }
    // for future development
    // if (var == "TRANSACTION_CODE")
    // {
    //     return String(transaction_code);
    // }
    // return String();

    if (var == "MODBUS_RESPONSE")
    {
        return String(mb_response);
    }
    return String();
}
