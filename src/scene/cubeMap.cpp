#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"

extern TraceUI *traceUI;

glm::dvec3 CubeMap::getColor(ray r) const
{
    // YOUR CODE HERE
    // FIXME: Implement Cube Map here
    glm::dvec3 direction = r.getDirection();
    double x = direction[0];
    double y = direction[1];
    double z = -direction[2];

    double abs_x = fabs(x);
    double abs_y = fabs(y);
    double abs_z = fabs(z);
    double max = fmax(fmax(abs_x, abs_y), fmax(abs_y, abs_z));
    double u, v;
    int idx;
    if (max == abs_x)
    {
        u = x > 0 ? -z : z;
        v = y;
        idx = x > 0 ? 0 : 1;
    }
    else if (max == abs_y)
    {
        u = x;
        v = y > 0 ? -z : z;
        idx = y > 0 ? 2 : 3;
    }
    else if (max == abs_z)
    {
        u = z > 0 ? x : -x;
        v = y;
        idx = z > 0 ? 4 : 5;
    }
    u = 0.5 * (u / max + 1.0);
    v = 0.5 * (v / max + 1.0);
    return tMap[idx]->getMappedValue(glm::dvec2(u, v));
}

CubeMap::CubeMap()
{
}

CubeMap::~CubeMap()
{
}

void CubeMap::setNthMap(int n, TextureMap *m)
{
    if (m != tMap[n].get())
        tMap[n].reset(m);
}
