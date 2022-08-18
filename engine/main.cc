//==================================================================================================
// Written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================

#include <iostream>
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "random.h"
#include "triangle.h"

#include "raylib.h"

#include <vector>
#include <ppl.h>
#include <thread>
#include <future>
#include <functional>

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

#include "mono.h"

vec3 color(const ray& r, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
             return attenuation*color(scattered, world, depth+1);
        }
        else {
            return vec3(0,0,0);
        }
    }
    else {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5*(unit_direction.y() + 1.0);
        return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
    }
}


hitable *random_scene() {
    std::vector< hitable*> list;
    list.push_back(new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5))));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = random_double();
            vec3 center(a+0.9*random_double(),0.2,b+0.9*random_double());
            if ((center-vec3(4,0.2,0)).length() > 0.9) {
                if (choose_mat < 0.8) {  // diffuse
                    list.push_back(new sphere(
                        center, 0.2,
                        new lambertian(vec3(random_double()*random_double(),
                                            random_double()*random_double(),
                                            random_double()*random_double()))
                    ));
                }
                else if (choose_mat < 0.95) { // metal
                    list.push_back(new sphere(
                        center, 0.2,
                        new metal(vec3(0.5*(1 + random_double()),
                                       0.5*(1 + random_double()),
                                       0.5*(1 + random_double())),
                                  0.5*random_double())
                    ));
                }
                else {  // glass
                    list.push_back(new sphere(center, 0.2, new dielectric(1.5)));
                }
            }
        }
    }

    //list.push_back(new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5)));
    //list.push_back(new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1))));
    //list.push_back(new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0)));
    const char* modelName = getConfig()->modelName.c_str();
    Model tower = LoadModel(modelName);
    for (int i = 0; i < tower.meshCount; ++i)
    {
        auto mesh = tower.meshes[i];        
        for (int v = 0; v < mesh.vertexCount; )
        {
            vec3 p1(mesh.vertices[v * 3], mesh.vertices[v * 3 + 1], mesh.vertices[v * 3 + 2]);
            v++;
            vec3 p2(mesh.vertices[v * 3], mesh.vertices[v * 3 + 1], mesh.vertices[v * 3 + 2]);
            v++;
            vec3 p3(mesh.vertices[v * 3], mesh.vertices[v * 3 + 1], mesh.vertices[v * 3 + 2]);
            v++;
            p1 *= 10;
            p2 *= 10;
            p3 *= 10;
            list.push_back(new Triangle(p1, p2, p3, new lambertian(vec3(0.4, 0.2, 0.1))));
        }

    }

    return new hitable_list(list, list.size());
}

void initializeScripts()
{
    if (!initialize("../../data/"))
    {
        printf("Error mono can not initialized");
    }
}


int main() {
    const int screenWidth = 1200;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Simple Engine");

    SetTargetFPS(60);

    initializeScripts();
    const int nx = screenWidth;
    const int ny = screenHeight;
    const int ns = getConfig()->sampleCount;
    
    hitable *world = random_scene();

    vec3 lookfrom(9,4,3);
    vec3 lookat(0,1,0);
    float dist_to_focus = 10.0f;
    float aperture = 0.1f;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);

    std::vector<unsigned char> data;
    data.resize(nx * ny * 3);

    std::packaged_task task([&]() {
        concurrency::parallel_for(0, 12 * 8, 1, [&](int d) {
            int xTile = d % 12;
            int yTile = d / 12;

            int nxStart = xTile * 100;
            int nxEnd = nxStart + 100;

            int nyStart = yTile * 100;
            int nyEnd = nyStart + 100;

            for (int j = nyStart; j < nyEnd; j++)
            {
                for (int i = nxStart; i < nxEnd; i++) {
                    vec3 col(0, 0, 0);
                    for (int s = 0; s < ns; s++) {
                        float u = float(i + random_double()) / float(nx);
                        float v = float(j + random_double()) / float(ny);
                        ray r = cam.get_ray(u, v);
                        col += color(r, world, 0);
                    }
                    col /= float(ns);
                    col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
                    int ir = int(255.99 * col[0]);
                    int ig = int(255.99 * col[1]);
                    int ib = int(255.99 * col[2]);
                    int pixelIndex = (i + (ny - j - 1) * nx) * 3;
                    data[pixelIndex] = ir;
                    data[pixelIndex + 1] = ig;
                    data[pixelIndex + 2] = ib;
                }
            }
            });
        });

    std::future<void> result = task.get_future();

    std::thread task_td(std::move(task));



  
    Image img;
    img.width = 1200;
    img.height = 800;
    img.data = data.data();
    img.mipmaps = 1;
    img.format = PixelFormat::PIXELFORMAT_UNCOMPRESSED_R8G8B8;

    Texture2D tex = LoadTextureFromImage(img);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawTexture(tex, 0, 0, WHITE);
        if (result._Is_ready())
            DrawText("Finished", 10, 10, 20, BLACK);
        else
            DrawText("Generating picture...", 10, 10, 20, BLACK);

        UpdateTexture(tex, data.data());

        EndDrawing();
    }

    CloseWindow();

    printf("Generating picture...");
    task_td.join();
}
