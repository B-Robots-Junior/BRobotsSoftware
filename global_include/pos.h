#pragma once

#ifndef _POS_H_
#define _POS_H_

#include <Arduino.h>
#include <debug.h>

// ----------------------------------------------------------------------------------------------------
// Pos
// ----------------------------------------------------------------------------------------------------

template <typename T = int16_t>
class Pos {
public:
    Pos(T x = 0, T y = 0) : x(x), y(y) {}

    T x, y;

    Pos<T> operator+(const Pos<T>& other) const {
        return Pos<T>(x + other.x, y + other.y);
    }
    Pos<T> operator-(const Pos<T>& other) const {
        return Pos<T>(x - other.x, y - other.y);
    }
    template <typename C>
    Pos<T> operator*(const C& other) const {
        return Pos<T>(x * other, y * other);
    }
    template <typename C>
    bool operator==(const Pos<C>& other) const {
        return x == other.x && y == other.y;
    }
    template <typename C>
    bool operator!=(const Pos<C>& other) const {
        return !operator==<C>(other);
    }
    template <typename C>
    bool operator>(const Pos<C>& other) const {
        if (y > other.y) return true;
        else if (y < other.y) return false;
        if (x > other.x) return true;
        return false;
    }
    template <typename C>
    bool operator>=(const Pos<C>& other) const {
        return operator>(other) && operator==(other);
    }
    template <typename C>
    bool operator<(const Pos<C>& other) const {
        return other > *this;
    }
    template <typename C>
    bool operator<=(const Pos<C>& other) const {
        return operator<(other) && operator==(other);
    }

    void print() const {
        DB_PRINT_MUL(("(")(x)(", ")(y)(")"));
    }
    void println() const {
        print();
        DB_PRINTLN();
    }
};

template <typename T, typename C>
inline Pos<T> operator*(const C& val, const Pos<T>& pos) {
    return Pos<T>(pos.x * val, pos.y * val);
}

// ----------------------------------------------------------------------------------------------------
// Pos3 class
// ----------------------------------------------------------------------------------------------------

template <typename T = int16_t>
class Pos3 {
public:
    Pos3(T x = 0, T y = 0, T z = 0) : x(x), y(y), z(z) {}

    T x, y, z;

    Pos3<T> operator+(const Pos3<T>& other) const {
        return Pos3<T>(x + other.x, y + other.y, z + other.z);
    }
    Pos3<T> operator-(const Pos3<T>& other) const {
        return Pos3<T>(x - other.x, y - other.y, z - other.z);
    }
    template <typename C>
    Pos3<T> operator*(const C& other) const {
        return Pos3<T>(x * other, y * other, z * other.z);
    }
    template <typename C>
    bool operator==(const Pos3<C>& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    template <typename C>
    bool operator!=(const Pos3<C>& other) const {
        return !operator==<C>(other);
    }
    template <typename C>
    bool operator>(const Pos3<C>& other) const {
        if (z > other.z) return true;
        else if (z < other.z) return false;
        if (y > other.y) return true;
        else if (y < other.y) return false;
        if (x > other.x) return true;
        return false;
    }
    template <typename C>
    bool operator>=(const Pos3<C>& other) const {
        return operator>(other) && operator==(other);
    }
    template <typename C>
    bool operator<(const Pos3<C>& other) const {
        return other > *this;
    }
    template <typename C>
    bool operator<=(const Pos3<C>& other) const {
        return operator<(other) && operator==(other);
    }

    void print() const {
        DB_PRINT_MUL(("(")(x)(", ")(y)(", ")(z)(")"));
    }
    void println() const {
        print();
        DB_PRINTLN();
    }
};

template <typename T, typename C>
inline Pos3<T> operator*(const C& val, const Pos3<T>& pos) {
    return Pos3<T>(pos.x * val, pos.y * val, pos.z * val);
}

#endif