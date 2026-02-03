#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <Arduino.h>
#include <HardwareSerial.h>

#include <debug.h>
#include <pos.h>

#define CAMERA_BAUD_RATE 9600

// camera schaut immer
// 1. camera -UART-> mega, wenn die camera etwas erkennt
// 2. mega -UART-> camera die tof daten
// 3. camera evaluiert neu
// 5. camera -UART-> mega, was wo wie

enum class CameraState {
    LOOKING,
    RECIEVE,
};

enum class VictimType : uint8_t {
    INVALID     = 0,   
    PSI         = 1,
    PHI         = 2,
    OMEGA       = 3,
    RING_SUM_0  = 4,
    RING_SUM_1  = 5,
    RING_SUM_2  = 6
};

enum class VictimSide : uint8_t {
    INVALID = 0,
    RIGHT   = 1,
    LEFT    = 2
};

typedef struct _VictimData {
    VictimType type;
    VictimSide side;
} VictimData;


class Camera {
private:
    CameraState _state = CameraState::LOOKING;
    HardwareSerial& _ser;

    bool awaitResponce(uint8_t size, uint32_t timeout) { // returns true on timeout
        uint32_t start = millis();
        while (_ser.available() < size)
            if ((millis() - start) > timeout)
                return true;
        return false;
    }
    
public:
    Camera() = delete;
    Camera(HardwareSerial& ser) : _ser(ser) { _ser.begin(CAMERA_BAUD_RATE); }
    Camera(Camera&& cam) : _state(cam._state), _ser(cam._ser) { cam._state = CameraState::LOOKING; }
    Camera(const Camera& cam) = delete;
    ~Camera() {}

    bool check() { // check if the camera has found something
        if (_state != CameraState::LOOKING)
            return false;
        if (_ser.available() == 0)
            return false;
        _state = CameraState::RECIEVE;
        return true;
    }

    VictimData getVictim(uint8_t fr, uint8_t br, uint8_t fl, uint8_t bl, uint32_t timeout) {
        if (_state != CameraState::RECIEVE)
            return { VictimType::INVALID, VictimSide::INVALID };
        if (_ser.available() == 0) {
            _state = CameraState::LOOKING;
            return { VictimType::INVALID, VictimSide::INVALID };
        }

        VictimData data;

        data.side = (VictimSide)_ser.read(); // read the current side
        if (data.side != VictimSide::LEFT && data.side != VictimSide::RIGHT)
            return { VictimType::INVALID, VictimSide::INVALID };

        while (_ser.available() != 0) // clear buffer
            _ser.read();
        
        _ser.write((data.side == VictimSide::LEFT ? fl : fr)); // send the front tof value
        _ser.write((data.side == VictimSide::LEFT ? bl : br)); // send the back tof value

        if (awaitResponce(1, timeout)) {
            ERROR_MINOR(F("getVictim timed out on get type!"), SET_RED);
            _state = CameraState::LOOKING;
            return { VictimType::INVALID, VictimSide::INVALID };
        }
        data.type = (VictimType)_ser.read(); // get the victim type

        if (data.type <= VictimType::INVALID || data.type > VictimType::RING_SUM_2) // check if the victim is valid
            return { VictimType::INVALID, VictimSide::INVALID };

        _state = CameraState::LOOKING;
        return data;
    }

    Camera& operator=(Camera&& cam) = delete;
    Camera& operator=(const Camera& cam) = delete;
};

#endif