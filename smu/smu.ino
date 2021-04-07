#include <stdlib.h>
#include <SPI.h>
#include <EEPROM.h>
#include "TeraLTC2449.h"
#include "TeraAD5664.h"
#include "TeraDigitalOutput.h"
#include "tera_EEPROM.h"

#define ESP32

#ifdef ESP32
    #include <BluetoothSerial.h> // Bluetooth ESP32
    #include <WiFi.h>
#else
    #include <SoftwareSerial.h> // import the serial library
#endif


//board name
#ifdef ESP32
    #define deviceID "TeraDAQ_8ch(Diff)"
    #define deviceVer "1.0" // ESP32
#elif
    #define deviceID "TeraDAQ_Nano"
    #define deviceVer "1.0" // arduino NANO
#endif


#ifdef ESP32
  //board pin number
    #define MOSI 23
    #define MISO 19
    #define CLK 18
    #define ADC_CS 14
    #define DAC_CS 5
    #define DO_CS 25
    #define DO_MR 33
    #define DO_OE 32
    #define shiftregisterboards 5

    //uninitalised pointers to SPI objects
    SPIClass * SPI3 = new SPIClass(VSPI);

#else
  //board pin number
    #define MOSI 11
    #define MISO 16
    #define CLK 13
    #define CS 10

    //uninitalised pointers to SPI objects
    SPIClass * SPI3 = &SPI;
#endif

//EEPROM Address
#define SsidAddress 200
#define PasswordAddress 220
#define WifiConAddress 240
#define SsidLengthAddress 260
#define PwLengthAddress 264

// #define SINGLE_ENDED
#ifdef SINGLE_ENDED
    LTC2449_SINGLE ADC(SPI3, CLK, MISO, MOSI, ADC_CS);
    #define ChannelLength 16
#else
    LTC2449 ADC(SPI3, CLK, MISO, MOSI, ADC_CS);
    #define ChannelLength 8
#endif


AD5664 DAC(SPI3, CLK, MISO, MOSI, DAC_CS);
DigitalOutput DO(SPI3, CLK, MISO, MOSI, DO_CS, DO_MR, DO_OE, shiftregisterboards);

#define VMODE 0;
#define IMODE 1;

class SMU
{
    protected:
        int SMU_ch;
        int DO_ch_En;
        int DO_ch_VImode;
        int DO_ch_1A;
        int DO_ch_100m;
        int DO_ch_10m;
        int DO_ch_1m;
        int DO_ch_100u;
        int DO_ch_10u;
        int DO_ch_1u;
        int DO_ch_Active;
        bool En;
        bool VI_mode;
        byte range;

        int DAC_ch;
        float OutputVoltage;
        
        int ADC_V_ch;
        int ADC_I_ch;
        float InputVoltage;
        float InputCurrent;

    public:
        SMU(int in_SMU_ch);
        void Init();
        void SetEn(bool value);
        bool GetEn();
        void SetMode(bool value);
        bool GetMode();
        void SetRange(byte input_range);
        byte GetRange();

        void SetOutputVoltage(float SetVoltage);
        float GetOutputVoltage();

        float GetInputVoltage();
        float GetInputCurrent();
};

SMU smu1(0); 
SMU smu2(1);
SMU smu3(2);
SMU smu4(3);
SMU * smuArray[4] = {&smu1, &smu2, &smu3, &smu4};

//functions
void ReadingSetupData ();
void monitor(Stream * Uart);
void scanWifi(int SerialCh);
void StartWifi(int SerialCh);
void StopWifi();
void checkSerial(int Max_SerialCh);
void serialEvent(int SerialCh);
void serialCommand(int SerialCh);
void clearSerialBuffer(int SerialCh);
void report(Stream *Uart);

// void notFound(AsyncWebServerRequest *request);
String test();

String inputString_Serial0 = ""; //Serial input string
#ifdef ESP32
    //variable for BT
    BluetoothSerial ESP_BT;
    String inputString_Serial1 = ""; //Serial input string
    String inputString_Serial2 = ""; //Serial input string
    #define SerialChannel 3
    Stream *Aserial[SerialChannel] = {&Serial,&ESP_BT,&Serial2};

    bool stringCompleteArray[SerialChannel];

#else
    #define SerialChannel 1
    Stream *Aserial[SerialChannel] = {&Serial};
#endif

//WIFI settings
char* ssid     = "teraleader";
char* password = "hansk2491";
char tempSSID[20];
char tempPASSWORD[20];

String SSID_str = "";
String PW_str = "";
int ssidLength;
int PwLength;
bool WifiCon = false;

//HTTP server settings
WiFiServer server(80);
// AsyncWebServer server(80);
String header;

// static const int spiClk = 1000000; // 1 MHz
static float LTC2449_lsb = 9.3132258E-9; 
static int32_t LTC2449_offset_code = 5905;
// static int32_t LTC2449_offset_code = 0;
float result[ChannelLength];

//test code
uint16_t tempdata = 0;
byte tempdata_do = 0;
bool temp[8];
// test variable
byte count = 0;

void setup() {
    for (int i = 0; i < SerialChannel; i++){
        stringCompleteArray[i] = false;
    }
    #ifdef ESP32
        EEPROM.begin(4096);
        ESP_BT.begin(deviceID);
        Serial2.begin(115200);
    #endif
    Serial.begin(115200);
    ADC.Init();
    DAC.Init();
    DO.Init();
    for (int i = 0; i< 8; i++){
        temp[i] = false;
    }
    delay(1);
    Serial.begin(115200);
    ADC.Init();
    DAC.Init();
    DO.Init();
    // ReadingSetupData ();
    delay(10);
    #ifdef ESP32
        SPI3->begin(CLK, MISO, MOSI, ADC_CS);
    #else
        SPI3->begin();
    #endif

    smu1.Init();
    smu2.Init();
    smu3.Init();
    smu4.Init();

    delay(1);
    // set_report(Aserial[0]);
    // set_report(Aserial[2]);
}

void loop() {
    checkSerial(SerialChannel); // serial을 읽어냄
    ADC.Refresh(LTC2449_lsb, LTC2449_offset_code, result);
    DO.Shiftregister_write();
    set_report(Aserial[0]);
    set_report(Aserial[2]);

    delay(100);
}

void set_report(Stream *Uart) {
    for (int i = 0 ; i < 4; i++){
        Uart->print("SMU");Uart->print(i+1);Uart->print(":");
        Uart->print(smuArray[i]->GetEn());Uart->print(",");
        Uart->print(smuArray[i]->GetMode());Uart->print(",");
        Uart->print(smuArray[i]->GetRange());Uart->print(",");
        Uart->print(smuArray[i]->GetOutputVoltage(),4);Uart->print(",");
        Uart->print(smuArray[i]->GetInputVoltage(),7);Uart->print(",");
        Uart->print(smuArray[i]->GetInputCurrent(),7);Uart->print(";");  
    }
    Uart->println(); 
}

SMU *c;
int current_GenEn = c->GetEn();
int current_GetMode = c->GetMode();
int current_GetRange = c->GetRange();

void report(Stream *Uart){
    for (int i = 0 ; i < 4; i++){
        if(current_GenEn != c->GetEn() || current_GetMode != c->GetMode() || current_GetRange != c->GetRange()) {
            Uart->print("SMU");Uart->print(i+1);Uart->print(":");
            Uart->print(smuArray[i]->GetEn());Uart->print(",");
            Uart->print(smuArray[i]->GetMode());Uart->print(",");
            Uart->print(smuArray[i]->GetRange());Uart->print(",");
            Uart->print(smuArray[i]->GetOutputVoltage(),4);Uart->print(",");
            Uart->print(smuArray[i]->GetInputVoltage(),7);Uart->print(",");
            Uart->print(smuArray[i]->GetInputCurrent(),7);Uart->print(";");  
        }
        else {
            Uart->print(smuArray[i]->GetOutputVoltage(),4);Uart->print(",");
            Uart->print(smuArray[i]->GetInputVoltage(),7);Uart->print(",");
            Uart->print(smuArray[i]->GetInputCurrent(),7);Uart->print(";");  
        }
    }
    Uart->println(); 
}
void monitor(Stream * Uart){
    Uart->print("data: ");
    Uart->print(tempdata_do,BIN);
    for ( int i =0; i< 8; i++){
        Uart->print(" ");
        Uart->print(temp[i]);
    }

    Uart->print("Output: "); Uart->print(tempdata);Uart->print(" ");
    Uart->print("result: ");
        for (int i = 0 ; i< ChannelLength; i++){
            Uart->print(result[i]);
            Uart->print(" ");
        }
    Uart->println(); 
}
void scanWifi(int SerialCh){
    Aserial[SerialCh]->println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Aserial[SerialCh]->println("scan done");
    if (n == 0) {
        Aserial[SerialCh]->println("no networks found");
    }
    else {
        Aserial[SerialCh]->print(n);
        Aserial[SerialCh]->println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Aserial[SerialCh]->print(i + 1);
            Aserial[SerialCh]->print(": ");
            Aserial[SerialCh]->print(WiFi.SSID(i));
            Aserial[SerialCh]->print(" (");
            Aserial[SerialCh]->print(WiFi.RSSI(i));
            Aserial[SerialCh]->print(")");
            Aserial[SerialCh]->println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    Aserial[SerialCh]->println("");

    // Wait a bit before scanning again
    delay(5000);
}

void StartWifi(int SerialCh){
    // Connect to Wi-Fi network with SSID and password
    Aserial[SerialCh]->print("Connecting to ");
    Aserial[SerialCh]->println(ssid);
    WiFi.begin(ssid, password);
    unsigned long startTime = millis();
    unsigned long currentTime = millis();
    int count = 0;
    bool WIFI_Enable = true;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Aserial[SerialCh]->print(".");
        currentTime = millis();
        if (count++ == 9){
            count = 0;
            unsigned long time = (currentTime - startTime)/1000;
            Aserial[SerialCh]->print(time);Serial.println(" sec");
        }
        if (( currentTime - startTime ) > 600000){
            WIFI_Enable = false;
            Aserial[SerialCh]->println();
            Aserial[SerialCh]->println("Can not connect WIFI!");
            break;
        }
    }
    // Print local IP address and start web server
    if ( WIFI_Enable){
        WifiCon = true;
        writeString_EEPROM(WifiConAddress, ssid);
        EEPROM.commit();
        Aserial[SerialCh]->println();
        Aserial[SerialCh]->print("Time: ");Aserial[SerialCh]->print((currentTime - startTime)/1000);Aserial[SerialCh]->println(" sec");
        Aserial[SerialCh]->println("");
        Aserial[SerialCh]->println("WiFi connected.");
        Aserial[SerialCh]->println("IP address: ");
        Aserial[SerialCh]->println(WiFi.localIP());
        server.begin();
        // StartServer(); //Async server start
    }

}

void StopWifi(){
    server.end();
    WiFi.disconnect();
}

void checkSerial(int Max_SerialCh){
    for (int i = 0; i < Max_SerialCh; i++){
        serialEvent(i);
    }
}

// 새 직렬데이터가 들어오면 문자열에 넣고 \n들어오면 프린트하고 초기화
void serialEvent(int SerialCh){
    while (Aserial[SerialCh]->available()) {
        Serial.println("avaiable : "+SerialCh);
        char inChar = (char)Aserial[SerialCh]->read();
        Serial.println("inChar : " + inChar);
        if (inChar == '\n') {
            stringCompleteArray[SerialCh] = true;
        }
        else {
            if ( SerialCh == 0){ inputString_Serial0 += inChar; Serial.print("inputString_Serial0 : "); Serial.println(inputString_Serial0);}
            else if ( SerialCh == 1){ inputString_Serial1 += inChar; Serial.print("inputString_Serial1 : "); Serial.println(inputString_Serial1);}
            else if ( SerialCh == 2){ inputString_Serial2 += inChar; Serial.print("inputString_Serial2 : "); Serial.println(inputString_Serial2);}
        }
    }
    if (stringCompleteArray[SerialCh]){
        serialCommand(SerialCh);
        clearSerialBuffer(SerialCh);
    }
}

void serialCommand(int SerialCh){
    String inputStringTemp = "";
    if ( SerialCh == 0){ inputStringTemp = inputString_Serial0; }
    else if ( SerialCh == 1){ inputStringTemp = inputString_Serial1; }
    else if ( SerialCh == 2){ inputStringTemp = inputString_Serial2; }

    byte length = inputStringTemp.length();
    int check_i;
    for (check_i = 0; check_i < length ; check_i++){
      if (inputStringTemp.substring(check_i,check_i+1) == " " ){
          break;
      }
    }
    String Trimed_command = inputStringTemp;
    Trimed_command = inputStringTemp.substring(0, check_i);
    Trimed_command.trim();
    String Trimed_arg = inputStringTemp;
    Trimed_arg = inputStringTemp.substring(check_i+1, length);
    Trimed_arg.trim();

    if ( Trimed_command == "*IDN?" ) {
      // delay(500);
      Aserial[SerialCh]->print(deviceID);
      Aserial[SerialCh]->print("_");
      Aserial[SerialCh]->println(deviceVer);
    }
    else if (Trimed_command == "SCAN"){
        scanWifi(SerialCh);
    }
    else if (Trimed_command == "CON"){
        StartWifi(SerialCh);
    }
    else if (Trimed_command == "DIS"){
        StopWifi();
    }
    else if (Trimed_command == "SSID"){
        // toCharArray(복사할 문자열 저장할 버퍼, 버퍼크기)
        Trimed_arg.toCharArray(tempSSID, Trimed_arg.length()+1);
        Aserial[SerialCh]->print("TrimedLength: ");Aserial[SerialCh]->println(Trimed_arg.length());
        ssid = tempSSID;
        int ssidlengthtemp = Trimed_arg.length();
        Aserial[SerialCh]->print("Length: ");Aserial[SerialCh]->println(ssidlengthtemp);
        // saveCharArray_eeprom(SsidAddress, ssid, ssidlengthtemp);
        writeString_EEPROM(SsidAddress, Trimed_arg);
        //EEPROM.commit();
        savefloat_eeprom(SsidLengthAddress, ssidlengthtemp);
        EEPROM.commit();
        Aserial[SerialCh]->println(ssid);
    }
    else if (Trimed_command == "PASSWD"){
        Trimed_arg.toCharArray(tempPASSWORD, Trimed_arg.length()+1);
        Aserial[SerialCh]->print("TrimedLength: ");Aserial[SerialCh]->println(Trimed_arg.length());
        password = tempPASSWORD;
        int pwlengthtemp = Trimed_arg.length();
        Aserial[SerialCh]->print("Length: ");Aserial[SerialCh]->println(pwlengthtemp);
        writeString_EEPROM(PasswordAddress, Trimed_arg);
        // saveCharArray_eeprom(PasswordAddress, password, pwlengthtemp);
        // EEPROM.commit();
        savefloat_eeprom(PwLengthAddress, pwlengthtemp);
        EEPROM.commit();
        Aserial[SerialCh]->println(password);
    }
    else if (Trimed_command == "ADCSPD"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        int tempvalue = atoi(tempdata); // 문자열을 정수로
        if ( tempvalue >=0 && tempvalue < 10){
            ADC.setSpeed(tempvalue);
            Aserial[0]->print("Speed: ");
            Aserial[0]->println(tempvalue);
        }
    }
    else if (Trimed_command == "ENA1"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata); // tempvalue: 값이 있으면 1, 값이 0이면 
        smu1.SetEn(tempvalue);
    }
    else if (Trimed_command == "ENA2"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata);
        smu2.SetEn(tempvalue);
    }
    else if (Trimed_command == "ENA3"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata);
        smu3.SetEn(tempvalue);
    }
    else if (Trimed_command == "ENA4"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata);
        smu4.SetEn(tempvalue);
    }
    else if (Trimed_command == "VIM1"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata);
        smu1.SetMode(tempvalue);
    }
    else if (Trimed_command == "VIM2"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata);
        smu2.SetMode(tempvalue);
    }
    else if (Trimed_command == "VIM3"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata);
        smu3.SetMode(tempvalue);
    }
    else if (Trimed_command == "VIM4"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata);
        smu4.SetMode(tempvalue);
    }
    else if (Trimed_command == "RAN1"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        byte tempvalue = atoi(tempdata);
        if ( tempvalue > 6){ tempvalue = 6; }
        else if ( tempvalue < 0){ tempvalue = 0; }
        smu1.SetRange(tempvalue);
    }
    else if (Trimed_command == "RAN2"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        byte tempvalue = atoi(tempdata);
        if ( tempvalue > 6){ tempvalue = 6; }
        else if ( tempvalue < 0){ tempvalue = 0; }
        smu2.SetRange(tempvalue);
    }
    else if (Trimed_command == "RAN3"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        byte tempvalue = atoi(tempdata);
        if ( tempvalue > 6){ tempvalue = 6; }
        else if ( tempvalue < 0){ tempvalue = 0; }
        smu3.SetRange(tempvalue);
    }
    else if (Trimed_command == "RAN4"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        byte tempvalue = atoi(tempdata);
        if ( tempvalue > 6){ tempvalue = 6; }
        else if ( tempvalue < 0){ tempvalue = 0; }
        smu4.SetRange(tempvalue);
    }
    else if (Trimed_command == "OUT1"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu1.SetOutputVoltage(tempvalue);
    }
    else if (Trimed_command == "OUT2"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu2.SetOutputVoltage(tempvalue);
    }
    else if (Trimed_command == "OUT3"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu3.SetOutputVoltage(tempvalue);
    }
    else if (Trimed_command == "OUT4"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu4.SetOutputVoltage(tempvalue);
    }
    else if (Trimed_command == "init") {
        inputStringTemp = "";
    }
}

void clearSerialBuffer(int SerialCh){
    Serial.println("clear");
    stringCompleteArray[SerialCh] = false;
    if ( SerialCh == 0){ inputString_Serial0 = ""; }
    else if ( SerialCh == 1){ inputString_Serial1 = ""; }
    else if ( SerialCh == 2){ inputString_Serial2 = ""; }
}

void ReadingSetupData (){
    ssidLength = (int) readfloat_eeprom(SsidLengthAddress);
    PwLength = (int) readfloat_eeprom(PwLengthAddress);
    if (ssidLength != 0 && ssidLength <= 20){ SSID_str = read_String_EEPROM(SsidAddress);}
    if (PwLength != 0 && PwLength <= 20 ){ PW_str = read_String_EEPROM(PasswordAddress);}
    if (read_String_EEPROM(WifiConAddress) == ssid){
        WifiCon = true;
    }
    // Serial.print("SSID: ");Serial.print(SSID_str);Serial.println();
    // Serial.print("P/W: ");Serial.print(PW_str);Serial.println();
    SSID_str.toCharArray(tempSSID, SSID_str.length()+1);
    PW_str.toCharArray(tempPASSWORD, PW_str.length()+1);
    ssid = tempSSID;
    password = tempPASSWORD;
}
/* 
void notFound(AsyncWebServerRequest *request) {

String test(){
    return String(count++);
}
/* 
void StartServer(){
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html");
    });

    server.on("/image/bg.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/image/bg.jpg");
    });

    server.on("/image/member.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/image/member.jpg");
    });

    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", test().c_str());
    });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", test().c_str());
    });
    server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", test().c_str());
    });

    server.onNotFound(notFound);
    server.begin();
} */

SMU::SMU(int in_SMU_ch){
        SMU_ch = in_SMU_ch;
        switch (SMU_ch){
            case 0:
                DO_ch_En=0;
                DO_ch_VImode=1;
                DO_ch_1A=8;
                DO_ch_100m=9;
                DO_ch_10m=10;
                DO_ch_1m=11;
                DO_ch_100u=12;
                DO_ch_10u=13;
                DO_ch_1u=14;
                DO_ch_Active=15;
                DAC_ch=1;
                ADC_V_ch=0;
                ADC_I_ch=1;
                break;
            case 1:
                DO_ch_En=2;
                DO_ch_VImode=3;
                DO_ch_1A=16;
                DO_ch_100m=17;
                DO_ch_10m=18;
                DO_ch_1m=19;
                DO_ch_100u=20;
                DO_ch_10u=21;
                DO_ch_1u=22;
                DO_ch_Active=23;
                DAC_ch=2;
                ADC_V_ch=2;
                ADC_I_ch=3;
                break;
            case 2:
                DO_ch_En=4;
                DO_ch_VImode=5;
                DO_ch_1A=24;
                DO_ch_100m=25;
                DO_ch_10m=26;
                DO_ch_1m=27;
                DO_ch_100u=28;
                DO_ch_10u=29;
                DO_ch_1u=30;
                DO_ch_Active=31;
                DAC_ch=3;
                ADC_V_ch=4;
                ADC_I_ch=5;
                break;
            case 3:
                DO_ch_En=6;
                DO_ch_VImode=7;
                DO_ch_1A=32;
                DO_ch_100m=33;
                DO_ch_10m=34;
                DO_ch_1m=35;
                DO_ch_100u=36;
                DO_ch_10u=37;
                DO_ch_1u=38;
                DO_ch_Active=39;
                DAC_ch=4;
                ADC_V_ch=6;
                ADC_I_ch=7;
                break;
        }
        En=false;
        VI_mode=VMODE;
        range=0;
        OutputVoltage=0;
        InputVoltage=0;
        InputCurrent=0;
}

// :: = 클래스의 멤버 함수 정의 할 때 사용 (외부클래스정의)
void SMU::Init(){
    DO.Set_DO(DO_ch_Active, HIGH); //Active High
    SetEn(false);
    SetMode(false);
    SetRange(0);
    SetOutputVoltage(0);
}

void SMU::SetEn(bool value){
    En = value;
    DO.Set_DO(DO_ch_En, En);
    // Serial.print("DO_");//test code
    // Serial.print(DO_ch_En);//test code
    // Serial.print(":");//test code
    // Serial.println(En);//test code
}

bool SMU::GetEn(){
    En = DO.Get_DO(DO_ch_En);
    return En;
}

void SMU::SetMode(bool value){
    VI_mode = value;
    DO.Set_DO(DO_ch_VImode, VI_mode);
}

bool SMU::GetMode(){
    VI_mode = DO.Get_DO(DO_ch_VImode);
    return VI_mode;
}

void SMU::SetRange(byte input_range){
    range = input_range;
    DO.Set_DO(DO_ch_1A, 0);
    delay(1);
    DO.Set_DO(DO_ch_100m, 0);
    delay(1);
    DO.Set_DO(DO_ch_10m, 0);
    delay(1);
    DO.Set_DO(DO_ch_1m, 0);
    delay(1);
    DO.Set_DO(DO_ch_100u, 0);
    delay(1);
    DO.Set_DO(DO_ch_10u, 0);
    delay(1);
    DO.Set_DO(DO_ch_1u, 0);
    delay(1);

    int ch_temp;
    switch (range){
        case 0:
            ch_temp = DO_ch_1A;
            break;
        case 1:
            ch_temp = DO_ch_100m;
            break;
        case 2:
            ch_temp = DO_ch_10m;
            break;
        case 3:
            ch_temp = DO_ch_1m;
            break;
        case 4:
            ch_temp = DO_ch_100u;
            break;
        case 5:
            ch_temp = DO_ch_10u;
            break;
        case 6:
            ch_temp = DO_ch_1u;
            break;

    }
    DO.Set_DO(ch_temp,HIGH);
}

byte SMU::GetRange(){
    return range;
}

void SMU::SetOutputVoltage(float SetVoltage){
    OutputVoltage = SetVoltage; //-10~ +10 V
    unsigned int tempvalue = (OutputVoltage + 10.0) / 20.0 * 65535;
    DAC.Write(DAC_ch,tempvalue);
}

float SMU::GetOutputVoltage(){
    return OutputVoltage;
}

float SMU::GetInputVoltage(){
    InputVoltage = result[ADC_V_ch];
    return InputVoltage;
}

float SMU::GetInputCurrent(){
    InputCurrent = result[ADC_I_ch];
    return InputCurrent;
}
