/**
*   @file StringUtils.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "src/defines.h"
#include <foxfire_export.h>

#include "src/modules/engine/Memory/DynamicArray.h"

/*
 * REMINDER OF STRING FUNCTIONS THAT ARE USEFUL
 * string.substr(start, legnth) returns a substring.
 * string.length() returns the length of the string.
 * string.find(char) returns the index of the first occurence of that char.
 */

/**
 * @brief Collection of string functions
 */
class FOXFIRE_API StringUtils {
public:
    /**
     * @brief Checks if strings are equal, ignoring their case.
     * @return True if equal
     */
    static bool equalsIgnoreCase(const String &a, const String &b);

    /**
     * @brief Copies the first n characters
     * @param string String to copy
     * @param size number of characters from index 0
     * @return Substring of only the first n characters
     */
    static String copyLimited(String string, long size);

    /**
     * @brief Removes whitespace from the start and end of the string
     * @param out String to trim.
     */
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
    static unsigned int findAll(const String &string, char toFind);
    static unsigned int recursiveSplit(const String &string, char regex, DynamicArray<String>& array);
};


