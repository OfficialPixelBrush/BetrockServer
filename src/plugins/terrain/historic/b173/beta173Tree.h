#pragma once

#include "javaRandom.h"
#include "world.h"
#include "blockHelper.h"

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
	int32_t totalHeight = 0;
	int32_t height;
	double heightFactor = 0.618;
	double field_753_h = 1.0;
	double field_752_i = 0.381;
	double branchLength = 1.0;
	double trunkShape = 1.0;
	int32_t branchDensity = 1;
	int32_t maximumTreeHeight = 12;
	int32_t trunkThickness = 4;
	std::vector<std::array<int32_t, 4>> branchStartEnd;

	bool ValidPlacement();
	void GenerateBranchPositions();
	void GenerateLeafClusters();
	void GenerateTrunk();
	void GenerateBranches();
	float func_431_a(int32_t var1);
	float GetTrunkLayerRadius(int32_t layerIndex);
	void func_423_a(int32_t var1, int32_t var2, int32_t var3);
	void DrawBlockLine(Int3 posA, Int3 posB, BlockType blockType);
	int32_t CheckIfPathClear(Int3 var1, Int3 var2);
	void PlaceCircularLayer(Int3 centerPos, float radius, int8_t axis, BlockType blockType);
	bool CanGenerateBranchAtHeight(int32_t var1);

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