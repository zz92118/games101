#pragma once
#include "Scene.hpp"

struct hit_payload//���е�����
{
    float tNear;
    uint32_t index;//����һ���������ཻ
    Vector2f uv;
    Object* hit_obj;
};

class Renderer
{
public:
    void Render(const Scene& scene);

private:
};