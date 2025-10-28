#pragma once

#include "world.h"
#include "javaRandom.h"

class Beta173Tree {
    public:
        Beta173Tree() {};
        virtual ~Beta173Tree() = default;
        virtual bool Generate(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock);
        virtual void Configure([[maybe_unused]] double treeHeight, [[maybe_unused]] double branchLength, [[maybe_unused]] double trunkShape) { };
};

class Beta173BigTree : public Beta173Tree {
    private:
        //static final byte[] branchOrientation = new byte[]{(byte)2, (byte)0, (byte)0, (byte)1, (byte)2, (byte)1};
        //Random rand = new Random();
        //World world;
        Int3 basePos = Int3{0, 0, 0};
        int totalHeight = 0;
        int height;
        double heightFactor = 0.618D;
        double field_753_h = 1.0D;
        double field_752_i = 0.381D;
        double branchLength = 1.0D;
        double trunkShape = 1.0D;
        int branchDensity = 1;
        int maximumTreeHeight = 12;
        int trunkThickness = 4;
        //int[][] branchStartEnd;
    public:
        Beta173BigTree() {};
        ~Beta173BigTree() = default;
        bool Generate(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock);
        void Configure(double treeHeight, double branchLength, double trunkShape);
};