#ifndef RASPI_DEBUG_H
#define RASPI_DEBUG_H

#include <Arduino.h>
#include <devices.h>
#include <MemoryFree.h>

#define RASPI_OUTPUT true

#define PRIM_CAT(a, b) a##b
#define CAT(a, b) PRIM_CAT(a, b)

#if RASPI_OUTPUT
    #define RP_ONLY(x) x
#else
    #define RP_ONLY(x) ((void)0)
#endif

#define RP_PRINT(message) RP_ONLY(Devices::comms.debugLog(message))
#define RP_PRINTLN(message) RP_ONLY(do {Devices::comms.debugLog(message); Devices::comms.debugLog(F("\n"));} while(0))

#define RP_PRINT_MUL(seq) RP_ONLY(do {CAT(_RP_PRINT_MUL_HELPER_1 seq, _END)} while(0))
#define _RP_PRINT_MUL_HELPER_1(message) RP_PRINT(message); _RP_PRINT_MUL_HELPER_2
#define _RP_PRINT_MUL_HELPER_2(message) RP_PRINT(message); _RP_PRINT_MUL_HELPER_1
#define _RP_PRINT_MUL_HELPER_1_END
#define _RP_PRINT_MUL_HELPER_2_END

#define RP_PRINT_VAR(var) RP_ONLY(RP_PRINT_MUL((F(#var))(F(": "))(String(var))))

#define RP_LACK RP_ONLY(RP_PRINT_MUL((F("Reached line: "))(String(__LINE__))(F(" in file: "))(__FILE__)))

#define RP_RACK RP_PRINT_MUL((F("Free Ram: "))(String(freeMemory()))(F(" bytes at: "))(F(__FILE__))(F(": "))(String(__LINE__))("\n"))

#endif