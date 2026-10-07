#include "raytracer.h"

void cornell_box_scene(hittable_list& world, hittable_list& lights, camera& cam) //ready
{
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(3, 3, 3));

	world.add(make_shared<quad>(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green));
	world.add(make_shared<quad>(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red));
	world.add(make_shared<quad>(point3(0, 555, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white));

	auto ceiling_light = make_shared<quad>(point3(113, 554, 127), vec3(330, 0, 0), vec3(0, 0, 305), light);
	world.add(ceiling_light);
	lights.add(ceiling_light);

	shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));

	shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));

	world.add(box1);
	world.add(box2);

	cam.aspect_ratio = 1.0;
	cam.image_width = 1920;
	cam.samples_per_pixel = 50;
	cam.max_depth = 20;

	cam.vfov = 40;
	cam.vup = vec3(0, 1, 0);

	cam.lookfrom = point3(278, 278, -800);
	cam.lookat = point3(278, 278, 0);

	cam.background = color(0.0, 0.0, 0.0);
}

void showcase_scene(hittable_list& world, hittable_list& lights, camera& cam) //ready
{
	auto red = make_shared<lambertian>(color(0.75, 0.12, 0.1));
	auto blue = make_shared<lambertian>(color(0.1, 0.25, 0.75));
	auto green = make_shared<lambertian>(color(0.1, 0.65, 0.25));
	auto white = make_shared<lambertian>(color(0.8, 0.8, 0.8));
	auto floor_mat = make_shared<lambertian>(color(0.25, 0.25, 0.25));
	auto light_mat = make_shared<diffuse_light>(color(8, 8, 8));

	world.add(make_shared<quad>(point3(-10, 0, -10), vec3(20, 0, 0), vec3(0, 0, 20), floor_mat));
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, red));
	world.add(make_shared<sphere>(point3(-2.5, 0.5, 1), 0.51, blue));
	world.add(make_shared<sphere>(point3(2.3, 0.7, 1.5), 0.7, green));
	world.add(make_shared<sphere>(point3(0, 0.35, 2.2), 0.35, white));

	auto light = make_shared<quad>(point3(-2, 6, -2), vec3(4, 0, 0), vec3(0, 0, 4), light_mat);

	world.add(light);
	lights.add(light);

	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 1920;
	cam.samples_per_pixel = 200;
	cam.max_depth = 15;

	cam.vfov = 45;
	cam.vup = vec3(0, 1, 0);

	cam.lookfrom = point3(6, 3.5, 8);
	cam.lookat = point3(0, 1, 0);

	cam.background = color(0.02, 0.02, 0.02);
}

void bvh_stress_scene(hittable_list& world, hittable_list& lights, camera& cam) //ready
{
	auto ground_material = make_shared<lambertian>(color(0.45, 0.45, 0.45));
	auto light = make_shared<diffuse_light>(color(12, 12, 12));

	world.add(make_shared<quad>(point3(-50.0, 0.0, -50.0), vec3(100.0, 0.0, 0.0), vec3(0.0, 0.0, 100.0), ground_material));

	for (int a = -50; a < 50; a++)
	{
		for (int b = -50; b < 50; b++)
		{
			double offset =  0.3 * random_double();
			point3 center(a * 0.9 + 0.5 * random_double(), offset, b * 0.9 + 0.5 * random_double());

			if ((center - point3(4, offset, 0)).length() > 0.9)
			{
				auto albedo = color::random() * color::random();
				auto mat = make_shared<lambertian>(albedo);

				world.add(make_shared<sphere>(center, offset, mat));
			}
		}
	}

	auto big_sphere = make_shared<sphere>(point3(-4, 1, 0), 1.0, light);
	world.add(big_sphere);
	lights.add(big_sphere);

	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 1920;
	cam.samples_per_pixel = 200;
	cam.max_depth = 20;

	cam.vfov = 30;
	cam.vup = vec3(0, 1, 0);

	cam.lookfrom = point3(13, 4, 6);
	cam.lookat = point3(0, 0, 0);

	cam.background = color(0.5, 0.7, 1.0);
}

int main()
{
	hittable_list world, lights;
	camera cam;
	renderer renderer(cam);

	renderer.denoiser = true;

	bvh_stress_scene(world, lights, cam);

	world = hittable_list(make_shared<bvh_node>(world));
	renderer.render(world, lights);
}