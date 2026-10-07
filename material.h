#ifndef MATERIAL_H
#define MATERIAL_H

#include "geometry/hittable.h"
#include "sampling/onb.h"
#include "sampling/pdf.h"

#include <memory>

struct scatter_record
{
	ray specular_ray;
	bool skip_pdf;

	color attenuation;

	std::shared_ptr<pdf> pdf_ptr;
};

class material
{
public:
	virtual ~material() = default;

	virtual color emitted(const ray& r_in, const hit_record& rec, const point3& p) const
	{
		return color(0, 0, 0);
	}

	virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const = 0;

	virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
	{
		return 0;
	}

	virtual color albedo_value() const
	{
		return color(0, 0, 0);
	}
};

class lambertian : public material
{
public:
	lambertian(const color& albedo) : albedo(albedo) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
		srec.attenuation = albedo;

		srec.pdf_ptr = std::make_shared<cosine_pdf>(rec.normal);

		srec.skip_pdf = false;

		return true;
	}

	color albedo_value()
	{
		return albedo;
	}

	double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
	{
		auto cosine = dot(rec.normal, unit_vector(scattered.direction()));

		return cosine < 0 ? 0 : cosine / pi;
	}

private:
	color albedo;
};

class diffuse_light : public material {
public:
	diffuse_light(const color& emit) : emit(emit) {}

	bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
	{
		return false;
	}

	color emitted(const ray& r_in, const hit_record& rec, const point3& p) const override
	{
		if (dot(rec.normal, r_in.direction()) < 0)
			return emit;

		return color(0, 0, 0);
	}

private:
	color emit;
};
#endif
