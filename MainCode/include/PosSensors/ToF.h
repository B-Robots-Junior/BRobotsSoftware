#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <VL6180X.h> // short range
#include <VL53L0X.h> // middle range
#include <VL53L1X.h> // long range
#include <PosSensors/VL53L4CD.h> // new long range
#include <SPI.h>


class ShortToF : public VL6180X {
    private:
        int _lastReading = 0;

    public:
        int address;
        int shutdownPin;
        
        ShortToF();
        ShortToF(char _shutdownPin, int _address);
        
        bool init_ToF();
        int read();
}; 


class MiddleToF : public VL53L0X {
    private:
        int _lastReading = 0;

    public:
        int address;
        int shutdownPin;
        
        MiddleToF();
        MiddleToF(char _shutdownPin, int _address);
        
        bool init_ToF();
        int read();
};


class LongToF : public VL53L1X {
    private:
        int _lastReading = 0;

    public:
        int address;  
        int shutdownPin;
        
        LongToF();
        LongToF(char _shutdownPin, int _address);  
        
        bool init_ToF();
        int read();
};


class NewLongToF : public VL53L4CD {
    private:
        int _lastReading = 0;

    public:
        int address;  
        int shutdownPin;
        
        NewLongToF();
        NewLongToF(char _shutdownPin, int _address);  
        
        bool init_ToF();
        int read();
};