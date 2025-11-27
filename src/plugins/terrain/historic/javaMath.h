#pragma once
#include <array>
#include <cmath>
#include <string>

// Library for emulating Java/Java Edition math functions

// Linear Interpolation
inline double lerp(double t, double a, double b) { return a + t * (b - a); }

inline double grad(int var0, double var1, double var3, double var5) {
	var0 &= 15;
	double var8 = var0 < 8 ? var1 : var3;
	double var10 = var0 < 4 ? var3 : (var0 != 12 && var0 != 14 ? var5 : var1);
	return ((var0 & 1) == 0 ? var8 : -var8) + ((var0 & 2) == 0 ? var10 : -var10);
}

// These may be very similar, dunno if this is some Java quirk
inline double altGrad(int var1, double var2, double var4) {
	int var6 = var1 & 15;
	double var7 = double(1 - ((var6 & 8) >> 3)) * var2;
	double var9 = var6 < 4 ? 0.0 : (var6 != 12 && var6 != 14 ? var4 : var2);
	return ((var6 & 1) == 0 ? var7 : -var7) + ((var6 & 2) == 0 ? var9 : -var9);
}

// Easing Function
inline double fade(double value) { return value * value * value * (value * (value * 6.0 - 15.0) + 10.0); }

inline int hashCode(std::string value) {
	int h = 0;
	if (h == 0 && value.size() > 0) {
		for (size_t i = 0; i < value.size(); i++) {
			h = 31 * h + value[i];
		}
	}
	return h;
}

struct MathHelper {
	static constexpr size_t TABLE_SIZE = 65536;
	static std::array<float, TABLE_SIZE> SIN_TABLE;

	static float sin(float x) { return SIN_TABLE[static_cast<int>(x * 10430.378f) & 0xFFFF]; }

	static float cos(float x) { return SIN_TABLE[(static_cast<int>(x * 10430.378f + 16384.0f)) & 0xFFFF]; }

	static float sqrt_float(float x) { return std::sqrt(x); }

	static float sqrt_double(double x) { return static_cast<float>(std::sqrt(x)); }

	static int floor_float(float x) {
		int i = static_cast<int>(x);
		return x < static_cast<float>(i) ? i - 1 : i;
	}

	static int floor_double(double x) {
		int i = static_cast<int>(x);
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
		table[i] = std::sinf(float(i) * float(M_PI) * 2.0f / MathHelper::TABLE_SIZE);
	return table;
}();