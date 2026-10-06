//
// Created by cmorg on 10/5/2026.
//

#pragma once
#include <charconv>
#include <cstring>

#include "foxfire_export.h"
#include "src/defines.h"

/**
 *  @file String.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 10/5/2026
 *
 *  @copyright (c) 2026
 */

class FOXFIRE_API String {
private:
    char* string = nullptr;
    ULong byteCount = 0;

    bool tempCheck = false;

    /**
     * @brief Allocates the string with the size but does not set any values yet.
     * @param newSize The size must NOT include \0.
     */
    void reserve(ULong newSize);

public:
    String() {tempCheck = true;}
    ~String() {destroy();}
    explicit String(const ULong newSize) {tempCheck = true; reserve(newSize);}
    String(const char* newString, const ULong newSize) {tempCheck = true; setString(newString, newSize);}

    String(const char* newString) {
        tempCheck = true;
        if (!newString) {
            return;
        }

        setString(newString,std::strlen(newString));
    }

    template<ULong S>
    String(const char (&newString)[S]) {
        tempCheck = true;
        setString(newString, S - 1);
    }

    //copy constructor
    String(const String& other) {
        tempCheck = true;
        if (other.string) {
            setString(other.string, other.byteCount);
        }
    }
    //copy assignment
    String& operator=(const String& other) {
        if (this == &other) {
            return *this;
        }

        if (other.string) {
            setString(other.string, other.byteCount);
        } else {
            destroy();
        }

        return *this;
    }

    //Move assignments
    String(String&& other) noexcept
     : string(other.string),
       byteCount(other.byteCount) {
        tempCheck = true;
        other.string = nullptr;
        other.byteCount = 0;
    }
    String& operator=(String&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        destroy();

        string = other.string;
        byteCount = other.byteCount;

        other.string = nullptr;
        other.byteCount = 0;

        return *this;
    }

    template<ULong S>
    String& operator=(const char (&newString)[S]) {
        if (newString[0] == '\0') {return *this;}

        ULong count = S;
        for (ULong i = 0; i < S; i++) {
            if (newString[i] == '\0') {
                count = i;
                break;
            }
        }

        setString(newString, count);

        return *this;
    }
    template<ULong S>
    bool operator==(const char (&other)[S]) const {
        constexpr ULong otherLength = S - 1;

        if (byteCount != otherLength) {
            return false;
        }

        for (ULong i = 0; i < byteCount; ++i) {
            if (string[i] != other[i]) {
                return false;
            }
        }

        return true;
    }

    String operator+(const String& other) const;
    String& operator+=(const String& other);
    bool operator==(const String& other) const;

    [[nodiscard]] bool isEmpty() const {return string == nullptr;}
    [[nodiscard]] ULong getSizeInBytes() const {return byteCount;}
    [[nodiscard]] const char* getAsCharString() const {return string;}
    [[nodiscard]] char getAt(ULong index) const;

    /**
     * @brief Sets the internal string and byte count.
     * @param newString The characters in the new string.
     * @param bytes The number of bytes not include \0.
     */
    void setString(const char* newString, ULong bytes);
    void setAt(ULong index, char value) const;

    void clear() const;
    void destroy();

    /**
     * @brief Trims whitespace out of this string. Can result in a null string.
     */
    void trim();

    String substringLeft(ULong index);
    String substringRight(ULong index);

    /**
     * @brief Copies the string into this one.
     * @param other The string to copy.
     * @param size The number of chars to copy. Must NOT include \0!. If not given, copies the full string.
     */
    void copy(const String& other, ULong size = 0);
    [[nodiscard]] bool equalsIgnoreCase(const String& other) const;

    /**
     * @brief Returns true if this string is equal to another string for n elements.
     * @param other The other string
     * @param amount The amount of indices to check.
     * @param first Optional index to start at.
     * @return True if the strings are equal in this range.
     */
    [[nodiscard]] bool equalsN(const String& other, ULong amount, ULong first = 0) const;
    [[nodiscard]] bool equalsNIgnoreCase(const String& other, ULong amount, ULong first = 0) const;

    unsigned int findAll(char toFind);

    /**
     * @brief Finds the first instance of a character.
     * @param toFind Character to find.
     * @param skip The number of time to skip a found char.
     * @return First index of character. INVALID_ID_U64 if no character is found.
     */
    [[nodiscard]] ULong find(char toFind, unsigned int skip = 0) const;
    [[nodiscard]] ULong findLast(char toFind) const;

    /**
     * @brief Returns an array of substrings split by the given regex. If strings in null, then the strings will need to be manually deallocated later on.
     * @param regex The char to split
     * @param strings Array of strings
     * @return Number of strings created.
     */
    unsigned int recursiveSplit(char regex, String *strings) const;

    //For loops
    char* begin() {
        return string;
    }

    char* end() {
        return string + byteCount;
    }

    const char* begin() const {
        return string;
    }

    const char* end() const {
        return string + byteCount;
    }

    //Subscripts
    char& operator[](const ULong index) {
        return string[index];
    }

    const char& operator[](const ULong index) const {
        return string[index];
    }
};

template<typename T>
requires (
    std::is_arithmetic_v<T> &&
    !std::same_as<std::remove_cv_t<T>, char> &&
    !std::same_as<std::remove_cv_t<T>, bool>
)
String toString(const T value) {
    char buffer[128]{};

    const auto [ptr, error] = std::to_chars(buffer, buffer + sizeof(buffer), value);

    if (error != std::errc{}) {
        return {};
    }

    *ptr = '\0';

    return String{buffer, static_cast<ULong>(ptr - buffer)};
}
template<typename T>
requires (std::same_as<std::remove_cv_t<T>, bool>)
String toString(const bool value) {
    String result{};
    if (value) {
        result = "True";
    } else {
        result = "False";
    }

    return result;
}

inline String toString(const char value) {
    String result{1};

    result.setAt(0, value);

    return result;
}

template<ULong S>
String operator+(const char (&left)[S], const String& right) {
    return String(left) + right;
}