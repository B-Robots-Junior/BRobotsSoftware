// set this to true to get a debug output or false to not
#define DEBUG_MODE false
// these only matter when in debug mode
#define DELAYS_ENABLED true
#define BREAKS_ENABLED true

#include <Arduino.h>
#include <ansiColorCodes.h>
#include <MemoryFree.h>

#define PRIM_CAT(a, b) a##b
#define CAT(a, b) PRIM_CAT(a, b)

#if DEBUG_MODE
#define DEBUG_ONLY(x) x
#define IF_DEBUG(yes, no) yes
#define DEBUG_MACRO(x) x
#else
#define DEBUG_ONLY(x)
#define IF_DEBUG(yes, no) no
#define DEBUG_MACRO(x) ((void)0)
#endif

#if DELAYS_ENABLED && DEBUG_MODE
#define DELAY_ONLY(x) x
#define IF_DELAY(yes, no) yes
#define DELAY_MACRO(x) x
#else
#define DELAY_ONLY(x)
#define IF_DELAY(yes, no) no
#define DELAY_MACRO(x) ((void)0)
#endif

#if BREAKS_ENABLED && DEBUG_MODE
#define BREAK_ONLY(x) x
#define IF_BREAK(yes, no) yes
#define BREAK_MACRO(x) x
#else
#define BREAK_ONLY(x)
#define IF_BREAK(yes, no) no
#define BREAK_MACRO(x) ((void)0)
#endif

#define DB_PRINT_MUL_EXP_1(val) DB_PRINT(val); DB_PRINT_MUL_EXP_2
#define DB_PRINT_MUL_EXP_1_END
#define DB_PRINT_MUL_EXP_2(val) DB_PRINT(val); DB_PRINT_MUL_EXP_1
#define DB_PRINT_MUL_EXP_2_END

// this is a define to put multiple prints in one line like:
// DB_PRINT_MUL(("Hello, ")(foo)(" World!")('\n'))
// would expand to:
// DB_PRINT("Hello, "); DB_PRINT(foo); DB_PRINT(" World!"); DB_PRINT('\n');
#define DB_PRINT_MUL(seq) DEBUG_MACRO(do {CAT(DB_PRINT_MUL_EXP_1 seq, _END)} while(0))

#define VOID_MACRO(...) ((void)0)

#define IF(cond, macro1, macro2, ...) CAT(IF_, cond)(macro1, macro2, __VA_ARGS__)

#define IF_1(m1, m2, ...) m1(__VA_ARGS__)
#define IF_0(m1, m2, ...) m2(__VA_ARGS__)
#define IF_true(m1, m2, ...) m1(__VA_ARGS__)
#define IF_false(m1, m2, ...) m2(__VA_ARGS__)

#define BEGIN_DEBUG(braud_rate) DEBUG_MACRO(Serial.begin(braud_rate))

#define CONNECTED IF_DEBUG(((bool)Serial), true)

#define DB_PRINT(message) DEBUG_MACRO(Serial.print(message))

#define DB_PRINTLN(message) DEBUG_MACRO(Serial.println(message))

#define DB_PRINT_BIN(value, b0, b1) DEBUG_MACRO(do { for (int bit = b0; bit < (b1); bit++) { DB_PRINT(((value) >> bit) & 1); } } while (0))

#define DB_PRINTLN_BIN(value, b0, b1) DEBUG_MACRO(do { DB_PRINT_BIN(value, b0, b1); DB_PRINTLN(""); } while (0))

#define DB_COLOR_PRINT(message, color) DEBUG_MACRO(do { DB_PRINT(color); DB_PRINT(message); DB_PRINT(RESET_COLOR); } while (0))

#define DB_COLOR_PRINTLN(message, color) DEBUG_MACRO(do { DB_COLOR_PRINT(message, color); DB_PRINTLN(); } while (0))

#define ERROR_MINOR(message, color)  DEBUG_MACRO(do {\
                            DB_PRINT(color);\
                            DB_PRINT(F("An error occoured in file: "));\
                            DB_PRINT(__FILE__);\
                            DB_PRINT(F(" in line: "));\
                            DB_PRINTLN(__LINE__);\
                            DB_PRINT(F("\tError: "));\
                            DB_PRINTLN(message);\
                            DB_PRINT(RESET_COLOR);\
                        } while (0))

#define ERROR(message) DEBUG_MACRO(do {ERROR_MINOR(message, SET_RED); while (1) {}} while (0))

#define LACK DEBUG_MACRO(do {DB_PRINT(__FILE__); DB_PRINT(F(": ")); DB_PRINT(__LINE__); DB_PRINTLN(F(" Reached!"));} while (0))

#define RACK DEBUG_MACRO(DB_PRINT_MUL((F("Free Ram: "))(freeMemory())(F(" bytes at: "))(F(__FILE__))(F(": "))(__LINE__)('\n')))

#define DELAY(ms) DELAY_MACRO(delay(ms))

#define BREAK \
    BREAK_MACRO(do {\
        DB_PRINT(SET_BLUE);\
        DB_PRINT(F("Breakpoint in file: "));\
        DB_PRINT(F(__FILE__));\
        DB_PRINT(F(" in line: "));\
        DB_PRINT(__LINE__);\
        DB_PRINTLN(F(" Triggered!"));\
        DB_PRINT(RESET_COLOR);\
        while (!Serial.available()) {}\
        delay(100);\
        while (Serial.available()) { Serial.read(); }\
    } while (0))

#define BREAK_CON(condition)\
    BREAK_MACRO(do { if (condition) {\
        DB_PRINT(SET_BLUE);\
        DB_PRINT(F("Because of contition \'"));\
        DB_PRINT(F(#condition));\
        DB_PRINTLN(F("\' being true:"));\
        BREAK\
    }} while (0))

#define VAR_PRINT(var) DEBUG_MACRO(do {DB_PRINT(F(#var)); DB_PRINT(": "); DB_PRINT(var);} while (0))

#define VAR_PRINTLN(var) DEBUG_MACRO(do {VAR_PRINT(var); DB_PRINTLN();} while (0))

#define VAR_FUNC_PRINT(var) DEBUG_MACRO(do {DB_PRINT(F(#var)); DB_PRINT(": "); (var).print();} while (0))

#define VAR_FUNC_PRINTLN(var) DEBUG_MACRO(do {VAR_FUNC_PRINT(var); DB_PRINTLN();} while (0))