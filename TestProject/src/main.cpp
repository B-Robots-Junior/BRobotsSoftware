#include <Arduino.h>
#include <debug.h>
#include <errorHandling.h>

Likely<float> errorTest(bool error) {
    if (error)
        return Likely<float>("Test Error!");
    return Likely<float>(100.0);
}

int main() {
    init();

    BEGIN_DEBUG(9600);

    RACK;

    DB_PRINTLN(errorTest(true).failed());
    DB_PRINTLN(*errorTest(true).getError());
    DB_PRINTLN(errorTest(false).failed());
    DB_PRINTLN(errorTest(false).getData());

    while (true) {}
}