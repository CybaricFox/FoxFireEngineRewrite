//
// Created by cmorg on 7/1/2026.
//

#pragma once
#include "src/defines.h"
#include <foxfire_export.h>

/*
 * REMINDER OF STRING FUNCTIONS THAT ARE USEFUL
 * string.substr(start, legnth) returns a substring.
 * string.length() returns the length of the string.
 * string.find(char) returns the index of the first occurence of that char.
 */

class FOXFIRE_API StringUtils {
public:
    //Checks if both strings are equal, ignoring their cases.
    static bool equalsIgnoreCase(const String &a, const String &b);
    //Copies the first n chars
    static String copyLimited(String string, long size);
    //Removes whitespace from the edges of the string
    static void trim(String &out);
    static bool stringToFloat(const String &string, float& out);
    static bool stringToDouble(const String &string, double& out);
    static bool stringToChar(const String &string, char& out);
    static bool stringToShort(const String &string, short& out);
    static bool stringToInt(const String &string, int& out);
    static bool stringToLong(const String &string, long& out);
    static bool stringToUChar(const String &string, unsigned char& out);
    static bool stringToUShort(const String &string, unsigned short& out);
    static bool stringToUInt(const String &string, unsigned int& out);
    static bool stringToULong(const String &string, unsigned long& out);
    static bool stringToBool(const String &string, bool& out);
};


