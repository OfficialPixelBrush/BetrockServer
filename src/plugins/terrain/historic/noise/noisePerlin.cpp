#include "noisePerlin.h"

NoisePerlin::NoisePerlin() : NoisePerlin(new JavaRandom()) {}

NoisePerlin::NoisePerlin(JavaRandom *rand) {
	this->xCoord = rand->nextDouble() * 256.0;
	this->yCoord = rand->nextDouble() * 256.0;
	this->zCoord = rand->nextDouble() * 256.0;

	for (int32_t i = 0; i < 256; ++i) {
		this->permutations[i] = i;
	}

	for (int32_t i = 0; i < 256; ++i) {
		int32_t j = rand->nextInt(256 - i) + i;
		std::swap(this->permutations[i], this->permutations[j]);
		this->permutations[i + 256] = this->permutations[i];
	}
}

// This is a rather standard implementation of "Improved Perlin Noise",
// as described by Ken Perlin in 2002
// This version is mainly used by the infdev generator but Beta still implements and uses it.
double NoisePerlin::GenerateNoiseBase(Vec3 pos) {
	pos.x += this->xCoord;
	pos.y += this->yCoord;
	pos.z += this->zCoord;
	// The farlands are caused by this getting cast to a 32-Bit Integer.
	// Change these int32_ts to int64_ts to fix the farlands,
	// however, this will change Beta tree generation slightly
	// due to rounding differences
	int32_t xInt = int32_t(pos.x);
	int32_t yInt = int32_t(pos.y);
	int32_t zInt = int32_t(pos.z);
	if (pos.x < double(xInt))
		--xInt;
	if (pos.y < double(yInt))
		--yInt;
	if (pos.z < double(zInt))
		--zInt;

	int32_t xIndex = xInt & 255;
	int32_t yIndex = yInt & 255;
	int32_t zIndex = zInt & 255;

	pos.x -= double(xInt);
	pos.y -= double(yInt);
	pos.z -= double(zInt);
	double w = fade(pos.x);
	double v = fade(pos.y);
	double u = fade(pos.z);
	int32_t permXY = this->permutations[xIndex] + yIndex;
	int32_t permXYZ = this->permutations[permXY] + zIndex;
	// Some of the following code is weird,
	// probably because it got optimized by Java to use
	// fewer variables or Notch did this to be efficient
	permXY = this->permutations[permXY + 1] + zIndex;
	xIndex = this->permutations[xIndex + 1] + yIndex;
	yIndex = this->permutations[xIndex] + zIndex;
	xIndex = this->permutations[xIndex + 1] + zIndex;
	return lerp(
		u,
		lerp(v, lerp(w, grad(this->permutations[permXYZ], pos.x, pos.y, pos.z), grad(this->permutations[yIndex], pos.x - 1.0, pos.y, pos.z)),
			 lerp(w, grad(this->permutations[permXY], pos.x, pos.y - 1.0, pos.z),
				  grad(this->permutations[xIndex], pos.x - 1.0, pos.y - 1.0, pos.z))),
		lerp(v,
			 lerp(w, grad(this->permutations[permXYZ + 1], pos.x, pos.y, pos.z - 1.0),
				  grad(this->permutations[yIndex + 1], pos.x - 1.0, pos.y, pos.z - 1.0)),
			 lerp(w, grad(this->permutations[permXY + 1], pos.x, pos.y - 1.0, pos.z - 1.0),
				  grad(this->permutations[xIndex + 1], pos.x - 1.0, pos.y - 1.0, pos.z - 1.0))));
}

double NoisePerlin::GenerateNoise(Vec2 coord) { return this->GenerateNoiseBase(Vec3{coord.x, coord.y, 0.0}); }

double NoisePerlin::GenerateNoise(Vec3 coord) { return this->GenerateNoiseBase(coord); }
void NoisePerlin::GenerateNoise(std::vector<double> &noiseField,
								Vec3 offset, Int3 size, Vec3 scale,
                                double amplitude) {
    if (size.y == 1) {
        int32_t index = 0;
        double invAmp = 1.0 / amplitude;

        for (int32_t x = 0; x < size.x; ++x) {
            double fx = (offset.x + x) * scale.x + this->xCoord;
            int32_t ix = int32_t(fx);
            if (fx < ix) --ix;
            int32_t px = ix & 255;
            fx -= ix;
            double u = fade(fx);

            for (int32_t z = 0; z < size.z; ++z) {
                double fz = (offset.z + z) * scale.z + this->zCoord;
                int32_t iz = int32_t(fz);
                if (fz < iz) --iz;
                int32_t pz = iz & 255;
                fz -= iz;
                double w = fade(fz);

                int32_t a = this->permutations[px] + 0;
                int32_t aa = this->permutations[a] + pz;
                int32_t b = this->permutations[px + 1] + 0;
                int32_t ba = this->permutations[b] + pz;

                double x1 = lerp(u,
                    altGrad(this->permutations[aa], fx, fz),
                    grad(this->permutations[ba], fx - 1.0, 0.0, fz));

                double x2 = lerp(u,
                    grad(this->permutations[aa + 1], fx, 0.0, fz - 1.0),
                    grad(this->permutations[ba + 1], fx - 1.0, 0.0, fz - 1.0));

                double result = lerp(w, x1, x2);
                noiseField[index++] += result * invAmp;
            }
        }
    } else {
        int32_t index = 0;
        double invAmp = 1.0 / amplitude;
        int32_t lastPermY = -1;

        double lerpAX = 0.0, lerpBX = 0.0;
        double lerpAY = 0.0, lerpBY = 0.0;

        for (int32_t x = 0; x < size.x; ++x) {
            double fx = (offset.x + x) * scale.x + this->xCoord;
            int32_t ix = int32_t(fx);
            if (fx < ix) --ix;
            int32_t px = ix & 255;
            fx -= ix;
            double u = fade(fx);

            for (int32_t z = 0; z < size.z; ++z) {
                double fz = (offset.z + z) * scale.z + this->zCoord;
                int32_t iz = int32_t(fz);
                if (fz < iz) --iz;
                int32_t pz = iz & 255;
                fz -= iz;
                double w = fade(fz);

                for (int32_t y = 0; y < size.y; ++y) {
                    double fy = (offset.y + y) * scale.y + this->yCoord;
                    int32_t iy = int32_t(fy);
                    if (fy < iy) --iy;
                    int32_t py = iy & 255;
                    fy -= iy;
                    double v = fade(fy);

                    if (y == 0 || py != lastPermY) {
                        lastPermY = py;

                        int32_t A = this->permutations[px] + py;
                        int32_t AA = this->permutations[A] + pz;
                        int32_t AB = this->permutations[A + 1] + pz;
                        int32_t B = this->permutations[px + 1] + py;
                        int32_t BA = this->permutations[B] + pz;
                        int32_t BB = this->permutations[B + 1] + pz;

                        lerpAX = lerp(u,
                            grad(this->permutations[AA], fx, fy, fz),
                            grad(this->permutations[BA], fx - 1.0, fy, fz));

                        lerpBX = lerp(u,
                            grad(this->permutations[AB], fx, fy - 1.0, fz),
                            grad(this->permutations[BB], fx - 1.0, fy - 1.0, fz));

                        lerpAY = lerp(u,
                            grad(this->permutations[AA + 1], fx, fy, fz - 1.0),
                            grad(this->permutations[BA + 1], fx - 1.0, fy, fz - 1.0));

                        lerpBY = lerp(u,
                            grad(this->permutations[AB + 1], fx, fy - 1.0, fz - 1.0),
                            grad(this->permutations[BB + 1], fx - 1.0, fy - 1.0, fz - 1.0));
                    }

                    double i1 = lerp(v, lerpAX, lerpBX);
                    double i2 = lerp(v, lerpAY, lerpBY);
                    double result = lerp(w, i1, i2);

                    noiseField[index++] += result * invAmp;
                }
            }
        }
    }
}