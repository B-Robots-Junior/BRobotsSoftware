#ifndef _MAIN_STATES_H_
#define _MAIN_STATES_H_

// steps that happen once in the start:
//      - initialize all of the peripherals

// steps that happen every loop:
//      - update all of the sensors (mainly falgs, because not everything is necessary)

enum MainStates {
    // main steps
    GET_MOVE,       // get the next move
    REINIT,         // reinit all peripherals
    START_TURN,     // start turn according to the current action
    TURN,           // continue turning according to the current action
    START_DRIVE,    // start drive according to the current action
    DRIVE,          // continue driving according to the current action
    COMPLETE_MOVE   // complete the current move
};

#endif