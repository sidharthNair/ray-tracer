#pragma once

#include "bbox.h"
#include "scene.h"
#include "ray.h"
#include <iostream>


class Scene;
class Geometry;
class Trimesh;
class TrimeshFace;
class MaterialSceneObject;

class BVHNode
{
public:
    BVHNode(int start, int end, BoundingBox box)
        : start(start), end(end), box(box), left(nullptr), right(nullptr)
    {
    }
    ~BVHNode()
    {
        delete left;
        delete right;
    }

    BoundingBox box;
    int start, end;
    BVHNode *left;
    BVHNode *right;
};

class BVH
{
public:
    BVH() {}
    ~BVH()
    {
        delete root;
    }
    void build(const Scene *scene);
    bool getIntersection(ray &r, isect &i) const;

private:
    BVHNode *root;
    std::vector<Geometry *> geometries;

    BVHNode *buildHelper(int start, int end);
    bool intersectionHelper(BVHNode *curr, ray &r, isect &i) const;
};

class Sorter
{
    int index_;

public:
    Sorter(int index) : index_(index) {}
    bool operator()(Geometry *a, Geometry *b);
};
