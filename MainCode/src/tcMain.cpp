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
#include <DisplayConf/displayConf.h>
#include <TC/tc.h>

#include <config.h>

#define USE_tc true
#if CAT(USE_, CURR_MAIN)
#undef USE_tc

void setMainState(MainStates state, MainStates currentCase);
void mainFunc(Display* parentDisplay, Menu* parentMenu);
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
bool isRight(RaspiEvent detection);

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

mapPos posToDisplay = mapPos(0, 0, 0);
RaspiEvent victimToDsiplay = RaspiEvent::NONE;

// ----------------------------------------------------------------------------------------------------

String getMappingPos() {
    return String(posToDisplay.x) + "," + posToDisplay.y + "," + posToDisplay.z;
}

String getCurrentVictim() {
    switch (victimToDsiplay) {
    case RaspiEvent::DETECTED_VICTIM_0_LEFT: return F("0L");
    case RaspiEvent::DETECTED_VICTIM_1_LEFT: return F("1L");
    case RaspiEvent::DETECTED_VICTIM_2_LEFT: return F("2L");
    case RaspiEvent::DETECTED_VICTIM_0_RIGHT: return F("0R");
    case RaspiEvent::DETECTED_VICTIM_1_RIGHT: return F("1R");
    case RaspiEvent::DETECTED_VICTIM_2_RIGHT: return F("2R");
    default: return F("NONE");
    }
}

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

    /*
    for (uint8_t i = 0; i < 28; i++) {
        if (i % 3 == 0)
            Devices::ledsTop.setPixelColor(i, 0xFF000000);
        else
            Devices::ledsTop.setPixelColor(i, 0x00000000);
    }
    Devices::ledsTop.show();
    */

    Devices::ledsTop.fill(0x40000000); // (0x40000000);
    Devices::ledsTop.show();

    /*
    for (uint8_t type = 0; type < 5; type++) {
        DB_PRINT_MUL((SET_GREEN)(F("Calibrate "))(colorTypeToName[type])('!')(RESET_COLOR)('\n'));
        do { DB_PRINT(SET_BLUE); DB_PRINT(F("Breakpoint in file: ")); DB_PRINT(F(__FILE__)); DB_PRINT(F(" in line: ")); DB_PRINT(__LINE__); DB_PRINTLN(F(" Triggered!")); DB_PRINT(RESET_COLOR); while (!Serial.available()) {} delay(100); while (Serial.available()) { Serial.read(); } } while (0);
        //Devices::refSensor.calibrate(ColorType(type));
        Devices::rgbcSensor.setColor(ColorType(type));
        Devices::spec.setColorCurrent(ColorType(type));
    }
    */
    Devices::display.clearDisplay();
    Devices::display.currMenu = &mainMenu;
    LACK;

    while (true) {
        while (!digitalRead(BUTTON1)) {
            if (getTofLBValid())
                digitalWrite(CALIB_LED, HIGH);
            else
                digitalWrite(CALIB_LED, LOW);
            Devices::display.update();
        } // wait until start
        while (digitalRead(BUTTON1)) {
            if (getTofLBValid())
                digitalWrite(CALIB_LED, HIGH);
            else
                digitalWrite(CALIB_LED, LOW);
            Devices::display.update();
        } // wait until button unpressed

        delay(100); // simply that the reset does not trigger accidentally

        BREAK;

        mainFunc(&Devices::display, &runMenu);
        Devices::display.currMenu = &runMenu;
        Devices::display.update();
    }
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

void mainFunc(Display* parentDisplay, Menu* parentMenu) {
    while (Serial1.available()) { Serial1.read(); }

    resetTC();

    Devices::display.currMenu = &runMenu;

    attachPCINT(digitalPinToPCINT(BUTTON1), resetInterrupt, FALLING); // atach reset button

    attachInterrupt(digitalPinToInterrupt(BUMPER1), bumperInterruptRight, FALLING);
    attachInterrupt(digitalPinToInterrupt(BUMPER2), bumperInterruptLeft, FALLING);

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

    // camera state stuff
    MainStates mainStateBeforeCamInt = mainState;
    uint32_t cameraStartTime = millis();

    // blue tile data:
    uint32_t blueTileStopStartTime = 0;

    // display update time:
    uint32_t lastDisplayUpdate = 0;

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

        posToDisplay = mapper.pos;

        // ----------------------------------------------------------------------------------------------------
        // update all sensors:

        updateTofs();
        gyro.update();
        Devices::spec.update();
        Devices::packageHandlerLeft.update();
        Devices::packageHandlerRight.update();

        if (millis() - lastDisplayUpdate >= 100) {
            Devices::display.update();
            lastDisplayUpdate = millis();
        }

        if (getTofLBValid())
            digitalWrite(CALIB_LED, HIGH);
        else
            digitalWrite(CALIB_LED, LOW);

        // ----------------------------------------------------------------------------------------------------
        // get current tile
        // mapPos pos = mapper.pos;

        // ----------------------------------------------------------------------------------------------------
        // update camera:

        RaspiEvent raspiEvent = Devices::comms.update(0, 0, 0, 0);
        uint32_t raspiLoopStartTime = millis();
        while (raspiEvent != RaspiEvent::NO_MORE_PACKETS && (millis() - raspiLoopStartTime) <= 5) {
            VAR_PRINTLN(static_cast<uint8_t>(raspiEvent));
#if USE_NEW_RASPI_COMMS
            if (raspiEvent >= RaspiEvent::DETECTED_VICTIM_0_RIGHT && raspiEvent <= RaspiEvent::DETECTED_VICTIM_2_LEFT) {
                bool right = isRight(raspiEvent);
                if ((bothWallRight() && right) || (bothWallLeft() && !right)) {
                    Devices::comms.debugLog(F("Detected victim, switching state!"));
                    DB_COLOR_PRINTLN(F("Detected Victim, switching state!"), SET_GREEN);
                    victimToDsiplay = raspiEvent;
                    mainStateBeforeCamInt = mainState;
                    uncondSetMainState(MainStates::CAMERA_DETECTION);
                    cameraStartTime = millis();
                }
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
            raspiEvent = Devices::comms.update(0, 0, 0, 0);
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

            TileCon tcData(0, 0, 0, 0, 0, getTileType(), mapper.rotation, getUp());
            if (tcData.type != RAMP_TILE) {
                tcData[mapper.rotation] = wallFront();
                tcData[(mapper.rotation + 1) % 4] = wallRight();
                tcData[(mapper.rotation + 2) % 4] = wallBack();
                tcData[(mapper.rotation + 3) % 4] = wallLeft();
            }

            addElement(tc_element_t {
                .x = mapper.pos.x,
                .y = mapper.pos.y,
                .type = TC_TILE,
                .data = (uint8_t)tcData,
            });

            DB_PRINT_MUL((F("StackPtr: "))((uint16_t)SP)(F(" HeapPtr: "))(getHeapUsage())(F(" Mapper Mem: "))((long)mapper.currentDynamicRamUsage())(F(" actions Data size: "))(mapper._actions.dataSize())('\n'));
            uint8_t tileType = getTileType();
            if (tileType == RAMP_TILE)
                tileType = NORMAL_TILE;
            currMove = mapper.currMove(wallFront(), wallRight(), wallLeft(), wallBack(), getUp(), tileType);
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
            targetTurnAngle = ReadGyroyaw() + angleDiffDEG(90 * currMove.rotation, calculateposition());
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

            int ret = Devices::control.turnRobot(targetTurnAngle, turnStartTime, turnTol, 0.3);
            if (ret == 0) {
                setMainState(MainStates::START_DRIVE, MainStates::TURN);
            } else if (ret == -1) {
                ERROR_MINOR(F("Devices::controll.turnRobot timeout occoured!"), SET_RED);
                turnStartTime = millis();
                turnTol *= 2; // increase tolerance and try agian.
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

            targetBackDist = getBackDistance() + 300; // drive 300 mm
            if (!getTofBValid())
                targetBackDist = -1;

            VAR_PRINTLN(getFrontTopDistance());
            VAR_PRINTLN(getTofFTValid());
            VAR_PRINTLN(rampInfront());
            targetFrontDist = getFrontTopDistance() - 300; // drive 300 mm
            if (!getTofFTValid()) // if the tof values are invalid
                targetFrontDist = -1;
            else if (!rampInfront()) { // if there is no ramp in front (center to the nearest tile)
                int numTiles = getFrontTopDistance() / 300.0;
                VAR_PRINTLN(numTiles);
                VAR_PRINTLN(getFrontTopDistance() % 300);
                if (getFrontTopDistance() % 300 > 180) numTiles++; // add a bit of tolerance
                VAR_PRINTLN(numTiles);
                targetFrontDist = (numTiles - 1) * 300 + FRONT_WALL_DIST_MM;
            }
            else if (getFrontTopDistance() < 300) // driving onto ramp
                targetFrontDist = -1;
            
            if (getTofFTValid() && !rampInfront() && getFrontTopDistance() < 180 && !mapper.isInPanicMode()) { // if there is no tile in front of you
                ERROR_MINOR(F("Mapper gave me bullshit data!"), SET_RED);
                mapper.panicMode();
                setMainState(MainStates::COMPLETE_MOVE, MainStates::START_DRIVE);
                break;
            } 

            targetEncoderDist = 300;
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
            /*BREAK_ONLY(
                if (INPUT_BOOL((F("Reset to last checkpoint? ")), false))
                    resetInterrupt();
            )*/
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // drive:
        case MainStates::DRIVE: {
            if (inBlackTile && !isRamp()) {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                setMainState(MainStates::BLACK_IR, MainStates::DRIVE);
            }

            int ret = Devices::control.driveAlong(targetBackDist, targetFrontDist, WALL_DIST_MM, targetDriveAngle, lastEncoderDist, trueEncoderDist, targetEncoderDist, 1.0f, FRONT_WALL_DIST_MM);
            if (ret == 0) {
                //! not using MainStates::ENSURE_HEADING
                // targetTurnAngle = targetDriveAngle + angleDiffDEG(0, calculateposition()); //! No idea if that makes sens
                // VAR_PRINTLN(targetTurnAngle);
                // VAR_PRINTLN(ReadGyroyaw());
                // turnStartTime = millis();
                // turnTol = 5;
                // BREAK;
                setMainState(MainStates::CHECK_BLUE, MainStates::DRIVE); //! Skipping ENSURE_HEADING, because I think it is pointless
            } else if (ret == -1) {
                ERROR_MINOR(F("Failed to complete drive!"), SET_RED);
                mapper.panicMode(); // we have no idea where we are anymore (tofs and encoder failed us)
                setMainState(MainStates::GET_MOVE, MainStates::DRIVE);
            }
            
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // make sure the heading is correct ater driving so that we don't get any incorrect wall detections:
        case MainStates::ENSURE_HEADING: {
            LACK;
            int ret = Devices::control.turnRobot(targetTurnAngle, turnStartTime, turnTol);
            if (ret == 0) {
                setMainState(MainStates::CHECK_BLUE, MainStates::ENSURE_HEADING);
            } else if (ret == -1) {
                ERROR_MINOR(F("Devices::controll.turnRobot timeout occoured!"), SET_RED);
                turnStartTime = millis();
                turnTol *= 2; // increase tolerance and try agian.
            }

            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // check if in a blue tile:
        case MainStates::CHECK_BLUE: {
            LACK;
            Devices::motors.setSpeeds(0, 0, 0, 0);
            if (getTileType() == BLUE_TILE) {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                blueTileStopStartTime = millis();
                setMainState(MainStates::BLUE_TILE_STOP, MainStates::CHECK_BLUE);
                break;
            }
            setMainState(MainStates::COMPLETE_MOVE, MainStates::CHECK_BLUE);
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // stop in a blue tile:
        case MainStates::BLUE_TILE_STOP: {
            Devices::motors.setSpeeds(0, 0, 0, 0);
            if (millis() - blueTileStopStartTime >= 5000) {
                setMainState(MainStates::COMPLETE_MOVE, MainStates::BLUE_TILE_STOP);
            }
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // complete action:
        case MainStates::COMPLETE_MOVE: {
            mapper.completeCurrMove();
            
            setMainState(MainStates::GET_MOVE, MainStates::COMPLETE_MOVE);

            /*BREAK_ONLY(
                if (INPUT_BOOL((F("Reset to last checkpoint? ")), false))
                    resetInterrupt();
            )*/
            BREAK;
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // black tile interrupt occoured:
        case MainStates::BLACK_IR: {
            LACK;
            Devices::motors.setSpeeds(0, 0, 0, 0);

            mapPos detectionPos = mapper._getDriveInDirec(mapper.pos, wrap(mapper.rotation + currMove.rotation, 0, 4));
            TileCon tcData(0, 0, 0, 0, 0, BLACK_TILE, 0, 0);
            addElement(tc_element_t {
                .x = detectionPos.x,
                .y = detectionPos.y,
                .type = TC_TILE,
                .data = (uint8_t)tcData,
            });

            // I don't think walls matter for black tiles
            mapper.currMoveBlackTile(false, false, false, false);

            setMainState(MainStates::BLACK_DRIVE, MainStates::BLACK_IR);
            targetDriveAngle = ReadGyroyaw();

            Devices::control.resetPIDs();
            /*BREAK_ONLY(
                if (INPUT_BOOL((F("Reset to last checkpoint? ")), false))
                    resetInterrupt();
            )*/
            BREAK;
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // drive out of black tile:
        case MainStates::BLACK_DRIVE: {
            // we wan't to subtract the driving out of the black tile if we are on a blue tile, because the driving into the black tile
            // counted and we want to counteract that by subtracting from the true distance so the blue tile isn't counted twice,
            // this is a very nieche edge case that saves us 10s, but easy enough to prevent

            if (!inBlackTile) {
                LACK;
                setMainState(MainStates::BLACK_CENTER, MainStates::BLACK_DRIVE);

                targetBackDist = getBackDistance() - FRONT_WALL_DIST_MM; // center to the nearest tile
                if (!getTofBValid())
                    targetBackDist = -1;

                targetFrontDist = getFrontTopDistance() + FRONT_WALL_DIST_MM; // standard drive FRONT_WALL_DIST_MM
                if (!getTofFTValid()) // if the tof values are invalid
                    targetFrontDist = -1;
                else if (!rampInfront()) { // if there is no ramp in front
                    int numTiles = getFrontTopDistance() / 300;
                    if (getFrontTopDistance() % 300 > 200) numTiles++;
                    targetFrontDist = numTiles * 300 + FRONT_WALL_DIST_MM;
                }
                else if (getFrontTopDistance() < 300) // driving onto ramp
                    targetFrontDist = -1;

                targetEncoderDist = FRONT_WALL_DIST_MM;
                lastEncoderDist = getEncoderValueMM();
                trueEncoderDist = 0;

                targetDriveAngle = ReadGyroyaw();
            }

            Devices::control.uncondDriveAlong(WALL_DIST_MM, targetDriveAngle, -0.5);

            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // center on the tile behind you:
        case MainStates::BLACK_CENTER: {
            int ret = Devices::control.driveAlong(targetBackDist, targetFrontDist, WALL_DIST_MM, targetDriveAngle, lastEncoderDist, trueEncoderDist, targetEncoderDist, -0.5, FRONT_WALL_DIST_MM);

            if (ret == 0) {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                setMainState(MainStates::BLACK_TURN, MainStates::BLACK_CENTER);
                targetTurnAngle = wrap360(ReadGyroyaw() + 180);
                VAR_PRINTLN(targetTurnAngle);
                turnStartTime = millis();
                VAR_PRINTLN(turnStartTime);
                turnTol = 1;
                Devices::control.resetPIDs();
                BREAK;
                break;
            } else if (ret == -1) {
                ERROR_MINOR(F("Failed to complete drive!"), SET_RED);
                mapper.panicMode(); // we have no idea where we are anymore (tofs and encoder failed us)
                setMainState(MainStates::GET_MOVE, MainStates::BLACK_CENTER);
            }

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
        // wait in a reset state until it should resume:
        case MainStates::RESET_STATE: {
            // simply do nothing in this sate and wait until the isr gets you out of it
            Devices::motors.setSpeeds(0, 0, 0, 0);
            /*BREAK_ONLY(
                if (INPUT_BOOL((F("Exit reset state? ")), true))
                    resetInterrupt();
            )*/
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
            mapPos detectionPos = mapper.pos;
            if (mainStateBeforeCamInt == MainStates::DRIVE)
                detectionPos = mapper._getDriveInDirec(detectionPos, wrap(mapper.rotation + currMove.rotation, 0, 4));

            uint16_t data = 0xFFFF;
            switch (victimToDsiplay)
            {
            case RaspiEvent::DETECTED_VICTIM_0_LEFT: ((uint8_t*)&data)[0] = 0; ((uint8_t*)&data)[1] = (mapper.rotation + 3) % 4; break;
            case RaspiEvent::DETECTED_VICTIM_0_RIGHT: ((uint8_t*)&data)[0] = 0; ((uint8_t*)&data)[1] = (mapper.rotation + 1) % 4; break;
            case RaspiEvent::DETECTED_VICTIM_1_LEFT: ((uint8_t*)&data)[0] = 1; ((uint8_t*)&data)[1] = (mapper.rotation + 3) % 4; break;
            case RaspiEvent::DETECTED_VICTIM_1_RIGHT: ((uint8_t*)&data)[0] = 1; ((uint8_t*)&data)[1] = (mapper.rotation + 1) % 4; break;
            case RaspiEvent::DETECTED_VICTIM_2_LEFT: ((uint8_t*)&data)[0] = 2; ((uint8_t*)&data)[1] = (mapper.rotation + 3) % 4; break;
            case RaspiEvent::DETECTED_VICTIM_2_RIGHT: ((uint8_t*)&data)[0] = 2; ((uint8_t*)&data)[1] = (mapper.rotation + 1) % 4; break;
            }

            addElement(tc_element_t {
                .x = detectionPos.x,
                .y = detectionPos.y,
                .type = TC_VICTIM,
                .data = data,
            });

            victimToDsiplay = RaspiEvent::NONE;
            setMainState(mainStateBeforeCamInt, MainStates::CAMERA_DETECTION);
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // when the left or right bumper triggers:
        case MainStates::RIGHT_BUMPER:
        case MainStates::LEFT_BUMPER: {
            /*
            if (mainState == MainStates::RIGHT_BUMPER)
                setMainState(MainStates::DRIVE, MainStates::RIGHT_BUMPER);
            else
                setMainState(MainStates::DRIVE, MainStates::LEFT_BUMPER);
            break; // this is just to remove the bumper code, because we didn't use it!
            */
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

        //uint64_t loopDur = millis() - loopStart;
        //DB_PRINT_MUL((SET_RED)(F("loop time: "))((long)loopDur)(RESET_COLOR)('\n'));
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

// black tile enter interrupt
void rgbcSensorOnEnter() {
    // I only really care about black tiles while driving, but I don't want to trigger
    // Devices::rgbcSensor.enterBlackTile(), because that could eat a black tile if it triggers while trurning or something
    if (mainState == MainStates::DRIVE && !isRamp()) {
        DB_PRINTLN(F("Enter black tile!"));
        Devices::motors.setSpeeds(0, 0, 0, 0);
        inBlackTile = true; // so we can drive until this is false
        mainState = MainStates::BLACK_IR;
        Devices::rgbcSensor.enterBlackTile();
        return;
    }
    // we want to clear the interrupt again, just so it can trigger when we start to drive
    Devices::rgbcSensor.enterBlackTile();
}

/*
void rgbcSensorOnEnter() {
    DB_PRINTLN(F("Enter black tile!"));
    inBlackTile = true;
    Devices::rgbcSensor.enterBlackTile();
}
*/

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
    // if (getFrontTopDistance() <= 100) // if front distance is too small ignore bumper
    //    return;
    if (mainState == MainStates::DRIVE) {
        Devices::motors.setSpeeds(0, 0, 0, 0);
        mainState = MainStates::RIGHT_BUMPER;
        bumperStartTime = millis();
    }
}

void bumperInterruptLeft() {
    // if (getFrontTopDistance() <= 100) // if front distance is too small ignore bumper
    //    return;
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
        return RAMP_TILE; //! CHANGE THIS BACK THIS IS BULLSHIT
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
    return ReadGyropitch() >= RAMP_INCLINE_THRESHOLD;
}

bool rampInfront() {
    if (!getTofFTValid() || !getTofFBRValid())
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
    case RaspiEvent::DETECTED_VICTIM_0_RIGHT: break;
    case RaspiEvent::DETECTED_VICTIM_0_LEFT: break;
    case RaspiEvent::DETECTED_VICTIM_1_RIGHT: Devices::packageHandlerRight.trigger(1); break;
    case RaspiEvent::DETECTED_VICTIM_1_LEFT: Devices::packageHandlerLeft.trigger(1); break;
    case RaspiEvent::DETECTED_VICTIM_2_RIGHT: Devices::packageHandlerRight.trigger(2); break;
    case RaspiEvent::DETECTED_VICTIM_2_LEFT: Devices::packageHandlerLeft.trigger(2); break;
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

bool isRight(RaspiEvent detection) {
    switch (detection)
    {
    case RaspiEvent::CAMERA_TRIGGERED_RIGTH: return true;
    case RaspiEvent::CAMERA_TRIGGERED_LEFT: return false;
#if USE_NEW_RASPI_COMMS
    case RaspiEvent::DETECTED_VICTIM_0_RIGHT: return true;
    case RaspiEvent::DETECTED_VICTIM_0_LEFT: return false;
    case RaspiEvent::DETECTED_VICTIM_1_RIGHT: return true;
    case RaspiEvent::DETECTED_VICTIM_1_LEFT: return false;
    case RaspiEvent::DETECTED_VICTIM_2_RIGHT: return true;
    case RaspiEvent::DETECTED_VICTIM_2_LEFT: return false;
#else
    case RaspiEvent::DETECTED_H_RIGHT: return true;
    case RaspiEvent::DETECTED_S_RIGHT: return true;
    case RaspiEvent::DETECTED_U_RIGHT: return true;
    case RaspiEvent::DETECTED_H_LEFT: return false;
    case RaspiEvent::DETECTED_S_LEFT: return false;
    case RaspiEvent::DETECTED_U_LEFT: return false;
    case RaspiEvent::DETECTED_GREEN_RIGHT: return true;
    case RaspiEvent::DETECTED_YELLOW_RIGHT: return true;
    case RaspiEvent::DETECTED_RED_RIGHT: return true;
    case RaspiEvent::DETECTED_GREEN_LEFT: return false;
    case RaspiEvent::DETECTED_YELLOW_LEFT: return false;
    case RaspiEvent::DETECTED_RED_LEFT: return false;
#endif
    default: return false;
    }
}

#endif