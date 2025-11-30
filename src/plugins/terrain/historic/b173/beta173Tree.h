#pragma once

#include "javaRandom.h"
#include "world.h"

/**
 * @brief Used for generating Oak or Birch Trees
 * 
 */
class Beta173Tree {
  public:
	Beta173Tree() {};
	virtual ~Beta173Tree() = default;
	virtual bool Generate(World *world, JavaRandom *rand, Int3 pos, bool birch = false);
	virtual void Configure([[maybe_unused]] double treeHeight, [[maybe_unused]] double branchLength,
						   [[maybe_unused]] double trunkShape) {};
};

/**
 * @brief Used for generating Big Oak Trees
 * 
 */
class Beta173BigTree : public Beta173Tree {
  private:
	int8_t branchOrientation[6] = {2, 0, 0, 1, 2, 1};
	std::unique_ptr<JavaRandom> rand;
	World *world;
	Int3 basePos = Int3{0, 0, 0};
	int totalHeight = 0;
	int height;
	double heightFactor = 0.618;
	double field_753_h = 1.0;
	double field_752_i = 0.381;
	double branchLength = 1.0;
	double trunkShape = 1.0;
	int branchDensity = 1;
	int maximumTreeHeight = 12;
	int trunkThickness = 4;
	std::vector<std::array<int, 4>> branchStartEnd;

	bool ValidPlacement();
	void GenerateBranchPositions();
	void GenerateLeafClusters();
	void GenerateTrunk();
	void GenerateBranches();
	float func_431_a(int var1);
	float func_429_b(int var1);
	void func_423_a(int var1, int var2, int var3);
	void drawBlockLine(Int3 var1, Int3 var2, int var3);
	int checkIfPathClear(Int3 var1, Int3 var2);
	void func_426_a(int var1, int var2, int var3, float var4, int8_t var5, int var6);
	bool canGenerateBranchAtHeight(int var1);

  public:
	Beta173BigTree() { this->rand = std::make_unique<JavaRandom>(); };
	~Beta173BigTree() = default;
	bool Generate(World *world, JavaRandom *rand, Int3 pos, bool birch = false);
	void Configure(double treeHeight, double branchLength, double trunkShape);
};

/**
 * @brief Used for generating Taiga Trees
 * 
 */
class Beta173TaigaTree : public Beta173Tree {
  public:
	Beta173TaigaTree() {};
	~Beta173TaigaTree() = default;
	bool Generate(World *world, JavaRandom *rand, Int3 pos, bool birch = false);
};

/**
 * @brief Used for generating Alternative Taiga Trees
 * 
 */
class Beta173TaigaAltTree : public Beta173Tree {
  public:
	Beta173TaigaAltTree() {};
	~Beta173TaigaAltTree() = default;
	bool Generate(World *world, JavaRandom *rand, Int3 pos, bool birch = false);
};