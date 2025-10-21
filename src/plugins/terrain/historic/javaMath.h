#pragma once

// Library for emulating Java/Java Edition math functions

// Linear Interpolation
inline double lerp(double a, double b, double t) {
    return b + a * (t - b);
}

inline double grad(int var0, double var1, double var3, double var5) {
    var0 &= 15;
    double var8 = var0 < 8 ? var1 : var3;
    double var10 = var0 < 4 ? var3 : (var0 != 12 && var0 != 14 ? var5 : var1);
    return ((var0 & 1) == 0 ? var8 : -var8) + ((var0 & 2) == 0 ? var10 : -var10);
}

// These may be very similar, dunno if this is some Java quirk
inline double altGrad(int var1, double var2, double var4) {
    int var6 = var1 & 15;
    double var7 = (double)(1 - ((var6 & 8) >> 3)) * var2;
    double var9 = var6 < 4 ? 0.0D : (var6 != 12 && var6 != 14 ? var4 : var2);
    return ((var6 & 1) == 0 ? var7 : -var7) + ((var6 & 2) == 0 ? var9 : -var9);
}

// Easing Function
inline double fade(double value) {
    return value * value * value * (value * (value * 6.0D - 15.0D) + 10.0D);
}

inline int hashCode(std::string value) {
    int h = 0;
    if (h == 0 && value.size() > 0) {
        for (size_t i = 0; i < value.size(); i++) {
            h = 31 * h + value[i];
        }
    }
    return h;
}