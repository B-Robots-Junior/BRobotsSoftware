#include <Arduino.h>
#include <debug.h>
#include <errorHandling.h>
#include <mainStates.h>
#include <array.h>
#include <Encoders/encoders.h>
#include <PosSensors/position.h>
#include <devices.h>
#include <Mapping/mapping.h>
#include <ColorSensors/spectrometer.h>
#include <util/atomic.h>
#include <PinChangeInterrupt.h>

#include <config.h>

#define USE_main true
#if CAT(USE_, CURR_MAIN)
#undef USE_main

void setMainState(MainStates state, MainStates currentCase);
void mainFunc();
void resetInterrupt();
void bumperInterruptRight();
void bumperInterruptLeft();
bool initEverything();
void rgbcSensorOnEnter();
void rgbcSensorOnExit();
bool isRamp();
uint8_t getTileType();
bool getUp();
bool rampInfront();
uint16_t getHeapUsage();
void triggerPackageThrow(RaspiEvent detection);

#define CALIB_LED 30
#define MAIN_LED 42

#define BUTTON1 PIN_A10 // BUTTON_PIN_3
#define BUTTON2 PIN_A11 // BUTTON_PIN_4
#define BUTTON3 PIN_A8 // BUTTON_PIN_1
#define BUTTON4 PIN_A9 // BUTTON_PIN_2

#define BUMPER1 2
#define BUMPER2 3

// ----------------------------------------------------------------------------------------------------
// global data:

// mainState is accessed by isrs, so have caution when setting it
volatile MainStates mainState = MainStates::GET_MOVE;
bool inBlackTile = false;

// bumper stuff
uint32_t bumperStartTime = millis();

// ----------------------------------------------------------------------------------------------------

int main() {
    init();
    sei();

    BEGIN_DEBUG(BAUDE_RATE);

    pinMode(MAIN_LED, OUTPUT);
    pinMode(CALIB_LED, OUTPUT);

    pinMode(BUTTON1, INPUT);
    pinMode(BUTTON2, INPUT);
    pinMode(BUTTON3, INPUT);
    pinMode(BUTTON4, INPUT);
    
    digitalWrite(CALIB_LED, LOW);
    digitalWrite(MAIN_LED, LOW);

    DB_PRINTLN("Start Main!");

    if (!initEverything())
        mainState = MainStates::REINIT;

    Devices::ledsBottom.fill(0xFF000000);
    Devices::ledsBottom.show();

    Devices::ledsTop.fill(0x40000000); // (0x40000000);
    Devices::ledsTop.show();

    for (uint8_t type = 0; type < 5; type++) {
        DB_PRINT_MUL((SET_GREEN)(F("Calibrate "))(colorTypeToName[type])('!')(RESET_COLOR)('\n'));
        do { DB_PRINT(SET_BLUE); DB_PRINT(F("Breakpoint in file: ")); DB_PRINT(F(__FILE__)); DB_PRINT(F(" in line: ")); DB_PRINT(__LINE__); DB_PRINTLN(F(" Triggered!")); DB_PRINT(RESET_COLOR); while (!Serial.available()) {} delay(100); while (Serial.available()) { Serial.read(); } } while (0);
        //Devices::refSensor.calibrate(ColorType(type));
        Devices::rgbcSensor.setColor(ColorType(type));
        Devices::spec.setColorCurrent(ColorType(type));
    }

    while (!digitalRead(BUTTON1)) {
        if (getTofLBValid())
            digitalWrite(CALIB_LED, HIGH);
        else
            digitalWrite(CALIB_LED, LOW);
    } // wait until start
    while (digitalRead(BUTTON1)) {
        if (getTofLBValid())
            digitalWrite(CALIB_LED, HIGH);
        else
            digitalWrite(CALIB_LED, LOW);
    } // wait until button unpressed

    delay(100); // simply that the reset does not trigger accidentally

    attachPCINT(digitalPinToPCINT(BUTTON1), resetInterrupt, FALLING); // atach reset button

    //attachInterrupt(digitalPinToInterrupt(BUMPER1), bumperInterruptRight, FALLING);
    //attachInterrupt(digitalPinToInterrupt(BUMPER2), bumperInterruptLeft, FALLING);

    BREAK;

    mainFunc();

    while (true) {}
}

// ----------------------------------------------------------------------------------------------------

// use this to set the main state in the while loop to avoid race conditions or conflicts
// isrs can set the mainState directly, because they allready block other isrs and can access it independent of the case
void setMainState(MainStates state, MainStates currentCase) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // a state transition may only access the main state if the current case is the current state
        // to avoid interrupt state changes to get overridden
        if (mainState == currentCase)
            mainState = state;
    }
}

void uncondSetMainState(MainStates state) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        mainState = state;
    }
}

void mainFunc() {

    // ----------------------------------------------------------------------------------------------------
    // initialize all the needed data:

    Mapper mapper;
    Move currMove;

    // turn data:
    float targetTurnAngle = 0;
    uint32_t turnStartTime = 0;
    float turnTol = 1;

    // drive data:
    int targetFrontDist = 0;
    int targetBackDist = 0;
    float targetDriveAngle = 0;
    int64_t targetEncoderDist = 0;
    int64_t lastEncoderDist = 0;
    int64_t trueEncoderDist = 0;

    // blue tile interrupt data:
    bool inBlueTile = false;
    bool stoppedInTile = false;
    int64_t blueTileLastMillis = millis();
    uint32_t blueTileStopStartTime = 0;
    uint32_t blueTileToleranceStarTime = 0;
    // the color sensor should check continuously if we are in a blue tile if so it should
    // set the inBlueTile flag correctly and if you enter a blue tile set the blueTileStartEncoderDist
    // so that when the encoder dist is greater than 150 + blueTileStartEncoderDist && !stoppedInTile you can stop for 5 seconds
    // and change the flag, also if the encoder dist is grater than 300 + blueTileStartEncoderDist then you can reset the
    // stoppedInTile flag and reset blueTileStartEncoderDist to the current encoder dist
    // encoders should be good enough for one or two tiles, so I think this is fine

    // camera state stuff
    MainStates mainStateBeforeCamInt = mainState;
    uint32_t cameraStartTime = millis();

    // ----------------------------------------------------------------------------------------------------
    // initialize all sensors:
    
    BREAK;

    //mapper.panicMode();
    
    BREAK;

    //saveToEEPROM<uint64_t>(256, 0); // num loops
    //saveToEEPROM<uint64_t>(256 + sizeof(uint64_t), 0); // sum durs
    //saveToEEPROM<uint64_t>(256 + sizeof(uint64_t) * 2, 0xFFFFFFFF); // min dur
    //saveToEEPROM<uint64_t>(256 + sizeof(uint64_t) * 3, 0); // max dur

    // required or else it won't have a value for the first tile and it will be balck and the maze will be instantly "solved"
    Devices::spec.forceUpdate();

    bool running = true;
    while (running) 
    {
        // uint64_t loopStart = millis();

        // ----------------------------------------------------------------------------------------------------
        // update all sensors:

        updateTofs();
        gyro.update();
        Devices::spec.update();
        Devices::packageHandlerLeft.update();
        Devices::packageHandlerRight.update();
        if (getTofLBValid())
            digitalWrite(CALIB_LED, HIGH);
        else
            digitalWrite(CALIB_LED, LOW);

        // blue tile stuff:
        ColorType currSpecType = Devices::spec.getColorId();
        if (currSpecType == ColorType::Blue && !inBlueTile) {
            // entered a blue tile, reset flags:
            inBlueTile = true;
            blueTileLastMillis = millis();
            stoppedInTile = false;
        } else if (currSpecType != ColorType::Blue && inBlueTile) {
            // exited a blue tile, set flags accordingly
            inBlueTile = false;
            stoppedInTile = false;
        }

        if (inBlueTile && (millis() - blueTileLastMillis) > 1000) {
            blueTileLastMillis = millis();
            stoppedInTile = false;
        }

        // ----------------------------------------------------------------------------------------------------
        // get current tile
        // mapPos pos = mapper.pos;

        // ----------------------------------------------------------------------------------------------------
        // update camera:

        RaspiEvent raspiEvent = Devices::comms.update(getRFDistance(), getRBDistance(), getLFDistance(), getLBDistance());
        while (raspiEvent != RaspiEvent::NO_MORE_PACKETS) {
            raspiEvent = Devices::comms.update(getRFDistance(), getRBDistance(), getLFDistance(), getLBDistance());
#if USE_NEW_RASPI_COMMS
            if (raspiEvent >= RaspiEvent::DETECTED_PSI_RIGHT && raspiEvent <= RaspiEvent::DETECTED_RING_SUM_2_LEFT
                    && (getEncoderValueMM() - lastDetectionEncoderVal) >= CAM_DISTANCE_TIMEOUT_MM) {
                mainStateBeforeCamInt = mainState;
                uncondSetMainState(MainStates::CAMERA_DETECTION);
                cameraStartTime = millis();
                triggerPackageThrow(raspiEvent);
            }
#else
            if (raspiEvent >= RaspiEvent::DETECTED_H_RIGHT && raspiEvent <= RaspiEvent::DETECTED_RED_LEFT) {
                if ((millis() - cameraStartTime) >= 10000) {
                    Devices::comms.debugLog(F("Detected victim, switching state!"));
                    DB_COLOR_PRINTLN(F("Detected Victim, switching state!"), SET_GREEN);
                    mainStateBeforeCamInt = mainState;
                    uncondSetMainState(MainStates::CAMERA_DETECTION);
                    cameraStartTime = millis();
                    triggerPackageThrow(raspiEvent);
                }
            }
#endif
        }
        

        // ----------------------------------------------------------------------------------------------------

        switch (mainState)
        {

        // ----------------------------------------------------------------------------------------------------
        // reinit state:
        case MainStates::REINIT: {
            ERROR(F("Should not enter MainStates::REINIT in debug mode!"));

            if (initEverything())
                setMainState(MainStates::GET_MOVE, MainStates::REINIT);

            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // get the current move:
        case MainStates::GET_MOVE: {
            LACK;
            VAR_PRINTLN(wallFront());
            VAR_PRINTLN(wallRight());
            VAR_PRINTLN(wallLeft());
            VAR_PRINTLN(wallBack());
            VAR_PRINTLN(getUp());
            VAR_PRINTLN(getTileType());
            DB_PRINT_MUL(
                (F("Data:\n"))
                (F("    fl: "))(getFrontBottomShortDistance())(F(", f: "))(getFrontTopDistance())(F(", fr: "))(getFrontBottomLongDistance())('\n')
                (F("    lf: "))(getLFDistance())(F(", rf: "))(getRFDistance())('\n')
                (F("    lb: "))(getLBDistance())(F(", rb: "))(getRBDistance())('\n')
                (F("    l: "))(getLeftDistance())(F(", r: "))(getRightDistance())('\n')
                (F("    b: "))(getBackDistance())('\n')
            );

            DB_PRINT_MUL((F("StackPtr: "))((uint16_t)SP)(F(" HeapPtr: "))(getHeapUsage())(F(" Mapper Mem: "))((long)mapper.currentDynamicRamUsage())(F(" actions Data size: "))(mapper._actions.dataSize())('\n'));
            currMove = mapper.currMove(wallFront(), wallRight(), wallLeft(), wallBack(), getUp(), getTileType());
            DB_PRINT_MUL((F("StackPtr: "))((uint16_t)SP)(F(" HeapPtr: "))(getHeapUsage())(F(" Mapper Mem: "))((long)mapper.currentDynamicRamUsage())(F(" actions Data size: "))(mapper._actions.dataSize())('\n'));
            VAR_FUNC_PRINTLN(currMove);

            if (currMove.distance == 0 && currMove.rotation == 0) {
                DB_COLOR_PRINTLN(F("Completed the Maze YAY!"), SET_GREEN);
                running = false;
                break;
            }

            setMainState(MainStates::START_TURN, MainStates::GET_MOVE);
            BREAK;
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // start turn:
        case MainStates::START_TURN: {
            LACK;
            if (currMove.rotation == 0) {
                setMainState(MainStates::START_DRIVE, MainStates::START_TURN);
                LACK;
                BREAK;
                break;
            }

            LACK;
            targetTurnAngle = ReadGyroyaw() + angleDiffDEG(-90 * currMove.rotation, calculateposition());
            VAR_PRINTLN(targetTurnAngle);
            turnStartTime = millis();
            VAR_PRINTLN(turnStartTime);
            turnTol = 1;

            Devices::control.resetPIDs();

            setMainState(MainStates::TURN, MainStates::START_TURN);
            BREAK;
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // turn:
        case MainStates::TURN: {
            LACK;

            int ret = Devices::control.turnRobot(targetTurnAngle, turnStartTime, turnTol);
            if (ret == 0) {
                setMainState(MainStates::START_DRIVE, MainStates::TURN);
            } else if (ret == -1) {
                ERROR_MINOR(F("Devices::controll.turnRobot timeout occoured!"), SET_RED);
                turnStartTime = millis();
                turnTol *= 2; // increase tolerance and try agian.
            }

            // we don't want to count the encoders when turing so we skip the current delta
            if (inBlueTile) {
                blueTileLastMillis = millis();
            }
            
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // start drive:
        case MainStates::START_DRIVE: {
            if (currMove.distance == 0) {
                setMainState(MainStates::COMPLETE_MOVE, MainStates::START_DRIVE);
                BREAK;
                break;
            }

            bool drivinOnToRamp = rampInfront() && (currMove.distance * 300 >= getFrontBottomLongDistance());

            if (!getTofBValid() || drivinOnToRamp || isRamp()) {
                LACK;
                targetBackDist = -1; // can't rely on the back dist
            } else {
                LACK;
                // its hard to check if there is ramp behind you, so you add 300 to the back of what you currently have and call it a day
                // and let the front tof aling to the nearest tile, because it is prioritized and can detect ramps
                targetBackDist = getBackDistance() + 300;
                VAR_PRINTLN(targetBackDist);
            }

            if (!getTofFTValid() || drivinOnToRamp || isRamp()) {
                LACK;
                targetFrontDist = -1; // can't rely of on front dist
            } else {
                LACK;
                // if there is a ramp in front of you and you are not driving on it simply subtract 300, because you can't align to the nearest tile like that
                VAR_PRINTLN(rampInfront());
                if (rampInfront()) {
                    targetFrontDist = getFrontTopDistance() - 300 * currMove.distance;
                } else { // if there is no ramp infront of you align to the full tile
                    float currentTilesFrontFloat = getFrontTopDistance() / 300.0;
                    int currentTilesFront = currentTilesFrontFloat;
                    // if there is more than 90% of a tile there you can count is as a full tile (to avoid uneccecary panic modes)
                    if (currentTilesFrontFloat - currentTilesFront >= 0.9) 
                        currentTilesFront++;
                    if (currMove.distance > currentTilesFront) {
                        ERROR_MINOR(F("Mapper gave me bullshit data going into panic mode!"), SET_RED);
                        mapper.panicMode();
                        setMainState(MainStates::GET_MOVE, MainStates::START_DRIVE);
                        BREAK;
                        break;
                    }
                    targetFrontDist = (currentTilesFront - currMove.distance) * 300 + FRONT_WALL_DIST_MM;
                }
                VAR_PRINTLN(targetFrontDist);
            }


            targetEncoderDist = 300 * currMove.distance;
            lastEncoderDist = getEncoderValueMM();
            trueEncoderDist = 0;

            targetDriveAngle = ReadGyroyaw();

            VAR_PRINTLN((long)targetEncoderDist);
            VAR_PRINTLN((long)lastEncoderDist);
            VAR_PRINTLN((long)trueEncoderDist);
            VAR_PRINTLN(targetFrontDist);
            VAR_PRINTLN(targetBackDist);
            VAR_PRINTLN(targetDriveAngle);
            
            setMainState(MainStates::DRIVE, MainStates::START_DRIVE);

            Devices::control.resetPIDs();

            BREAK;
            BREAK_ONLY(
                if (INPUT_BOOL((F("Reset to last checkpoint? ")), false))
                    resetInterrupt();
            )
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // drive:
        case MainStates::DRIVE: {
            if (inBlackTile) {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                setMainState(MainStates::BLACK_IR, MainStates::DRIVE);
            }

            if (inBlueTile && !stoppedInTile && blueTileLastMillis > 100) {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                DB_COLOR_PRINTLN(F("Stopping in blue tile!"), SET_BLUE);
                blueTileStopStartTime = millis();
                setMainState(MainStates::BLUE_TILE_STOP, MainStates::DRIVE);
                break;
            }

            int ret = Devices::control.driveAlong(targetBackDist, targetFrontDist, WALL_DIST_MM, targetDriveAngle, lastEncoderDist, trueEncoderDist, targetEncoderDist, 1.0f, FRONT_WALL_DIST_MM);
            if (ret == 0) {
                setMainState(MainStates::COMPLETE_MOVE, MainStates::DRIVE);
            } else if (ret == -1) {
                ERROR_MINOR(F("Failed to complete drive!"), SET_RED);
                mapper.panicMode(); // we have no idea where we are anymore (tofs and encoder failed us)
                setMainState(MainStates::GET_MOVE, MainStates::DRIVE);
            }
            
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // complete action:
        case MainStates::COMPLETE_MOVE: {
            mapper.completeCurrMove();
            
            setMainState(MainStates::GET_MOVE, MainStates::COMPLETE_MOVE);

            BREAK_ONLY(
                if (INPUT_BOOL((F("Reset to last checkpoint? ")), false))
                    resetInterrupt();
            )
            BREAK;
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // black tile interrupt occoured:
        case MainStates::BLACK_IR: {
            LACK;
            Devices::motors.setSpeeds(0, 0, 0, 0);
            // I don't think walls matter for black tiles
            mapper.currMoveBlackTile(false, false, false, false);

            setMainState(MainStates::BLACK_DRIVE, MainStates::BLACK_IR);
            targetDriveAngle = ReadGyroyaw();

            blueTileToleranceStarTime = 0;

            Devices::control.resetPIDs();
            BREAK_ONLY(
                if (INPUT_BOOL((F("Reset to last checkpoint? ")), false))
                    resetInterrupt();
            )
            BREAK;
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // drive out of black tile:
        case MainStates::BLACK_DRIVE: {
            // we wan't to subtract the driving out of the black tile if we are on a blue tile, because the driving into the black tile
            // counted and we want to counteract that by subtracting from the true distance so the blue tile isn't counted twice,
            // this is a very nieche edge case that saves us 10s, but easy enough to prevent

            if (!inBlackTile && blueTileToleranceStarTime == 0) {
                LACK;
                blueTileToleranceStarTime = millis();
            }
            // simply add a bit of toleranz here
            if (blueTileToleranceStarTime != 0 && (millis() - blueTileToleranceStarTime) >= 100) {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                setMainState(MainStates::BLACK_TURN, MainStates::BLACK_DRIVE);
                targetTurnAngle = ReadGyroyaw() + 180;
                VAR_PRINTLN(targetTurnAngle);
                turnStartTime = millis();
                VAR_PRINTLN(turnStartTime);
                turnTol = 1;
                Devices::control.resetPIDs();
                BREAK;
                break;
            }

            Devices::control.uncondDriveAlong(WALL_DIST_MM, targetDriveAngle, -0.5);

            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // turn 180° after driving out of the black tile:
        case MainStates::BLACK_TURN: {
            int ret = Devices::control.turnRobot(targetTurnAngle, turnStartTime, turnTol);
            if (ret == 0) {
                LACK;
                setMainState(MainStates::GET_MOVE, MainStates::BLACK_TURN);
            } else if (ret == -1) {
                ERROR_MINOR(F("Devices::controll.turnRobot timeout occoured!"), SET_RED);
                turnStartTime = millis();
                turnTol *= 2; // increase tolerance and try agian.
            }

            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // stop in a blue tile:
        case MainStates::BLUE_TILE_STOP: {
            Devices::motors.setSpeeds(0, 0, 0, 0);
            if (millis() - blueTileStopStartTime >= 5000) {
                stoppedInTile = true;
                setMainState(MainStates::DRIVE, MainStates::BLUE_TILE_STOP);
            }
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // wait in a reset state until it should resume:
        case MainStates::RESET_STATE: {
            // simply do nothing in this sate and wait until the isr gets you out of it
            Devices::motors.setSpeeds(0, 0, 0, 0);
            BREAK_ONLY(
                if (INPUT_BOOL((F("Exit reset state? ")), true))
                    resetInterrupt();
            )
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // exit the reset state and reset back to checkpoint:
        case MainStates::EXIT_RESET_STATE: {
            mapper.resetToLastCheckpoint(wallFront(), wallRight(), wallLeft(), wallBack());
            setMainState(MainStates::GET_MOVE, MainStates::EXIT_RESET_STATE);
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // case that is entered if the camera detects something:
        case MainStates::CAMERA_DETECTION: {
            Devices::motors.setSpeeds(0, 0, 0, 0);
            digitalWrite(MAIN_LED, !(((millis() - cameraStartTime) / 500) % 2)); // blink in 500ms intervals 
            if (millis() - cameraStartTime >= 5000) {
                setMainState(mainStateBeforeCamInt, MainStates::CAMERA_DETECTION);
                digitalWrite(MAIN_LED, LOW);
            }
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // when the left or right bumper triggers:
        case MainStates::RIGHT_BUMPER:
        case MainStates::LEFT_BUMPER: {
            if (mainState == MainStates::RIGHT_BUMPER)
                setMainState(MainStates::DRIVE, MainStates::RIGHT_BUMPER);
            else
                setMainState(MainStates::DRIVE, MainStates::LEFT_BUMPER);
            break; // this is just to remove the bumper code, because we don't use it!
            // because we interrupted a drive and we should subtract the distance from the true Encoder dist, so the drive does not get confused
            trueEncoderDist -= getEncoderValueMM() - lastEncoderDist;
            lastEncoderDist = getEncoderValueMM();

            if (Devices::control.driveSCurve(100, 80, bumperStartTime, 500, (mainState == MainStates::RIGHT_BUMPER) ? 1 : -1) == 0) {
                // go back to the driing state
                if (mainState == MainStates::RIGHT_BUMPER)
                    setMainState(MainStates::DRIVE, MainStates::RIGHT_BUMPER);
                else
                    setMainState(MainStates::DRIVE, MainStates::LEFT_BUMPER);
            }
            break;
        }

        }

        // uint64_t loopDur = millis() - loopStart;
        // saveToEEPROM<uint64_t>(256, readFromEEPROM<uint64_t>(256) + 1); // num loops
        // saveToEEPROM<uint64_t>(256 + sizeof(uint64_t), readFromEEPROM<uint64_t>(256 + sizeof(uint64_t)) + loopDur); // sum durs
        // if (loopDur < readFromEEPROM<uint64_t>(256 + sizeof(uint64_t) * 2))
        //     saveToEEPROM<uint64_t>(256 + sizeof(uint64_t) * 2, loopDur); // min dur
        // if (loopDur > readFromEEPROM<uint64_t>(256 + sizeof(uint64_t) * 3))
        //     saveToEEPROM<uint64_t>(256 + sizeof(uint64_t) * 3, loopDur); // max dur
    }

    // TODO: Actions after finishing the maze

    for (uint8_t i = 0; i < 5; i++) {
        digitalWrite(MAIN_LED, HIGH);
        delay(1000);
        digitalWrite(MAIN_LED, LOW);
        delay(1000);
    }
    BREAK;
}

#define CHECK_INIT(x) do { if (!(x)) {worked = false; DB_PRINT_MUL((SET_RED)(F("Init of '"))(F(#x))("' Failed!\n")(RESET_COLOR));}} while (0)
bool initEverything() {
    bool worked = true;
    CHECK_INIT(initEncoders());
    CHECK_INIT(initTofs());
    CHECK_INIT(initgyro());
    CHECK_INIT(Devices::init());
    return worked;
}
#undef CHECK_INIT

/*
// black tile enter interrupt
void rgbcSensorOnEnter() {
    // I only really care about black tiles while driving, but I don't want to trigger
    // Devices::rgbcSensor.enterBlackTile(), because that could eat a black tile if it triggers while trurning or something
    if (mainState == MainStates::DRIVE) {
        DB_PRINTLN(F("Enter black tile!"));
        Devices::motors.setSpeeds(0, 0, 0, 0);
        inBlackTile = true; // so we can drive until this is false
        mainState = MainStates::BLACK_IR;
        Devices::rgbcSensor.enterBlackTile();
        return;
    }
    // we want to clear the interrupt again, just so it can trigger when we start to drive
    Devices::rgbcSensor.clearInterrupt();
}
*/

void rgbcSensorOnEnter() {
    DB_PRINTLN(F("Enter black tile!"));
    inBlackTile = true;
    Devices::rgbcSensor.enterBlackTile();
}

// black tile exit interrupt
void rgbcSensorOnExit() {
    DB_PRINTLN(F("Exit black tile!"));
    inBlackTile = false;
    Devices::rgbcSensor.exitBlackTile();
}

void resetInterrupt() {
    if (mainState != MainStates::RESET_STATE) {
        Devices::motors.setSpeeds(0, 0, 0, 0);
        mainState = MainStates::RESET_STATE;
    } else {
        mainState = MainStates::EXIT_RESET_STATE;
    }
}

void bumperInterruptRight() {
    if (getFrontTopDistance() <= 100) // if front distance is too small ignore bumper
        return;
    if (mainState == MainStates::DRIVE) {
        Devices::motors.setSpeeds(0, 0, 0, 0);
        mainState = MainStates::RIGHT_BUMPER;
        bumperStartTime = millis();
    }
}

void bumperInterruptLeft() {
    if (getFrontTopDistance() <= 100) // if front distance is too small ignore bumper
        return;
    if (mainState == MainStates::DRIVE) {
        Devices::motors.setSpeeds(0, 0, 0, 0);
        mainState = MainStates::LEFT_BUMPER;
        bumperStartTime = millis();
    }
}

bool isRamp() {
    return abs(wrap180(gyro.getPitch())) >= RAMP_INCLINE_THRESHOLD;
}

uint8_t getTileType() {
    if (isRamp())
        return RAMP_TILE;
    DB_PRINT_MUL((F("getTileType spec output: "))(colorTypeToName[static_cast<int8_t>(Devices::spec.getColorId())])('\n'));
    switch (Devices::spec.getColorId()) {
    case ColorType::Blue:
        return BLUE_TILE;
    case ColorType::Black:
        return BLACK_TILE;
    case ColorType::Checkpoint:
        return CHECKPOINT_TILE;
    default:
        return NORMAL_TILE;
    }
    return NORMAL_TILE;
}

bool getUp() {
    return -wrap180(gyro.getPitch()) >= RAMP_INCLINE_THRESHOLD;
}

bool rampInfront() {
    if (!getTofFTValid() || getTofFBRValid()) // not reliable enough if too large
        return false;
    return getFrontAngle() <= FRONT_RAMP_THRESHOLD;
}

extern char *__brkval;
extern char __heap_start;

uint16_t getHeapUsage() {
    if (__brkval == 0)
        return 0;
    return (uint16_t)__brkval - (uint16_t)&__heap_start;
}

void triggerPackageThrow(RaspiEvent detection) {
    switch (detection)
    {
    case RaspiEvent::NONE:
    case RaspiEvent::NO_MORE_PACKETS:
    case RaspiEvent::CAMERA_TRIGGERED_RIGTH:
    case RaspiEvent::CAMERA_TRIGGERED_LEFT:
    case RaspiEvent::CAMERA_INVALID: break;
#if USE_NEW_RASPI_COMMS
    case RaspiEvent::DETECTED_PSI_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_PHI_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_OMEGA_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_0_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_1_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_2_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_PSI_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_PHI_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_OMEGA_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_0_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_1_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_RING_SUM_2_LEFT: Devices::packageHandlerLeft.trigger(2); break; 
#else
    case RaspiEvent::DETECTED_H_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_S_RIGHT: Devices::packageHandlerRight.trigger(1); break;
    case RaspiEvent::DETECTED_U_RIGHT: break;
    case RaspiEvent::DETECTED_H_LEFT: Devices::packageHandlerLeft.trigger(2); break;
    case RaspiEvent::DETECTED_S_LEFT: Devices::packageHandlerLeft.trigger(1); break;
    case RaspiEvent::DETECTED_U_LEFT: break;
    case RaspiEvent::DETECTED_GREEN_RIGHT: break;
    case RaspiEvent::DETECTED_YELLOW_RIGHT: Devices::packageHandlerRight.trigger(1); break;
    case RaspiEvent::DETECTED_RED_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_GREEN_LEFT: break;
    case RaspiEvent::DETECTED_YELLOW_LEFT: Devices::packageHandlerLeft.trigger(1); break;
    case RaspiEvent::DETECTED_RED_LEFT: Devices::packageHandlerLeft.trigger(2); break;
#endif
    }
}

#endif