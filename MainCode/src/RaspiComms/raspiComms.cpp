#include <RaspiComms/raspiComms.h>

Packet invalidPacket(PacketType::INVALID, 0);

// ----------------------------------------------------------------------------------------------------
// class RaspiComms
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// private methods
// --------------------------------------------------

void RaspiComms::_sendPacket(PacketType use, PacketSize size, const uint8_t* data) {
    _ser.write((uint8_t)use);
    _ser.write((uint8_t*)&size, sizeof(PacketSize));
    if (size > 0)
        _ser.write(data, size);
}

void RaspiComms::_sendPacket(PacketType use, PacketSize size, const __FlashStringHelper* data) {
    _ser.write((uint8_t)use);
    _ser.write((uint8_t*)&size, sizeof(PacketSize));
    if (size > 0)
        _ser.print(data);
}

void RaspiComms::_sendPacket(PacketType use, const __FlashStringHelper* data) {
    size_t len = strlen_P((const char*)data);
    if (len > 0xFFFF)
        len = 0xFFFF;
    _sendPacket(use, len, data);
}

Packet RaspiComms::_recievePacket(uint32_t timeout) {
    if (_ser.available() == 0)
        return invalidPacket;
    PacketType type = (PacketType)_ser.read();
    if (type <= PacketType::INVALID || type > PacketType::LOCATION) {
        debugLog(String(F("_recievePacket got invalid type: ")) + String((int)type));
        return invalidPacket;
    }
    uint32_t start = millis();
    while (_ser.available() < sizeof(PacketSize)) {
        if (millis() - start >= timeout) {
            debugLog(F("_recievePacket get size timeout!"));
            return invalidPacket;
        }
    }
    PacketSize size;
    _ser.readBytes((uint8_t*)&size, sizeof(PacketSize));
    if (size == 0)
        return Packet(type, 0);
    while (_ser.available() < size) {
        if (millis() - start >= timeout) {
            debugLog(F("_recievePacket get data timeout!"));
            return invalidPacket;
        }
    }
    Packet packet(type, size);
    _ser.readBytes(packet.data(), size);
    return packet;
}

bool RaspiComms::_checkPacket() {
    return _ser.available() > 0;
}

// --------------------------------------------------
// public methods
// --------------------------------------------------

void RaspiComms::debugLog(const __FlashStringHelper* text) {
    size_t len = strlen_P((const char*)text);
    if (len > 0xFFFF)
        len = 0xFFFF;
    _sendPacket(PacketType::DEBUG_CONSOLE, len, text);
}

void RaspiComms::debugLog(String str) {
    PacketSize size = str.length();
    if (str.length() > 0xFFFF)
        size = 0xFFFF;
    _sendPacket(PacketType::DEBUG_CONSOLE, size, (uint8_t*)str.c_str());
}

void RaspiComms::sendTile(uint8_t x, uint8_t y, uint8_t z, uint8_t data) {
    uint8_t pack_data[4] = {x, y, z, data};
    _sendPacket(PacketType::MAP_TILE, 4, pack_data);
}

void RaspiComms::sendPos(uint8_t x, uint8_t y, uint8_t z, uint8_t rotation) {
    uint8_t pack_data[4] = {x, y, z, rotation};
    _sendPacket(PacketType::LOCATION, 4, pack_data);
}

RaspiEvent RaspiComms::update(uint8_t fr, uint8_t br, uint8_t fl, uint8_t bl) {
    if (!_checkPacket())
        return RaspiEvent::NO_MORE_PACKETS;

    Packet packet = _recievePacket(10);

    switch (packet.type())
    {
    // --------------------------------------------------
    case PacketType::CAM_RESET:
        if (_camState != CameraState::LOOKING) {
            _camState = CameraState::LOOKING;
            return RaspiEvent::CAMERA_INVALID;
        }
        return RaspiEvent::NONE;

    // --------------------------------------------------
    case PacketType::CAM_DETECT_SIDE:
        if (_camState != CameraState::LOOKING) {
            _sendPacket(PacketType::CAM_RESET, F("CAM_DETECT_SIDE out of sequence!"));
            _camState = CameraState::LOOKING;
            return RaspiEvent::CAMERA_INVALID;
        }
        if (packet.size() != 1) {
            _sendPacket(PacketType::CAM_RESET, F("CAM_DETECT_SIDE sent invalid data amount, should be 1 byte!"));
            _camState = CameraState::LOOKING;
            return RaspiEvent::NONE;
        }
        if (packet[0] == 1) {
            _camState = CameraState::TRIGGERED_RIGHT;
            uint16_t data = 0;
            ((uint8_t*)&data)[0] = fr;
            ((uint8_t*)&data)[1] = br;
            _sendPacket(PacketType::CAM_TOF_DATA, 2, (uint8_t*)&data);
            return RaspiEvent::CAMERA_TRIGGERED_RIGTH;
        }
        else if (packet[0] == 2) {
            _camState = CameraState::TRIGGERED_LEFT;
            uint16_t data = 0;
            ((uint8_t*)&data)[0] = fl;
            ((uint8_t*)&data)[1] = bl;
            _sendPacket(PacketType::CAM_TOF_DATA, 2, (uint8_t*)&data);
            return RaspiEvent::CAMERA_TRIGGERED_LEFT;
        }
        _sendPacket(PacketType::CAM_RESET, F("CAM_DETECT_SIDE invalid data!"));
        _camState = CameraState::LOOKING;
        return RaspiEvent::NONE;

    // --------------------------------------------------
    case PacketType::CAM_VICTIM_VALID:
        if (_camState != CameraState::TRIGGERED_LEFT && _camState != CameraState::TRIGGERED_RIGHT) {
            _sendPacket(PacketType::CAM_RESET, F("CAM_VICTIM_VALID out of sequence!"));
            _camState = CameraState::LOOKING;
            return RaspiEvent::NONE;
        }
        if (packet.size() != 1) {
            _sendPacket(PacketType::CAM_RESET, F("CAM_VICTIM_VALID sent invalid data amount, should be 1 byte!"));
            _camState = CameraState::LOOKING;
            return RaspiEvent::CAMERA_INVALID;
        }
        debugLog(F("got CAM_VICTIM_VALID, successfully!"));
        _camState = CameraState::LOOKING;
        switch (packet[0]) {
        case 0: return _camState == CameraState::TRIGGERED_RIGHT ? RaspiEvent::DETECTED_PSI_RIGHT : RaspiEvent::DETECTED_PSI_LEFT;
        case 1: return _camState == CameraState::TRIGGERED_RIGHT ? RaspiEvent::DETECTED_PHI_RIGHT : RaspiEvent::DETECTED_PHI_LEFT;
        case 2: return _camState == CameraState::TRIGGERED_RIGHT ? RaspiEvent::DETECTED_OMEGA_RIGHT : RaspiEvent::DETECTED_OMEGA_LEFT;
        case 3: return _camState == CameraState::TRIGGERED_RIGHT ? RaspiEvent::DETECTED_RING_SUM_0_RIGHT : RaspiEvent::DETECTED_RING_SUM_0_LEFT;
        case 4: return _camState == CameraState::TRIGGERED_RIGHT ? RaspiEvent::DETECTED_RING_SUM_1_RIGHT : RaspiEvent::DETECTED_RING_SUM_1_LEFT;
        case 5: return _camState == CameraState::TRIGGERED_RIGHT ? RaspiEvent::DETECTED_RING_SUM_2_RIGHT : RaspiEvent::DETECTED_RING_SUM_2_LEFT;
        }
        _sendPacket(PacketType::CAM_RESET, F("CAM_VICTIM_VALID sent invalid data!"));
        return RaspiEvent::NONE;

    // --------------------------------------------------
    case PacketType::CAM_VICTIM_INVALID:
        if (_camState == CameraState::LOOKING)
            return RaspiEvent::NONE;
        _camState = CameraState::LOOKING;
        return RaspiEvent::CAMERA_INVALID;

    // --------------------------------------------------
    case PacketType::DEBUG_CONSOLE:
        return RaspiEvent::NONE;

    // --------------------------------------------------
    case PacketType::CAM_TOF_DATA:
        return RaspiEvent::NONE;
    
    // --------------------------------------------------
    case PacketType::MAP_TILE:
        return RaspiEvent::NONE;
    
    // --------------------------------------------------
    case PacketType::LOCATION:
        return RaspiEvent::NONE;

    // --------------------------------------------------
    case PacketType::INVALID:
        return RaspiEvent::NONE;
    }

    return RaspiEvent::NONE;
}