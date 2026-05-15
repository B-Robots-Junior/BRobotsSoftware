#ifndef _MAIN_STATES_H_
#define _MAIN_STATES_H_

// steps that happen once in the start:
//      - initialize all of the peripherals

// steps that happen every loop:
//      - update all of the sensors (mainly falgs, because not everything is necessary)

enum MainStates {
    // main steps
    GET_MOVE,           // get the next move
    REINIT,             // reinit all peripherals
    START_TURN,         // start turn according to the current action
    TURN,               // continue turning according to the current action
    START_DRIVE,        // start drive according to the current action
    DRIVE,              // continue driving according to the current action
    ENSURE_HEADING,     // correct any heading driving errors, just to be safe
    CHECK_BLUE,         // check if on a blue tile
    BLUE_TILE_STOP,     // stop in a blue tile
    COMPLETE_MOVE,      // complete the current move
    BLACK_IR,           // when a black tile interrupt gets triggered
    BLACK_DRIVE,        // the drive out of the black tile (der hat wohl kein ticket gekauft)
    BLACK_CENTER,       // center to the last tile
    BLACK_TURN,         // turn 180 deg after driving out of black tile
    RESET_STATE,        // simply a paused state caused by an isr and removed by the same isr
    EXIT_RESET_STATE,   // simply a state to exit the RESET_STATE and reset the mapper for example
    CAMERA_DETECTION,   // when the camera detected something
    RIGHT_BUMPER,       // when the right bumper triggered
    LEFT_BUMPER         // when the left bumper triggered
};

#endif