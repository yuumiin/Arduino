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
#define ch1_v_fx_m 300 
#define ch1_v_fx_b 800 
#define ch1_c_fx_m 1300 
#define ch1_c_fx_b 1800 
#define ch2_v_fx_m 2300 
#define ch2_v_fx_b 2800 
#define ch2_c_fx_m 3300 
#define ch2_c_fx_b 3800 
#define ch3_v_fx_m 4300 
#define ch3_v_fx_b 4800 
#define ch3_c_fx_m 5300 
#define ch3_c_fx_b 5800 
#define ch4_v_fx_m 6300 
#define ch4_v_fx_b 6800 
#define ch4_c_fx_m 7300 
#define ch4_c_fx_b 7800 
#define out_fx_m 8400
#define out_fx_b 8600

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

        unsigned int eepromAddrVM;
        unsigned int eepromAddrVB;
        unsigned int eepromAddrCM;
        unsigned int eepromAddrCB;

        int DAC_ch;
        float OutputVoltage, OutputVoltage2;
        
        int ADC_V_ch;
        int ADC_I_ch;

        float InputVoltage;
        float InputCurrent;

    public:
        String readBuffer;
        String sumBuffer;
         
        //inputCalibration
        double v_fx_m[500];
        double v_fx_b[500];
        double c_fx_m[500];
        double c_fx_b[500];
        double arrStand[100];
        double arrRawV[100]; 
        double arrRawC[100]; 
        int arrIdx;
        int v_cali_index;
        int c_cali_index;

        //outputCalibration
        String stringX;
        String stringY;
        float x_arr[100];
        float y_arr[100];
        double output_fx_m[100];
        double output_fx_b[100];
        int outputIdx;

        SMU(int in_SMU_ch);
        void Init();
        void SetEn(bool value);
        bool GetEn();
        void SetMode(bool value);
        bool GetMode();
        void SetRange(byte input_range);
        byte GetRange();
        void SetOutputVoltage(float SetVoltage, float printVoltage);
        float GetOutputVoltage();
        double GetInputVoltage(float printValue);
        double GetInputCurrent(float printValue);

        void Calibration(float stand);
        double GetCaliValue(bool type);
        
        void outputCalibration(String str);
        void GetoutputCalibration(float tempvalue);

        void SetAddr(unsigned int AddrVM,unsigned int AddrVB,unsigned int AddrCM,unsigned int AddrCB);
        void ReadEEPROM();


        double GetM ( double x1, double x2, double y1, double y2);
        double GetB ( double x1, double x2, double y1, double y2);

        void HandleInput(String inBuff);
        void FileCalibration(String voltage_m, String voltage_b, String current_m, String current_b);
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

// static const int spiClk = 1000000; // 1 MHz
static float LTC2449_lsb = 9.3132258E-9; 
static int32_t LTC2449_offset_code = 5905;
// static int32_t LTC2449_offset_code = 0;
float result[ChannelLength];

String test();

//test code
uint16_t tempdata = 0;
byte tempdata_do = 0;
bool temp[8];
// test variable
byte count = 0;

String inStr = "";

void setup() {
    for (int i = 0; i < SerialChannel; i++){
        stringCompleteArray[i] = false;
    }
    #ifdef ESP32
        EEPROM.begin(8096);
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

    #ifdef ESP32
        SPI3->begin(CLK, MISO, MOSI, ADC_CS);
    #else
        SPI3->begin();
    #endif

    smu1.Init();
    smu2.Init();
    smu3.Init();
    smu4.Init();
    smu1.SetAddr( ch1_v_fx_m,ch1_v_fx_b,ch1_c_fx_m,ch1_c_fx_b);
    smu2.SetAddr( ch2_v_fx_m,ch2_v_fx_b,ch2_c_fx_m,ch2_c_fx_b);
    smu3.SetAddr( ch3_v_fx_m,ch3_v_fx_b,ch3_c_fx_m,ch3_c_fx_b);
    smu4.SetAddr( ch4_v_fx_m,ch4_v_fx_b,ch4_c_fx_m,ch4_c_fx_b);
    smu1.ReadEEPROM();
    smu2.ReadEEPROM();
    smu3.ReadEEPROM();
    smu4.ReadEEPROM();

    // Aserial[0]->println("Start");
    // Aserial[2]->println("Start");
    Serial.println("made by Leeyoumin 21.03.16");
    Serial.println("start");
}

void loop() {
    checkSerial(SerialChannel);
    ADC.Refresh(LTC2449_lsb, LTC2449_offset_code, result);
    DO.Shiftregister_write();
    // report(Aserial[2]);
    report(Aserial[0]);
    delay(1000);
}

 void report(Stream *Uart){
    for (int i = 0 ; i < 4; i++){
        Uart->print("SMU");Uart->print(i+1);Uart->print(":");
        Uart->print(smuArray[i]->GetEn());Uart->print(",");
        Uart->print(smuArray[i]->GetMode());Uart->print(",");
        Uart->print(smuArray[i]->GetRange());Uart->print(",");
        Uart->print(smuArray[i]->GetOutputVoltage(), 4);Uart->print(",");
        Uart->print(smuArray[i]->GetCaliValue(true), 7);Uart->print(",");
        Uart->print(smuArray[i]->GetCaliValue(false), 7);Uart->print(";");
    }
    Uart->println(); 
}

void checkSerial(int Max_SerialCh){
    for (int i = 0; i < Max_SerialCh; i++){
        serialEvent(i);
    }
}

void serialEvent(int SerialCh){
    while (Aserial[SerialCh]->available()) {
        char inChar = (char)Aserial[SerialCh]->read();
        if (inChar == '\n') {
            stringCompleteArray[SerialCh] = true;
        }
        else {
            if ( SerialCh == 0){ 
                inputString_Serial0 += inChar; 
                inStr += inChar;
            }
            else if ( SerialCh == 1){ inputString_Serial1 += inChar; }
            else if ( SerialCh == 2){ inputString_Serial2 += inChar; } 
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
      if (    inputStringTemp.substring(check_i,check_i+1) == " " ){
          break;
      }
    }
    String Trimed_command = inputStringTemp;
    Trimed_command = inputStringTemp.substring(0, check_i);
    Trimed_command.trim();
    String Trimed_arg = inputStringTemp;
    Trimed_arg = inputStringTemp.substring(check_i+1, length);
    Trimed_arg.trim();

    if (Trimed_command == "ENA1"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        bool tempvalue = atoi(tempdata);
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
    else if (Trimed_command == "OUT1") {
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu1.SetOutputVoltage(tempvalue,tempvalue);
    }
    else if (Trimed_command == "OUT2"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu2.SetOutputVoltage(tempvalue, tempvalue);
    }
    else if (Trimed_command == "OUT3"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu3.SetOutputVoltage(tempvalue, tempvalue);
    }
    else if (Trimed_command == "OUT4"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu4.SetOutputVoltage(tempvalue, tempvalue);
    }
    else if (Trimed_command == "DAQ1"){
        smu1.outputCalibration(Trimed_arg);
    }
    else if (Trimed_command == "DAQ2"){
        smu2.outputCalibration(Trimed_arg);
    }
    else if (Trimed_command == "DAQ3"){
        smu3.outputCalibration(Trimed_arg);
    }
    else if (Trimed_command == "DAQ4"){
        smu4.outputCalibration(Trimed_arg);
    }
    else if (Trimed_command == "CAL1"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu1.GetoutputCalibration(tempvalue);
    }
    else if (Trimed_command == "CAL2"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu2.GetoutputCalibration(tempvalue);
    }
    else if (Trimed_command == "CAL3"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu3.GetoutputCalibration(tempvalue);
    }
    else if (Trimed_command == "CAL4"){
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length()+1);
        float tempvalue = atof(tempdata);
        smu4.GetoutputCalibration(tempvalue);
    }
    else if (Trimed_command == "IN1") {
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length() + 1);
        float tempvalue = atof(tempdata);
        smu1.Calibration(tempvalue);
        smu1.arrIdx++;
    }
    else if (Trimed_command == "IN2") {
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length() + 1);
        float tempvalue = atof(tempdata);
        smu2.Calibration(tempvalue);
        smu2.arrIdx++;
    }
    else if (Trimed_command == "IN3") {
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length() + 1);
        float tempvalue = atof(tempdata);
        smu3.Calibration(tempvalue);
        smu3.arrIdx++;
    }
    else if (Trimed_command == "IN4") {
        char tempdata[20];
        Trimed_arg.toCharArray(tempdata, Trimed_arg.length() + 1);
        float tempvalue = atof(tempdata);
        smu4.Calibration(tempvalue);
        smu4.arrIdx++;
    }
    else if (Trimed_command == "FILE1") {
        smu1.HandleInput(Trimed_arg);
    }
    else if (Trimed_command == "FILE2") {
        smu2.HandleInput(Trimed_arg);
    }
    else if (Trimed_command == "FILE3") {
        smu3.HandleInput(Trimed_arg);
    }
    else if (Trimed_command == "FILE4") {
        smu4.HandleInput(Trimed_arg);
    }
 }

void clearSerialBuffer(int SerialCh){
    stringCompleteArray[SerialCh] = false;
    if ( SerialCh == 0){ inputString_Serial0 = ""; }
    else if ( SerialCh == 1){ inputString_Serial1 = ""; }
    else if ( SerialCh == 2){ inputString_Serial2 = ""; }
}
    
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

void SMU::Init(){
    DO.Set_DO(DO_ch_Active, HIGH); //Active High
    SetEn(false);
    SetMode(false);
    SetRange(0);
    SetOutputVoltage(0,0);
    v_cali_index = -1;
    c_cali_index = -1;
    arrIdx = 0;
    stringX = "";
    stringY = "";
    readBuffer = "";
}

void SMU::SetEn(bool value){
    En = value;
    DO.Set_DO(DO_ch_En, En);
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

void SMU::SetOutputVoltage(float SetVoltage, float printVoltage){
    OutputVoltage = printVoltage; //-10~ +10 V
    OutputVoltage2 = SetVoltage; // esp32에 써줄 값
    unsigned int tempvalue = (OutputVoltage2 + 10.0) / 20.0 * 65535;
    DAC.Write(DAC_ch,tempvalue);
}

float SMU::GetOutputVoltage(){
    return OutputVoltage;
}

double SMU::GetInputVoltage(float printValue){
    if(printValue != 0) {
        // change_calValue = true;
        InputVoltage = printValue;
        // InputVoltage = result[ADC_V_ch];
    }
    else {
        InputVoltage = result[ADC_V_ch];
    }
    return InputVoltage * 4;
}

double SMU::GetInputCurrent(float printValue){
    if(printValue != 0) {
        InputCurrent = printValue;
    }
    else {
        InputCurrent = result[ADC_I_ch];
    }
    return InputCurrent * 4;
} 

double SMU::GetM ( double x1, double x2, double y1, double y2){
    double temp_m;
    if ( (x2 - x1) == 0)
    {
        temp_m = 0;
    }
    else
    {
        temp_m = (y2 - y1) / (x2 - x1);
    }
    return temp_m;
}
double SMU::GetB ( double x1, double x2, double y1, double y2){

    return y1 - this->GetM( x1,  x2,  y1,  y2) * x1;

}

void SMU::SetAddr(unsigned int AddrVM,unsigned int AddrVB,unsigned int AddrCM,unsigned int AddrCB){
    eepromAddrVM = AddrVM;
    eepromAddrVB = AddrVB;
    eepromAddrCM = AddrCM;
    eepromAddrCB = AddrCB;
}

void SMU::Calibration(float stand){
    double rawV, rawC;
    double Vx1, Vx2, Vy1, Vy2, temp_Vm, temp_Vb;
    double Cx1, Cx2, Cy1, Cy2, temp_Cm, temp_Cb;
    int tempCnt ;
    delay(1000);
    rawV =  this->GetInputVoltage(0);
    rawC =  this->GetInputCurrent(0);
    arrStand[this->arrIdx] = stand /4;
    arrRawV[this->arrIdx] = rawV / 4;
    arrRawC[this->arrIdx] = rawC / 4;
    Serial.println("done");

    if (stand == 10){
        for ( int i = 1; i <= arrIdx ;i++){
            Vx1 = arrRawV[i-1];
            Vx2 = arrRawV[i];
            Vy1 = arrStand[i-1];
            Vy2 = arrStand[i];

            Cx1 = arrRawC[i-1];
            Cx2 = arrRawC[i];
            Cy1 = arrStand[i-1];
            Cy2 = arrStand[i];

            temp_Vm = this->GetM(Vx1 ,Vx2, Vy1, Vy2);
            temp_Vb = this->GetB(Vx1 ,Vx2, Vy1, Vy2); 

            temp_Cm = this->GetM(Cx1 ,Cx2, Cy1, Cy2);
            temp_Cb = this->GetB(Cx1 ,Cx2, Cy1, Cy2); 

            this->v_fx_m[i-1] = temp_Vm;
            this->v_fx_b[i-1] = temp_Vb;
            this->c_fx_m[i-1] = temp_Cm;
            this->c_fx_b[i-1] = temp_Cb;
            tempCnt = i;
        }
        arrIdx = -1;
        delay(1000);
        Serial.print("#");
        for(int i = 0; i < tempCnt; i++) {
            Serial.print(v_fx_m[i],7);
            if(i != tempCnt-1) {
                Serial.print(",");
            }
        }
        Serial.print(";");
        for(int i = 0; i < tempCnt; i++) {
            Serial.print(v_fx_b[i],7);
            if(i != tempCnt-1) {
                Serial.print(",");
            }
        }
        Serial.print("&");
        for (int i = 0; i < tempCnt; i++)
        {
            Serial.print(c_fx_m[i], 7);
            if(i != tempCnt-1) {
                Serial.print(",");
            }
        }
        Serial.print(";");
        for (int i = 0; i < tempCnt; i++)
        {
            Serial.print(c_fx_b[i], 7);
            if(i != tempCnt-1) {
                Serial.print(",");
            }
        }
        Serial.println("*");

        savefloat_eeprom(this->eepromAddrVM - 4, tempCnt);
        savefloat_eeprom(this->eepromAddrVB - 4, tempCnt);
        savefloat_eeprom(this->eepromAddrCM - 4, tempCnt);
        savefloat_eeprom(this->eepromAddrCB - 4, tempCnt);
        for (  int i  =0 ; i < tempCnt ; i++){
            savefloat_eeprom(this->eepromAddrVM + i * 4, this->v_fx_m[i]);
            savefloat_eeprom(this->eepromAddrVB + i * 4, this->v_fx_b[i]);
            savefloat_eeprom(this->eepromAddrCM + i * 4, this->c_fx_m[i]);
            savefloat_eeprom(this->eepromAddrCB + i * 4, this->c_fx_b[i]);
        }
        EEPROM.commit();
        this->v_cali_index = tempCnt; // 20
        this->c_cali_index = tempCnt; // 20
        Serial.print("v_cali_index = "); Serial.println(this->v_cali_index);
        Serial.print("c_cali_index = "); Serial.println(this->c_cali_index);
    }
}

double SMU::GetCaliValue(bool type){
    double raw;
    double *b_arr;
    double *m_arr;
    double b_tmp;
    double m_tmp;
    double tempOutput;
    
    int arrCount;
    int tempCaliIdx ;

    if ( type ){
        raw = this->GetInputVoltage(0); // 범위에 넣을 값
        b_arr = this->v_fx_b;
        m_arr = this->v_fx_m;
        arrCount = v_cali_index;//20
    }
    else {
        raw = this->GetInputCurrent(0); // 범위에 넣을 값
        b_arr = this->c_fx_b;
        m_arr = this->c_fx_m;
        arrCount = c_cali_index; //20
    }

    if ( arrCount !=-1){
        if(raw < -10.0) {
            b_tmp = b_arr[0];
            m_tmp = m_arr[0];
        }
        else if (-10.0 <= raw && raw < 10.0) {
            int temp_index = (int)(raw + 10);
            b_tmp = b_arr[temp_index];
            m_tmp = m_arr[temp_index];
            
            if(temp_index > (arrCount-1)) {
                b_tmp = b_arr[temp_index];
                m_tmp = m_arr[temp_index];
            }
        }
        else if (raw >= 10) {
            b_tmp = b_arr[arrCount-1];
            m_tmp = m_arr[arrCount-1];
        }
        tempOutput = m_tmp * (raw/4) + b_tmp;
    }
    else{
        tempOutput = raw/4;
    }

    if (type){
        this->GetInputVoltage(tempOutput);
        // return VtempOutput;
        return tempOutput*4;
    }
    else{
        this->GetInputCurrent(tempOutput);
        // return CtempOutput;
        return tempOutput*4;
    }

}

void SMU::outputCalibration(String str) {
    String input = str;
    int counter = 0;
    int idx, idx2;
    float x_f, y_f;
    double x1, x2, y1, y2, m, b, m_tmp, b_tmp;
    int idxCnt;

    for (int i = 0; i < input.length(); i++) {
        if (input.substring(i, i + 1) == ";") {
            this->stringX += input.substring(0, i);
            this->stringY += input.substring(i + 1, input.length());
            Serial.println(this->stringX);
            Serial.println(this->stringY);
        }
    }

    do{
        idx = this->stringY.indexOf(',');
        idx2 = this->stringX.indexOf(',');
        if (idx != -1){
            x_f = this->stringX.substring(0, idx2).toFloat();
            this->stringX = this->stringX.substring(idx2 + 1, this->stringX.length());
            y_f = this->stringY.substring(0, idx).toFloat();
            this->stringY = this->stringY.substring(idx + 1, this->stringY.length());
            this->x_arr[counter] = x_f;
            this->y_arr[counter] = y_f;
            counter++;
        }
        else{
            if (this->stringY.length() > 0){
                x_f = this->stringX.toFloat();
                y_f = this->stringY.toFloat();
                this->x_arr[counter] = x_f;
                this->y_arr[counter] = y_f;
                idx = -1;
            }
        }
    } while (idx != -1);
    this->outputIdx = counter;
    for (int i = 0; i < this->outputIdx; i++)
    {
        if (this->x_arr[i] != this->x_arr[i + 1])
        {
            x1 = x_arr[i];
            x2 = x_arr[i + 1];
            y2 = y_arr[i + 1];
            y1 = y_arr[i];

            m = this->GetM(x1, x2, y1, y2);
            b = this->GetB(x1, x2, y1, y2); 

            this->output_fx_m[i] = m;
            this->output_fx_b[i] = b;
            idxCnt = i;
        }
    }

    savefloat_eeprom(out_fx_m - 4, idxCnt);
    savefloat_eeprom(out_fx_b - 4, idxCnt);

    for (int i = 0; i < idxCnt; i++)
    {
        savefloat_eeprom(out_fx_m + i * 4, output_fx_m[i]);
        savefloat_eeprom(out_fx_b + i * 4, output_fx_b[i]);
    }
    EEPROM.commit();
}

void SMU::GetoutputCalibration(float tempvalue) {
    double m_tmp, b_tmp, caliValue;
    if(tempvalue < -10.0) {
        b_tmp = this->output_fx_b[0];
        m_tmp = this->output_fx_m[0];
    }
    else if (-10.0 <= tempvalue && tempvalue < 10.0)
    {
        int temp_index = (int)(tempvalue + 10);
        b_tmp = this->output_fx_b[temp_index];
        m_tmp = this->output_fx_m[temp_index];
    }
    else if (tempvalue >= 10)
    {
        b_tmp = this->output_fx_b[19];
        m_tmp = this->output_fx_m[19];
    }

    if(this->stringX == NULL && this->stringY== NULL) {
        caliValue = tempvalue;
    }
    else {
        caliValue = m_tmp * tempvalue + b_tmp;
    }

    this->SetOutputVoltage(caliValue, tempvalue);
}

void SMU::HandleInput(String inBuff) {
    sumBuffer = readBuffer + inBuff;

    String split[4];
    int splitIdx = 0;

    String voltage_m = "";
    String voltage_b = "";
    String current_m = "";
    String current_b = "";

    if (sumBuffer.indexOf('*') != -1 && sumBuffer.indexOf('$') != -1)
    {
        int startCMD = sumBuffer.indexOf('*');
        int endCMD = sumBuffer.indexOf('$');

        // String strSplit;
        int idx;

        String tmpString = sumBuffer.substring(startCMD + 1, endCMD - startCMD);
        tmpString.trim();

        while(tmpString.length() > 0 ) {
            idx = tmpString.indexOf(';');
            if(idx == -1) {
                split[splitIdx] = tmpString;
                voltage_m = split[0];
                voltage_b = split[1];
                current_m = split[2];
                current_b = split[3];
                this->FileCalibration(voltage_m, voltage_b, current_m, current_b);
                tmpString = "";
            }
            else {
                split[splitIdx] = tmpString.substring(0, idx);
                Serial.print("split[");Serial.print(splitIdx);Serial.print("] = ");Serial.println(split[splitIdx]);
                // strSplit = tmpString.substring(0, idx+1);
                tmpString = tmpString.substring(idx+1);
                tmpString.trim();
            }
            splitIdx++;
            // strSplit.trim();
        }
        readBuffer = "";
    }
    else {
        readBuffer = sumBuffer;
    }
}

void SMU::FileCalibration(String voltage_m, String voltage_b, String current_m, String current_b) {
    int counter = 0;
    int Vm_idx, Vb_idx, Cm_idx, Cb_idx;
    double Vm_f, Vb_f, Cm_f, Cb_f;
    int idxCnt;

    do {
        Vm_idx = voltage_m.indexOf(',');
        Vb_idx = voltage_b.indexOf(',');
        Cm_idx = current_m.indexOf(',');
        Cb_idx = current_b.indexOf(',');

        if(Vm_idx != -1) {
            Vm_f = voltage_m.substring(0, Vm_idx).toFloat();
            voltage_m = voltage_m.substring(Vm_idx + 1, voltage_m.length());
            Vb_f = voltage_b.substring(0, Vb_idx).toFloat();
            voltage_b = voltage_b.substring(Vb_idx + 1, voltage_b.length());
            
            Cm_f = current_m.substring(0, Cm_idx).toFloat();
            current_m = current_m.substring(Cm_idx + 1, current_m.length());
            Cb_f = current_b.substring(0, Cb_idx).toFloat();
            current_b = current_b.substring(Cb_idx + 1, current_b.length());
        }
        else {
            Vm_f = voltage_m.toFloat();
            Vb_f = voltage_b.toFloat();
            Cm_f = current_m.toFloat();
            Cb_f = current_b.toFloat();
            Vm_idx = -1;
        }
        v_fx_m[counter] = Vm_f;
        v_fx_b[counter] = Vb_f;
        c_fx_m[counter] = Cm_f;
        c_fx_b[counter] = Cb_f;
        counter++;

    }while (Vm_idx != -1);
    v_cali_index = counter;
    c_cali_index = counter;
    this->GetCaliValue(true);
    this->GetCaliValue(false);
}

void SMU::ReadEEPROM() {
    int inputLength = readfloat_eeprom(this->arrIdx-4);
    int outputLength = readfloat_eeprom(this->outputIdx-4);

    for(int i = 0; i < inputLength; i++) {
        v_fx_m[i] = readfloat_eeprom(this->eepromAddrVM);
        v_fx_b[i] = readfloat_eeprom(this->eepromAddrVM);
        c_fx_m[i] = readfloat_eeprom(this->eepromAddrVM);
        c_fx_b[i] = readfloat_eeprom(this->eepromAddrVM);
    }
    for(int i = 0; i < outputLength; i++) {
        output_fx_m[i] = readfloat_eeprom(out_fx_m);
        output_fx_b[i] = readfloat_eeprom(out_fx_b);
    }
}