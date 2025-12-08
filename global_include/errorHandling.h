#ifndef _ERROR_HANDLING_H_
#define _ERROR_HANDLING_H_

#include <Arduino.h>
#include <debug.h>
#include <new.h>

// the Likely class can be used for return types to possibly return an error.
// the Likely class should allways be checked before getting the data.
template<typename T>
class Likely {
private:
    bool failed_;
    union {
        uint8_t data_[sizeof(T)];
        const __FlashStringHelper* error_;
    };

public:
    Likely(const T& value) : failed_(false) {
        new (&data_) T(value);
    }

    Likely(const __FlashStringHelper* err) : failed_(true), error_(err) {}

    ~Likely() {
        if (!failed_) {
            reinterpret_cast<T*>(&data_)->~T();
        }
    }

    bool failed() const { return failed_; }
    const __FlashStringHelper* error() const {
        if (!failed_)
            ERROR(F("failed() was not checked before getError was called! No error occoured floating pointer immenent!"));
        return error_;
    }
    T& value() {
        if (failed_)
            ERROR_MUL((F("failed() was not checked before getData was called! Following error occoured: "))(error_));
        return *reinterpret_cast<T*>(&data_);
    }
};

#endif