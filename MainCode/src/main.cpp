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

// black tile enter interrupt
void rgbcSensorOnEnter() {
    Devices::rgbcSensor.enterBlackTile();
}

// black tile exit interrupt
void rgbcSensorOnExit() {
    Devices::rgbcSensor.exitBlackTile();
}

#define USE_main_func true
#if CAT(USE_, CURR_MAIN)
#undef USE_main_func

void mainFunc();

int main() {
    init();
    sei();

    BEGIN_DEBUG(BAUDE_RATE);

    DB_PRINTLN("Start Main!");
    // BREAK;

    mainFunc();

    while (true) {}
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

uint8_t getTileType() {
    //! update this with more sensors
    if (abs(wrap180(gyro.getPitch())) >= RAMP_INCLINE_THRESHOLD)
        return RAMP_TILE;
    return NORMAL_TILE;
}

bool getUp() {
    return -wrap180(gyro.getPitch()) >= RAMP_INCLINE_THRESHOLD;
}

void mainFunc() {

    // ----------------------------------------------------------------------------------------------------
    // initialize all the needed data:

    MainStates mainState = MainStates::GET_MOVE;
    Mapper mapper;
    Move currMove;

    // turn data:
    float targetTurnAngle = 0;
    uint32_t turnStartTime = 0;
    float turnTol = 1;

    // drive data:
    int targetFrontDist = 0;
    int targetBackDist = 0;
    uint64_t targetEncoderDist = 0;

    // ----------------------------------------------------------------------------------------------------
    // initialize all sensors:

    if (!initEverything())
        mainState = MainStates::REINIT;
    
    LACK;

    bool running = true;
    while (running) 
    {
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
            currMove = mapper.currMove(wallFront(), wallRight(), wallLeft(), wallBack(), getUp(), getTileType());
            VAR_FUNC_PRINTLN(currMove);

            if (currMove.distance == 0 && currMove.rotation == 0) {
                DB_COLOR_PRINTLN(F("Completed the Maze YAY!"), SET_GREEN);
                running = false;
                break;
            }

            mainState = MainStates::START_TURN;
            // BREAK;
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // start turn:
        case MainStates::START_TURN: {
            LACK;
            if (currMove.rotation == 0) {
                mainState = MainStates::START_DRIVE;
                LACK;
                // BREAK;
                break;
            }

            LACK;
            targetTurnAngle = ReadGyroyaw() - 90 * currMove.rotation; //! I think this is -90 deg, but I am not sure
            VAR_PRINTLN(targetTurnAngle);
            turnStartTime = millis();
            VAR_PRINTLN(turnStartTime);
            turnTol = 1;

            Devices::control.resetPIDs();

            mainState = MainStates::TURN;
            // BREAK;
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
                // BREAK;
                break;
            }

            // TODO: add a case to handle ramps, maybe just do off of encoders

            if (getBackDistance() >= TOF_TIMEOUT_VALUE) {
                LACK;
                targetBackDist = -1; // can't rely on the back dist
            } else {
                LACK;
                float currentTilesBackFloat = getBackDistance() / 300.0;
                int currentTilesBack = currentTilesBackFloat;
                // if there is more than 90% of a tile there you can count is as a full tile
                if (currentTilesBackFloat - currentTilesBack >= 0.9) 
                    currentTilesBack++;
                targetBackDist = (currentTilesBack + currMove.distance) * 300 + (300 - ROBOT_LENGHT_MM - FRONT_WALL_DIST_MM);
                VAR_PRINTLN(targetBackDist);
            }

            if (getFrontTopDistance() >= TOF_TIMEOUT_VALUE) {
                LACK;
                targetFrontDist = -1;
            } else {
                LACK;
                float currentTilesFrontFloat = getFrontTopDistance() / 300.0;
                int currentTilesFront = currentTilesFrontFloat;
                // if there is more than 90% of a tile there you can count is as a full tile (to avoid uneccecary panic modes)
                if (currentTilesFrontFloat - currentTilesFront >= 0.9) 
                    currentTilesFront++;
                if (currMove.distance > currentTilesFront) {
                    ERROR_MINOR(F("Mapper gave me bullshit data going into panic mode!"), SET_RED);
                    mapper.panicMode();
                    mainState = MainStates::GET_MOVE;
                    // BREAK;
                    break;
                }
                targetFrontDist = (currentTilesFront - currMove.distance) * 300 + FRONT_WALL_DIST_MM;
                VAR_PRINTLN(targetFrontDist);
            }

            targetEncoderDist = getEncoderValueMM() + 300 * currMove.distance;
            VAR_PRINTLN((long)targetEncoderDist);
            
            mainState = MainStates::DRIVE;

            // BREAK;
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // drive:
        case MainStates::DRIVE: {
            LACK;

            if (getFrontTopDistance() >= TOF_TIMEOUT_VALUE)
                targetFrontDist = -1;

            if (getBackDistance() >= TOF_TIMEOUT_VALUE)
                targetBackDist = -1;

            if ((targetBackDist == -1 && targetFrontDist == -1 && getEncoderValueMM() >= targetEncoderDist) || // both sensors failed (relying only on encoders)
                (targetFrontDist != -1 && getFrontTopDistance() <= targetFrontDist) || // front tof did not fail, checking if finished
                (targetBackDist != -1 && getBackDistance() >= targetBackDist))  // back tof did not fail, checking if finished
            {
                Devices::motors.setSpeeds(0, 0, 0, 0);
                mainState = MainStates::COMPLETE_MOVE;
                // BREAK;
                break;
            }

            Devices::control.keepHeading(WALL_DIST_MM, 1);
            
            break;
        }
        
        // ----------------------------------------------------------------------------------------------------
        // complete action:
        case MainStates::COMPLETE_MOVE: {

            mapper.completeCurrMove();
            
            mainState = MainStates::GET_MOVE;

            // BREAK;
            break;
        }

        }
    }

    // TODO: Actions after finishing the maze
    // BREAK;
}

#endif