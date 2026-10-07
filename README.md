# CPU Path Tracer

A CPU-based path tracer written from scratch in C++. The renderer implements Monte Carlo path tracing with **Next Event Estimation**, **BVH acceleration**, **multithreading**, and **Intel Open Image Denoise** to produce clear image output.

<p align="center">
  <img src="images/showcase_scene.png" width="800" alt="Showcase Scene">
</p>

## Features

### Rendering
 - Monte Carlo path tracing with recursive ray traversal
 - Cosine-weighted hemisphere sampling (**Indirect Lighting**)
 - Next Event Estimation (**Direct Lighting**)
 - **Lambertian** and **Emissive** material types 
 - Sphere and Quad entity types 
### Acceleration 
 - Bounding Volume Hierarchy (**BVH**)
 - Multithreaded rendering 
 - Russian Roulette path termination
### Post Processing 
 - Intel Open Image Denoise (**OIDN**)
 - HDR framebuffer
 - PPM image output 

## Gallery

### Cornell Box

<img src="images/cornell_box.png" width="500" alt="Cornell Box">

*Classic Cornell Box validating global illumination and color bleeding*

### BVH Stress Test

<img src="images/bvh_stress_scene.png" width="500" alt="Stress Scene">

*Hundreds of diffuse spheres with Lambertian distribution, rendered using BVH acceleration*

### Showcase Scene

<img src="images/showcase_scene.png" width="500" alt="Showcase Scene">

*Diffuse material showcase demonstrating soft shadows, emissive lighting and light traversal from an object to another*

## Performance

| Scene | Resolution | Samples | Threads | Render Time | OIDN Denoising Time |
|------------|---------|---------|-------------|-----------|-----------|
| Cornell Box | 1920x1920 | 50 | 22 | 53.53s | 1.43s |
| Cornell Box | 1920x1920 | 200 | 22 | 194.46s | 1.29s |
| Showcase Scene | 1920x1080 | 50 | 22 | 11.44s | 1.04s |
| Showcase Scene | 1920x1080 | 200 | 22 | 44.21s | 0.93s |
| BVH Stress Scene | 1920x1080 | 50 | 22 | 24.77s | 0.75s |
| BVH Stress Scene | 1920x1080 | 200 | 22 | 145.58s | 0.96s |

Benchmark machine:
 - CPU: Intel Core Ultra 9 185H
 - RAM: 64 GB
 - Compiler: MSVC (Visual Studio 2026, v145)
 - Build: Release Mode

## Technical Overview

The renderer uses Monte Carlo path tracing to simulate the transport of light through the scene. For each pixel, the camera generates multiple randomly sampled rays. Each ray is traced through the scene and recursively evaluated as it interacts with geometry and materials.

### 1. Ray Generation

For every pixel, the camera generates a primary ray originating from the camera and passing through a randomly sampled position within the pixel. Multiple samples per pixel are used to reduce aliasing and Monte Carlo noise.

### 2. Ray-Scene Intersection

The ray is tested against the scene to determine the closest visible surface. Scene geometry is organized in a Bounding Volume Hierarchy (BVH), allowing large portions of the scene to be rejected without testing every object individually.

### 3. Material Scattering

When a ray hits a surface, the material determines how the ray interacts with it. The renderer currently uses Lambertian diffuse materials, where incoming light is scattered according to a cosine-weighted probability distribution.

### 4. Direct Illumination

Next Event Estimation is used to explicitly sample emissive objects in the scene. A shadow ray is generated toward the sampled light to determine whether it is visible from the current surface. This provides a lower-variance estimate of direct illumination.

### 5. Indirect Illumination

The scattered ray is recursively traced through the scene to account for indirect illumination and multiple light bounces. Russian roulette is used at deeper path depths to probabilistically terminate paths while maintaining an unbiased estimator.

### 6. Monte Carlo Integration

The final pixel value is obtained by averaging the radiance contributions from multiple randomly sampled paths. Increasing the number of samples per pixel reduces noise and causes the image to converge toward the solution of the rendering equation.

### 7. Multithreaded Rendering

Pixels are rendered concurrently across multiple CPU threads. Each thread processes independent portions of the image, allowing the renderer to take advantage of the available CPU cores.

### 8. Denoising and Image Output

The resulting floating-point framebuffer contains residual Monte Carlo noise. Intel Open Image Denoise (OIDN) is applied as a final post-processing step to reduce this noise while preserving important image features.

After denoising, the image undergoes gamma correction and is written to a PPM image file.

## Future Implementations

- Metallic and dielectric materials
- Texture and normal mapping
- Depth of field and motion blur
- Further sampling and rendering optimizations

## References

- Peter Shirley, *Ray Tracing in One Weekend*
  https://raytracing.github.io/

- Matt Pharr, Wenzel Jakob, and Greg Humphreys, *Physically Based Rendering: From Theory to Implementation*
  https://pbr-book.org/

- Intel Open Image Denoise Documentation
  https://www.openimagedenoise.org/
