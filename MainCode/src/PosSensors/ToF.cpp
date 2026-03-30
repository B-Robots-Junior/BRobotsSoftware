#define _TOF_C
#include <PosSensors/ToF.h>
#include <debug.h>


ShortToF::ShortToF(){
}

ShortToF::ShortToF(char _shutdownPin, int _address){
    address = _address;
    shutdownPin = _shutdownPin;
    pinMode(shutdownPin, OUTPUT);
    digitalWrite(shutdownPin, LOW);
}

bool ShortToF::init_ToF(){
    digitalWrite(shutdownPin, HIGH);
    delay(5);
    init();
    configureDefault();
    setTimeout(150);
    setAddress(address);
    startRangeContinuous(60);
    
    int tmp = millis();
    while(millis() - tmp < 160){
        if(timeoutOccurred()) return true;
        read(); 
    }
    return false;
}

int ShortToF::read(){
    if ((readReg(RESULT__INTERRUPT_STATUS_GPIO) & 0x07) == 0x04)
    {
        _lastReading = readReg(RESULT__RANGE_VAL);
        writeReg(SYSTEM__INTERRUPT_CLEAR, 0x01);
    }
    return _lastReading;
}


MiddleToF::MiddleToF(){
}

MiddleToF::MiddleToF(char _shutdownPin, int _address){
    address = _address;
    shutdownPin = _shutdownPin;
    pinMode(shutdownPin, OUTPUT);
    digitalWrite(shutdownPin, LOW);
}

bool MiddleToF::init_ToF(){
    digitalWrite(shutdownPin, HIGH);
    delay(5);
    bool failed = false;
    if(!init()) 
    {
        failed = true;
        DB_PRINTLN(F("Middletof init failed"));
    }
    
    setTimeout(250);
    startContinuous();
    setAddress(address);
    return failed;
}

int MiddleToF::read(){
    if ((readReg(0x13) & 0x07) != 0) {
        _lastReading = readRangeContinuousMillimeters();
    }
    return _lastReading;
}


LongToF::LongToF(){
}

LongToF::LongToF(char _shutdownPin, int _address){
    address = _address;
    shutdownPin = _shutdownPin;
    pinMode(shutdownPin, OUTPUT);
    digitalWrite(shutdownPin, LOW);
}

bool LongToF::init_ToF(){
    digitalWrite(shutdownPin, HIGH);
    delay(5);
    bool failed = false;
    if(!init()) failed = true;
    setTimeout(250);
    startContinuous(10);
    setAddress(address);
    return failed;
}

int LongToF::read(){
    if ((readReg(VL53L1X::GPIO__TIO_HV_STATUS) & 0x01) != 0) {
        _lastReading = readRangeContinuousMillimeters();
    }
    return _lastReading;
}


NewLongToF::NewLongToF(){
}

NewLongToF::NewLongToF(char _shutdownPin, int _address){
    address = _address;
    shutdownPin = _shutdownPin;
    pinMode(shutdownPin, OUTPUT);
    digitalWrite(shutdownPin, LOW);
}

bool NewLongToF::init_ToF(){
    digitalWrite(shutdownPin, HIGH);
    delay(5);
    bool failed = false;
    if(!init()) failed = true;
    setAddress(address);
    startContinuous();
    return failed;
}

int NewLongToF::read(){

    if (dataReady()) { // (readReg(VL53L4CD::GPIO__TIO_HV_STATUS) & 0x01) != 0
        _lastReading = readRangeContinuousMillimeters();
    }
    return _lastReading;
}