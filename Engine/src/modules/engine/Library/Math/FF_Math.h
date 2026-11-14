/**
*   @file FF_Math.h
 *  @layer Engine
 *  @module Library
 *  @author CybaricFox
 *  @brief Collection of Mathematic structures and functions.
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "foxfire_export.h"
#include "Vectors.h"
#include "VectorUtils.h"
#include "../Logger.h"
#include "src/defines.h"
#include "src/modules/engine/Memory/FF_Memory.h"

/**
 * @brief A collection of math related functions and random numbers.
 */
class FOXFIRE_API FF_Math {
private:
    /**
     * @brief Whether the random numbers from FF_Math are using a seed.
     */
    static bool bIsSeeded;

    /**
     * @brief Sets the seed to a new seed.
     */
    static void setSeed();
public:
    /**
     * @brief Applies sin to the value
     * @param value
     * @return
     */
    static float sin(float value);
    /**
     * @brief Applies cos to the value
     * @param value
     * @return
     */
    static float cos(float value);
    /**
     * @brief Applies acos to the value
     * @param value
     * @return
     */
    static float acos(float value);
    /**
     * @brief Applies tan to the value
     * @param value
     * @return
     */
    static float tan(float value);
    /**
     * @brief Square roots the value.
     * @param value
     * @return
     */
    static float sqrt(float value);
    /**
     * @brief Returns the absolute value of the value.
     * @param value
     * @return
     */
    static float abs(float value);

    /**
     * @brief Returns a random integer.
     * @return
     */
    static int randomInt();

    /**
     * @brief Returns a random float.
     * @return
     */
    static float randomFloat();

    /**
     * @brief Returns a random integer from a range.
     * @param min Minimum value.
     * @param max Maximum value.
     * @return
     */
    static int randomRange(int min, int max);
    /**
     * @brief Returns a random float from a range.
     * @param min Minimum value.
     * @param max Maximum value.
     * @return
     */
    static float randomRange(float min, float max);

    /**
        * @brief Determines if a number is a power of 2.
        * @param value The number to check.
        * @return True if the number is a power of 2.
    */
    static bool isPowerOfTwo(const unsigned long value) {return value != 0 && ((value & (value - 1)) == 0);}
    /**
 * @brief Converts degrees to radians.
 * @param degrees
 * @return
 */
    static float degreesToRadians(const float degrees) {
        return degrees * FF_DEGREE_TO_RADIAN_MULTIPLIER;
    }

    /**
     * @brief Converts radians to degrees.
     * @param radians
     * @return
     */
    static float radiansToDegrees(const float radians) {
        return radians * FF_RADIAN_TO_DEGREE_MULTIPLIER;
    }

    static float convertFloatRange(const float value, const float oldMin, const float oldMax, const float newMin, const float newMax) {
        return (((value - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin;
    }

    static unsigned int rgbToUInt(const unsigned int r, const unsigned int g, const unsigned int b) {
        return (((r & 0x0FF) << 16) | ((g & 0x0FF) << 8) | (b & 0x0FF));
    }

    static void uIntToRGB(const unsigned int rgb, unsigned int& outR, unsigned int& outG, unsigned int& outB) {
        outR = (rgb >> 16) & 0x0FF;
        outG = (rgb >> 8) & 0x0FF;
        outB = rgb & 0x0FF;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    static T min(T x, T y) {return x < y ? x : y;}

    template <typename T>
    requires std::is_arithmetic_v<T>
    static T max(T x, T y) {return x > y ? x : y;}
};

struct Vertex {};

/**
 * @brief Contains data for a 3d vertex
 */
struct Vertex3d : Vertex{
    /**
     * @brief Position of the vertex.
     */
    Vector3f position;
    /**
     * @brief The direction the vertex is facing (for lighting).
     */
    Vector3f normal;
    /**
     * @brief Coordinate in the texture to take color from.
     */
    Vector2f textureCoordinate;
    /**
     * @brief The color of this vertex.
     */
    Vector4f color;
    /**
     * @brief
     */
    Vector3f tangent;
};
inline bool operator==(const Vertex3d& left, const Vertex3d& right) {
    return VectorUtils::compareVectors(left.position, right.position, FF_EPSILON) &&
        VectorUtils::compareVectors(left.normal, right.normal, FF_EPSILON) &&
            VectorUtils::compareVectors(left.textureCoordinate, right.textureCoordinate, FF_EPSILON) &&
                VectorUtils::compareVectors(left.color, right.color, FF_EPSILON) &&
                    VectorUtils::compareVectors(left.tangent, right.tangent, FF_EPSILON);
}

/**
 * @brief Data for a 2d vertex
 */
struct Vertex2d : Vertex{
    /**
     * @brief Position of the vertex.
     */
    Vector2f position;
    /**
     * @brief Coordinate in the texture to take color from.
     */
    Vector2f textureCoordinate;
};

/**
 * @brief How far the object's vertices go from the center.
 */
struct Extent2D {
    Vector2f min{};
    Vector2f max{};
};

/**
 * @brief How far the object's vertices go from the center.
 */
struct Extent3D {
    Vector3f min{};
    Vector3f max{};
};

/**
 * @brief Converts a string to a vector4.
 * @param string
 * @param out
 * @return False if the string cannot be converted.
 */
inline bool stringToVector4f(const String &string, Vector4f& out) {
    FF_Memory::ff_clear(&out, sizeof(Vector4f));
    const int result = sscanf(string.getAsCharString(), "%f %f %f %f", &out.x, &out.y, &out.z, &out.w);
    return result != -1;
}
/**
 * @brief Converts a string to a vector3.
 * @param string
 * @param out
 * @return False if the string cannot be converted.
 */
inline bool stringToVector3f(const String &string, Vector3f& out) {
    FF_Memory::ff_clear(&out, sizeof(Vector3f));
    const int result = sscanf(string.getAsCharString(), "%f %f %f", &out.x, &out.y, &out.z);
    return result != -1;
}
/**
 * @brief Converts a string to a vector2.
 * @param string
 * @param out
 * @return False if the string cannot be converted.
 */
inline bool stringToVector2f(const String &string, Vector2f& out) {
    FF_Memory::ff_clear(&out, sizeof(Vector2f));
    const int result = sscanf(string.getAsCharString(), "%f %f", &out.x, &out.y);
    return result != -1;
}

/**
 * @brief Converts a vector4 to a string.
 * @param outString
 * @param vector
 */
inline void vector4fToString(String& outString, const Vector4f vector) {
    outString.destroy();
    outString += toString(vector.x);
    outString += " ";
    outString += toString(vector.y);
    outString+=(" ");
    outString +=toString(vector.z);
    outString+=(" ");
    outString += toString(vector.w);
}

/**
 * @brief Converts a vector4 to a string.
 * @param vector
 * @param decimal
 */
inline String vector3fToString(const Vector3f vector, const unsigned char decimal) {
    String result{};
    result = toString(vector.x, decimal);
    result += " ";
    result += toString(vector.y, decimal);
    result+=(" ");
    result += toString(vector.z, decimal);

    return result;
}

inline Vector3f rgbToVector3(const unsigned int r, const unsigned int g, const unsigned int b) {
    return {static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f};
}

inline void vector3ToRGB(const Vector3f vector, unsigned int& outR, unsigned int& outG, unsigned int& outB) {
    outR = static_cast<unsigned int>(vector.r * 255);
    outG = static_cast<unsigned int>(vector.g * 255);
    outB = static_cast<unsigned int>(vector.b * 255);
}