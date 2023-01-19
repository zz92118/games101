# hw7 蒙特卡洛光线追踪

```C++
// 每个shaddingpoint打一根光线 防止指数爆炸
// 但每个pixel进行多次采样 采样的变量为 render.cpp 中的spp
Vector3f Scene::castRay(const Ray& ray, int depth) const
{
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
    //定义直接光照和简介光照 并且在光源区域采样
    Vector3f L_dir, L_indir;
    Intersection light_intersection;
    float light_pdf;
    sampleLight(light_intersection, light_pdf);// 在光源区域进行采样 打一根光线
    Ray sample_ray(p, ws);
    Intersection check = intersect(sample_ray);//用于判断是直接光照还是间接光照

    // 2, 计算直接光照
    if ((check.distance - distance) > -EPSILON) // 直接光照
    {
        //m->eval 为brdf函数
        L_dir = emit * m->eval(wo, ws, N) * dotProduct(N, ws) * dotProduct(-ws, NN) * pow(distance, 2) / light_pdf;
    }
    // 3, 计算简介光照 使用RR算法
    float random_pro = get_random_float();
    if (random_pro < RussianRoulette)
    {
        return L_dir;// 无法继续打光线
    }

    // 在材质的表面进行采样 sample一根反射光线
    Vector3f wi = (m->sample(wo, N)).normalized();
    Ray reflect_ray(p, wi);
    Intersection i = intersect(reflect_ray);// 简介光照打到的位置
    if (i.happened && !i.m->hasEmission())// 与不是光源的物体相交（获得反射光）
    {
        L_indir = castRay(reflect_ray, depth + 1) * m->eval(wo, wi, N) * dotProduct(wi, N) / m->pdf(wo, wi, N) / RussianRoulette;
    }
    //4. 返回直接光照与间接光照 求和
    return L_dir + L_indir;

}

```

