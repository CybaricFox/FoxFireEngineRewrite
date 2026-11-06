//
// Created by cmorg on 11/5/2026.
//

#include "UUID.h"

#include "FF_Math.h"

void UUIDUtils::setSeed(ULong seed) {
    //Does Nothing For Now
}

UUID UUIDUtils::generate() {
    char buffer[37]{};

    static String v = "0123456789abcdef";

    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            buffer[i] = '-';
        } else {
            const int offset = FF_Math::randomInt() % 16;
            buffer[i] = v[offset];
        }
    }

    buffer[36] = '\0';

    return {buffer};
}