#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <errorHandling.h>

template <typename T>
class QueueLink {
public:
    QueueLink(const T& data, QueueLink<T>* next) : data(data), next(next) {}

    T data;
    QueueLink<T>* next;
};

template <typename T>
class Queue {
private:
    QueueLink<T>* _first = nullptr;
    QueueLink<T>* _last = nullptr;

    void _freeAll() {
        while (_first != nullptr) {
            QueueLink<T>* next = _first->next;
            delete _first;
            _first = next;
        }
        _last = nullptr;
    }

    void _copyQueue(const Queue<T>& other) {
        _freeAll();
        if (other._first == nullptr)
            return;
        QueueLink<T>* lastNew = nullptr;
        QueueLink<T>* currNew = new QueueLink<T>(other._first->data, nullptr);
        _first = currNew;
        QueueLink<T>* currNode = other._first;
        while (currNode != other._last)
        {
            currNode = currNode->next;
            lastNew = currNew;
            currNew = new QueueLink<T>(currNode->data, nullptr);
            lastNew->next = currNew;
        }
        currNew->next = nullptr;
        _last = currNew;
    }

public:
    Queue() {}
    Queue(const Queue<T>& other) {
        _copyQueue(other);
    }
    Queue(Queue<T>&& other) : _first(other._first), _last(other._last) {
        other._first = nullptr;
        other._last = nullptr;
    }
    ~Queue() {
        _freeAll();
    }

    bool empty() const {return _first == nullptr;}
    void appendData(const T& data) {
        if (_last == nullptr) {
            _last = new QueueLink<T>(data, nullptr);
            _first = _last;
            return;
        }

        QueueLink<T>* newLink = new QueueLink<T>(data, nullptr);
        _last->next = newLink;
        _last = newLink;
    }
    Likely<T> popFront() {
        if (_first == nullptr)
            return Likely<T>(String(F("Queue is Empty No data to pop!")));
        
        QueueLink<T>* next = _first->next;
        Likely<T> ret(_first->data);
        delete _first;
        _first = next;
        if (_first == nullptr)
            _last = nullptr;
        return ret;
    }
    QueueLink<T>* getFirst() {return _first;}
    QueueLink<T>* getLast() {return _last;}

    Queue<T>& operator=(const Queue<T>& other) {
        if (this != &other)
            _copyQueue(other);
        return this;
    }

    Queue<T>& operator=(Queue<T>&& other) {
        if (this != &other) {
            _first = other._first;
            _last = other._last;
            other._first = nullptr;
            other._last = nullptr;
        }
        return this;
    }
};

#endif