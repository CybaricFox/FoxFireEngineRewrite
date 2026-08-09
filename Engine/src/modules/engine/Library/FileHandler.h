/**
*   @file FileHandler.h
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
#include "foxfire_export.h"

enum FileMode {
    READ = 0x1,
    WRITE = 0x2,
    BOTH = 0x3
};

/**
 * @brief Handles file system operations, one file at a time.
 */
class FOXFIRE_API FileHandler {
private:
    /** @brief pointer to a file */
    FILE* handle = nullptr;
    /** @brief Whether the file is valid */
    bool bIsValid = false;

public:
    ~FileHandler();

    /**
     * @brief Gets the size of a file
     * @param outSize OUT size of the file.
     * @return False if somethign went wrong
     */
    bool getFileSize(unsigned long& outSize) const;

    /**
     * @brief Checks if a file exists
     * @param name path of the file
     * @return Whether the file exists
     */
    bool exists(const String& name);

    /**
     * @brief Opens a file
     * @param path Path of the file
     * @param mode READ/WRITE/BOTH
     * @param isBinary Whether the file is binary
     * @return True on success, False on failure
     */
    bool openFile(const String& path, FileMode mode, bool isBinary);

    /**
     * @brief Closes the file
     */
    void closeFile();

    /**
     * @brief Reads until it reaches an end of line character
     * @param line OUT string
     * @param maxLength The maximum number of bytes that can be read
     * @param outLength The number of bytes read.
     * @return True on success, False on failure.
     */
    bool readLine(String &line, unsigned long maxLength, unsigned long &outLength) const;

    /**
     * @brief Writes a String line to the file
     * @param text String to write
     * @return True on success, False on failure
     */
    [[nodiscard]] bool writeLine(const String& text) const;

    /**
     * @brief Reads a given number of bytes from a file
     * @param size Number of bytes to read
     * @param outData OUT Data read
     * @param outBytesRead OUT bytes read
     * @return True on success, False on failure
     */
    bool read(unsigned long size, void* outData, unsigned long& outBytesRead) const;

    /**
     * @brief Reads the entire file (Applicable to binary files)
     * @param outBytes OUT bytes
     * @param outBytesRead OUT bytes read
     * @return True on Success, False on failure
     */
    bool readAll(unsigned char *&outBytes, unsigned long &outBytesRead) const;
    /**
     * @brief Reads the entire file (Applicable to Text files)
     * @param outText OUT Text
     * @param outBytesRead OUT bytes read
     * @return True on Success, False on failure
     */
    bool readAll(String& outText, unsigned long& outBytesRead) const;

    /**
     * @brief Writes bytes to a file
     * @param size Size of the data to be written
     * @param inData Data to be written
     * @param outBytesWritten OUT bytes written
     * @return Returns true on success, False on failure.
     */
    bool write(unsigned long size, const void* inData, unsigned long& outBytesWritten) const;
};