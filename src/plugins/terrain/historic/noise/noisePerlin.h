// A recreation of the the Infdev 20100227-1433 Perlin noise function
#pragma once
#include "noiseGenerator.h"

class NoisePerlin : public NoiseGenerator {
  private:
	int permutations[512];
	double xCoord;
	double yCoord;
	double zCoord;
	double GenerateNoiseBase(double x, double y, double z);

  public:
	NoisePerlin();
	NoisePerlin(JavaRandom *rand);
	double GenerateNoise(double x, double y);
	double GenerateNoise(double x, double y, double z);
	void GenerateNoise(std::vector<double> &noiseField, double xOffset, double yOffset, double zOffset, int xSize,
					   int ySize, int zSize, double xScale, double yScale, double zScale, double amplitude);
};