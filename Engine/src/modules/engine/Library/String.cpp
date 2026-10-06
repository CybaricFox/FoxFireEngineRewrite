//
// Created by cmorg on 10/5/2026.
//

#include "String.h"

#include "src/modules/engine/Memory/FF_Memory.h"

void String::destroy() {
    if (string) {
        FF_Memory::ff_clear(string, sizeof(char) * (byteCount + 1));
        FF_Memory::ff_free_recursive<char>(string, STRING, byteCount + 1);
        string = nullptr;
    }

    byteCount = 0;
}

void String::trim() {
    if (isEmpty()) return;

    for (ULong i = 0; i < byteCount; i++) {
        if (!isspace(string[i])) {
            const String temp = substringRight(i);
            setString(temp.getAsCharString(), temp.getSizeInBytes());
            break;
        }

        //If we reach this, then the line is just blank.
        if (i == byteCount - 1) {
            destroy();
            return;
        }
    }

    for (int i = byteCount - 1; i > 0; i--) {
        if (!isspace(string[i])) {
            const String temp = substringLeft(i);
            setString(temp.getAsCharString(), temp.getSizeInBytes());
            break;
        }
    }
}

String String::substringLeft(const ULong index) {
    if (index >= byteCount) {
        Logger::logError("Cannot create a substring. The index is greater than the size!");
        return *this;
    }

    String result{};

    const ULong newSize = index + 1;
    result.setString(string, newSize);

    return result;
}

String String::substringRight(const ULong index) {
    if (index >= byteCount) {
        Logger::logError("Cannot create a substring. The index is greater than the size!");
        return *this;
    }

    String result{};

    const ULong newSize = byteCount - index;
    result.setString(&string[index], newSize);

    return result;
}

void String::copy(const String &other, const ULong size) {
    ULong copySize = size;
    if (copySize == 0) copySize = other.getSizeInBytes();
    if (copySize > other.getSizeInBytes()) {
        copySize = other.getSizeInBytes();
        Logger::logWarn("Copy size is greater than the strings actual size! Copy size: " + toString(size) + " Actual Size: " + toString(other.getSizeInBytes()));
    }

    setString(other.getAsCharString(), copySize);
}

bool String::equalsIgnoreCase(const String &other) const {
    if (byteCount != other.getSizeInBytes()) return false;

    const char* ca = string;
    const char* cb = other.getAsCharString();

    for (int i = 0; i <= byteCount; i++) {
        if (std::tolower(static_cast<unsigned char>(ca[i])) != std::tolower(static_cast<unsigned char>(cb[i]))) return false;
    }

    return true;
}

bool String::equalsN(const String &other, const ULong amount, const ULong first) const {
    if (byteCount < amount + first || other.getSizeInBytes() < amount + first) return false;

    for (ULong i = 0; i < amount; i++) {
        if (string[i + first] != other[i + first]) return false;
    }

    return true;
}

bool String::equalsNIgnoreCase(const String &other, const ULong amount, const ULong first) const {
    if (byteCount < amount + first || other.getSizeInBytes() < amount + first) return false;

    const char* ca = string;
    const char* cb = other.getAsCharString();

    for (unsigned long i = 0; i < amount; i++) {
        if (std::tolower(static_cast<unsigned char>(ca[i + first])) != std::tolower(static_cast<unsigned char>(cb[i + first]))) {
            return false;
        }
    }

    return true;
}

unsigned int String::findAll(const char toFind) {
    ULong count = 0;
    for (const char& c : *this) {
        if (c == toFind) {
            count++;
        }
    }

    return count;
}

ULong String::find(const char toFind, unsigned int skip) const {
    for (ULong i = 0; i < byteCount; i++) {
        if (string[i] == toFind) {
            if (skip > 0) {
                skip--;
            } else {
                return i;
            }
        }
    }

    return INVALID_ID_U64;
}

ULong String::findLast(const char toFind) const {
    for (ULong i = byteCount; i > 0; i--) {
        if (string[i] == toFind) return i;
    }

    return INVALID_ID_U64;
}

unsigned int String::recursiveSplit(const char regex, String* strings) const {
    String remaining = *this;
    const unsigned int substrings = remaining.findAll(regex) + 1;

    if (strings == nullptr) {
        strings = FF_Memory::ff_allocate_recursive<String>(STRING, substrings);

        for (unsigned int i = 0; i < substrings; i++) {
            std::construct_at(&strings[i]);
        }
    }

    unsigned int stringCount = 0;
    while (!remaining.isEmpty()) {
        const ULong index = remaining.find(regex);

        //This should occur on the last substring.
        if (index == INVALID_ID_U64) {
            remaining.trim();
            strings[stringCount] = remaining;
            return substrings;
        }

        if (substrings == stringCount) {
            Logger::logFatal("A fatal error occurred while recursive substringing. Substring count surpassed allocation size! String: " + *this);
            return 0;
        }

        strings[stringCount] = remaining.substringLeft(index - 1);
        strings[stringCount].trim();
        stringCount++;
        remaining = remaining.substringRight(index + 1);
    }

    Logger::logError("Recursive split reached the end of splitting without returning properly. " + *this + " -> is likely corrupt.");
    return 0;
}

void String::setString(const char *newString, const ULong bytes) {
    destroy();

    if (!newString) return;

    byteCount = bytes;
    string = FF_Memory::ff_allocate_recursive<char>(STRING, byteCount + 1);
    FF_Memory::ff_copy(string, newString, sizeof(char) * byteCount);

    string[byteCount] = '\0';
}

void String::reserve(const ULong newSize) {
    if (string) {
        Logger::logWarn("Cannot reserve a string that is already created.");
        return;
    }

    byteCount = newSize;
    string = FF_Memory::ff_allocate_recursive<char>(STRING, newSize + 1);
    string[byteCount] = '\0';
}

String String::operator+(const String &other) const {
    String result{};

    if (isEmpty()) return other;
    if (other.isEmpty()) return *this;

    result.byteCount = byteCount + other.getSizeInBytes();
    result.string = FF_Memory::ff_allocate_recursive<char>( STRING, result.byteCount + 1);

    FF_Memory::ff_copy(result.string, string, sizeof(char) * byteCount);
    FF_Memory::ff_copy(&result.string[byteCount], other.string, sizeof(char) * other.getSizeInBytes());
    result.string[result.byteCount] = '\0'; //We could just presume other has this at its end, but its safer to force it here.

    return result;
}

String & String::operator+=(const String &other) {
    if (other.isEmpty()) return *this;

    char* temp = string;
    const ULong newSize = byteCount + other.getSizeInBytes();
    string = FF_Memory::ff_allocate_recursive<char>(STRING, newSize + 1);

    if (temp) {
        FF_Memory::ff_copy(string, temp, sizeof(char) * byteCount);
        FF_Memory::ff_clear(temp, sizeof(char) * (byteCount + 1));
        FF_Memory::ff_free_recursive<char>(temp, STRING, byteCount + 1);
    }

    FF_Memory::ff_copy(&string[byteCount], other.string, sizeof(char) * other.getSizeInBytes());

    byteCount = newSize;
    string[byteCount] = '\0';

    return *this;
}

bool String::operator==(const String &other) const {
    if (byteCount != other.byteCount) {
        return false;
    }

    for (ULong i = 0; i < byteCount; ++i) {
        if (string[i] != other.string[i]) {
            return false;
        }
    }

    return true;
}

char String::getAt(const ULong index) const {
    if (index >= byteCount) {
        Logger::logError("Cannot get character at " + toString(index) + "! String size: " + toString(byteCount));
        return 0;
    }

    return string[index];
}

void String::setAt(const ULong index, const char value) const {
    if (!string) {
        Logger::logError("Cannot set " + toString(value) + " at " + toString(index) + " because string is null!");
        return;
    }

    //byteCount is the index of \0. DO NOT SET THE BYTECOUNT INDEX.
    if (index >= byteCount) {
        Logger::logError("Cannot set " + toString(value) + " at " + toString(index) + " because index is greater than the size!");
        return;
    }

    string[index] = value;
}

void String::clear() const {
    FF_Memory::ff_clear(string, sizeof(char) * byteCount); //This will not clear the \0.
}

