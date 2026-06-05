#include <Arduino.h>
// #include <smartArray.h>
#include <pos.h>
#include <TC/tc.h>
#include <Mapping/mapping.h>

#include <config.h>

#define USE_tcOut true
#if CAT(USE_, CURR_MAIN)
#undef USE_tcOut

int main() {
    init();
    sei();

    Serial.begin(BAUDE_RATE);

    Serial.println(F("MAP v1"));
    Serial.println(F("META team=BRobots\n"));

    SimpleArray<Pos<int8_t>> seenTiles;
    SimpleArray<Pos<int8_t>> seenVictims;

    uint16_t numElements = getNumElements();

    for (uint16_t i = 0; i < numElements; i++) {
        tc_element_t element = getElement(i);
        element.y = -element.y;
        
        if (element.type == TC_TILE) {
            bool exsits = false;
            for (uint16_t j = 0; j < seenTiles.size(); j++) {
                if (seenTiles[j].x == element.x && seenTiles[j].y == element.y) {
                    exsits = true;
                    break;
                }
            }
            if (exsits) continue;
            seenTiles.push_back(Pos<int8_t>(element.x, element.y));

            Serial.print(F("TILE "));
            Serial.print(element.x);
            Serial.print(F(" "));
            Serial.print(element.y);
            Serial.print(F(" "));
            if (element.x == 0 && element.y == 0)
                Serial.println(F("start"));
            else {
                switch (_GET_TYPE(element.data)) {
                case NORMAL_TILE: Serial.println("normal"); break;
                case BLACK_TILE: Serial.println("black"); break;
                case CHECKPOINT_TILE: Serial.println("silver"); break;
                case BLUE_TILE: Serial.println("blue"); break;
                case RAMP_TILE: Serial.println("ramp"); break;
                }
            }

            if (_GET_NORTH(element.data)) {
                Serial.print(F("WALL "));
                Serial.print(element.x);
                Serial.print(F(" "));
                Serial.print(element.y);
                Serial.println(F(" N"));
            }
            if (_GET_EAST(element.data)) {
                Serial.print(F("WALL "));
                Serial.print(element.x);
                Serial.print(F(" "));
                Serial.print(element.y);
                Serial.println(F(" E"));
            }
            if (_GET_SOUTH(element.data)) {
                Serial.print(F("WALL "));
                Serial.print(element.x);
                Serial.print(F(" "));
                Serial.print(element.y);
                Serial.println(F(" S"));
            }
            if (_GET_WEST(element.data)) {
                Serial.print(F("WALL "));
                Serial.print(element.x);
                Serial.print(F(" "));
                Serial.print(element.y);
                Serial.println(F(" W"));
            }
        } else if (element.type == TC_VICTIM) {
            bool exsits = false;
            for (uint16_t j = 0; j < seenVictims.size(); j++) {
                if (seenVictims[j].x == element.x && seenVictims[j].y == element.y) {
                    exsits = true;
                    break;
                }
            }
            if (exsits) continue;
            seenVictims.push_back(Pos<int8_t>(element.x, element.y));

            Serial.print(F("VICTIM "));
            Serial.print(element.x);
            Serial.print(F(" "));
            Serial.print(element.y);
            switch ((element.data >> 8) & 0xFF) {
            case 0: Serial.print(F(" N ")); break;
            case 1: Serial.print(F(" E ")); break;
            case 2: Serial.print(F(" S ")); break;
            case 3: Serial.print(F(" W ")); break;
            }
            switch (element.data & 0xFF) {
            case 0: Serial.println(F("U")); break;
            case 1: Serial.println(F("S")); break;
            case 2: Serial.println(F("H")); break;
            }
        }
    }

    Serial.println(F("\nEND"));

    while (true) {}
}

#endif