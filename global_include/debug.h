// set this to true to get a debug output or false to not
#define DEBUG_MODE true
#define DELAYS_ENABLED true

#include <Arduino.h>
#include <ansiColorCodes.h>
#include <MemoryFree.h>

#define PRIM_CAT(a, b) a##b
#define CAT(a, b) PRIM_CAT(a, b)

#define DB_PRINT_MUL_EXP_1(val) DB_PRINT(val); DB_PRINT_MUL_EXP_2
#define DB_PRINT_MUL_EXP_1_END
#define DB_PRINT_MUL_EXP_2(val) DB_PRINT(val); DB_PRINT_MUL_EXP_1
#define DB_PRINT_MUL_EXP_2_END

// this is a define to put multiple prints in one line like:
// DB_PRINT_MUL(("Hello, ")(foo)(" World!")('\n'))
// would expand to:
// DB_PRINT("Hello, "); DB_PRINT(foo); DB_PRINT(" World!"); DB_PRINT('\n');
#define DB_PRINT_MUL(seq) if(1) {CAT(DB_PRINT_MUL_EXP_1 seq, _END)}

#define VOID_MACRO(...) ((void)0)

#define IF(cond, macro1, macro2, ...) CAT(IF_, cond)(macro1, macro2, __VA_ARGS__)

#define IF_1(m1, m2, ...) m1(__VA_ARGS__)
#define IF_0(m1, m2, ...) m2(__VA_ARGS__)
#define IF_true(m1, m2, ...) m1(__VA_ARGS__)
#define IF_false(m1, m2, ...) m2(__VA_ARGS__)

#if DEBUG_MODE

    #ifdef BEGIN_DEBUG
    #undef BEGIN_DEBUG
    #endif
    #define BEGIN_DEBUG(braud_rate) Serial.begin(braud_rate)
    
    #ifdef CONNECTED
    #undef CONNECTED
    #endif
    #define CONNECTED ((bool)Serial)
    
    #ifdef DB_PRINT
    #undef DB_PRINT
    #endif
    #define DB_PRINT(message) Serial.print(message)
    
    #ifdef DB_PRINTLN
    #undef DB_PRINTLN
    #endif
    #define DB_PRINTLN(message) Serial.println(message)

    #ifdef DB_PRINT_BIN
    #undef DB_PRINT_BIN
    #endif
    #define DB_PRINT_BIN(value, b0, b1) for (int bit = b0; bit < (b1); bit++) { DB_PRINT(((value) >> bit) & 1); }
    
    #ifdef DB_PRINTLN_BIN
    #undef DB_PRINTLN_BIN
    #endif
    #define DB_PRINTLN_BIN(value, b0, b1) if (1) { DB_PRINT_BIN(value, b0, b1); DB_PRINTLN(""); }

    #ifdef DB_COLOR_PRINT
    #undef DB_COLOR_PRINT
    #endif
    #define DB_COLOR_PRINT(message, color) if (1) {DB_PRINT(color); DB_PRINT(message); DB_PRINT(RESET_COLOR);}

    #ifdef DB_COLOR_PRINTLN
    #undef DB_COLOR_PRINTLN
    #endif
    #define DB_COLOR_PRINTLN(message, color) if (1) {DB_COLOR_PRINT(message, color); DB_PRINTLN();}

    #ifdef ERROR_MINOR
    #undef ERROR_MINOR
    #endif
    #define ERROR_MINOR(message, color)  if (1) {\
                                DB_PRINT(color);\
                                DB_PRINT(F("An error occoured in file: "));\
                                DB_PRINT(__FILE__);\
                                DB_PRINT(F(" in line: "));\
                                DB_PRINTLN(__LINE__);\
                                DB_PRINT(F("\tError: "));\
                                DB_PRINTLN(message);\
                                DB_PRINT(RESET_COLOR);\
                            }
    
    #ifdef ERROR
    #undef ERROR
    #endif
    #define ERROR(message) if (1) {ERROR_MINOR(message, SET_RED); while (1) {}}
    
    #ifdef LACK // line acknowledge
    #undef LACK
    #endif
    #define LACK if (1) {DB_PRINT(__FILE__); DB_PRINT(F(": ")); DB_PRINT(__LINE__); DB_PRINTLN(F(" Reached!"));}

    #ifdef RACK // ram acknowledge
    #undef RACK
    #endif
    #define RACK DB_PRINT_MUL((F("Free Ram: "))(freeMemory())(F(" bytes at: "))(F(__FILE__))(F(": "))(__LINE__)('\n'))

    #ifdef DELAY
    #undef DELAY
    #endif
    #if DELAYS_ENABLED
    #define DELAY(ms) delay(ms)
    #else
    #define DELAY(ms) ((void)0)
    #endif

    #ifdef BREAK
    #undef BREAK
    #endif
    #define BREAK \
        if (1) {\
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
        }

    #ifdef BREAK_CON
    #undef BREAK_CON
    #endif
    #define BREAK_CON(condition)\
        if (condition) {\
            DB_PRINT(SET_BLUE);\
            DB_PRINT(F("Because of contition \'"));\
            DB_PRINT(F(#condition));\
            DB_PRINTLN(F("\' being true:"));\
            BREAK\
        }

    #ifdef VAR_PRINT
    #undef VAR_PRINT
    #endif
    #define VAR_PRINT(var) if (1) {DB_PRINT(F(#var)); DB_PRINT(": "); DB_PRINT(var);}

    #ifdef VAR_PRINTLN
    #undef VAR_PRINTLN
    #endif
    #define VAR_PRINTLN(var) if (1) {VAR_PRINT(var); DB_PRINTLN();}

    #ifdef VAR_FUNC_PRINT
    #undef VAR_FUNC_PRINT
    #endif
    #define VAR_FUNC_PRINT(var) if (1) {DB_PRINT(F(#var)); DB_PRINT(": "); (var).print();}

    #ifdef VAR_FUNC_PRINTLN
    #undef VAR_FUNC_PRINTLN
    #endif
    #define VAR_FUNC_PRINTLN(var) if (1) {VAR_FUNC_PRINT(var); DB_PRINTLN();}

#else

    #ifdef BEGIN_DEBUG
    #undef BEGIN_DEBUG
    #endif
    #define BEGIN_DEBUG(braud_rate) ((void)0)
    
    #ifdef CONNECTED
    #undef CONNECTED
    #endif
    #define CONNECTED true
    
    #ifdef DB_PRINT
    #undef DB_PRINT
    #endif
    #define DB_PRINT(message) ((void)0)
    
    #ifdef DB_PRINTLN
    #undef DB_PRINTLN
    #endif
    #define DB_PRINTLN(message) ((void)0)

    #ifdef DB_PRINT_BIN
    #undef DB_PRINT_BIN
    #endif
    #define DB_PRINT_BIN(value, b0, b1) ((void)0)
    
    #ifdef DB_PRINTLN_BIN
    #undef DB_PRINTLN_BIN
    #endif
    #define DB_PRINTLN(value, b0, b1) ((void)0)

    #ifdef DB_COLOR_PRINT
    #undef DB_COLOR_PRINT
    #endif
    #define DB_COLOR_PRINT(message, color) ((void)0);

    #ifdef DB_COLOR_PRINTLN
    #undef DB_COLOR_PRINTLN
    #endif
    #define DB_COLOR_PRINTLN(message, color) ((void)0);

    #ifdef ERROR_MINOR
    #undef ERROR_MINOR
    #endif
    #define ERROR_MINOR(message, color) ((void)0)

    #ifdef ERROR
    #undef ERROR
    #endif
    #define ERROR(message) ((void)0)

    #ifdef LACK // line acknowledge
    #undef LACK
    #endif
    #define LACK ((void)0);

    #ifdef RACK // ram acknowledge
    #undef RACK
    #endif
    #define RACK ((void)0);

    #ifdef DELAY
    #undef DELAY
    #endif
    #define DELAY(ms) ((void)0)

    #ifdef BREAK
    #undef BREAK
    #endif
    #define BREAK ((void)0);

    #ifdef BREAK_CON
    #undef BREAK_CON
    #endif
    #define BREAK_CON(condition) ((void)0);

    #ifdef VAR_PRINT
    #undef VAR_PRINT
    #endif
    #define VAR_PRINT(var) ((void)0);

    #ifdef VAR_PRINTLN
    #undef VAR_PRINTLN
    #endif
    #define VAR_PRINTLN(var) ((void)0);

    #ifdef VAR_FUNC_PRINT
    #undef VAR_FUNC_PRINT
    #endif
    #define VAR_FUNC_PRINT(var) ((void)0);

    #ifdef VAR_FUNC_PRINTLN
    #undef VAR_FUNC_PRINTLN
    #endif
    #define VAR_FUNC_PRINTLN(var) ((void)0);

#endif