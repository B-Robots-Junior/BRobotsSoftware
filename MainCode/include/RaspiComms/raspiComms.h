#ifndef _RASPI_COMMS_H_
#define _RASPI_COMMS_H_

#include <Arduino.h>
#include <debug.h>

#define RASPI_BAUDE_RATE 115200

typedef uint16_t PacketSize;

enum class PacketType : uint8_t {
    INVALID             = 0, // invalid packet type (placeholder)
    DEBUG_CONSOLE       = 1, // packet to send to console
    CAM_RESET           = 2, // send this and reason to restart cam communication if anything goes out of sequence or is invalid (must also reset state if this is recieved)
    CAM_DETECT_SIDE     = 3, // initial packet that the camera sends to the mega containing one byte (first and only byte: rigth: 0, left: 1)
    CAM_TOF_DATA        = 4, // send the respective side 2 bytes val front and val back
    CAM_VICTIM_VALID    = 5, // get from the camera that the victim is valid, 1 byte type
    CAM_VICTIM_INVALID  = 6, // get from the camera that the victim is invalid, no data
    MAP_TILE            = 7, // send map tile data 3 bytes x, y, z position and 1 byte map data
    LOCATION            = 8  // send 3 bytes x, y, z location and 1 byte rotation 0-3
};

class Packet {
private:
    PacketType _type;
    PacketSize _size;
    uint8_t* _data = nullptr;

public:
    Packet(PacketType type, PacketSize size) : _type(type), _size(size), _data(new uint8_t[size]) {}
    Packet(const Packet& packet) : _type(packet._type), _size(packet._size), _data(new uint8_t[packet._size]) {
        if (_size > 0)
            memcpy(_data, packet._data, _size);
    }
    Packet(Packet&& packet) : _type(packet._type), _size(packet._size), _data(packet._data) {
        packet._type = PacketType::INVALID;
        packet._size = 0;
        packet._data = nullptr;
    }
    ~Packet() { delete[] _data; }

    PacketSize size() const { return _size; }
    PacketType type() const { return _type; }
    uint8_t* data() { return _data; }

    uint8_t& operator[](PacketSize index) {
        return _data[index];
    }

    Packet& operator=(const Packet& other) {
        if (this != &other) {
            delete[] _data;
            _type = other._type;
            _size = other._size;
            _data = new uint8_t[_size];
            memcpy(_data, other._data, _size);
        }
        return *this;
    }
    Packet& operator=(Packet&& other) {
        if (this != &other) {
            delete[] _data;
            _type = other._type;
            _size = other._size;
            _data = other._data;
            other._type = PacketType::INVALID;
            other._size = 0;
            other._data = nullptr; 
        }
        return *this;
    }
};

enum class RaspiEvent {
    NO_MORE_PACKETS,
    NONE,
    CAMERA_TRIGGERED_RIGTH,
    CAMERA_TRIGGERED_LEFT,
    CAMERA_INVALID, // if the camera comms becomes invalid for some reason 
    DETECTED_PSI_RIGHT,
    DETECTED_PHI_RIGHT,
    DETECTED_OMEGA_RIGHT,
    DETECTED_RING_SUM_0_RIGHT,
    DETECTED_RING_SUM_1_RIGHT,
    DETECTED_RING_SUM_2_RIGHT,
    DETECTED_PSI_LEFT,
    DETECTED_PHI_LEFT,
    DETECTED_OMEGA_LEFT,
    DETECTED_RING_SUM_0_LEFT,
    DETECTED_RING_SUM_1_LEFT,
    DETECTED_RING_SUM_2_LEFT
};

enum class CameraState {
    LOOKING,
    TRIGGERED_RIGHT,
    TRIGGERED_LEFT
};

extern Packet invalidPacket;

class RaspiComms {
private:
    HardwareSerial& _ser;
    CameraState _camState = CameraState::LOOKING;

    void _sendPacket(PacketType use, PacketSize size, const uint8_t* data); // data must at least contain size bytes of data
    void _sendPacket(PacketType use, PacketSize size, const __FlashStringHelper* data);
    void _sendPacket(PacketType use, const __FlashStringHelper* data);
    Packet _recievePacket(uint32_t timeout);
    bool _checkPacket();

public:
    RaspiComms() = delete;
    RaspiComms(HardwareSerial& ser) : _ser(ser) { _ser.begin(RASPI_BAUDE_RATE); }
    RaspiComms(RaspiComms&& comms) : _ser(comms._ser) {}
    RaspiComms(const RaspiComms& comms) = delete;
    ~RaspiComms() {}

    void debugLog(const __FlashStringHelper* text);
    void debugLog(String str);
    void sendTile(uint8_t x, uint8_t y, uint8_t z, uint8_t data);
    void sendPos(uint8_t x, uint8_t y, uint8_t z, uint8_t rotation);
    RaspiEvent update(uint8_t fr, uint8_t br, uint8_t fl, uint8_t bl);

    RaspiComms& operator=(RaspiComms&& cam) = delete;
    RaspiComms& operator=(const RaspiComms& cam) = delete;
};

#endif