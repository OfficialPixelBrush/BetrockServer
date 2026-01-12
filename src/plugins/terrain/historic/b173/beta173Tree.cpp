#include "beta173Tree.h"
#include <cstdint>

/**
 * @brief Attempts to generate an oak or birch tree.
 * 
 * @param world Pointer to the world where it'll generate
 * @param rand Pointer to the JavaRandom that'll be utilized
 * @param pos Position of the lowest trunk-block
 * @param birch If the tree should be birch or oak
 * @return If tree successfully generated
 */
bool Beta173Tree::Generate(World *world, JavaRandom *rand, Int3 pos, bool birch) {
	// Decide on the tree height (birches are one block taller)
	int32_t treeHeight = rand->nextInt(3) + 4;
	if (birch)
		treeHeight++;
	
	// Check if there's space to generate the tree
	// If any block in the desired area is neither air or
	// leaves, we fail the placement check
	if (pos.y >= 1 && pos.y + treeHeight + 1 <= CHUNK_HEIGHT) {
		Int3 check;
		for (check.y = pos.y; check.y <= pos.y + 1 + treeHeight; ++check.y) {
			int8_t width = 1;
			if (check.y == pos.y)
				width = 0;

			if (check.y >= pos.y + 1 + treeHeight - 2)
				width = 2;

			for (check.x = pos.x - width; check.x <= pos.x + width; ++check.x) {
				for (check.z = pos.z - width; check.z <= pos.z + width; ++check.z) {
					// Only test blocks that're within chunk boundaries
					if (check.y >= 0 && check.y < CHUNK_HEIGHT) {
						BlockType blockTest = world->GetBlockType(check);
						if (blockTest != BLOCK_AIR && blockTest != BLOCK_LEAVES) {
							return false;
						}
					} else {
						return false;
					}
				}
			}
		}

		// Check if the bock under the source block is grass or dirt
		BlockType rootBlock = world->GetBlockType(Int3{pos.x, pos.y - 1, pos.z});
		if ((rootBlock == BLOCK_GRASS || rootBlock == BLOCK_DIRT) && pos.y < CHUNK_HEIGHT - treeHeight - 1) {
			// Replace the underlying block with dir
			world->SetBlockType(BLOCK_DIRT, Int3{pos.x, pos.y - 1, pos.z});

			// Place leaves
			Int3 offset;
			for (offset.y = pos.y - 3 + treeHeight; offset.y <= pos.y + treeHeight; ++offset.y) {
				int32_t widthBase = offset.y - (pos.y + treeHeight);
				int32_t treeWidth = 1 - widthBase / 2;

				for (offset.x = pos.x - treeWidth; offset.x <= pos.x + treeWidth; ++offset.x) {
					int32_t xLeaf = offset.x - pos.x;

					for (offset.z = pos.z - treeWidth; offset.z <= pos.z + treeWidth; ++offset.z) {
						int32_t zLeaf = offset.z - pos.z;
						// Leaves are placed within the tree width
						// and replace any non-opaque block
						if ((
								(
								JavaMath::abs(xLeaf) != treeWidth ||
								JavaMath::abs(zLeaf) != treeWidth ||
								(rand->nextInt(2) != 0 && widthBase != 0)
							)) && !IsOpaque(
								world->GetBlockType(offset))) {
							world->PlaceBlock(offset, BLOCK_LEAVES, birch ? 2 : 0);
						}
					}
				}
			}

			// Replace air and leaves with trunk
			for (int32_t h = 0; h < treeHeight; ++h) {
				BlockType futureLog = world->GetBlockType(Int3{pos.x, pos.y + h, pos.z});
				if (futureLog == BLOCK_AIR || futureLog == BLOCK_LEAVES) {
					world->PlaceBlock(Int3{pos.x, pos.y + h, pos.z}, BLOCK_LOG, birch ? 2 : 0);
				}
			}

			return true;
		}
		return false;
	}
	return false;
}

/**
 * @brief Configures the settings of a BigTree
 * 
 * @param pTreeHeight Sets the maximum tree height (this is internally multiplied by 12)
 * @param pBranchLength Sets the maximum branch length
 * @param pTrunkShape Determines the trunk shape
 */
void Beta173BigTree::Configure(double pTreeHeight, double pBranchLength, double pTrunkShape) {
	this->maximumTreeHeight = int32_t(pTreeHeight * 12.0);
	if (pTreeHeight > 0.5) {
		this->trunkThickness = 5;
	}

	this->branchLength = pBranchLength;
	this->trunkShape = pTrunkShape;
}


/**
 * @brief Attempts to generate a big oak tree
 * 
 * @param pWorld Pointer to the world where it'll generate
 * @param pRand Pointer to the JavaRandom that'll be utilized
 * @param pPos Position of the lowest trunk-block
 * @param pBirch If the tree should be birch or oak (not used for big trees)
 * @return If tree successfully generated
 */
bool Beta173BigTree::Generate([[maybe_unused]] World *pWorld, JavaRandom *pRand, [[maybe_unused]] Int3 pPos,
							  [[maybe_unused]] bool pBirch) {
	this->world = pWorld;
	int64_t seed = pRand->nextLong();
	this->rand->setSeed(seed);
	this->basePos = pPos;
	// If the height wasn't set, generate a new random height
	if (this->totalHeight == 0) {
		this->totalHeight = 5 + this->rand->nextInt(this->maximumTreeHeight);
	}

	if (!this->ValidPlacement()) {
		return false;
	} else {
		this->GenerateBranchPositions();
		this->GenerateLeafClusters();
		this->GenerateTrunk();
		this->GenerateBranches();
		return true;
	}
}

void Beta173BigTree::GenerateBranchPositions() {
	this->height = int32_t((double)this->totalHeight * this->heightFactor);
	if (this->height >= this->totalHeight) {
		this->height = this->totalHeight - 1;
	}

	int32_t var1 = int32_t(1.382 + std::pow(this->trunkShape * (double)this->totalHeight / 13.0, 2.0));
	if (var1 < 1) {
		var1 = 1;
	}

	std::vector<std::array<int32_t, 4>> var2(var1 * this->totalHeight);
	int32_t var3 = this->basePos.y + this->totalHeight - this->trunkThickness;
	int32_t var4 = 1;
	int32_t var5 = this->basePos.y + this->height;
	int32_t var6 = var3 - this->basePos.y;
	var2[0][0] = this->basePos.x;
	var2[0][1] = var3;
	var2[0][2] = this->basePos.z;
	var2[0][3] = var5;
	--var3;

	while (true) {
		while (var6 >= 0) {
			int32_t var7 = 0;
			float var8 = this->GetCanopyRadius(var6);
			if (var8 < 0.0F) {
				--var3;
				--var6;
			} else {
				for (double var9 = 0.5; var7 < var1; ++var7) {
					double var11 = this->branchLength * (double)var8 * ((double)this->rand->nextFloat() + 0.328);
					// Oh hey, look! An approximation of pi!
					double var13 = (double)this->rand->nextFloat() * 2.0 * 3.14159;
					int32_t var15 = MathHelper::floor_double(var11 * double(std::sin(var13)) +
														 (double)this->basePos.x + var9);
					int32_t var16 = MathHelper::floor_double(var11 * double(std::cos(var13)) +
														 (double)this->basePos.z + var9);
					Int3 var17 = Int3{var15, var3, var16};
					Int3 var18 = Int3{var15, var3 + this->trunkThickness, var16};
					if (this->CheckIfPathClear(var17, var18) == -1) {
						Int3 var19 = Int3{this->basePos.x, this->basePos.y, this->basePos.z};
						double var20 = std::sqrt(std::pow((double)JavaMath::abs(this->basePos.x - var17.x), 2.0) +
												 std::pow((double)JavaMath::abs(this->basePos.z - var17.z), 2.0));
						double var22 = var20 * this->field_752_i;
						if ((double)var17.y - var22 > (double)var5) {
							var19.y = var5;
						} else {
							var19.y = int32_t((double)var17.y - var22);
						}

						if (this->CheckIfPathClear(var19, var17) == -1) {
							var2[var4][0] = var15;
							var2[var4][1] = var3;
							var2[var4][2] = var16;
							var2[var4][3] = var19.y;
							++var4;
						}
					}
				}

				--var3;
				--var6;
			}
		}

		this->branchStartEnd = std::vector<std::array<int32_t, 4>>(var4);
		std::copy(var2.begin(), var2.begin() + var4, this->branchStartEnd.begin());
		return;
	}
}

/**
 * @brief Place a circle of blocks along the specified plane
 * 
 * @param centerPos Center position of the circle
 * @param radius Radius of the circle
 * @param axis Axis along which the circle will grow
 * @param blockType Blocktype of the circle
 */
void Beta173BigTree::PlaceCircularLayer(Int3 centerPos, float radius, int8_t axis, BlockType blockType) {
	int32_t intRadius = int32_t(double(radius) + 0.618);
	int8_t axisU = branchOrientation[axis];
	int8_t axisV = branchOrientation[axis + AXIS_OFFSET];
	Int3 currentPos{0, 0, 0};

	for (int32_t du = -intRadius; du <= intRadius; ++du) {
		currentPos[axis] = centerPos[axis];
		currentPos[axisU] = centerPos[axisU] + du;

		for (int32_t dv = -intRadius; dv <= intRadius; ++dv) {
			// Pythagorean Theorem
			double distance = std::sqrt(
				std::pow(JavaMath::abs(du) + 0.5, 2.0) +
				std::pow(JavaMath::abs(dv) + 0.5, 2.0)
			);

			if (distance > (double)(radius))
				continue;

			currentPos[axisV] = centerPos[axisV] + dv;
			BlockType otherType = this->world->GetBlockType(currentPos);

			// Only place if block can be overwritten
			if (otherType == BLOCK_AIR || otherType == BLOCK_LEAVES) {
				this->world->PlaceBlock(currentPos, blockType);
			}
		}
	}
}

/**
 * @brief Get the radius of the leaf canopy at the desired height
 * 
 * @param y Height to check
 * @return Radius in blocks
 */
float Beta173BigTree::GetCanopyRadius(int32_t y) {
	if ((double)y < (double)((float)this->totalHeight) * 0.3) {
		return -1.618F;
	} else {
		float halfHeight = (float)this->totalHeight / 2.0F;
		float distanceFromCenter = (float)this->totalHeight / 2.0F - (float)y;
		float radius;
		if (distanceFromCenter == 0.0F) {
			radius = halfHeight;
		} else if (JavaMath::abs(distanceFromCenter) >= halfHeight) {
			radius = 0.0F;
		} else {
			radius = (float)std::sqrt(
				std::pow((double)JavaMath::abs(halfHeight), 2.0) -
				std::pow((double)JavaMath::abs(distanceFromCenter), 2.0));
		}

		radius *= 0.5F;
		return radius;
	}
}

/**
 * @brief Get the radius of the current trunk layer
 * 
 * @param layerIndex The index of the layer
 * @return float Trunk layer radius
 */
float Beta173BigTree::GetTrunkLayerRadius(int32_t layerIndex) {
    if (layerIndex < 0 || layerIndex >= this->trunkThickness)
        return -1.0f;
    // Top and bottom of trunk are thinner
    if (layerIndex == 0 || layerIndex == this->trunkThickness - 1)
        return 2.0f;
    // Middle of trunk is thicker
    return 3.0f;
}

/**
 * @brief Iterate over the height 
 * 
 * @param base 
 */
void Beta173BigTree::PlaceLeavesAroundPoint(Int3 base) {
	int32_t baseHeight = base.y + this->trunkThickness;
	
	for (int32_t y = base.y; y < baseHeight; ++y) {
		float trunkRadius = this->GetTrunkLayerRadius(y - base.y);
		this->PlaceCircularLayer(
			Int3{base.x, y, base.z},
			trunkRadius,
			AXIS_Y, 
			BLOCK_LEAVES
		);
	}
}

/**
 * @brief Draws a line of blockType between two coordinates
 * 
 * @param posA The start position
 * @param posB The end position
 * @param blockType The block that should be drawn along this line
 */
void Beta173BigTree::DrawBlockLine(Int3 posA, Int3 posB, BlockType blockType) {
	Int3 diff = Int3{0, 0, 0};
	int8_t dominantAxis = 0;
	diff = posB - posA;
	// Determine which axis was the largest magnitude
	for (int axis = 0; axis < 3; axis++) {
		if (JavaMath::abs(diff[axis]) > JavaMath::abs(diff[dominantAxis])) {
			dominantAxis = axis;
		}
	}
	// If an axis was chosen, we can continue
	if (diff[dominantAxis] == 0) {
		return;
	}
	// Determine secondary axes
	// X -> Y/Z
	// Y -> X/Z
	// Z -> X/Y
	BranchAxis secondaryA = branchOrientation[dominantAxis];
	BranchAxis secondaryB = branchOrientation[dominantAxis + AXIS_OFFSET];
	int8_t var9;
	if (diff[dominantAxis] > 0) {
		var9 = 1;
	} else {
		var9 = -1;
	}

	double var10 = (double)diff[secondaryA] / (double)diff[dominantAxis];
	double var12 = (double)diff[secondaryB] / (double)diff[dominantAxis];
	Int3 blockPos = Int3{0, 0, 0};
	int32_t var15 = 0;

	for (int32_t var16 = diff[dominantAxis] + var9; var15 != var16; var15 += var9) {
		blockPos[dominantAxis] = MathHelper::floor_double((double)(posA[dominantAxis] + var15) + 0.5);
		blockPos[secondaryA] = MathHelper::floor_double((double)posA[secondaryA] + (double)var15 * var10 + 0.5);
		blockPos[secondaryB] = MathHelper::floor_double((double)posA[secondaryB] + (double)var15 * var12 + 0.5);
		this->world->PlaceBlock(blockPos, blockType);
	}
}

/**
 * @brief Iterate through the branches and generate the leaves
 * 
 */
void Beta173BigTree::GenerateLeafClusters() {
	size_t maxBranchNodes = this->branchStartEnd.size();
	for (size_t i = 0; i < maxBranchNodes; ++i) {
		Int3 pos = Int3{
			this->branchStartEnd[i][AXIS_X],
			this->branchStartEnd[i][AXIS_Y],
			this->branchStartEnd[i][AXIS_Z]
		};
		this->PlaceLeavesAroundPoint(pos);
	}
}

bool Beta173BigTree::CanGenerateBranchAtHeight(int32_t y) {
	return double(y) >= (double(this->totalHeight) * 0.2);
}

void Beta173BigTree::GenerateTrunk() {
	int32_t var1 = this->basePos.x;
	int32_t var2 = this->basePos.y;
	int32_t var3 = this->basePos.y + this->height;
	int32_t var4 = this->basePos.z;
	Int3 var5 = Int3{var1, var2, var4};
	Int3 var6 = Int3{var1, var3, var4};
	this->DrawBlockLine(var5, var6, BLOCK_LOG);
	if (this->branchDensity == 2) {
		++var5[AXIS_X];
		++var6[AXIS_X];
		this->DrawBlockLine(var5, var6, BLOCK_LOG);
		++var5[AXIS_Z];
		++var6[AXIS_Z];
		this->DrawBlockLine(var5, var6, BLOCK_LOG);
		var5[AXIS_X] += -1;
		var6[AXIS_X] += -1;
		this->DrawBlockLine(var5, var6, BLOCK_LOG);
	}
}

void Beta173BigTree::GenerateBranches() {
	Int3 var3 = this->basePos;
	size_t var2 = this->branchStartEnd.size();
	for (size_t var1 = 0; var1 < var2; ++var1) {
		Int3 var4 = Int3{
			this->branchStartEnd[var1][AXIS_X], 
			this->branchStartEnd[var1][AXIS_Y], 
			this->branchStartEnd[var1][AXIS_Z]
		};
		int32_t varBaseY = this->branchStartEnd[var1][3];
		Int3 var5 = var4;
		var3[AXIS_Y] = varBaseY;
		int32_t var6 = var3[AXIS_Y] - this->basePos.y;
		if (this->CanGenerateBranchAtHeight(var6)) {
			this->DrawBlockLine(var3, var5, BLOCK_LOG);
		}
	}
}

/**
 * @brief Check if the path is unobstructed between start and end in a straight line
 * 
 * @param posA The start position
 * @param posB The end position
 * @return int32_t 
 */
int32_t Beta173BigTree::CheckIfPathClear(Int3 posA, Int3 posB) {
	Int3 diff = Int3{0, 0, 0};

	int8_t dominantAxis = 0;
	diff = posB - posA;
	for (int8_t axis = 0; axis < 3; ++axis) {
		if (JavaMath::abs(diff[axis]) > JavaMath::abs(diff[dominantAxis])) {
			dominantAxis = axis;
		}
	}

	if (diff[dominantAxis] == 0) {
		return -1;
	}
	// Determine secondary axes
	int8_t secondaryA = branchOrientation[dominantAxis];
	int8_t secondaryB = branchOrientation[dominantAxis + AXIS_OFFSET];
	int8_t var8;
	if (diff[dominantAxis] > 0) {
		var8 = 1;
	} else {
		var8 = -1;
	}

	double var9 = (double)diff[secondaryA] / (double)diff[dominantAxis];
	double var11 = (double)diff[secondaryB] / (double)diff[dominantAxis];
	Int3 var13 = Int3{0, 0, 0};
	int32_t var14 = 0;

	int32_t var15;
	for (var15 = diff[dominantAxis] + var8; var14 != var15; var14 += var8) {
		var13[dominantAxis] = posA[dominantAxis] + var14;
		var13[secondaryA] = MathHelper::floor_double((double)posA[secondaryA] + (double)var14 * var9);
		var13[secondaryB] = MathHelper::floor_double((double)posA[secondaryB] + (double)var14 * var11);
		BlockType blockType = this->world->GetBlockType(var13);
		if (blockType != BLOCK_AIR && blockType != BLOCK_LEAVES) {
			break;
		}
	}

	if (var14 == var15) {
		return -1;
	}
	return JavaMath::abs(var14);
}

bool Beta173BigTree::ValidPlacement() {
	Int3 var1 = basePos;
	Int3 var2 = Int3{this->basePos.x, this->basePos.y + this->totalHeight - 1, this->basePos.z};
	BlockType blockType = this->world->GetBlockType(Int3{this->basePos.x, this->basePos.y - 1, this->basePos.z});
	if (blockType != BLOCK_GRASS && blockType != BLOCK_DIRT) {
		return false;
	}
	int32_t var4 = this->CheckIfPathClear(var1, var2);
	if (var4 == -1) {
		return true;
	} else if (var4 < 6) {
		return false;
	} else {
		this->totalHeight = var4;
		return true;
	}
}

bool Beta173TaigaTree::Generate(World *world, JavaRandom *rand, Int3 pos, [[maybe_unused]] bool birch) {
	int32_t var6 = rand->nextInt(5) + 7;
	int32_t var7 = var6 - rand->nextInt(2) - 3;
	int32_t var8 = var6 - var7;
	int32_t var9 = 1 + rand->nextInt(var8 + 1);
	bool canGenerate = true;
	if (pos.y >= 1 && pos.y + var6 + 1 <= CHUNK_HEIGHT) {
		int32_t var18;
		for (int32_t var11 = pos.y; var11 <= pos.y + 1 + var6 && canGenerate; ++var11) {
			// bool var12 = true;
			if (var11 - pos.y < var7) {
				var18 = 0;
			} else {
				var18 = var9;
			}

			for (int32_t var13 = pos.x - var18; var13 <= pos.x + var18 && canGenerate; ++var13) {
				for (int32_t var14 = pos.z - var18; var14 <= pos.z + var18 && canGenerate; ++var14) {
					if (var11 >= 0 && var11 < CHUNK_HEIGHT) {
						BlockType blockType = world->GetBlockType(Int3{var13, var11, var14});
						if (blockType != BLOCK_AIR && blockType != BLOCK_LEAVES) {
							canGenerate = false;
						}
					} else {
						canGenerate = false;
					}
				}
			}
		}

		if (!canGenerate)
			return false;

		BlockType blockType = world->GetBlockType(Int3{pos.x, pos.y - 1, pos.z});
		if ((blockType == BLOCK_GRASS || blockType == BLOCK_DIRT) && pos.y < CHUNK_HEIGHT - var6 - 1) {
			world->SetBlockType(BLOCK_DIRT, Int3{pos.x, pos.y - 1, pos.z});
			var18 = 0;

			for (int32_t var13 = pos.y + var6; var13 >= pos.y + var7; --var13) {
				for (int32_t var14 = pos.x - var18; var14 <= pos.x + var18; ++var14) {
					int32_t var15 = var14 - pos.x;

					for (int32_t var16 = pos.z - var18; var16 <= pos.z + var18; ++var16) {
						int32_t var17 = var16 - pos.z;
						if ((JavaMath::abs(var15) != var18 || JavaMath::abs(var17) != var18 || var18 <= 0) &&
							!IsOpaque(world->GetBlockType(Int3{var14, var13, var16}))) {
							// Spruce leaves
							world->SetBlockTypeAndMeta(BLOCK_LEAVES, 1, Int3{var14, var13, var16});
						}
					}
				}

				if (var18 >= 1 && var13 == pos.y + var7 + 1) {
					--var18;
				} else if (var18 < var9) {
					++var18;
				}
			}

			for (int32_t var13 = 0; var13 < var6 - 1; ++var13) {
				blockType = world->GetBlockType(Int3{pos.x, pos.y + var13, pos.z});
				if (blockType == BLOCK_AIR || blockType == BLOCK_LEAVES) {
					world->SetBlockTypeAndMeta(BLOCK_LOG, 1, Int3{pos.x, pos.y + var13, pos.z});
				}
			}

			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool Beta173TaigaAltTree::Generate(World *world, JavaRandom *rand, Int3 pos,
								   [[maybe_unused]] bool birch) {
	int32_t var6 = rand->nextInt(4) + 6;
	int32_t var7 = 1 + rand->nextInt(2);
	int32_t var8 = var6 - var7;
	int32_t var9 = 2 + rand->nextInt(2);
	bool canGenerate = true;
	if (pos.y >= 1 && pos.y + var6 + 1 <= CHUNK_HEIGHT) {
		int32_t var11;
		int32_t var13;
		int32_t var15;
		int32_t var21;
		for (var11 = pos.y; var11 <= pos.y + 1 + var6 && canGenerate; ++var11) {
			// bool var12 = true;
			if (var11 - pos.y < var7) {
				var21 = 0;
			} else {
				var21 = var9;
			}

			for (var13 = pos.x - var21; var13 <= pos.x + var21 && canGenerate; ++var13) {
				for (int32_t var14 = pos.z - var21; var14 <= pos.z + var21 && canGenerate; ++var14) {
					if (var11 >= 0 && var11 < CHUNK_HEIGHT) {
						var15 = world->GetBlockType(Int3{var13, var11, var14});
						if (var15 != 0 && var15 != BLOCK_LEAVES) {
							canGenerate = false;
						}
					} else {
						canGenerate = false;
					}
				}
			}
		}

		if (!canGenerate)
			return false;
	
		var11 = world->GetBlockType(Int3{pos.x, pos.y - 1, pos.z});
		if ((var11 == BLOCK_GRASS || var11 == BLOCK_DIRT) && pos.y < CHUNK_HEIGHT - var6 - 1) {
			world->SetBlockType(BLOCK_DIRT, Int3{pos.x, pos.y - 1, pos.z});
			var21 = rand->nextInt(2);
			var13 = 1;
			int8_t var22 = 0;

			int32_t var16;
			int32_t var17;
			for (var15 = 0; var15 <= var8; ++var15) {
				var16 = pos.y + var6 - var15;

				for (var17 = pos.x - var21; var17 <= pos.x + var21; ++var17) {
					int32_t var18 = var17 - pos.x;

					for (int32_t var19 = pos.z - var21; var19 <= pos.z + var21; ++var19) {
						int32_t var20 = var19 - pos.z;
						if ((JavaMath::abs(var18) != var21 || JavaMath::abs(var20) != var21 || var21 <= 0) &&
							!IsOpaque(world->GetBlockType(Int3{var17, var16, var19}))) {
							world->SetBlockTypeAndMeta(BLOCK_LEAVES, 1, Int3{var17, var16, var19});
						}
					}
				}

				if (var21 >= var13) {
					var21 = var22;
					var22 = 1;
					++var13;
					if (var13 > var9) {
						var13 = var9;
					}
				} else {
					++var21;
				}
			}

			var15 = rand->nextInt(3);

			for (var16 = 0; var16 < var6 - var15; ++var16) {
				var17 = world->GetBlockType(Int3{pos.x, pos.y + var16, pos.z});
				if (var17 == BLOCK_AIR || var17 == BLOCK_LEAVES) {
					world->SetBlockTypeAndMeta(BLOCK_LOG, 1, Int3{pos.x, pos.y + var16, pos.z});
				}
			}

			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}