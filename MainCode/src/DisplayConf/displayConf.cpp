#include <DisplayConf/displayConf.h>
#include <Display/display.h>
#include <Display/menu.h>
#include <Display/element.h>
#include <Display/inputHandler.h>
#include <pins.h>

GEN_ENCODER_INPUT(mainInput, ROTARY_ENCODER_CLK_PIN, ROTARY_ENCODER_DT_PIN, ROTARY_ENCODER_SW_PIN);

// shared:
Rect outline(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
GEN_MENU_SELECTABLE(back, 4, 55, 1, "< Back", &mainMenu);
FuncWalls tofWalls(4, 25, 20, 20, 4, wallBack, wallLeft, wallFront, wallRight);

// main menu:
GEN_CONST_TEXT(title, 3, 3, "BRobots", 2);

GEN_SCRIPT_SELECTABLE(start, 4, 25, 1, "Start >", mainFunc, &runMenu);
GEN_MENU_SELECTABLE(calib, 4, 34, 1, "Calib >", &calibMenu);
GEN_MENU_SELECTABLE(test, 4, 43, 1, "Test >", &testMenu);

Menu mainMenu({ &outline, &title }, { &start, &calib, &test}, &mainInput);

// run menu:
GEN_CONST_TEXT(runningTitle, 3, 3, "Running", 2);
Menu runMenu({ &outline, &runningTitle, &tofWalls }, {}, &mainInput);

// calibration menu:
GEN_SCRIPT_SELECTABLE(calibNormal, 4, 3, 1, "Normal >", calibrateScript<ColorType::Normal>, &calibMenu);
GEN_SCRIPT_SELECTABLE(calibBlue, 4, 13, 1, "Blue >", calibrateScript<ColorType::Blue>, &calibMenu);
GEN_SCRIPT_SELECTABLE(calibBlack, 4, 23, 1, "Black >", calibrateScript<ColorType::Black>, &calibMenu);
GEN_SCRIPT_SELECTABLE(calibCheck, 4, 33, 1, "Check >", calibrateScript<ColorType::Checkpoint>, &calibMenu);
GEN_SCRIPT_SELECTABLE(calibRed, 4, 43, 1, "Red >", calibrateScript<ColorType::Red>, &calibMenu);
Menu calibMenu({ &outline }, { &calibNormal, &calibBlue, &calibBlack, &calibCheck, &calibRed, &back }, &mainInput);

// in calibration menu:
GEN_CONST_TEXT(calibrating, 3, 30, "Calibrating!", 1);
Menu inCalibrationMenu({ &outline, &calibrating }, {}, &mainInput);

// test menu:
GEN_CONST_TEXT(testTitle, 3, 3, "Test", 2);
Menu testMenu({ &outline, &testTitle, &tofWalls }, { &back }, &mainInput);