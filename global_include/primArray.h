#ifndef _PRIM_ARRAY_H_
#define _PRIM_ARRAY_H_

#include <Arduino.h>
#include <debug.h>
#include <errorHandling.h>

template <typename T>
class PrimArray {
private:
    T* _data = nullptr;
    uint8_t _size = 0;

public:
    PrimArray() {}
    template <typename... Args>
    PrimArray(Args... args);
    PrimArray(const PrimArray<T>& other);
    PrimArray(PrimArray<T>&& other);

    ~PrimArray();

    uint8_t size() const {return _size;}

    Likely<uint8_t> pushBack(const T& element);
    Likely<T> popBack();

    void migrateTo(PrimArray<T>* other);

    PrimArray<T>& operator=(const PrimArray<T>& other);
    PrimArray<T>& operator=(PrimArray<T>&& other);

    Likely<T*> operator[](uint8_t index);
};

template <typename T>
template <typename... Args>
PrimArray<T>::PrimArray(Args... args) : _data((T*)malloc(sizeof...(args) * sizeof(T))), _size(sizeof...(args)) {
    T values[] = {args...};
    for (uint8_t i = 0; i < _size; i++)
        _data[i] = values[i];
}

template <typename T>
PrimArray<T>::PrimArray(const PrimArray<T>& other) : _data((T*)malloc(_size * sizeof(T))), _size(other.size()) {
    if (_data == NULL)
        ERROR(F("RAN OUT OF RAM!!! AHHHHH!!!"));
    memcpy((void*)_data, (void*)other._data, _size * sizeof(T));
}

template <typename T>
PrimArray<T>::PrimArray(PrimArray<T>&& other) : _data(other._data), _size(other.size()) {
    other._data = nullptr;
    other._size = 0;
}

template <typename T>
PrimArray<T>::~PrimArray() {
    if (_data != nullptr)
        free(_data);
    _size = 0;
}

template <typename T>
PrimArray<T>& PrimArray<T>::operator=(const PrimArray<T>& other) {
    if (this == &other)
        return *this;

    if (other._size != _size) {
        free(_data);
        _data = (T*)malloc(other._size * sizeof(T));
        if (_data == NULL)
            ERROR(F("RAN OUT OF RAM!!! AHHHHH!!!"));
    }
    _size = other._size;
    memcpy((void*)_data, (void*)other._data, _size * sizeof(T));

    return *this;
}

template <typename T>
PrimArray<T>& PrimArray<T>::operator=(PrimArray<T>&& other) {
    if (this == &other)
        return *this;

    free(_data);
    _data = other._data;
    other._data = nullptr;
    _size = other._size;
    other._size = 0;

    return *this;
}

template <typename T>
Likely<uint8_t> PrimArray<T>::pushBack(const T& element) {
    if (_size == 0xFF)
        return Likely<uint8_t>(F("Prim Array at capacity, can't push back data!"));
    T* newData = (T*)malloc((_size + 1) * sizeof(T));
    if (newData == NULL)
        ERROR(F("RAN OUT OF RAM!!! AHHHHH!!!"));
    memcpy(newData, _data, _size * sizeof(T));
    newData[_size] = element;
    _size++;
    free(_data);
    _data = newData;
    return Likely<uint8_t>(_size);
}

template <typename T>
Likely<T> PrimArray<T>::popBack() {
    if (_size == 0)
        return Likely<T>(F("Can't pop back Element Array is empty!"));
    _size--;
    T* newData = (T*)malloc(_size * sizeof(T));
    if (newData == NULL)
        ERROR(F("RAN OUT OF RAM!!! AHHHHH!!!"));
    memcpy(newData, _data, _size * sizeof(T));
    T ret = _data[_size];
    free(_data);
    _data = newData;
    return Likely<T>(ret);
}

template <typename T>
void PrimArray<T>::migrateTo(PrimArray<T>* other) {
    if (this == other)
        return;

    free(other->_data);
    other->_data = _data;
    _data = nullptr;
    other->_size = _size;
    _size = 0;
}

template <typename T>
Likely<T*> PrimArray<T>::operator[](uint8_t index) {
    if (index >= _size)
        return Likely<T*>(F("index out of bounds!"));
    return Likely<T*>(&_data[index]);
}

#endif // _PRIM_ARRAY_H_