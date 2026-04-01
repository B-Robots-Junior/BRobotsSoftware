#ifndef CONSTANTS_H
#define CONSTANTS_H

#define ROBOT_WIDTH_MM 130
#define ROBOT_LENGHT_MM 190

#define FRONT_HEIGHT_DELTA_MM 42

#define WALL_DETECT_EXTRA 20

#define WALL_DIST_MM 70 // 85 // 30 // distance to keep away from the walls when no two walls are given
#define FRONT_WALL_DIST_MM 55 // 30 // 67 // (300 - ROBOT_LENGHT_MM) / 2

#define BATTERY_REF_VOLT -1 //! Set this to a sensable value
#define BATTERY_VOLT_DIVIDER_FACTOR -1 //! Set this to a sensable value

// bumper values: (adj = adjust)
#define BUMPER_ADJ_DURATION 1000
#define BUMPER_ADJ_SPEED 50
#define BUMPER_ADJ_RADIUS 20
#define BUMPER_FRONT_TIME_MUL 1.1

// tof adjustment values:
#define TOF_LF_ADJ 3
#define TOF_LB_ADJ 19
#define TOF_RF_ADJ 15
#define TOF_RB_ADJ 13

#define TOF_FRONT_ADJ -14
#define TOF_FRONT_TOP_ADJ -10

#define TOF_FRONT_BOTTOM_R_ADJ -8
#define TOF_FRONT_BOTTOM_L_ADJ 7

#define TOF_BACK_ADJ -15

#define RAMP_INCLINE_THRESHOLD 15

#define FRONT_RAMP_THRESHOLD 45

#define BLACK_TILE_TOL 75

#endif