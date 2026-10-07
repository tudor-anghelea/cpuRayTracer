#ifndef SPHERE_H
#define SPHERE_H

#include "geometry/hittable.h"
#include "core/vec3.h"
#include "sampling/onb.h"

class sphere : public hittable
{
public:
	sphere(const point3& center, double radius, shared_ptr<material> mat)
		: center(center), radius(std::fmax(0, radius)), mat(mat)
	{
		auto rvec = vec3(radius, radius, radius);
		bbox = aabb(center - rvec, center + rvec);
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		vec3 oc = center - r.origin();
		auto a = r.direction().length_squared();
		auto h = dot(r.direction(), oc);
		auto c = oc.length_squared() - radius * radius;
		auto discriminant = h * h - a * c;

		if (discriminant < 0) return false;

		auto sqrtd = std::sqrt(discriminant);

		auto root = (h - sqrtd) / a;
		if (!ray_t.surrounds(root))
		{
			root = (h + sqrtd) / a;
			if (!ray_t.surrounds(root))
			{
				return false;
			}
		}

		rec.t = root;
		rec.p = r.at(rec.t);
		vec3 outward_normal = (rec.p - center) / radius;
		rec.set_face_normal(r, outward_normal);
		rec.mat = mat;

		return true;
	}

	vec3 sample_point() const
	{
		return center + random_unit_vector() * radius;
	}

	aabb bounding_box() const override { return bbox; }

	vec3 random(const point3& origin) const override
	{
		vec3 direction = center - origin;
		double distance_squared = direction.length_squared();

		onb uvw;
		uvw.build_from_w(direction);

		return uvw.local(random_to_sphere(radius, distance_squared));
	}

	double pdf_value(const point3& origin, const vec3& direction) const override
	{
		hit_record rec;

		if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
			return 0;

		auto cos_theta_max = sqrt(1 - radius * radius / (center - origin).length_squared());

		auto solid_angle = 2 * pi * (1 - cos_theta_max);

		return 1 / solid_angle;
	}

private:
	point3 center;
	double radius;
	shared_ptr<material> mat;
	aabb bbox;
};
#endif