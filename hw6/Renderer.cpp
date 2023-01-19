//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = std::tan(deg2rad(scene.fov * 0.5f));
    float imageAspectRatio = scene.width / (float)scene.height;

    // Use this variable as the eye position to start your rays.
    Vector3f eye_pos(-1,10,10);
    int m = 0;
    //(i,j) ---> (x,y) 从屏幕坐标获得世界坐标
    //想象在z=-1处有一个平面
    // ①横纵方向的长度并不是1:1的；

    //②物体本身并不是在[-1, 1]的，[-1, 1]是缩放后的范围。
    // 
    // fov  原本图像屏幕高度与图像到X-Y平面距离的比值
    // 
    // ��Ļ����[width,height]
    // ��׼������[-1,1] (2 * ((i + 0.5) / scene.width) - 1) �� (1 - 2 * ((j + 0.5) / scene.height))
    // �������� �ӽǱ���fov������
    for (int j = 0; j < scene.height; ++j)
    {
        for (int i = 0; i < scene.width; ++i)
        {
            // generate primary ray direction
            float x = 0;
            float y = 0;
            // TODO: Find the x and y positions of the current pixel to get the direction
            // vector that passes through it.
            // Also, don't forget to multiply both of them with the variable *scale*, and
            // x (horizontal) variable with the *imageAspectRatio*            

            x = (2 * ((i + 0.5) / scene.width) - 1) * scale * imageAspectRatio;
            y = (1 - 2 * ((j + 0.5) / scene.height)) * scale;

            //std::cout << x << " " << y << std::endl;
            Vector3f dir = Vector3f(x, y, -1) - eye_pos; // Don't forget to normalize this direction!
            dir = normalize(dir);
            auto ray = Ray(eye_pos, dir, 0.0);
            framebuffer[m++] = scene.castRay(ray, 0);
        }
        UpdateProgress(j / (float)scene.height);//��������
    }

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {///写入文件
        static unsigned char color[3];
        color[0] = (char)(255 * clamp(0, 1, framebuffer[i].x));
        color[1] = (char)(255 * clamp(0, 1, framebuffer[i].y));
        color[2] = (char)(255 * clamp(0, 1, framebuffer[i].z));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}
