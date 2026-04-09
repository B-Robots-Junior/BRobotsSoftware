#include <Drive/Control.h>
#include <Util/utility.h>
#include <constants.h>

Control::Control(DualTB9051FTGMotorShield &motors) : _motors(motors) {
    _turnActive = false;
}

void Control::resetPIDs() {
    _pidTurn.reset();
    _pidDrive.reset();
    _pidHeading.reset();
    _pidGyro.reset(); 
}



int Control::turnRobot(float targetAngle, uint32_t startTime, float tolerance) {
    if (!_turnActive) {
        _pidTurn.reset();
        _turnActive = true;
    }

    float currentAngle = ReadGyroyaw();
    float angleDifference = angleDiffDEG(targetAngle, currentAngle);
    static unsigned long lastTurnPrint = 0;
    if (millis() - lastTurnPrint > 100) {
        DB_PRINTLN(F("Ist-Winkel: "));
        DB_PRINTLN(currentAngle);
        DB_PRINTLN(F(" | Diff: "));
        DB_PRINTLN(angleDifference);
        lastTurnPrint = millis();
    }

    if (millis() - startTime > 5000) {
        _motors.setSpeeds(0, 0, 0, 0);
        _turnActive = false;
        return -1; // Timeout
    }

    if (abs(angleDifference) <= tolerance) {
        _motors.setSpeeds(0, 0, 0, 0);
        _turnActive = false;
        return 0; 
    }

    float correction = _pidTurn.calculate(angleDifference, TURN_KP, TURN_KI, TURN_KD, TURN_INT_LIMIT);

    float speed = constrain(abs(correction), MIN_SPEED_TURN, MAX_SPEED_TURN);
    
    int direction = (correction < 0) ? -1 : 1;

    int m1Speed =  -direction * speed; 
    int m2Speed =  -direction * speed; 
    
    int m3Speed = direction * speed; 
    int m4Speed = direction * speed; 

    _motors.setSpeeds(m1Speed, m2Speed, m3Speed, m4Speed);
    
    return 1; 
}

void Control::keepCentered(float speedMul) {
    _turnActive = false; 

    float leftDistance = getLeftDistance();
    float rightDistance = getRightDistance();
    
    float centerError = ((leftDistance + rightDistance) / 2.0) - TARGET_CENTER_WALL_DIST;
    float errorDiff = leftDistance - rightDistance;
    float error = centerError + 0.2 * errorDiff;

    float correction = _pidDrive.calculate(error, CENTER_KP, CENTER_KI, CENTER_KD, CENTER_INT_LIMIT);

    correction = constrain(correction, -MAX_CORRECTION_DRIVE, MAX_CORRECTION_DRIVE);
    int baseSpeed = BASE_SPEED_DRIVE;

    int m1Speed = (baseSpeed + correction) * speedMul;
    int m2Speed = (baseSpeed - correction) * speedMul;
    int m3Speed = (baseSpeed + correction) * speedMul;
    int m4Speed = (baseSpeed - correction) * speedMul;

    _motors.setSpeeds(m1Speed, m2Speed, m3Speed, m4Speed);
}

void Control::keepHeading(float targetDistance, float speedMul) {
    float rightDist = getRightDistance();
    float leftDist = getLeftDistance();
    float averageDistance;
    int invertMove = 1;

    if (rightDist < 200 && rightDist >= 0) {
        averageDistance = rightDist;
        invertMove = 1;  // Rechte Wand
    } else {
        averageDistance = leftDist;
        invertMove = -1; // Linke Wand
    }

    if (rightDist > 200 && leftDist > 200) {
        int speed = BASE_SPEED_DRIVE * speedMul;
        _motors.setSpeeds(speed, speed, speed, speed);
        return; 
    }

    float angle = calculateposition() * invertMove * -1;
    float error = (targetDistance - averageDistance) + angle * 0.4;
    float correction = _pidHeading.calculate(error, HEADING_KP, HEADING_KI, HEADING_KD, HEADING_INT_LIMIT);

    correction = constrain(correction, -MAX_CORRECTION_DRIVE, MAX_CORRECTION_DRIVE);

    int baseSpeed = BASE_SPEED_DRIVE; 

    
    int m1Speed = (baseSpeed - correction * invertMove) * speedMul; // Links
    int m2Speed = (baseSpeed - correction * invertMove) * speedMul; // Links
    
    int m3Speed = (baseSpeed + correction * invertMove) * speedMul; // Rechts
    int m4Speed = (baseSpeed + correction * invertMove) * speedMul; // Rechts

    _motors.setSpeeds(m1Speed, m2Speed, m3Speed, m4Speed);
}

void Control::keepCenteredgyro(float angle, float speedMul) {
    float currentYaw = ReadGyroyaw();
    float error = fmod(angle - currentYaw + 180, 360) - 180;

    float correction = _pidGyro.calculate(error, GYRO_KP, GYRO_KI, GYRO_KD, GYRO_INT_LIMIT);

    int baseSpeed = BASE_SPEED_DRIVE; // 130
    correction = constrain(correction, -MAX_CORRECTION_DRIVE, MAX_CORRECTION_DRIVE); // 30

    int m1Speed = (baseSpeed + correction) * speedMul;  
    int m2Speed = (baseSpeed - correction) * speedMul;  
    int m3Speed = (baseSpeed + correction) * speedMul;  
    int m4Speed = (baseSpeed - correction) * speedMul;  

    _motors.setSpeeds(m1Speed, m2Speed, m3Speed, m4Speed);
}

int Control::driveAlong(int targetBackDist, int targetFrontDist, float targetDist, float target_angle, int64_t& lastEncoderDist, int64_t& trueEncoderDist, int64_t targetEncoderDist, float speedMul, int wallStoppingDist) { 
    float incline = wrap180(-gyro.getPitch());
    incline = wrap180(incline);
    
    int64_t currEncoderDist = getEncoderValueMM();
    if (abs(incline) >= 12) {
        _driveAlongFailed = true;
        trueEncoderDist += (currEncoderDist - lastEncoderDist) / 1.20; // pythagoras shit
    } else {
        trueEncoderDist += (currEncoderDist - lastEncoderDist);
    }
    DB_PRINT_MUL((F("curr: "))((long)currEncoderDist)(F(" last: "))((long)lastEncoderDist)(F(" diff: "))((long)(currEncoderDist - lastEncoderDist))(F(" true: "))((int)trueEncoderDist)(F(" target: "))((long)targetEncoderDist)(" inc: ")(incline)('\n'));
    lastEncoderDist = currEncoderDist;

    if (!_driveAlongFailed) {
        int frontDist = getFrontTopDistance();
        int backDist = getBackDistance();
        
        if (targetBackDist != -1 || targetFrontDist != -1) {
            if (getFrontTopDistance() < wallStoppingDist && abs(incline) >= 12 && speedMul > 0) {
                _motors.setSpeeds(0, 0, 0, 0);
                _driveAlongFailed = false; 
                _frontFailed = false;
                _backFailed = false;
                LACK;
                return 0;
            }

            bool frontValid = frontDist > 0 && frontDist < TOF_TIMEOUT_VALUE && targetFrontDist > 0 && targetFrontDist < TOF_TIMEOUT_VALUE;
            bool backValid = backDist > 0 && backDist < TOF_TIMEOUT_VALUE && targetBackDist > 0 && targetBackDist < TOF_TIMEOUT_VALUE;
            bool frontDistFinished = speedMul > 0 ? frontDist <= targetFrontDist : frontDist >= targetFrontDist;
            bool backDistFinished = speedMul > 0 ? backDist >= targetBackDist : backDist <= targetBackDist;
            
            _frontFailed = !frontValid || _frontFailed;
            _backFailed = !backValid || _backFailed;

            // prioritize the front dist, because that is used for tile alignment, because it can check for ramps
            if (!_frontFailed) {
                if (frontDistFinished && abs(incline) < 3.0) {
                    _motors.setSpeeds(0, 0, 0, 0);
                    _driveAlongFailed = false; 
                    _frontFailed = false;
                    _backFailed = false;
                    LACK;
                    return 0;
                }
            }
            else if (!_backFailed) { // then use the back dist, because it is a simple +300
                if (backDistFinished && abs(incline) < 3.0) {
                    _motors.setSpeeds(0, 0, 0, 0);
                    _driveAlongFailed = false; 
                    _frontFailed = false;
                    _backFailed = false;
                    LACK;
                    return 0;
                }
            } else {
                _driveAlongFailed = true;
                if (targetEncoderDist == -1) {
                    _motors.setSpeeds(0, 0, 0, 0);
                    _driveAlongFailed = false; 
                    _frontFailed = false;
                    _backFailed = false;
                    LACK;
                    return -1; 
                }
            }
        }
    }

    if (_driveAlongFailed && targetEncoderDist != -1) {
        // if it is extremely obvious that you should stop in front of the next wall just drive with the front tof
        if (abs(incline) < 12 && getFrontTopDistance() < 200 && speedMul > 0) { // abs(targetEncoderDist - trueEncoderDist) < 200
            if (getFrontTopDistance() <= wallStoppingDist) {
                _motors.setSpeeds(0, 0, 0, 0);
                _driveAlongFailed = false; 
                _frontFailed = false;
                _backFailed = false;
                LACK;
                return 0;
            }
        }
        else if (trueEncoderDist >= targetEncoderDist) { // encoder values only increment and don't care about direction
            _motors.setSpeeds(0, 0, 0, 0);
            _driveAlongFailed = false; 
            _frontFailed = false;
            _backFailed = false;
            LACK;
            return 0;
        }
    }

    uncondDriveAlong(targetDist, target_angle, speedMul);

    return 1;
}

void Control::uncondDriveAlong(float targetDist, float target_angle, float speedMul) {
    float rfDist = getRFDistance();
    float rbDist = getRBDistance();
    float lfDist = getLFDistance();
    float lbDist = getLBDistance();
    
    bool rightValid = getTofRFValid() && getTofRBValid();
    bool leftValid = getTofLFValid() && getTofLBValid(); 
    
    if (rightValid && leftValid) {
        keepCentered(speedMul);
    }
    if (rightValid || leftValid) {
        keepHeading(targetDist, speedMul);
    }
    else {
        keepCenteredgyro(target_angle, speedMul);
    }
}

int Control::driveAlongEncoders(double targetEncoderVal, float targetDist, float targetAngle, float speedMul) {
    float incline = -gyro.getPitch();
    incline = wrap180(incline);
    
    if (getFrontTopDistance() <= FRONT_WALL_DIST_MM && abs(incline) <= 12)
        return 0;
        
    if (getEncoderValueMM() >= targetEncoderVal) {
        if (getFrontTopDistance() <= FRONT_WALL_DIST_MM || getFrontTopDistance() > FRONT_WALL_DIST_MM + 100 || abs(incline) >= 12)
            return 0;
    }
    
    float rfDist = getRFDistance();
    float rbDist = getRBDistance();
    float lfDist = getLFDistance();
    float lbDist = getLBDistance();
    
    bool rightValid = getTofRFValid() && getTofRBValid();
    bool leftValid = getTofLFValid() && getTofLBValid(); 
    
    //if (rightValid && leftValid) {
    //    keepCentered(speedMul);
    if (rightValid || leftValid) {
        keepHeading(targetDist, speedMul);
    } else {
        keepCenteredgyro(targetAngle, speedMul);
    }

    return 1;
}

int Control::driveBezier(Pos<float> p0, Pos<float> p1, Pos<float> pa, int baseSpeed, int turnSpeed, uint32_t startTime, uint32_t duration) {
    float t = (millis() - startTime) / (float)duration;
    
    if (t >= 1) {
        _startBezierAngle = -1; 
        return 0;
    }
    if (t < 0) { 
        _startBezierAngle = -1; 
        return 1;
    }
    
    if (_startBezierAngle == -1)
        _startBezierAngle = ReadGyroyaw();

    float angle = ReadGyroyaw();
    Pos<float> pd0 = 2 * (pa - p0);
    Pos<float> pd = 2 * (p1 - 2 * pa + p0) * t + 2 * (pa - p0);
    float targetAngle = wrap(_startBezierAngle + angleDiffDEG(atan2f(pd.y, pd.x) * RAD_TO_DEG, atan2f(pd0.y, pd0.x) * RAD_TO_DEG), 0, 360);
    
    float diff = angleDiffDEG(targetAngle, angle);
    int direc = diff < 0 ? -1 : 1;
    diff = abs(diff) / 90.0;
    diff = min(max(diff, 0.0f), 1.0f); // Sicherstellen, dass es float ist

    int motorRight = baseSpeed + turnSpeed * diff * direc;
    int motorLeft = baseSpeed - turnSpeed * diff * direc;
    _motors.setSpeeds(motorLeft, motorRight, motorLeft, motorRight);
    
    return 1;
}

int Control::driveSCurve(int speed, uint16_t radius, uint32_t startTime, uint32_t duration, int direc) {
    uint32_t time = millis() - startTime;
    if (time >= duration) {
        _motors.setSpeeds(0, 0, 0, 0);
        return 0;
    }
    int timeDirec = ((int)(time >= (duration / 2.0) * BUMPER_FRONT_TIME_MUL)) * 2 - 1; 
    
    int speedR = speed * ((radius + (ROBOT_WIDTH_MM / 2.0) * direc * timeDirec) / (float)radius);
    int speedL = speed * ((radius - (ROBOT_WIDTH_MM / 2.0) * direc * timeDirec) / (float)radius);
    
    _motors.setSpeeds(-speedL, -speedL, -speedR, -speedR);
    return 1;
}