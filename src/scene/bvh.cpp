#include "bvh.h"
#include "../SceneObjects/trimesh.h"

static int leaves, nodes;

bool Sorter::operator()(Geometry *a, Geometry *b)
{
    return a->getBoundingBox().bmax[index_] < b->getBoundingBox().bmin[index_];
}

void BVH::build(const Scene *scene)
{
    // std::cout << "BUILDING BVH" << std::endl;
    for (auto iter = scene->beginObjects(); iter != scene->endObjects(); iter++)
    {
        Geometry *geometry = iter->get();
        if (geometry->isTrimesh())
        {
            // Trimesh is a subclass of geometry, and overrides this method to return true.
            Trimesh *trimesh = (Trimesh *)geometry;
            for (TrimeshFace *face : trimesh->faces)
            {
                face->ComputeBoundingBox();
                geometries.push_back((Geometry *)face);
            }
        }
        else {
            geometries.push_back(geometry);
        }
    }

    if (geometries.size() == 0)
    {
        return;
    }

    // std::cout << "NUM GEOMETRIES: " << geometries.size() << std::endl;
    root = buildHelper(0, geometries.size());
    // std::cout << nodes << " " << leaves << std::endl;
    // std::cout << "DONE BUILDING" << std::endl;
}

BVHNode *BVH::buildHelper(int start, int end)
{
    if (start >= end)
    {
        return nullptr;
    }
    // std::cout << start << " " << end << "\t" << end - start << std::endl;
    if ((end - start) == 1)
    {
        // std::cout << "SINGLE ELEMENT " << std::endl;
        nodes++;
        leaves++;
        return new BVHNode(start, end, geometries[start]->getBoundingBox());
    }

    BoundingBox box;
    for (int i = start; i < end; i++)
    {
        box.merge(geometries[i]->getBoundingBox());
    }
    BVHNode *curr = new BVHNode(start, end, box);

    int dim = box.largestDimension();
    std::partial_sort(geometries.begin() + start, geometries.begin() + end,
                      geometries.begin() + end, Sorter(dim));
    int partition = (start + end + 1) / 2;

    // std::cout << "RECURSING LEFT: " << LEFT(idx) << std::endl;
    curr->left = buildHelper(start, partition);
    // std::cout << "RECURSING LEFT: " << RIGHT(idx) << std::endl;
    curr->right = buildHelper(partition, end);

    nodes++;
    return curr;
}

bool BVH::getIntersection(ray &r, isect &i) const
{
    return intersectionHelper(root, r, i);
}

bool BVH::intersectionHelper(BVHNode *curr, ray &r, isect &i) const
{
    bool foundIntersection = false;
    double tMin, tMax;
    if (curr == nullptr || !curr->box.intersect(r, tMin, tMax))
    {
        return false;
    }

    if ((curr->end - curr->start) == 1)
    {
        isect intersection;
        if (geometries[curr->start]->intersect(r, intersection))
        {
            if (i.getT() == 0 || intersection.getT() < i.getT())
            {
                i = intersection;
                foundIntersection = true;
            }
        }
    }
    else
    {
        bool left = intersectionHelper(curr->left, r, i);
        bool right = intersectionHelper(curr->right, r, i);
        foundIntersection = left || right;
    }

    return foundIntersection;
}
