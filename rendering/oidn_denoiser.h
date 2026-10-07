#ifndef OIDN_DENOISER_H
#define OIDN_DENOISER_H

#include <OpenImageDenoise/oidn.hpp>

#include <vector>
#include <iostream>

#include "core/color.h"

class oidn_denoiser
{
public:

    void denoise(std::vector<color>& framebuffer, int width, int height)
    {
        oidn::DeviceRef device = oidn::newDevice();
        device.commit();

        oidn::BufferRef buffer = device.newBuffer(width * height * 3 * sizeof(float));

        float* oidn_buffer = static_cast<float*>(buffer.getData());

        oidn::FilterRef filter = device.newFilter("RT");

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = y * width + x;

                oidn_buffer[index * 3 + 0] = float(framebuffer[index].x());
                oidn_buffer[index * 3 + 1] = float(framebuffer[index].y());
                oidn_buffer[index * 3 + 2] = float(framebuffer[index].z());
            }
        }

        filter.setImage("color", buffer, oidn::Format::Float3, width, height);

        filter.setImage("output", buffer, oidn::Format::Float3, width, height);

        filter.set("hdr", true);

        filter.commit();

        filter.execute();

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = y * width + x;

                framebuffer[index] = color(
                    oidn_buffer[index * 3 + 0],
                    oidn_buffer[index * 3 + 1],
                    oidn_buffer[index * 3 + 2]
                );
            }
        }

        const char* message;

        if (device.getError(message) != oidn::Error::None)
        {
            std::cerr << "OIDN Error: " << message << '\n';
        }
    }
};

#endif