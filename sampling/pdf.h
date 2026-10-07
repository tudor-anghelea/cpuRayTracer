#ifndef PDF_H
#define PDF_H

#include "sampling/onb.h"
#include "geometry/hittable.h"

class pdf
{
public:
    virtual ~pdf() = default;

    virtual double value(const vec3& direction) const = 0;
    virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf
{
public:
    cosine_pdf(const vec3& w)
    {
        uvw.build_from_w(w);
    }

    double value(const vec3& direction) const override
    {
        auto cosine = dot(unit_vector(direction), uvw.w());

        return cosine <= 0 ? 0 : cosine / pi;
    }

    vec3 generate() const override
    {
        return uvw.local(random_cosine_direction());
    }

private:
    onb uvw;
};

class hittable_pdf : public pdf
{
public:
    hittable_pdf(const hittable& objects, const point3& origin) : objects(objects), origin(origin) {}

    double value(const vec3& direction) const override
    {
        return objects.pdf_value(origin, direction);
    }

    vec3 generate() const override
    {
        return objects.random(origin);
    }

private:
    const hittable& objects;
    point3 origin;
};

class mixture_pdf : public pdf
{
public:
    mixture_pdf(const pdf& p0, const pdf& p1) : p {&p0, &p1} {}

    double value(const vec3& direction) const override
    {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    vec3 generate() const override
    {
        if (random_double() < 0.5) return p[0]->generate();

        return p[1]->generate();
    }

private:
    const pdf* p[2];
};

#endif
