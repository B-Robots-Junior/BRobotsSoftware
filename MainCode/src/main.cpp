#include <Arduino.h>
#include <debug.h>
#include <errorHandling.h>
#include <mainStates.h>
#include <array.h>
#include <Encoders/encoders.h>
#include <PosSensors/position.h>
#include <devices.h>
#include <Mapping/mapping.h>

#include <config.h>

#define USE_main_func true
#if CAT(USE_, CURR_MAIN)
#undef USE_main_func

void mainFunc();

int main() {
    init();
    sei();

    BEGIN_DEBUG(BAUDE_RATE);

    DB_PRINTLN("Start Main!");
    BREAK;

    mainFunc();

    while (true) {}
}

// ----------------------------------------------------------------------------------------------------
// global data:

MainStates mainState = MainStates::GET_MOVE;
bool inBlackTile = false;

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

bool isRamp() {
    return abs(wrap180(gyro.getPitch())) >= RAMP_INCLINE_THRESHOLD;
}

uint8_t getTileType() {
    //! update this with more sensors
    if (isRamp())
        return RAMP_TILE;
    return NORMAL_TILE;
}

bool getUp() {
    return -wrap180(gyro.getPitch()) >= RAMP_INCLINE_THRESHOLD;
}

bool rampInfront() {
    if (getFrontTopDistance() >= 900 || getFrontBottomLongDistance() >= 900) // not reliable enough if too large
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

    // ----------------------------------------------------------------------------------------------------
    // initialize all sensors:

    if (!initEverything())
        mainState = MainStates::REINIT;

    //mapper.panicMode();

    /*
    while (true) {
        DB_PRINT_MUL(("front: ")(wallFront())(" right: ")(wallRight())(" back: ")(wallBack())(" left: ")(wallLeft())('\n'));
        delay(100);
    }
    */

    /*
    for (uint8_t type = 0; type < 5; type++) {
        DB_PRINT_MUL((SET_GREEN)(F("Calibrate "))(colorTypeToName[type])('!')(RESET_COLOR)('\n'));
        BREAK;
        Devices::refSensor.calibrate(ColorType(type));
        Devices::rgbcSensor.setColor(ColorType(type));
    }
    */

    for (uint8_t type = 0; type < 5; type++) {
        // DB_PRINT_MUL((F("EEPROM_DATA: Type: "))(colorTypeToName[type])
        //              (F(" r: "))(readFromEEPROM<uint16_t>(RGBC_SENSOR_OFFSET_EEPROM + (static_cast<int8_t>(type) * 4) * sizeof(uint16_t)))
        //              (F(" g: "))(readFromEEPROM<uint16_t>(RGBC_SENSOR_OFFSET_EEPROM + (static_cast<int8_t>(type) * 4 + 1) * sizeof(uint16_t)))
        //              (F(" b: "))(readFromEEPROM<uint16_t>(RGBC_SENSOR_OFFSET_EEPROM + (static_cast<int8_t>(type) * 4 + 2) * sizeof(uint16_t)))
        //              (F(" c: "))(readFromEEPROM<uint16_t>(RGBC_SENSOR_OFFSET_EEPROM + (static_cast<int8_t>(type) * 4 + 3) * sizeof(uint16_t)))('\n'));

        DB_PRINT_MUL((F("Type: "))(colorTypeToName[type])
                     (F(" r: "))(Devices::rgbcSensor.colors[type][0])
                     (F(" g: "))(Devices::rgbcSensor.colors[type][1])
                     (F(" b: "))(Devices::rgbcSensor.colors[type][2])
                     (F(" c: "))(Devices::rgbcSensor.colors[type][3])('\n'));
    }

    // DB_PRINT_MUL(("num loops: ")((long)readFromEEPROM<uint64_t>(256))('\n'));
    // DB_PRINT_MUL(("sum durs: ")((long)readFromEEPROM<uint64_t>(256 + sizeof(uint64_t)))('\n'));
    // DB_PRINT_MUL(("min dur: ")((long)readFromEEPROM<uint64_t>(256 + sizeof(uint64_t) * 2))('\n'));
    // DB_PRINT_MUL(("max dur: ")((long)readFromEEPROM<uint64_t>(256 + sizeof(uint64_t) * 3))('\n'));

    /*
    while (true) {
        uint16_t r, g, b, c;
        Devices::rgbcSensor.sensor.getRawData(&r, &g, &b, &c);
        DB_PRINT_MUL(("r: ")(r)(" g: ")(g)(" b: ")(b)(" c: ")(c)(" type: ")(static_cast<int8_t>(Devices::rgbcSensor.getCurrentId()))('\n'));
        delay(100);
    }
    */

    /*
    while (true) {
        updateTofs();
        gyro.update();
        DB_PRINT_MUL(("tof angle: ")(calculateposition())(" gyro angle: ")(ReadGyroyaw())('\n'));
        delay(100);
    }
    */

    BREAK;
    
    LACK;

    //saveToEEPROM<uint64_t>(256, 0); // num loops
    //saveToEEPROM<uint64_t>(256 + sizeof(uint64_t), 0); // sum durs
    //saveToEEPROM<uint64_t>(256 + sizeof(uint64_t) * 2, 0xFFFFFFFF); // min dur
    //saveToEEPROM<uint64_t>(256 + sizeof(uint64_t) * 3, 0); // max dur

    bool running = true;
    while (running) 
    {
        // uint64_t loopStart = millis();

        // ----------------------------------------------------------------------------------------------------
        // update all sensors:

        updateTofs();
        gyro.update();

        // ----------------------------------------------------------------------------------------------------

        switch (mainState)
        {

        // ----------------------------------------------------------------------------------------------------
        // reinit state:
        case MainStates::REINIT: {
            ERROR(F("Should not enter MainStates::REINIT in debug mode!"));

            if (!initEverything())
                mainState = MainStates::REINIT;
            else
                mainState = MainStates::GET_MOVE;

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

            DB_PRINT_MUL((F("StackPtr: "))((uint16_t)SP)(F(" HeapPtr: "))(getHeapUsage())(F(" Mapper Mem: "))((long)mapper.currentDynamicRamUsage())(F(" actions Data size: "))(mapper._actions.dataSize())('\n'));
            currMove = mapper.currMove(wallFront(), wallRight(), wallLeft(), wallBack(), getUp(), getTileType());
            DB_PRINT_MUL((F("StackPtr: "))((uint16_t)SP)(F(" HeapPtr: "))(getHeapUsage())(F(" Mapper Mem: "))((long)mapper.currentDynamicRamUsage())(F(" actions Data size: "))(mapper._actions.dataSize())('\n'));
            VAR_FUNC_PRINTLN(currMove);

            if (currMove.distance == 0 && currMove.rotation == 0) {
                DB_COLOR_PRINTLN(F("Completed the Maze YAY!"), SET_GREEN);
                running = false;
                break;
            }

            mainState = MainStates::START_TURN;
            BREAK;
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // start turn:
        case MainStates::START_TURN: {
            LACK;
            if (currMove.rotation == 0) {
                mainState = MainStates::START_DRIVE;
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

            mainState = MainStates::TURN;
            BREAK;
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // turn:
        case MainStates::TURN: {
            LACK;

            int ret = Devices::control.turnRobot(targetTurnAngle, turnStartTime, turnTol);
            if (ret == 0) {
                mainState = MainStates::START_DRIVE;
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
                mainState = MainStates::COMPLETE_MOVE;
                BREAK;
                break;
            }

            bool drivinOnToRamp = rampInfront() && (currMove.distance * 300 >= getFrontBottomLongDistance());

            if (getBackDistance() >= TOF_TIMEOUT_VALUE || drivinOnToRamp || isRamp()) {
                LACK;
                targetBackDist = -1; // can't rely on the back dist
            } else {
                LACK;
                // its hard to check if there is ramp behind you, so you add 300 to the back of what you currently have and call it a day
                // and let the front tof aling to the nearest tile, because it is prioritized and can detect ramps
                targetBackDist = getBackDistance() + 300;
                VAR_PRINTLN(targetBackDist);
            }

            if (getFrontTopDistance() >= TOF_TIMEOUT_VALUE || drivinOnToRamp || isRamp()) {
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
                        mainState = MainStates::GET_MOVE;
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
            
            mainState = MainStates::DRIVE;

            Devices::control.resetPIDs();

            BREAK;
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // drive:
        case MainStates::DRIVE: {
            if (inBlackTile) {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                mainState = MainStates::BLACK_IR;
            }

            int ret = Devices::control.driveAlong(targetBackDist, targetFrontDist, WALL_DIST_MM, targetDriveAngle, lastEncoderDist, trueEncoderDist, targetEncoderDist, 1.0f, FRONT_WALL_DIST_MM);
            if (ret == 0) {
                mainState = MainStates::COMPLETE_MOVE;
            } else if (ret == -1) {
                ERROR_MINOR(F("Failed to complete drive!"), SET_RED);
                mapper.panicMode(); // we have no idea where we are anymore (tofs and encoder failed us)
                mainState = MainStates::GET_MOVE;
            }
            
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // complete action:
        case MainStates::COMPLETE_MOVE: {
            mapper.completeCurrMove();
            
            mainState = MainStates::GET_MOVE;

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

            mainState = MainStates::BLACK_DRIVE;
            targetDriveAngle = ReadGyroyaw();

            lastEncoderDist = -1; // currently abusing this to add a tolerance

            Devices::control.resetPIDs();
            BREAK;
            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // drive out of black tile:
        case MainStates::BLACK_DRIVE: {
            if (!inBlackTile && lastEncoderDist == -1) {
                LACK;
                lastEncoderDist = getEncoderValueMM();
            }
            // simply add a bit of toleranz here
            if (lastEncoderDist != -1 && (getEncoderValueMM() - lastEncoderDist) >= BLACK_TILE_TOL) {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                mainState = MainStates::BLACK_TURN;
                targetTurnAngle = ReadGyroyaw() + 180;
                VAR_PRINTLN(targetTurnAngle);
                turnStartTime = millis();
                VAR_PRINTLN(turnStartTime);
                turnTol = 1;
                Devices::control.resetPIDs();
                BREAK;
                break;
            }

            Devices::control.uncondDriveAlong(WALL_DIST_MM, targetDriveAngle, -1.0);

            break;
        }

        // ----------------------------------------------------------------------------------------------------
        // turn 180° after driving out of the black tile:
        case MainStates::BLACK_TURN: {
            int ret = Devices::control.turnRobot(targetTurnAngle, turnStartTime, turnTol);
            if (ret == 0) {
                LACK;
                mainState = MainStates::GET_MOVE;
            } else if (ret == -1) {
                ERROR_MINOR(F("Devices::controll.turnRobot timeout occoured!"), SET_RED);
                turnStartTime = millis();
                turnTol *= 2; // increase tolerance and try agian.
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
    BREAK;
}

#else


#endif