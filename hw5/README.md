# hw5 Whitted Style 光线追踪


```C++
//渲染过程 render主函数
    vector3f eye_pos  ;// 视点位置
    for (int j = 0; j < scene.height; ++j)
    {
        for (int i = 0; i < scene.width; ++i) 
        { 
            x = (2 * ((i + 0.5) / scene.width) - 1) * scale * imageAspectRatio;
            y = (1 - 2 * ((j + 0.5) / scene.height)) * scale;
            Vector3f dir = Vector3f(x, y, -1) - eye_pos; 
            castRay(eye_pos, dir, scene, 0);//从视点位置投射出一根光线
            {
                 // castRay函数 从着色点向场景中投射出一根反射光线，通过非尔涅定律进行着色计算
                 // 1.trace一根光线，并求交点，获取交点信息
                  trace(eye_pos, dir, get_objects()))//中间涉及到求交操作
                  getSurfaceProperties()
                 // 2.基于物体材质，使用非尔涅定律进行着色计算 递归光线追踪
                 case(REFLECTION_AND_REFRACTION)
                 {
                 }
                 case(REFLECTION)
                 {
                 }
                 defalut//光源
                 {
                 }
                 
            }
        }
    }
```