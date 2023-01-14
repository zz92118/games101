#pragma once
#include "Scene.hpp"

struct hit_payload//击中的物体
{
    float tNear;
    uint32_t index;//和哪一个三角形相交
    Vector2f uv;
    Object* hit_obj;
};

class Renderer
{
public:
    void Render(const Scene& scene);

private:
};