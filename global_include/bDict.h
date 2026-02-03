#ifndef _BINARY_DICTIONARY_H_
#define _BINARY_DICTIONARY_H_

#include <stdint.h>

template <typename Key, typename Val>
struct KeyValPair {
    Key key;
    Val val;
};

template <typename Key, typename Val>
class DictIndex;

template <typename Key, typename Val>
class BinaryDict {
private:
    uint16_t _size = 0;
    KeyValPair<Key, Val>* _data = nullptr;

    bool _find(uint16_t& index, const Key& key); // returns if it is contained and where it would be
    void _insertAt(const Key& key, const Val& val, uint16_t index);

public:
    BinaryDict() {}
    BinaryDict(const BinaryDict<Key, Val>& other) {
        _size = other._size;
        _data = new KeyValPair<Key, Val>[other._size];
        for (uint16_t i = 0; i < _size; i++)
            _data[i] = other._data[i];
    }
    BinaryDict(BinaryDict<Key, Val>&& other) {
        _size = other._size;
        _data = other._data;
        other._size = 0;
        other._data = nullptr;
    }

    ~BinaryDict() {
        delete[] _data;
    }

    uint16_t size() const {return _size;}

    Val& getVal(const Key& key);
    void setVal(const Key& key, const Val& val);

    uint16_t getIndex(const Key& key);
    Val& getAtIndex(uint16_t index);
    Key& getKeyAtIndex(uint16_t index);
    void setAtIndex(uint16_t index, const Val& val);

    void remove(const Key& key);
    void removeIndex(uint16_t index);

    DictIndex<Key, Val> operator[](const Key& key);

    BinaryDict<Key, Val>& operator=(BinaryDict<Key, Val>& other) {
        if (this == &other)
            return *this;
        delete[] _data;
        _size = other._size;
        _data = new KeyValPair<Key, Val>[other._size];
        for (uint16_t i = 0; i < _size; i++)
            _data[i] = other._data[i];
        return *this;
    }

    BinaryDict<Key, Val>& operator=(BinaryDict<Key, Val>&& other) {
        if (this == &other)
            return *this;
        delete[] _data;
        _size = other._size;
        _data = other._data;
        other._size = 0;
        other._data = nullptr;
        return *this;
    }
};

template <typename Key, typename Val>
class DictIndex {
private:
    Key _key;
    BinaryDict<Key, Val>& _parent;

public:
    DictIndex(const Key& key, BinaryDict<Key, Val>& parent) : _key(key), _parent(parent) {}

    operator Val&() {
        return _parent.getVal(_key);
    }

    DictIndex<Key, Val>& operator=(const Val& val) {
        _parent.setVal(_key, val);
        return *this;
    }
};

// ----------------------------------------------------------------------------------------------------
// private methods
// ----------------------------------------------------------------------------------------------------

template <typename Key, typename Val>
bool BinaryDict<Key, Val>::_find(uint16_t& index, const Key& key) {
    uint16_t left = 0;
    uint16_t right = _size;

    while (left < right) {
        uint16_t mid = left + (right - left) / 2;

        if (_data[mid].key < key)
            left = mid + 1;
        else
            right = mid;
    }

    index = left;

    if (index < _size && _data[index].key == key)
        return true;

    return false;
}

template <typename Key, typename Val>
void BinaryDict<Key, Val>::_insertAt(const Key& key, const Val& val, uint16_t index) {
    if (_size > 0 && index < _size) {
        if (key > _data[index].key)
            index += 1;
        else if (_data[index].key == key) {
            _data[index].val = val;
            return;
        }
    }
    KeyValPair<Key, Val>* newData = new KeyValPair<Key, Val>[_size + 1];

    for (uint16_t i = 0; i < index; i++)
        newData[i] = _data[i];

    newData[index].key = key;
    newData[index].val = val;

    for (uint16_t i = index; i < _size; i++)
        newData[i + 1] = _data[i];

    delete[] _data;
    _data = newData;
    _size++;
}

// ----------------------------------------------------------------------------------------------------
// public methods
// ----------------------------------------------------------------------------------------------------

template <typename Key, typename Val>
Val& BinaryDict<Key, Val>::getVal(const Key& key) {
    uint16_t index = 0;
    _find(index, key);
    return _data[index].val; //! add error handling for invalid indexes
}

template <typename Key, typename Val>
void BinaryDict<Key, Val>::setVal(const Key& key, const Val& val) {
    uint16_t index = 0;
    _find(index, key);
    _insertAt(key, val, index);
}

template <typename Key, typename Val>
uint16_t BinaryDict<Key, Val>::getIndex(const Key& key) {
    uint16_t index = 0;
    _find(index, key);
    return index; //! add error handling for invalid indexes
}

template <typename Key, typename Val>
Val& BinaryDict<Key, Val>::getAtIndex(uint16_t index) {
    return _data[index].val; //! add error handling for invalid indexes
}

template <typename Key, typename Val>
Key& BinaryDict<Key, Val>::getKeyAtIndex(uint16_t index) {
    return _data[index].key; //! add error handling for invalid indexes
}

template <typename Key, typename Val>
void BinaryDict<Key, Val>::setAtIndex(uint16_t index, const Val& val) {
    return _data[index].val = val; //! add error handling for invalid indexes
}

template <typename Key, typename Val>
void BinaryDict<Key, Val>::remove(const Key& key) {
    uint16_t index = 0;
    if (!_find(index, key))
        return;
    if (index >= _size)
        return;
    removeIndex(index);
}

template <typename Key, typename Val>
void BinaryDict<Key, Val>::removeIndex(uint16_t index) {
    KeyValPair<Key, Val>* newData = new KeyValPair<Key, Val>[_size - 1];

    for (uint16_t i = 0; i < index; i++)
        newData[i] = _data[i];
    for (uint16_t i = index + 1; i < _size; i++)
        newData[i - 1] = _data[i];
    
    delete[] _data;
    _data = newData;
    _size--;
}

template <typename Key, typename Val>
DictIndex<Key, Val> BinaryDict<Key, Val>::operator[](const Key& key) {
    return DictIndex<Key, Val>(key, *this);
}

#endif // _BINARY_DICTIONARY_H_