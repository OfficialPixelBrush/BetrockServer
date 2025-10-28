#include "beta173Tree.h"

bool Beta173Tree::Generate(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock) {
    int treeHeight = rand->nextInt(3) + 4;
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
                        if(blockType != 0 && blockType != BLOCK_LEAVES) {
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
                            world->SetBlockType(BLOCK_LEAVES, Int3{blockType, yIt, var14});
                        }
                    }
                }
            }

            for(yIt = 0; yIt < treeHeight; ++yIt) {
                xI = world->GetBlockType(Int3{xBlock, yBlock + yIt, zBlock});
                if(xI == 0 || xI == BLOCK_LEAVES) {
                    world->SetBlockType(BLOCK_LOG, Int3{xBlock, yBlock + yIt, zBlock});
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
    this->maximumTreeHeight = (int)(treeHeight * 12.0D);
    if(treeHeight > 0.5D) {
        this->trunkThickness = 5;
    }

    this->branchLength = branchLength;
    this->trunkShape = trunkShape;
}

bool Beta173BigTree::Generate(
    [[maybe_unused]] World* world,
    JavaRandom* rand,
    [[maybe_unused]] int xBlock,
    [[maybe_unused]] int yBlock,
    [[maybe_unused]] int zBlock) {
    // Waste one rand cycle to get closer to being accurate
    rand->nextLong();
    return true;
}