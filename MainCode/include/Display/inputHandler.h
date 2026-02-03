#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include <Display/menu.h>
#include <Display/display.h>
#include <RotaryEncoder.h>
#include <PinChangeInterrupt.h>

class Menu;
class Display;

// class to inherit from when writing an input handler
class InputHandler {
public:
    InputHandler() {}

    virtual void update(Display* displayParent, Menu* menuParent) = 0;
};

class EncoderInput: public InputHandler {
protected:
    int16_t _lastValue;
    bool pressed = false;

public:
    EncoderInput(uint8_t pin_a, uint8_t pin_b, uint8_t button_pin) : InputHandler(), encoder(pin_a, pin_b) {}
    EncoderInput(uint8_t pin_a, uint8_t pin_b, uint8_t button_pin, void (*intFunc)(), void (*butInt)()) : InputHandler(), encoder(pin_a, pin_b) {
        attachPCINT(digitalPinToPCINT(pin_a), intFunc, CHANGE);
        attachPCINT(digitalPinToPCINT(pin_b), intFunc, CHANGE);
        attachPCINT(digitalPinToPCINT(button_pin), butInt, FALLING);
    }

    RotaryEncoder encoder;

    void buttonInt() {pressed = true;}
    void update(Display* displayParent, Menu* menuParent) override;
};

#define GEN_ENCODER_INPUT(name, pinA, pinB, buttonPin) \
    void name##PCINT(); \
    void name##ButtonInt(); \
    EncoderInput name(pinA, pinB, buttonPin, name##PCINT, name##ButtonInt); \
    void name##PCINT() {name.encoder.tick();} \
    void name##ButtonInt() {name.buttonInt();}

#endif