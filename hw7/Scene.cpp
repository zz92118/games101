//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0; // 场景中所有光源的面积
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf); // 只计算光源的 在光源上获取一个随机采样点
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}


// Implementation of Path Tracing
// Implementation of Path Tracing
//Vector3f Scene::castRay(const Ray& ray, int depth) const
//{
//    // TO DO Implement Path Tracing Algorithm here
//    Intersection inter = intersect(ray);// 获取相交信息
//    if (!inter.happened) return Vector3f();// 没有相交 直接返回
//    if (inter.m->hasEmission()) return inter.m->getEmission();// 打到光源 直接返回光源颜色
//
//    Vector3f l_dir(0, 0, 0), l_indir(0, 0, 0);// 入射光方向与反方向
//    // 直接光照
//    Intersection lightPos;
//    float lightPdf = 0.0f;
//    sampleLight(lightPos, lightPdf);// 采样得到直接光照的交点与pdf
//    auto objtolight = lightPos.coords - inter.coords, objtolightDir = objtolight.normalized();
//    float dist = objtolight.x * objtolight.x + objtolight.y * objtolight.y + objtolight.z * objtolight.z;
//
//    // 判断是否被遮挡
//    // 光源采样的光线做一次求交 如果距离小于到光源距离 说明遮挡
//    Ray objtolightRay(inter.coords, objtolightDir);
//    Intersection check = intersect(objtolightRay);
//    if (check.distance - objtolight.norm() > -EPSILON)// 浮点数的比较
//    {
//        l_dir = lightPos.emit * inter.m->eval(ray.direction, objtolightDir, inter.normal) * dotProduct(objtolightDir, inter.normal) * dotProduct(-objtolightDir, lightPos.normal) / dist / lightPdf;
//    }
//
//    // 间接光照
//    // 俄罗斯轮盘赌判断是否继续
//    if (get_random_float() > RussianRoulette) return l_dir; // 死了 直接返回直接光照
//    Vector3f objtoobjDir = inter.m->sample(ray.direction, inter.normal).normalized();
//    Ray objtoobjRay(inter.coords, objtoobjDir);
//    Intersection objInter = intersect(objtoobjRay);
//    if (objInter.happened && !objInter.m->hasEmission())// 击中了不是光源的物体
//    {
//        float pdf = inter.m->pdf(ray.direction, objtoobjDir, inter.normal);
//        l_indir = castRay(objtoobjRay, depth + 1) * inter.m->eval(ray.direction, objtoobjDir, inter.normal) * dotProduct(objtoobjDir, inter.normal) / pdf / RussianRoulette;
//    }
//    return l_dir + l_indir;
//}

// Implementation of Path Tracing


//shade(p, wo)
//2 sampleLight(inter, pdf_light)
//3 Get x, ws, NN, emit from inter
//4 Shoot a ray from p to x
//5 If the ray is not blocked in the middle
//6 L_dir = emit * eval(wo, ws, N) * dot(ws, N) * dot(ws, NN) / |x - p | ^ 2 / pdf_light
//7
//8 L_indir = 0.0
//9 Test Russian Roulette with probability RussianRoulette
//10 wi = sample(wo, N)
//11 Trace a ray r(p, wi)
//12 If ray r hit a non - emitting object at q
//13 L_indir = shade(q, wi) * eval(wo, wi, N) * dot(wi, N) / pdf(wo, wi, N) / RussianRoulette
//14
//15 Return L_dir + L_indir

// 每个shaddingpoint打一根光线 防止指数爆炸
// 但每个pixel进行多次采样 采样的变量为 render.cpp 中的spp
Vector3f Scene::castRay(const Ray& ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    std::cout << "22222" << std::endl;
    Intersection point_intersection = intersect(ray); //获得场景和物体的交点 object求交点
    // 1、special case
    //如果没有打到物体，则返回背景颜色
    //如果打到了光源（光源是一种特殊的object），直接返回光源颜色
    if (point_intersection.m->hasEmission())
    {
        return point_intersection.m->getEmission();
    }
    
    if (!point_intersection.happened)
    {
        return Vector3f();
    }

    Vector3f L_dir, L_indir;
    // 从inter获得x, ws, NN, emit 
    Vector3f p = point_intersection.coords;
    Vector3f N = point_intersection.normal;
    Vector3f wo = (ray.origin - p).normalized();
    Material* m = point_intersection.m;
    Intersection light_intersection;
    float light_pdf;
    sampleLight(light_intersection, light_pdf);// 在光源区域进行采样

    Vector3f x = light_intersection.coords;//光源采样点的坐标
    Vector3f NN = light_intersection.normal;//光源采样点的法线
    Vector3f emit = light_intersection.emit;//光源采样点的材质
    Vector3f ws = (x - p).normalized(); // 光源打到point的光线

    float distance = (x - p).norm();// 打到光源的距离 zheli

    Ray sample_ray(p, ws);
    Intersection check = intersect(sample_ray);//用于判断是直接光照还是间接光照
    // 1, 计算直接光照
    if ((check.distance - distance) > -EPSILON) // 直接光照
    {
        //m->eval 为brdf函数
        L_dir = emit * m->eval(wo, ws, N) * dotProduct(N, ws) * dotProduct(-ws, NN) * pow(distance, 2) / light_pdf;
    }
    // 2, 计算简介光照 使用RR算法
    float random_pro = get_random_float();
    if (random_pro < RussianRoulette)
    {
        return L_dir;// 无法继续打光线
    }

    // 在材质的表面进行采样 打一根反射光线
    Vector3f wi = (m->sample(wo, N)).normalized();
    Ray reflect_ray(p, wi);
    Intersection i = intersect(reflect_ray);// 简介光照打到的位置
    if (i.happened && !i.m->hasEmission())// 与不是光源的物体相交（获得反射光）
    {
        L_indir = castRay(reflect_ray, depth + 1) * m->eval(wo, wi, N) * dotProduct(wi, N) / m->pdf(wo, wi, N) / RussianRoulette;
    }
    return L_dir + L_indir;

}
