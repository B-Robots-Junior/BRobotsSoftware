#pragma once

#ifndef _BIT_PTR_H_
#define _BIT_PTR_H_

#include <Arduino.h>

template <uint8_t amount>
class BitPtr {
public:
    BitPtr(uint8_t* byte, uint8_t pos) : byte(byte), pos(pos) {}

    uint8_t* byte;
    uint8_t pos;

    operator uint8_t() {
        return (uint8_t)BitPtr<1>(byte, pos) | ((uint8_t)BitPtr<amount-1>(byte, pos + 1) << 1);
    }
    void operator=(uint8_t value) {
        BitPtr<1>(byte, pos) = value;
        BitPtr<amount-1>(byte, pos + 1) = (value >> 1);
    }
};

template <>
class BitPtr<1> {
public:
    BitPtr(uint8_t* byte, uint8_t pos) : byte(byte), pos(pos) {}

    uint8_t* byte;
    uint8_t pos;

    operator uint8_t() {
        return (*byte >> pos) & 1;
    }
    void operator=(uint8_t value) {
        *byte = (*byte & ~(1 << pos)) | ((value & 1) << pos);
    }
};

#endif