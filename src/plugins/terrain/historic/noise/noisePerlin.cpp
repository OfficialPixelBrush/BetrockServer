#include "noisePerlin.h"

NoisePerlin::NoisePerlin() : NoisePerlin(new JavaRandom()) {}

NoisePerlin::NoisePerlin(JavaRandom *rand) {
	this->xCoord = rand->nextDouble() * 256.0;
	this->yCoord = rand->nextDouble() * 256.0;
	this->zCoord = rand->nextDouble() * 256.0;

	for (int i = 0; i < 256; ++i) {
		this->permutations[i] = i;
	}

	for (int i = 0; i < 256; ++i) {
		int j = rand->nextInt(256 - i) + i;
		std::swap(this->permutations[i], this->permutations[j]);
		this->permutations[i + 256] = this->permutations[i];
	}
}

// This is a rather standard implementation of "Improved Perlin Noise",
// as described by Ken Perlin in 2002
double NoisePerlin::GenerateNoiseBase(double x, double y, double z) {
	x += this->xCoord;
	y += this->yCoord;
	z += this->zCoord;
	// The farlands are caused by this getting cast to a 32-Bit Integer.
	// Change these ints to longs to fix the farlands.
	// TODO: Apparently not? Fix this PLEASE
	int64_t xInt = (int64_t)x;
	int64_t yInt = (int64_t)y;
	int64_t zInt = (int64_t)z;
	if (x < (double)xInt)
		--xInt;
	if (y < (double)yInt)
		--yInt;
	if (z < (double)zInt)
		--zInt;

	int64_t xIndex = xInt & 255;
	int64_t yIndex = yInt & 255;
	int64_t zIndex = zInt & 255;

	x -= (double)xInt;
	y -= (double)yInt;
	z -= (double)zInt;
	double w = fade(x);
	double v = fade(y);
	double u = fade(z);
	int64_t permXY = this->permutations[xIndex] + yIndex;
	int64_t permXYZ = this->permutations[permXY] + zIndex;
	// Some of the following code is weird,
	// probably because it got optimized by Java to use
	// fewer variables or Notch did this to be efficient
	permXY = this->permutations[permXY + 1] + zIndex;
	xIndex = this->permutations[xIndex + 1] + yIndex;
	yIndex = this->permutations[xIndex] + zIndex;
	xIndex = this->permutations[xIndex + 1] + zIndex;
	return lerp(
		u,
		lerp(v, lerp(w, grad(this->permutations[permXYZ], x, y, z), grad(this->permutations[yIndex], x - 1.0, y, z)),
			 lerp(w, grad(this->permutations[permXY], x, y - 1.0, z),
				  grad(this->permutations[xIndex], x - 1.0, y - 1.0, z))),
		lerp(v,
			 lerp(w, grad(this->permutations[permXYZ + 1], x, y, z - 1.0),
				  grad(this->permutations[yIndex + 1], x - 1.0, y, z - 1.0)),
			 lerp(w, grad(this->permutations[permXY + 1], x, y - 1.0, z - 1.0),
				  grad(this->permutations[xIndex + 1], x - 1.0, y - 1.0, z - 1.0))));
}

double NoisePerlin::GenerateNoise(double x, double y) { return this->GenerateNoiseBase(x, y, 0.0); }

double NoisePerlin::GenerateNoise(double x, double y, double z) { return this->GenerateNoiseBase(x, y, z); }

void NoisePerlin::GenerateNoise(std::vector<double> &noiseField, double xOffset, double yOffset, double zOffset,
								int xSize, int ySize, int zSize, double xScale, double yScale, double zScale,
								double amplitude) {
	int zSize001;
	int noiseField9;
	int xOffset2;
	double var31;
	double var35;
	int var37;
	double var38;
	int yOffset0;
	int yOffset1;
	double yOffset2;
	int var75;
	if (ySize == 1) {
		/*
		bool zOffset4 = false;
		bool zOffset5 = false;
		bool xOffset1 = false;
		bool zOffset8 = false;
		*/
		double var70 = 0.0;
		double var73 = 0.0;
		var75 = 0;
		double var77 = 1.0 / amplitude;

		for (int var30 = 0; var30 < xSize; ++var30) {
			var31 = (xOffset + (double)var30) * xScale + this->xCoord;
			int var78 = (int)var31;
			if (var31 < (double)var78) {
				--var78;
			}

			int var34 = var78 & 255;
			var31 -= (double)var78;
			var35 = var31 * var31 * var31 * (var31 * (var31 * 6.0 - 15.0) + 10.0);

			for (var37 = 0; var37 < zSize; ++var37) {
				var38 = (zOffset + (double)var37) * zScale + this->zCoord;
				yOffset0 = (int)var38;
				if (var38 < (double)yOffset0) {
					--yOffset0;
				}

				yOffset1 = yOffset0 & 255;
				var38 -= (double)yOffset0;
				yOffset2 = var38 * var38 * var38 * (var38 * (var38 * 6.0 - 15.0) + 10.0);
				noiseField9 = this->permutations[var34] + 0;
				int zOffset6 = this->permutations[noiseField9] + yOffset1;
				int zOffset7 = this->permutations[var34 + 1] + 0;
				xOffset2 = this->permutations[zOffset7] + yOffset1;
				var70 = lerp(var35, altGrad(this->permutations[zOffset6], var31, var38),
							 grad(this->permutations[xOffset2], var31 - 1.0, 0.0, var38));
				var73 = lerp(var35, grad(this->permutations[zOffset6 + 1], var31, 0.0, var38 - 1.0),
							 grad(this->permutations[xOffset2 + 1], var31 - 1.0, 0.0, var38 - 1.0));
				double var79 = lerp(yOffset2, var70, var73);
				zSize001 = var75++;
				noiseField[zSize001] += var79 * var77;
			}
		}

	} else {
		noiseField9 = 0;
		double xOffset0 = 1.0 / amplitude;
		xOffset2 = -1;
		/*
		bool xOffset3 = false;
		bool xOffset4 = false;
		bool xOffset5 = false;
		bool xOffset6 = false;
		bool xOffset7 = false;
		bool xOffset8 = false;
		*/
		double xOffset9 = 0.0;
		var31 = 0.0;
		double var33 = 0.0;
		var35 = 0.0;

		for (var37 = 0; var37 < xSize; ++var37) {
			var38 = (xOffset + (double)var37) * xScale + this->xCoord;
			yOffset0 = (int)var38;
			if (var38 < (double)yOffset0) {
				--yOffset0;
			}

			yOffset1 = yOffset0 & 255;
			var38 -= (double)yOffset0;
			yOffset2 = var38 * var38 * var38 * (var38 * (var38 * 6.0 - 15.0) + 10.0);

			for (int yOffset4 = 0; yOffset4 < zSize; ++yOffset4) {
				double yOffset5 = (zOffset + (double)yOffset4) * zScale + this->zCoord;
				int yOffset7 = (int)yOffset5;
				if (yOffset5 < (double)yOffset7) {
					--yOffset7;
				}

				int yOffset8 = yOffset7 & 255;
				yOffset5 -= (double)yOffset7;
				double yOffset9 = yOffset5 * yOffset5 * yOffset5 * (yOffset5 * (yOffset5 * 6.0 - 15.0) + 10.0);

				for (int var51 = 0; var51 < ySize; ++var51) {
					double var52 = (yOffset + (double)var51) * yScale + this->yCoord;
					int var54 = (int)var52;
					if (var52 < (double)var54) {
						--var54;
					}

					int var55 = var54 & 255;
					var52 -= (double)var54;
					double var56 = var52 * var52 * var52 * (var52 * (var52 * 6.0 - 15.0) + 10.0);
					if (var51 == 0 || var55 != xOffset2) {
						xOffset2 = var55;
						int zOffset9 = this->permutations[yOffset1] + var55;
						int var71 = this->permutations[zOffset9] + yOffset8;
						int var72 = this->permutations[zOffset9 + 1] + yOffset8;
						int var74 = this->permutations[yOffset1 + 1] + var55;
						var75 = this->permutations[var74] + yOffset8;
						int var76 = this->permutations[var74 + 1] + yOffset8;
						xOffset9 = lerp(yOffset2, grad(this->permutations[var71], var38, var52, yOffset5),
										grad(this->permutations[var75], var38 - 1.0, var52, yOffset5));
						var31 = lerp(yOffset2, grad(this->permutations[var72], var38, var52 - 1.0, yOffset5),
									 grad(this->permutations[var76], var38 - 1.0, var52 - 1.0, yOffset5));
						var33 = lerp(yOffset2, grad(this->permutations[var71 + 1], var38, var52, yOffset5 - 1.0),
									 grad(this->permutations[var75 + 1], var38 - 1.0, var52, yOffset5 - 1.0));
						var35 = lerp(yOffset2, grad(this->permutations[var72 + 1], var38, var52 - 1.0, yOffset5 - 1.0),
									 grad(this->permutations[var76 + 1], var38 - 1.0, var52 - 1.0, yOffset5 - 1.0));
					}

					double var58 = lerp(var56, xOffset9, var31);
					double zOffset0 = lerp(var56, var33, var35);
					double zOffset2 = lerp(yOffset9, var58, zOffset0);
					zSize001 = noiseField9++;
					noiseField[zSize001] += zOffset2 * xOffset0;
				}
			}
		}
	}
}