#include <devices.h>
#include <debug.h>

#include <config.h>

#define USE_drive true
#if CAT(USE_, CURR_MAIN)
#undef USE_drive

int main() {
    init();
    BEGIN_DEBUG(BAUDE_RATE);

    while (true) {

    }
}

#endif