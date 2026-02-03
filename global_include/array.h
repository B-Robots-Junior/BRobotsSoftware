#pragma once

#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <Arduino.h>
#include <debug.h>

template <typename T>
class Array {
private:
    T* _data = nullptr;
    unsigned int _size = 0;

public:
    Array() {}
    // Variadic constructor
    template <typename... Args>
    Array(Args... args) : _data((T*)malloc(sizeof...(args) * sizeof(T))), _size(sizeof...(args)) {
        T values[] = {args...}; // Expands the parameter pack into an array
        for (unsigned int i = 0; i < _size; i++) {
            _data[i] = values[i];
        }
    }
    Array(unsigned int numElements, T defaultValue) : 
        _data((T*)malloc(numElements * sizeof(T))), _size(numElements) 
    {
        for (unsigned int i = 0; i < _size; i++)
            _data[i] = defaultValue;
    }
    Array(const Array& array) : 
        _data((T*)malloc(array._size * sizeof(T))), _size(array._size) 
    {
        for (unsigned int i = 0; i < _size; i++)
            _data[i] = T(array._data[i]);
    }
    ~Array() {
        if (_data == nullptr)
            return;
        for (unsigned int i = 0; i < _size; i++)
            _data[i].~T();
        free(_data);
    }

    unsigned int size() const { return _size; }
    void clear() {
        for (unsigned int i = 0; i < _size; i++)
            _data[i].~T();
        free(_data);
        _data = nullptr;
        _size = 0;
    }
    T& back() {return _data[_size - 1];}
    void push_back(const T& element) {
        T* newData = (T*)malloc((_size + 1) * sizeof(T));
        memcpy(newData, _data, _size * sizeof(T));
        newData[_size] = T(element);
        free(_data);
        _data = newData;
        _size++;
    }
    void pop_back() {
        if (_size == 0) {
            if (!CONNECTED)
                BEGIN_DEBUG(9600);
            ERROR("Can't decrease size of array below zero!");
            return;
        }
        _size--;
        _data[_size].~T();
        T* newData = (T*)malloc(_size * sizeof(T));
        memcpy(newData, _data, _size * sizeof(T));
        free(_data);
        _data = newData;
    }
    void insert(unsigned int pos, const T& element) {
        if (pos >= _size) {
            if (!CONNECTED)
                BEGIN_DEBUG(9600);
            ERROR("Can't insert element outside of the bounds of the array!");
        }
        T* newData = (T*)malloc((_size + 1) * sizeof(T));
        memcpy(newData, _data, pos * sizeof(T));
        newData[pos] = T(element);
        memcpy(&newData[pos + 1], &_data[pos], (_size - pos) * sizeof(T));
        free(_data);
        _data = newData;
        _size++;
    }
    void pop(unsigned int pos) {
        if (pos >= _size) {
            if (!CONNECTED)
                BEGIN_DEBUG(9600);
            ERROR("Can't remove an element outside of the bounds of the array!");
            return;
        }
        _size--;
        _data[pos].~T();
        T* newData = (T*)malloc(_size * sizeof(T));
        memcpy(newData, _data, pos * sizeof(T));
        memcpy(&newData[pos], &_data[pos + 1], (_size - pos) * sizeof(T));
        free(_data);
        _data = newData;
    }

    T& operator[](unsigned int index) {
        if (index < 0 || index >= _size || _data == nullptr) {
            if (!CONNECTED)
                BEGIN_DEBUG(9600);
            ERROR("Index out of Range!");
        }
        return _data[index];
    }
    T operator[](unsigned int index) const {
        if (index < 0 || index >= _size || _data == nullptr) {
            if (!CONNECTED)
                BEGIN_DEBUG(9600);
            ERROR("Index out of Range!");
        }
        return _data[index];
    }
    void operator=(const Array<T>& array) {
        if (this == &array)
            return;
        
        for (unsigned int i = 0; i < _size; i++)
            _data[i].~T();
        if (_data != nullptr)
            free(_data);

        if (array._size == 0) {
            _data = nullptr;
            _size = 0;
            return;
        }

        _data = (T*)malloc(array._size * sizeof(T));
        _size = array._size;

        for (unsigned int i = 0; i < _size; i++)
            _data[i] = T(array._data[i]);
    }
};

#endif