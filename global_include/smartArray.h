#ifndef SMART_ARRAY_H
#define SMART_ARRAY_H

#include <Arduino.h>
#include <new>
#include <panikFlags.h>
#include <errorHandling.h>
#include <utility.h>

// Types used in this array should optimally include the following function (if you don't know what they do ask)
//\     T()
//\     ~T()
//\     T& operator=(const T& other)
//\     T& operator=(T&& other)
//! Only use unsigned integers for Size type
template <typename T, typename Size, typename GrowPolicy>
class SmartArray {
public:
    SmartArray();
    template <typename... Args>
    SmartArray(Args... args);
    SmartArray(const SmartArray<T, Size, GrowPolicy>& other);
    SmartArray(SmartArray<T, Size, GrowPolicy>&& other);
    ~SmartArray() { _free(); }

    Size size() const { return _size; } // returns the number of elements
    Size dataSize() const { return _dataSize; } // returns the number of elements before the array gets expanded
    void push_back(const T& element);
    void pop_back();
    void insert(Size i, const T& element);
    void remove(Size i);
    void remove_after(Size i); // removes all elements after the given index

    T& operator[](Size i); //! this will fail silently if not in DEBUG_MODE, where it will fail and block
    const T& operator[](Size i) const; //! this will fail silently if not in DEBUG_MODE, where it will fail and block

    SmartArray<T, Size, GrowPolicy>& operator=(const SmartArray<T, Size, GrowPolicy>& other);
    SmartArray<T, Size, GrowPolicy>& operator=(SmartArray<T, Size, GrowPolicy>&& other);

private:
    T* _data = nullptr;
    Size _dataSize;
    Size _size;
    GrowPolicy _growPolicy;

    void _free();
    void _expand();
};

template<typename Size, Size buffer>
struct GrowConstBuffer {
    Size operator()(Size s) const {
        Size newSize = s + buffer;
        if (newSize < s)
            return s;
        return newSize;
    }
};

// growth is size_n = size_(n - 1) + size_(n - 1) / div
template <typename Size, Size base, int div>
struct GrowExponential {
    static_assert(div > 0, "GrowExponential div must be > 0");

    Size operator()(Size s) const {
        if (s == 0)
            return base;
        Size inc = s / div;
        if (inc == 0)
            inc = 1;
        Size newSize = s + inc;
        if (newSize < s)
            return s;
        return newSize;
    }
};

template <typename T, typename S, int growth>
using ConstGrowthArray = SmartArray<T, S, GrowConstBuffer<S, growth>>;

template <typename T>
using SimpleArray = ConstGrowthArray<T, uint16_t, 5>;

template <typename T, typename S, S base, int div>
using ExpArray = SmartArray<T, S, GrowExponential<S, base, div>>;

template <typename T>
using FastArray = ExpArray<T, uint16_t, 4, 2>; // grows approximately with size = 1.5^x * 4 (or just 1.5x per expasion)

// ----------------------------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// constructors / destructor
// --------------------------------------------------

template <typename T, typename Size, typename GrowPolicy>
SmartArray<T, Size, GrowPolicy>::SmartArray() : _data(nullptr), _dataSize(0), _size(0), _growPolicy() {}

template <typename T, typename Size, typename GrowPolicy>
template <typename... Args>
SmartArray<T, Size, GrowPolicy>::SmartArray(Args... args)
    : _data(nullptr), _dataSize(0), _size(sizeof...(args)), _growPolicy() {
    if (_size == 0)
        return;

    while (_dataSize < _size) {
        Size next = _growPolicy(_dataSize);
        if (next <= _dataSize) {
            _dataSize = _size;
            break;
        }
        _dataSize = next;
    }

    _data = new(std::nothrow) T[_dataSize];
    if (_data == nullptr) {
        PanikFlags::getInstance().triggerOutOfRam();
        _size = 0;
        _dataSize = 0;
        return;
    }

    T values[] = {args...};
    for (Size i = 0; i < _size; i++)
        _data[i] = move(values[i]);
}

template <typename T, typename Size, typename GrowPolicy>
SmartArray<T, Size, GrowPolicy>::SmartArray(const SmartArray<T, Size, GrowPolicy>& other)
    : _data(nullptr), _dataSize(other._dataSize), _size(other._size), _growPolicy(other._growPolicy) {
    _data = new(std::nothrow) T[_dataSize];
    if (_data == nullptr) {
        PanikFlags::getInstance().triggerOutOfRam();
        return;
    }
    for (Size i = 0; i < _size; i++)
        _data[i] = other._data[i];
}

template <typename T, typename Size, typename GrowPolicy>
SmartArray<T, Size, GrowPolicy>::SmartArray(SmartArray<T, Size, GrowPolicy>&& other)
    : _data(other._data), _dataSize(other._dataSize), _size(other._size), _growPolicy(other._growPolicy) {
    other._data = nullptr;
    other._dataSize = 0;
    other._size = 0;
    other._growPolicy = GrowPolicy();
}

// --------------------------------------------------
// public methods
// --------------------------------------------------

template <typename T, typename Size, typename GrowPolicy>
void SmartArray<T, Size, GrowPolicy>::push_back(const T& element) {
    if (PanikFlags::getInstance().outOfRam()) {
        _free();
        return;
    }
    if (_dataSize == _size)
        _expand();
    if (PanikFlags::getInstance().outOfRam() || _dataSize == _size)
        return;
    _data[_size] = element;
    _size++;
}

template <typename T, typename Size, typename GrowPolicy>
void SmartArray<T, Size, GrowPolicy>::pop_back() {
    if (PanikFlags::getInstance().outOfRam()) {
        _free();
        return;
    }
    if (_size == 0) {
        ERROR_MINOR(F("Can't pop element from empty Array"), SET_RED);
        return;
    }
    _size--;
    _data[_size] = T();
}

template <typename T, typename Size, typename GrowPolicy>
void SmartArray<T, Size, GrowPolicy>::insert(Size i, const T& element) {
    if (PanikFlags::getInstance().outOfRam()) {
        _free();
        return;
    }
    if (i > _size) {
        ERROR_MINOR(F("Trying to insert out of bounds!"), SET_RED);
        return;
    }
    if (_size == _dataSize)
        _expand();
    if (PanikFlags::getInstance().outOfRam() || _dataSize == _size)
        return;
    for (Size j = _size; j > i; j--)
        _data[j] = move(_data[j - 1]);
    _data[i] = element;
    _size++;
}

template <typename T, typename Size, typename GrowPolicy>
void SmartArray<T, Size, GrowPolicy>::remove(Size i) {
    if (PanikFlags::getInstance().outOfRam()) {
        _free();
        return;
    }
    if (i >= _size) {
        ERROR_MINOR(F("Trying to remove out of bounds!"), SET_RED);
        return;
    }
    _size--;
    for (Size j = i; j < _size; j++)
        _data[j] = move(_data[j + 1]);
    _data[_size] = T();
}

template <typename T, typename Size, typename GrowPolicy>
void SmartArray<T, Size, GrowPolicy>::remove_after(Size i) {
    if (PanikFlags::getInstance().outOfRam()) {
        _free();
        return;
    }
    if (i >= _size) {
        ERROR_MINOR(F("Given index of remove_after is out of bounds!"), SET_RED);
        return;
    }
    for (Size j = i; j < _size; j++)
        _data[j] = T();
    _size = i + 1;
}

// --------------------------------------------------
// operator overloads
// --------------------------------------------------

template <typename T, typename Size, typename GrowPolicy>
T& SmartArray<T, Size, GrowPolicy>::operator[](Size i) {
#ifdef DEBUG_MODE
    if (i >= _size || _data == nullptr)
        ERROR(F("Index out of range!"));
#endif
    return _data[i];
}

template <typename T, typename Size, typename GrowPolicy>
const T& SmartArray<T, Size, GrowPolicy>::operator[](Size i) const {
#ifdef DEBUG_MODE
    if (i >= _size || _data == nullptr)
        ERROR(F("Index out of range!"));
#endif
    return _data[i];
}

template <typename T, typename Size, typename GrowPolicy>
SmartArray<T, Size, GrowPolicy>& SmartArray<T, Size, GrowPolicy>::operator=(const SmartArray<T, Size, GrowPolicy>& other) {
    if (this != &other) {
        _free();
        _dataSize = other._dataSize;
        _size = other._size;
        _growPolicy = other._growPolicy;
        _data = new(std::nothrow) T[other._dataSize];
        if (_data == nullptr) {
            PanikFlags::getInstance().triggerOutOfRam();
            return *this;
        }
        for (Size i = 0; i < other._size; i++)
            _data[i] = other._data[i];
    }
    return *this;
}

template <typename T, typename Size, typename GrowPolicy>
SmartArray<T, Size, GrowPolicy>& SmartArray<T, Size, GrowPolicy>::operator=(SmartArray<T, Size, GrowPolicy>&& other) {
    if (this != &other) {
        _free();
        _data = other._data;
        _dataSize = other._dataSize;
        _size = other._size;
        _growPolicy = other._growPolicy;
        other._data = nullptr;
        other._dataSize = 0;
        other._size = 0;
        other._growPolicy = GrowPolicy();
    }
    return *this;
}

// --------------------------------------------------
// private methods
// --------------------------------------------------

template <typename T, typename Size, typename GrowPolicy>
void SmartArray<T, Size, GrowPolicy>::_free() {
    if (_data)
        delete[] _data;
    _data = nullptr;
    _dataSize = 0;
    _size = 0;
}

template <typename T, typename Size, typename GrowPolicy>
void SmartArray<T, Size, GrowPolicy>::_expand() {
    Size newSize = _growPolicy(_dataSize);
    if (newSize <= _dataSize)
        return;
    T* newData = new(std::nothrow) T[newSize];
    if (newData == nullptr) {
        PanikFlags::getInstance().triggerOutOfRam();
        _free();
        return;
    }
    for (Size i = 0; i < _size; i++)
        newData[i] = move(_data[i]);
    delete[] _data;
    _data = newData;
    _dataSize = newSize;
}

#endif