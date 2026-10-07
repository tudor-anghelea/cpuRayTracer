#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "tile.h"
#include "oidn_denoiser.h"

#include <vector>
#include <thread>
#include <atomic>
#include <fstream>
#include <chrono>

class renderer
{
public:

	renderer(camera& cam) : cam(cam) {}

	bool denoiser = true;

	void render(const hittable& world, const hittable& lights)
	{
		std::ofstream image("output.ppm");

		cam.initialize();

		auto start = std::chrono::high_resolution_clock::now();
		store_pixels(world, lights);
		auto render_end = std::chrono::high_resolution_clock::now();

		std::clog << std::endl << std::endl << "Framebuffer completed." << std::flush;


		double denoise_time;
		if (denoiser)
		{
			oidn_denoiser denoiser;
			denoiser.denoise(framebuffer, cam.image_width, cam.image_height);

			auto denoise_end = std::chrono::high_resolution_clock::now();
			std::clog << std::endl << "OIDN completed." << std::flush << std::endl;
			denoise_time = std::chrono::duration<double>(denoise_end - render_end).count();
		}

		double render_time = std::chrono::duration<double>(render_end - start).count();

		image << "P3\n" << cam.image_width << ' ' << cam.image_height << "\n255\n";

		for (int y = 0; y < cam.image_height; y++)
		{
			int remaining = cam.image_height - y;
			float percent = 100.0f * y / cam.image_height;

			std::clog << "\rProgress: " << percent << "% (" << remaining << " scanlines left)" << std::flush;

			for (int x = 0; x < cam.image_width; x++)
			{
				write_color(image, framebuffer[y * cam.image_width + x]);
			}
		}
		std::clog << "\r\033[K" << std::flush;

		std::clog << std::endl << "Render time: " << render_time << " seconds\n";
		if(denoiser) std::clog << "OIDN time: " << denoise_time << " seconds\n";
		std::clog << "Total time: " << render_time + denoise_time << " seconds\n";
	}

private:

	camera& cam;

	std::vector<color> framebuffer;

	std::vector<color> sample_squared;
	std::vector<int> samples_count;

	static constexpr int tile_size = 16;

	void store_pixels(const hittable& world, const hittable& lights)
	{
		framebuffer.resize(cam.image_width * cam.image_height);

		sample_squared.resize(cam.image_width * cam.image_height);
		samples_count.resize(cam.image_width * cam.image_height);

		std::clog << "Storing pixels in a framebuffer..." << std::flush;

		std::vector<Tile> tiles;

		// build tiles
		int total_tiles_x = (cam.image_width + tile_size - 1) / tile_size;
		int total_tiles_y = (cam.image_height + tile_size - 1) / tile_size;
		int total_tiles = total_tiles_x * total_tiles_y;

		int current_tile = 0;

		for (int tile_y = 0; tile_y < cam.image_height; tile_y += tile_size)
		{
			for (int tile_x = 0; tile_x < cam.image_width; tile_x += tile_size)
			{
				Tile tile{
					tile_x,
					tile_y,
					std::min(tile_x + tile_size, cam.image_width),
					std::min(tile_y + tile_size, cam.image_height)
				};
				tiles.push_back(tile);

				current_tile++;
			}
		}

		std::atomic<int> next_tile(0);

		unsigned int num_threads = std::thread::hardware_concurrency();

		std::vector<std::thread> workers;

		for (unsigned int i = 0; i < num_threads; ++i)
		{
			workers.emplace_back(&renderer::worker, this, std::cref(tiles), std::ref(next_tile), std::cref(world), std::cref(lights));
		}

		for (auto& t : workers)
		{
			t.join();
		}
	}

	void render_tile(const Tile& tile, const hittable& world, const hittable& lights)
	{
		for (int y = tile.start_y; y < tile.end_y; y++)
		{
			for (int x = tile.start_x; x < tile.end_x; x++)
			{
				int index = y * cam.image_width + x;

				color pixel_color(0, 0, 0);
				color pixel_squared(0, 0, 0);


				while (samples_count[index] < cam.samples_per_pixel)
				{

					ray r = cam.get_ray(x, y);

					color sample = ray_color(r, cam.max_depth, world, lights);

					pixel_color += sample;
					pixel_squared += sample * sample;

					samples_count[index]++;

					if (pixel_converged(pixel_color, pixel_squared, samples_count[index]))
					{
						break;
					}
				}

				framebuffer[index] = pixel_color / samples_count[index];

				sample_squared[index] = pixel_squared;
			}
		}
	}

	void worker(const std::vector<Tile>& tiles, std::atomic<int>& next_tile, const hittable& world, const hittable& lights)
	{
		while (true)
		{
			int index = next_tile.fetch_add(1);

			if (index >= tiles.size())
				break;

			render_tile(tiles[index], world, lights);
		}
	}

	bool pixel_converged(const color& sum, const color& sum_squared, int samples)
	{
		if (samples < 20) return false;

		color mean = sum / samples;

		color variance = sum_squared / samples - mean * mean;

		double variance_value =(variance.x() + variance.y() + variance.z()) / 3.0;

		return variance_value < 0.0005;
	}

	color ray_color(const ray& r, int depth, const hittable& world, const hittable& lights)
	{
		if (depth <= 0)	return color(0, 0, 0);

		hit_record rec;

		if (!world.hit(r, interval(0.001, infinity), rec)) return cam.background;

		// emitted light from this surface
		color color_from_emission = rec.mat->emitted(r, rec, rec.p);

		scatter_record srec;

		// If the material does not scatter, return emission
		if (!rec.mat->scatter(r, rec, srec)) return color_from_emission;

		if (depth < cam.max_depth - 5)
		{
			double p = std::max(srec.attenuation.x(), std::max(srec.attenuation.y(), srec.attenuation.z()));
			p = std::min(p, 0.95);

			if (random_double() > p) return color_from_emission;

			srec.attenuation /= p;
		}
		
		auto light_pdf = std::make_shared<hittable_pdf>(lights, rec.p);

		mixture_pdf mixed_pdf(*light_pdf, *srec.pdf_ptr);

		vec3 direction = mixed_pdf.generate();

		ray scattered(rec.p + rec.normal * 1e-4, direction);

		double pdf_value = mixed_pdf.value(scattered.direction());

		if (pdf_value <= 0)	return color_from_emission;

		double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

		return color_from_emission + srec.attenuation * scattering_pdf * ray_color(scattered, depth - 1, world, lights) / pdf_value;
	}
};

#endif