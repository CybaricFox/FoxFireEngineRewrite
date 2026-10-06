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

/**
 * @brief Collection of string functions
 */
class FOXFIRE_API StringUtils {
public:
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToFloat(const String &string, float& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToDouble(const String &string, double& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToChar(const String &string, char& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToShort(const String &string, short& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToInt(const String &string, int& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToLong(const String &string, long& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToUChar(const String &string, unsigned char& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToUShort(const String &string, unsigned short& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToUInt(const String &string, unsigned int& out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToULong(const String &string, ULong &out);
    /**
     * @brief Converts a string to the out type.
     * @param string
     * @param out
     * @return False if the conversion failed.
     */
    static bool stringToBool(const String &string, bool& out);

    /**
     * @brief Gets an array of substrings split by the given regex. Calls the strings inner recursiveSplit but because this version uses a dynamic array, string memory is automated.
     * @param string String to split.
     * @param regex Char to act as the split location.
     * @param array OUT array of strings.
     * @return Number of substrings.
     */
    static unsigned int recursiveSplit(String &string, char regex, DynamicArray<String> &array);

    /**
     * @brief Returns the parent directory in the path.
     * @param path
     * @return
     */
    static String getDirectoryFromPath(String &path);

    /**
     * @brief Returns the file name and extension in the path.
     * @param path
     * @return
     */
    static String getFilenameFromPath(String &path);

    /**
     * @brief Returns the file name in the path without the extension.
     * @param path
     * @return
     */
    static String getFilenameNoExtensionFromPath(String &path);
};


