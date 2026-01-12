#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

// Library for emulating Java/Java Edition math functions

/**
 * @brief Linear interpolation function
 * 
 * @param t Interpolation factor
 * @param a Start value (t = 0.0)
 * @param b End value (t = 1.0)
 * @return Interpolated value between a and b
 */
inline double lerp(double t, double a, double b) { return a + t * (b - a); }

inline double grad(int32_t var0, double var1, double var3, double var5) {
	var0 &= 15;
	double var8 = var0 < 8 ? var1 : var3;
	double var10 = var0 < 4 ? var3 : (var0 != 12 && var0 != 14 ? var5 : var1);
	return ((var0 & 1) == 0 ? var8 : -var8) + ((var0 & 2) == 0 ? var10 : -var10);
}

// These may be very similar, dunno if this is some Java quirk
inline double altGrad(int32_t var1, double var2, double var4) {
	int32_t var6 = var1 & 15;
	double var7 = double(1 - ((var6 & 8) >> 3)) * var2;
	double var9 = var6 < 4 ? 0.0 : (var6 != 12 && var6 != 14 ? var4 : var2);
	return ((var6 & 1) == 0 ? var7 : -var7) + ((var6 & 2) == 0 ? var9 : -var9);
}

/**
 * @brief Perlin-noise easing function
 * 
 * @param value Input value
 * @return Eased output value 
 */
inline double fade(double value) { return value * value * value * (value * (value * 6.0 - 15.0) + 10.0); }

/**
 * @brief Java-equivalent functions
 * 
 */
struct Java {
	// The following should be somewhat faithful implementation of
	// Java's casting functions, as defined in
	// "Chapter 5. Conversions and Contexts"
	/**
	 * @brief Casts a double to a 64-bit integer
	 */
	static int64_t DoubleToInt64(double value) {
		if (std::isnan(value))
			return 0;
		if (value > double(INT64_MAX))
			return INT64_MAX;
		if (value < double(INT64_MIN))
			return INT64_MIN;
		if (value > 0)
			return int64_t(std::floor(value));
		if (value < 0)
			return int64_t(std::ceil(value));
		return 0;
	}
	/**
	 * @brief Casts a double to a 32-bit integer
	 */
	static int32_t DoubleToInt32(double value) {
		if (std::isnan(value))
			return 0;
		if (value > double(INT32_MAX))
			return INT32_MAX;
		if (value < double(INT32_MIN))
			return INT32_MIN;
		if (value > 0)
			return int32_t(std::floor(value));
		if (value < 0)
			return int32_t(std::ceil(value));
		return 0;
	}
	/**
	 * @brief Casts a float to a 64-bit integer
	 */
	static int64_t FloatToInt64(float value) {
		if (std::isnan(value))
			return 0;
		if (value > float(INT64_MAX))
			return INT64_MAX;
		if (value < float(INT64_MIN))
			return INT64_MIN;
		if (value > 0)
			return int64_t(std::floor(value));
		if (value < 0)
			return int64_t(std::ceil(value));
		return 0;
	}
	/**
	 * @brief Casts a float to a 32-bit integer
	 */
	static int32_t FloatToInt32(float value) {
		if (std::isnan(value))
			return 0;
		if (value > float(INT32_MAX))
			return INT32_MAX;
		if (value < float(INT32_MIN))
			return INT32_MIN;
		if (value > 0)
			return int32_t(std::floor(value));
		if (value < 0)
			return int32_t(std::ceil(value));
		return 0;
	}
};

/**
* @brief Java-equivalent hashing function
* 
* @param value The input string
* @return Hashed string expressed as an integer
*/
inline int32_t hashCode(std::string value) {
	int32_t h = 0;
	if (h == 0 && value.size() > 0) {
		for (size_t i = 0; i < value.size(); i++) {
			h = 31 * h + value[i];
		}
	}
	return h;
}

/**
 * @brief A struct that's used like Javas Math.java library
 * 
 */
struct JavaMath {
	static constexpr double PI = 3.141592653589793;
	static int32_t abs(int32_t a) { return (a < 0) ? -a : a; }
};

/**
 * @brief A small helper that's used to simplify or speed up some code
 * 
 */
struct MathHelper {
	static constexpr size_t TABLE_SIZE = 65536;
	static std::array<float, TABLE_SIZE> SIN_TABLE;

	static float sin(float x) { return SIN_TABLE[Java::FloatToInt32(x * 10430.378f) & 0xFFFF]; }

	static float cos(float x) { return SIN_TABLE[(Java::FloatToInt32(x * 10430.378f + 16384.0f)) & 0xFFFF]; }

	static float sqrt_float(float x) { return std::sqrt(x); }

	static float sqrt_double(double x) { return static_cast<float>(std::sqrt(x)); }

	static int32_t floor_float(float x) {
		int32_t i = Java::FloatToInt32(x);
		return x < static_cast<float>(i) ? i - 1 : i;
	}

	static int32_t floor_double(double x) {
		int32_t i = Java::DoubleToInt32(x);
		return x < static_cast<double>(i) ? i - 1 : i;
	}
	
	static float abs(float x) { return x >= 0.0f ? x : -x; }

	static double abs_max(double a, double b) {
		if (a < 0.0)
			a = -a;
		if (b < 0.0)
			b = -b;
		return a > b ? a : b;
	}
};

// initialize lookup table
inline std::array<float, MathHelper::TABLE_SIZE> MathHelper::SIN_TABLE = [] {
	std::array<float, MathHelper::TABLE_SIZE> table{};
	for (size_t i = 0; i < MathHelper::TABLE_SIZE; ++i)
		table[i] = std::sin(float(i) * float(JavaMath::PI) * 2.0f / MathHelper::TABLE_SIZE);
	return table;
}();