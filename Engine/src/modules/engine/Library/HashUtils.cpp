//
// Created by cmorg on 7/31/2026.
//

#include "HashUtils.h"

unsigned long HashUtils::multiplier = 97;

unsigned long HashUtils::generateStringHash(const String &key) {
    unsigned long hash = 0;
    for (const char c : key) {
        hash = hash * multiplier + c;
    }

    return hash;
}
