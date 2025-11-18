#ifndef _ERROR_HANDLING_H_
#define _ERROR_HANDLING_H_

#include <Arduino.h>

// the Likely class can be used for return types to possibly return an error.
// the Likely class should allways be checked before getting the data.
template <typename T>
class Likely {
private:
    union
    {
        T _data;
        String* _error;
    };
    bool _failed;

public:
    Likely(T data) : _data(data), _failed(false) {}
    Likely(const String& error) : _error(new String(error)), _failed(true) {}
    Likely(const Likely<T>& likely) : _failed(likely._failed) {
        if (_failed)
            _error = new String(*likely._error);
        else
            _data = T(likely._data);
    }
    ~Likely() {
        if (_failed)
            delete _error;
        else
            _data.~T();
    }

    bool failed() {return _failed;}
    String* getError() {return _error;} //! this will fail if failed is false
    T& getData() {return _data;} //! this will fail if failed is true

    Likely<T>& operator=(const Likely<T>& other) {
        if (this == &other)
            return *this;

        if (_failed)
            delete _error;
        else
            _data.~T();
        
        _failed = other._failed;
        if (_failed)
            _error = new String(*other._error);
        else
            _data = T(other._data);
        
        return *this;
    }
};

#endif