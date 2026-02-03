#include <Display/inputHandler.h>

// ----------------------------------------------------------------------------------------------------
// EncoderInput class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

void EncoderInput::update(Display* displayParent, Menu* menuParent) {
    int16_t pos = encoder.getPosition();
    if (_lastValue != pos) {
        displayParent->currMenu->selectNextBy(_lastValue - pos);
        _lastValue = pos;
    }
    if (pressed) {
        pressed = false;
        displayParent->runCurrentSelectable();
    }
}