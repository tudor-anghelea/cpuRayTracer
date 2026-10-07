#ifndef ONB_H
#define ONB_H

#include "core/vec3.h"

class onb
{
public:
    void build_from_w(const vec3& n)
    {
        vec3 a;

        axis[2] = unit_vector(n);

        if (fabs(axis[2].x()) > 0.9)
        {
            a = vec3(0, 1, 0);
        }
        else
        {
            a = vec3(1, 0, 0);
        }

        axis[0] = unit_vector(cross(axis[2], a));
        axis[1] = unit_vector(cross(axis[2], axis[0]));
    }

    vec3 local(const vec3& a) const
    {
        return a.x() * axis[0]
            + a.y() * axis[1]
            + a.z() * axis[2];
    }

    const vec3& u() const { return axis[0]; }
    const vec3& v() const { return axis[1]; }
    const vec3& w() const { return axis[2]; }

private:
    vec3 axis[3]; //u, v, w
};

#endif