//
// Created by cmorg on 7/31/2026.
//

#pragma once
#include "src/defines.h"
#include <foxfire_export.h>


class FOXFIRE_API HashUtils {
private:
    static unsigned long multiplier; //prime number

public:
    static unsigned long generateStringHash(const String& key);
};
