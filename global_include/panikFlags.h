#ifndef PANIK_FLAGS_H
#define PANIK_FLAGS_H

// Singelton of one time triggered panik flags
class PanikFlags {
public:
    static PanikFlags& getInstance() {
        static PanikFlags instance;
        return instance;
    }

    PanikFlags(const PanikFlags&) = delete;
    PanikFlags(PanikFlags&&) = delete;

    bool outOfRam() { return _outOfRam; }
    void triggerOutOfRam() {
        ERROR_MINOR("Ran out of RAM!", SET_RED);
        _outOfRam = true;
    }

    PanikFlags& operator=(const PanikFlags& other) = delete;
    PanikFlags& operator=(PanikFlags&& other) = delete;

private:
    PanikFlags() = default;
    ~PanikFlags() = default;

    bool _outOfRam = false;
};

#endif