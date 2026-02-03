#ifndef _MAIN_STATES_H_
#define _MAIN_STATES_H_

// steps that happen once in the start:
//      - initialize all of the peripherals

// steps that happen every loop:
//      - update all of the sensors (mainly falgs, because not everything is necessary)

enum MainStates {
    // main steps
    REINIT,         // reinit all peripherals
    DISCOVER,       // discover the current tile and add it to mapping
    GET_ACTIONS,    // check if new actions list is needed and get the data from mapping
    DRIVE,          // drive the current action
    TURN            // turn according to the current action
};

#endif