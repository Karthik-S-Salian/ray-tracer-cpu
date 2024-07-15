#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include <iostream>

#include "color.h"
#include "hittable.h"
#include "material.h"
#include <thread>
#include <string>
#include<vector>

class camera
{
public:
    double aspect_ratio = 1.0;          // Ratio of image width over height         // Rendered image width in pixel count
    int samples_per_pixel = 10;         // Count of random samples for each pixel
    int max_depth = 10;                 // Maximum number of ray bounces into scene
    double vfov = 90;                   // Vertical view angle (field of view)
    point3 lookfrom = point3(0, 0, -1); // Point camera is looking from
    point3 lookat = point3(0, 0, 0);    // Point camera is looking at
    vec3 vup = vec3(0, 1, 0);           // Camera-relative "up" direction

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10;   // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable &world)
    {
        initialize();

        std::cout << "P3\n"
                  << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; ++j)
        {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i)
            {
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; ++sample)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                write_color(std::cout, pixel_color, samples_per_pixel);
            }
        }

        std::clog << "\rDone.                 \n";
    }

    void render_2(const hittable &world)
    {
        initialize();

        char image[image_height * image_width + 3][12];
        auto maxThread = std::thread::hardware_concurrency();
        std::thread threads[maxThread];

        int i = 0;

        image[i++] = "P3";
        image[i++] = std::to_string(image_width) + ' ' + std::to_string(image_height);
        image[i++] = "n255";

        int rows_per_thread = image_height / maxThread;

        for (int k = 0; k < maxThread; k++)
        {
            auto upper_bound = (k + 1) * rows_per_thread;
            if ((image_height - upper_bound) < rows_per_thread)
                upper_bound = image_height;
            computeColor(k*rows_per_thread,upper_bound,image,world);
        }

        std::clog << "\rDone.                 \n";
    }

    void computeColor(int lower_row, int upper_row,char image[][12], const hittable &world)
    {
        for (int j = lower_row; j < upper_row; ++j)
        {
            for (int i = 0; i < image_width; ++i)
            {
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; ++sample)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }

                image[j] = get_color_string(pixel_color, samples_per_pixel);
            }
        }
    }

    std::string get_color_string( color pixel_color, int samples_per_pixel)
    {
        auto r = pixel_color.x();
        auto g = pixel_color.y();
        auto b = pixel_color.z();

        // Divide the color by the number of samples.
        auto scale = 1.0 / samples_per_pixel;
        r *= scale;
        g *= scale;
        b *= scale;

        // Apply the linear to gamma transform.
        r = linear_to_gamma(r);
        g = linear_to_gamma(g);
        b = linear_to_gamma(b);
        static const interval intensity(0.000, 0.999);

        auto sr = static_cast<int>(256 * intensity.clamp(r));
        auto sg = static_cast<int>(256 * intensity.clamp(g));
        auto sb = static_cast<int>(256 * intensity.clamp(b));

        return std::to_string(sr) + ' ' + std::to_string(sg) + ' ' + std::to_string(sb);
    }

private:
     int image_height;    // Rendered image height
     int image_width;     
    point3 center;       // Camera center
    point3 pixel00_loc;  // Location of pixel 0, 0
    vec3 pixel_delta_u;  // Offset to pixel to the right
    vec3 pixel_delta_v;  // Offset to pixel below
    vec3 u, v, w;        // Camera frame basis vectors
    vec3 defocus_disk_u; // Defocus disk horizontal radius
    vec3 defocus_disk_v; // Defocus disk vertical radius

    void initialize()
    {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        center = point3(0, 0, 0);

        center = lookfrom;

        // Determine viewport dimensions.
        // auto focal_length = (lookfrom - lookat).length();
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;   // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v; // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        // auto viewport_upper_left = center - (focal_length * w) - viewport_u/2 - viewport_v/2;
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    color ray_color(const ray &r, int depth, const hittable &world) const
    {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0, 0, 0);

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec))
        {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
            return color(0, 0, 0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }

    ray get_ray(int i, int j) const
    {
        // Get a randomly sampled camera ray for the pixel at location i,j.

        // Get a randomly-sampled camera ray for the pixel at location i,j, originating from
        // the camera defocus disk

        auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
        auto pixel_sample = pixel_center + pixel_sample_square();

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();

        // auto ray_origin = center;
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    point3 defocus_disk_sample() const
    {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    vec3 pixel_sample_square() const
    {
        // Returns a random point in the square surrounding a pixel at the origin.
        auto px = -0.5 + random_double();
        auto py = -0.5 + random_double();
        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }
};

#endif