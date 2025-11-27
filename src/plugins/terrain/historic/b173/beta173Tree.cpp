#include "beta173Tree.h"

bool Beta173Tree::Generate(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock, bool birch) {
    int treeHeight = rand->nextInt(3) + 4;
    if (birch) treeHeight++;
    bool canPlace = true;
    if(yBlock >= 1 && yBlock + treeHeight + 1 <= CHUNK_HEIGHT) {
        int yI;
        int xI;
        int zI;
        int blockType;
        for(yI = yBlock; yI <= yBlock + 1 + treeHeight; ++yI) {
            int8_t width = 1;
            if(yI == yBlock) {
                width = 0;
            }

            if(yI >= yBlock + 1 + treeHeight - 2) {
                width = 2;
            }

            for(xI = xBlock - width; xI <= xBlock + width && canPlace; ++xI) {
                for(zI = zBlock - width; zI <= zBlock + width && canPlace; ++zI) {
                    if(yI >= 0 && yI < CHUNK_HEIGHT) {
                        blockType = world->GetBlockType(Int3{xI, yI, zI});
                        if(blockType != BLOCK_AIR && blockType != BLOCK_LEAVES) {
                            canPlace = false;
                        }
                    } else {
                        canPlace = false;
                    }
                }
            }
        }

        if(!canPlace) {
            return false;
        }
        yI = world->GetBlockType(Int3{xBlock, yBlock - 1, zBlock});
        if(
            (yI == BLOCK_GRASS || yI == BLOCK_DIRT) &&
            yBlock < CHUNK_HEIGHT - treeHeight - 1
        ) {
            world->SetBlockType(BLOCK_DIRT, Int3{xBlock, yBlock - 1, zBlock});

            int yIt;
            for(yIt = yBlock - 3 + treeHeight; yIt <= yBlock + treeHeight; ++yIt) {
                xI = yIt - (yBlock + treeHeight);
                zI = 1 - xI / 2;

                for(blockType = xBlock - zI; blockType <= xBlock + zI; ++blockType) {
                    int zEnd = blockType - xBlock;

                    for(int var14 = zBlock - zI; var14 <= zBlock + zI; ++var14) {
                        int yStart = var14 - zBlock;
                        if ((
                                (
                                    std::abs(zEnd) != zI ||
                                    std::abs(yStart) != zI ||
                                    (
                                        rand->nextInt(2) != 0 && xI != 0
                                    )
                                )
                            ) && !IsOpaque(world->GetBlockType(Int3{blockType, yIt, var14}))
                        ) {
                            world->PlaceBlock(Int3{blockType, yIt, var14}, BLOCK_LEAVES, birch ? 2 : 0);
                        }
                    }
                }
            }

            for(yIt = 0; yIt < treeHeight; ++yIt) {
                xI = world->GetBlockType(Int3{xBlock, yBlock + yIt, zBlock});
                if(xI == 0 || xI == BLOCK_LEAVES) {
                    world->PlaceBlock(Int3{xBlock, yBlock + yIt, zBlock}, BLOCK_LOG, birch ? 2 : 0);
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

void Beta173BigTree::Configure(double treeHeight, double branchLength, double trunkShape) {
    this->maximumTreeHeight = (int)(treeHeight * 12.0);
    if(treeHeight > 0.5) {
        this->trunkThickness = 5;
    }

    this->branchLength = branchLength;
    this->trunkShape = trunkShape;
}

bool Beta173BigTree::Generate(
    [[maybe_unused]] World* world,
    JavaRandom* otherRand,
    [[maybe_unused]] int xBlock,
    [[maybe_unused]] int yBlock,
    [[maybe_unused]] int zBlock,
    [[maybe_unused]] bool birch
) {
    // Waste one rand cycle to get closer to being accurate
    
    this->world = world;
    long seed = otherRand->nextLong();
    this->rand->setSeed(seed);
    this->basePos = Int3{xBlock, yBlock, zBlock};
    if(this->totalHeight == 0) {
        this->totalHeight = 5 + this->rand->nextInt(this->maximumTreeHeight);
    }

    if(!this->ValidPlacement()) {
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
    this->height = (int)((double)this->totalHeight * this->heightFactor);
    if(this->height >= this->totalHeight) {
        this->height = this->totalHeight - 1;
    }

    int var1 = (int)(1.382 + std::pow(this->trunkShape * (double)this->totalHeight / 13.0, 2.0));
    if(var1 < 1) {
        var1 = 1;
    }

    std::vector<std::array<int, 4>> var2(var1 * this->totalHeight);
    int var3 = this->basePos.y + this->totalHeight - this->trunkThickness;
    int var4 = 1;
    int var5 = this->basePos.y + this->height;
    int var6 = var3 - this->basePos.y;
    var2[0][0] = this->basePos.x;
    var2[0][1] = var3;
    var2[0][2] = this->basePos.z;
    var2[0][3] = var5;
    --var3;

    while(true) {
        while(var6 >= 0) {
            int var7 = 0;
            float var8 = this->func_431_a(var6);
            if(var8 < 0.0F) {
                --var3;
                --var6;
            } else {
                for(double var9 = 0.5; var7 < var1; ++var7) {
                    double var11 = this->branchLength * (double)var8 * ((double)this->rand->nextFloat() + 0.328);
                    // Oh hey, look! An approximation of pi!
                    double var13 = (double)this->rand->nextFloat() * 2.0 * 3.14159;
                    int var15 = MathHelper::floor_double(var11 * double(MathHelper::sin(var13)) + (double)this->basePos.x + var9);
                    int var16 = MathHelper::floor_double(var11 * double(MathHelper::cos(var13)) + (double)this->basePos.z + var9);
                    Int3 var17 = Int3{var15, var3, var16};
                    Int3 var18 = Int3{var15, var3 + this->trunkThickness, var16};
                    if(this->checkIfPathClear(var17, var18) == -1) {
                        Int3 var19 = Int3{this->basePos.x, this->basePos.y, this->basePos.z};
                        double var20 = std::sqrt(std::pow((double)std::abs(this->basePos.x - var17.x), 2.0) + std::pow((double)std::abs(this->basePos.z - var17.z), 2.0));
                        double var22 = var20 * this->field_752_i;
                        if((double)var17.y - var22 > (double)var5) {
                            var19.y = var5;
                        } else {
                            var19.y = (int)((double)var17.y - var22);
                        }

                        if(this->checkIfPathClear(var19, var17) == -1) {
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

        this->branchStartEnd = std::vector<std::array<int, 4>>(var4);
        std::copy(var2.begin(), var2.begin() + var4, this->branchStartEnd.begin());
        return;
    }
}

void Beta173BigTree::func_426_a(int var1, int var2, int var3, float var4, int8_t var5, int var6) {
    int var7 = int(double(var4) + 0.618);
    int8_t var8 = branchOrientation[var5];
    int8_t var9 = branchOrientation[var5 + 3];
    Int3 var10{var1, var2, var3};
    Int3 var11{0, 0, 0};

    for (int var12 = -var7; var12 <= var7; ++var12) {
        var11[var5] = var10[var5];
        var11[var8] = var10[var8] + var12;

        for (int var13 = -var7; var13 <= var7; ++var13) {
            double var15 = std::sqrt(std::pow(std::abs(var12) + 0.5, 2.0) + std::pow(std::abs(var13) + 0.5, 2.0));

            if (var15 > (double)(var4)) {
                continue;
            }

            var11[var9] = var10[var9] + var13;
            int var14 = this->world->GetBlockType(var11);

            if (var14 == 0 || var14 == 18) {
                this->world->PlaceBlock(var11, var6);
            }
        }
    }
}

float Beta173BigTree::func_431_a(int var1) {
    if((double)var1 < (double)((float)this->totalHeight) * 0.3) {
        return -1.618F;
    } else {
        float var2 = (float)this->totalHeight / 2.0F;
        float var3 = (float)this->totalHeight / 2.0F - (float)var1;
        float var4;
        if(var3 == 0.0F) {
            var4 = var2;
        } else if(std::abs(var3) >= var2) {
            var4 = 0.0F;
        } else {
            var4 = (float)std::sqrt(std::pow((double)std::abs(var2), 2.0) - std::pow((double)std::abs(var3), 2.0));
        }

        var4 *= 0.5F;
        return var4;
    }
}

float Beta173BigTree::func_429_b(int var1) {
    return var1 >= 0 && var1 < this->trunkThickness ? (var1 != 0 && var1 != this->trunkThickness - 1 ? 3.0F : 2.0F) : -1.0F;
}

void Beta173BigTree::func_423_a(int var1, int var2, int var3) {
    int var4 = var2;

    for(int var5 = var2 + this->trunkThickness; var4 < var5; ++var4) {
        float var6 = this->func_429_b(var4 - var2);
        this->func_426_a(var1, var4, var3, var6, (int8_t)1, 18);
    }

}

void Beta173BigTree::drawBlockLine(Int3 var1, Int3 var2, int blockType) {
    Int3 var4 = Int3{0, 0, 0};
    int8_t var5 = 0;

    int8_t var6;
    for(var6 = 0; var5 < 3; ++var5) {
        var4[var5] = var2[var5] - var1[var5];
        if(std::abs(var4[var5]) > std::abs(var4[var6])) {
            var6 = var5;
        }
    }

    if(var4[var6] != 0) {
        int8_t var7 = branchOrientation[var6];
        int8_t var8 = branchOrientation[var6 + 3];
        int8_t var9;
        if(var4[var6] > 0) {
            var9 = 1;
        } else {
            var9 = -1;
        }

        double var10 = (double)var4[var7] / (double)var4[var6];
        double var12 = (double)var4[var8] / (double)var4[var6];
        Int3 var14 = Int3{0, 0, 0};
        int var15 = 0;

        for(int var16 = var4[var6] + var9; var15 != var16; var15 += var9) {
            var14[var6] = MathHelper::floor_double((double)(var1[var6] + var15) + 0.5);
            var14[var7] = MathHelper::floor_double((double)var1[var7] + (double)var15 * var10 + 0.5);
            var14[var8] = MathHelper::floor_double((double)var1[var8] + (double)var15 * var12 + 0.5);
            this->world->PlaceBlock(var14, blockType);
        }

    }
}

void Beta173BigTree::GenerateLeafClusters() {
    size_t var1 = 0;

    for(size_t var2 = this->branchStartEnd.size(); var1 < var2; ++var1) {
        int var3 = this->branchStartEnd[var1][0];
        int var4 = this->branchStartEnd[var1][1];
        int var5 = this->branchStartEnd[var1][2];
        this->func_423_a(var3, var4, var5);
    }

}

bool Beta173BigTree::canGenerateBranchAtHeight(int var1) {
    return (double)var1 >= (double)this->totalHeight * 0.2;
}

void Beta173BigTree::GenerateTrunk() {
    int var1 = this->basePos.x;
    int var2 = this->basePos.y;
    int var3 = this->basePos.y + this->height;
    int var4 = this->basePos.z;
    Int3 var5 = Int3{var1, var2, var4};
    Int3 var6 = Int3{var1, var3, var4};
    this->drawBlockLine(var5, var6, 17);
    if(this->branchDensity == 2) {
        ++var5[0];
        ++var6[0];
        this->drawBlockLine(var5, var6, 17);
        ++var5[2];
        ++var6[2];
        this->drawBlockLine(var5, var6, 17);
        var5[0] += -1;
        var6[0] += -1;
        this->drawBlockLine(var5, var6, 17);
    }

}

void Beta173BigTree::GenerateBranches() {
    size_t var1 = 0;
    size_t var2 = this->branchStartEnd.size();

    for(Int3 var3 = Int3{this->basePos.x, this->basePos.y, this->basePos.z}; var1 < var2; ++var1) {
        Int3 var4 = Int3{this->branchStartEnd[var1][0], this->branchStartEnd[var1][1], this->branchStartEnd[var1][2]};
        int varBaseY = this->branchStartEnd[var1][3];  
        Int3 var5 = Int3{var4[0], var4[1], var4[2]};
        var3[1] = varBaseY;
        int var6 = var3[1] - this->basePos.y;
        if(this->canGenerateBranchAtHeight(var6)) {
            this->drawBlockLine(var3, var5, 17);
        }
    }

}

int Beta173BigTree::checkIfPathClear(Int3 var1, Int3 var2) {
    Int3 var3 = Int3{0, 0, 0};
    int8_t var4 = 0;

    int8_t var5;
    for(var5 = 0; var4 < 3; ++var4) {
        var3[var4] = var2[var4] - var1[var4];
        if(std::abs(var3[var4]) > std::abs(var3[var5])) {
            var5 = var4;
        }
    }

    if(var3[var5] == 0) {
        return -1;
    } else {
        int8_t var6 = branchOrientation[var5];
        int8_t var7 = branchOrientation[var5 + 3];
        int8_t var8;
        if(var3[var5] > 0) {
            var8 = 1;
        } else {
            var8 = -1;
        }

        double var9 = (double)var3[var6] / (double)var3[var5];
        double var11 = (double)var3[var7] / (double)var3[var5];
        Int3 var13 = Int3{0, 0, 0};
        int var14 = 0;

        int var15;
        for(var15 = var3[var5] + var8; var14 != var15; var14 += var8) {
            var13[var5] = var1[var5] + var14;
            var13[var6] = MathHelper::floor_double((double)var1[var6] + (double)var14 * var9);
            var13[var7] = MathHelper::floor_double((double)var1[var7] + (double)var14 * var11);
            int var16 = this->world->GetBlockType(var13);
            if(var16 != 0 && var16 != 18) {
                break;
            }
        }

        return var14 == var15 ? -1 : std::abs(var14);
    }
}

bool Beta173BigTree::ValidPlacement() {
    Int3 var1 = Int3{this->basePos.x, this->basePos.y, this->basePos.z};
    Int3 var2 = Int3{this->basePos.x, this->basePos.y + this->totalHeight - 1, this->basePos.z};
    int var3 = this->world->GetBlockType(Int3{this->basePos.x, this->basePos.y - 1, this->basePos.z});
    if(var3 != 2 && var3 != 3) {
        return false;
    } else {
        int var4 = this->checkIfPathClear(var1, var2);
        if(var4 == -1) {
            return true;
        } else if(var4 < 6) {
            return false;
        } else {
            this->totalHeight = var4;
            return true;
        }
    }
}

bool Beta173TaigaTree::Generate(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock, [[maybe_unused]] bool birch) {
    int var6 = rand->nextInt(5) + 7;
    int var7 = var6 - rand->nextInt(2) - 3;
    int var8 = var6 - var7;
    int var9 = 1 + rand->nextInt(var8 + 1);
    bool var10 = true;
    if(yBlock >= 1 && yBlock + var6 + 1 <= CHUNK_HEIGHT) {
        int var18;
        for(int var11 = yBlock; var11 <= yBlock + 1 + var6 && var10; ++var11) {
            //bool var12 = true;
            if(var11 - yBlock < var7) {
                var18 = 0;
            } else {
                var18 = var9;
            }

            for(int var13 = xBlock - var18; var13 <= xBlock + var18 && var10; ++var13) {
                for(int var14 = zBlock - var18; var14 <= zBlock + var18 && var10; ++var14) {
                    if(var11 >= 0 && var11 < CHUNK_HEIGHT) {
                        int8_t blockType = world->GetBlockType(Int3{var13, var11, var14});
                        if(blockType != BLOCK_AIR && blockType != BLOCK_LEAVES) {
                            var10 = false;
                        }
                    } else {
                        var10 = false;
                    }
                }
            }
        }

        if(!var10) {
            return false;
        } else {
            int8_t blockType = world->GetBlockType(Int3{xBlock, yBlock - 1, zBlock});
            if((blockType == BLOCK_GRASS || blockType == BLOCK_DIRT) && yBlock < CHUNK_HEIGHT - var6 - 1) {
                world->SetBlockType(BLOCK_DIRT, Int3{xBlock, yBlock - 1, zBlock});
                var18 = 0;

                for(int var13 = yBlock + var6; var13 >= yBlock + var7; --var13) {
                    for(int var14 = xBlock - var18; var14 <= xBlock + var18; ++var14) {
                        int var15 = var14 - xBlock;

                        for(int var16 = zBlock - var18; var16 <= zBlock + var18; ++var16) {
                            int var17 = var16 - zBlock;
                            if(
                                (
                                    std::abs(var15) != var18 ||
                                    std::abs(var17) != var18 ||
                                    var18 <= 0
                                ) && !IsOpaque(world->GetBlockType(Int3{var14, var13, var16}))
                            ) {
                                // Spruce leaves
                                world->SetBlockTypeAndMeta(BLOCK_LEAVES, 1, Int3{var14, var13, var16});
                            }
                        }
                    }

                    if(var18 >= 1 && var13 == yBlock + var7 + 1) {
                        --var18;
                    } else if(var18 < var9) {
                        ++var18;
                    }
                }

                for(int var13 = 0; var13 < var6 - 1; ++var13) {
                    int8_t blockType = world->GetBlockType(Int3{xBlock, yBlock + var13, zBlock});
                    if(blockType == BLOCK_AIR || blockType == BLOCK_LEAVES) {
                        world->SetBlockTypeAndMeta(BLOCK_LOG, 1, Int3{xBlock, yBlock + var13, zBlock});
                    }
                }

                return true;
            } else {
                return false;
            }
        }
    } else {
        return false;
    }
}

bool Beta173TaigaAltTree::Generate(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock, [[maybe_unused]] bool birch) {
    int var6 = rand->nextInt(4) + 6;
    int var7 = 1 + rand->nextInt(2);
    int var8 = var6 - var7;
    int var9 = 2 + rand->nextInt(2);
    bool var10 = true;
    if(yBlock >= 1 && yBlock + var6 + 1 <= CHUNK_HEIGHT) {
        int var11;
        int var13;
        int var15;
        int var21;
        for(var11 = yBlock; var11 <= yBlock + 1 + var6 && var10; ++var11) {
            //bool var12 = true;
            if(var11 - yBlock < var7) {
                var21 = 0;
            } else {
                var21 = var9;
            }

            for(var13 = xBlock - var21; var13 <= xBlock + var21 && var10; ++var13) {
                for(int var14 = zBlock - var21; var14 <= zBlock + var21 && var10; ++var14) {
                    if(var11 >= 0 && var11 < CHUNK_HEIGHT) {
                        var15 = world->GetBlockType(Int3{var13, var11, var14});
                        if(var15 != 0 && var15 != BLOCK_LEAVES) {
                            var10 = false;
                        }
                    } else {
                        var10 = false;
                    }
                }
            }
        }

        if(!var10) {
            return false;
        } else {
            var11 = world->GetBlockType(Int3{xBlock, yBlock - 1, zBlock});
            if((var11 == BLOCK_GRASS || var11 == BLOCK_DIRT) && yBlock < CHUNK_HEIGHT - var6 - 1) {
                world->SetBlockType(BLOCK_DIRT, Int3{xBlock, yBlock - 1, zBlock});
                var21 = rand->nextInt(2);
                var13 = 1;
                int8_t var22 = 0;

                int var16;
                int var17;
                for(var15 = 0; var15 <= var8; ++var15) {
                    var16 = yBlock + var6 - var15;

                    for(var17 = xBlock - var21; var17 <= xBlock + var21; ++var17) {
                        int var18 = var17 - xBlock;

                        for(int var19 = zBlock - var21; var19 <= zBlock + var21; ++var19) {
                            int var20 = var19 - zBlock;
                            if(
                                (
                                    std::abs(var18) != var21 ||
                                    std::abs(var20) != var21 ||
                                    var21 <= 0
                                ) && !IsOpaque(world->GetBlockType(Int3{var17, var16, var19}))) {
                                world->SetBlockTypeAndMeta(BLOCK_LEAVES, 1, Int3{var17, var16, var19});
                            }
                        }
                    }

                    if(var21 >= var13) {
                        var21 = var22;
                        var22 = 1;
                        ++var13;
                        if(var13 > var9) {
                            var13 = var9;
                        }
                    } else {
                        ++var21;
                    }
                }

                var15 = rand->nextInt(3);

                for(var16 = 0; var16 < var6 - var15; ++var16) {
                    var17 = world->GetBlockType(Int3{xBlock, yBlock + var16, zBlock});
                    if(var17 == 0 || var17 == BLOCK_LEAVES) {
                        world->SetBlockTypeAndMeta(BLOCK_LOG, 1, Int3{xBlock, yBlock + var16, zBlock});
                    }
                }

                return true;
            } else {
                return false;
            }
        }
    } else {
        return false;
    }
}