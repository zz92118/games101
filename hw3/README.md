
作业三常见问题集合：

(1) bump mapping 部分的 h(u,v)=texture_color(u,v).norm, 其中 u,v 是 tex_coords, w,h 是 texture 的宽度与高度

(2) rasterizer.cpp 中 v = t.toVector4()

(3) get_projection_matrix 中的 eye_fov 应该被转化为弧度制

(4) bump 与 displacement 中修改后的 normal 仍需要 normalize

(5) 可能用到的 eigen 方法：norm(), normalized(), cwiseProduct()

(6) 实现 h(u+1/w,v) 的时候要写成 h(u+1.0/w,v)

(7) 正规的凹凸纹理应该是只有一维参量的灰度图，而本课程为了框架使用的简便性而使用了一张 RGB 图作为凹凸纹理的贴图，因此需要指定一种规则将彩色投影到灰度，而我只是「恰好」选择了 norm 而已。为了确保你们的结果与我一致，我才要求你们都使用 norm 作为计算方法。

(8) bump mapping & displacement mapping 的计算的推导日后将会在光线追踪部分详细介绍，目前请按照注释实现。

# 代码框架



需要改动的地方：
rasterizer.cpp中的void rst::rasterizer::rasterize_triangle(const Triangle& t, const std::array<Eigen::Vector3f, 3>& view_pos)函数，开始执行光栅化过程


## 渲染流水线

主渲染流水线开始于 rasterizer::draw(std::vector<Triangle> &TriangleList).

* MVP 变换获得相机视角的顶点位置 同时单独计算法线
* 视口变换获得屏幕上的坐标
* 逐像素光栅化深度测试
    1. 获取bounding box
    2. 通过zbuffer测试着色
        * 重心坐标计算
        * 纹理 颜色 法向量 viewpoint进行插值
        *通过fragement shader payload进行逐像素渲染

```C++

// draw函数中单独计算的法线（normal不具备齐次变换的性质）

Eigen::Matrix4f inv_trans = (view * model).inverse().transpose();//单独计算法线
Eigen::Vector4f n[] = {
        inv_trans * to_vec4(t->normal[0], 0.0f),
        inv_trans * to_vec4(t->normal[1], 0.0f),
        inv_trans * to_vec4(t->normal[2], 0.0f)
};
```


## rasterizer  光栅化 渲染

//1。先找到bounding box
 // 2.逐像素处理
 //3.获得中心坐标
 //3 zbuffer 深度测试进行着色
 //着色器渲染得到每个fragment的color 和上一个作业不同的是，这里需要获得更多的属性，传递给fragment shader payload进行渲染
```C++
//rasterizer.cpp

// 作业2 逐像素做深度测试 不同的是这里需要通过插值的方法获得
// auto interpolated_color
// auto interpolated_normal
// auto interpolated_texcoords
// auto interpolated_shadingcoords
//1。先找到bounding box
    for (i = x_min; i < x_max; i++)
    {
        for (j = y_min; j < y_max; j++)
        {
            // 2.逐像素处理 
            if (insideTriangle(i + 0.5, j + 0.5, t.v)) // 在三角形内部 根据zbuffer着色 不在一个平面内部啊？
            {
                //3.获得中心坐标
                auto tup = computeBarycentric2D(i + 0.5, j + 0.5, t.v);
                float alpha;
                float beta;
                float gamma;
                std::tie(alpha, beta, gamma) = tup;
                //    * v[i].w() is the vertex view space depth value z.
                //    * Z is interpolated view space depth for the current pixel
                //    * zp is depth between zNear and zFar, used for z-buffer
                float Z = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float zp = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                zp *= Z;
                //3 zbuffer 深度测试进行着色
                int buf_index = get_index(i, j);
                if (depth_buf[buf_index] > zp) //depth_buf 表示当前画面上该位置的深度值 depth_buf[buf_index]越大 说明 i j更近，需要覆盖更新
                {
                    depth_buf[buf_index] = zp;
                    //Vector3f point = Vector3f(float(i), float(j), z_interpolated );
                    // TODO: Interpolate the attributes:
                    // auto interpolated_color
                    // auto interpolated_normal
                    // auto interpolated_texcoords
                    // auto interpolated_shadingcoords
                    auto interpolated_color = interpolate(alpha, beta, gamma, t.color[0], t.color[1], t.color[2], 1);
                    auto interpolated_normal = interpolate(alpha, beta, gamma, t.normal[0], t.normal[1], t.normal[2], 1);
                    auto interpolated_texcoords = interpolate(alpha, beta, gamma, t.tex_coords[0], t.tex_coords[1], t.tex_coords[2], 1);//二维纹理坐标
                    auto interpolated_viewpos = interpolate(alpha, beta, gamma, view_pos[0], view_pos[1], view_pos[2], 1);
                    //着色器渲染得到每个fragment的color 和上一个作业不同的是，这里需要获得更多的属性，传递给fragment shader payload进行渲染
                    fragment_shader_payload payload(interpolated_color, interpolated_normal.normalized(), interpolated_texcoords, texture ? &*texture : nullptr);
                    payload.view_pos = interpolated_viewpos;
                    auto pixel_color = fragment_shader(payload);
                    //Eigen::Vector3f result;
                    //result << 255, 0, 0;
                    set_pixel(Vector2i(i, j), pixel_color);
                }
                //}
            }
        }
    }



```





## Shader

Phong shader

```C++
Eigen::Vector3f phong_fragment_shader(const fragment_shader_payload& payload)
{
    //ka kd ks是物体的材料属性 都是一个vector分量
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{ {20, 20, 20}, {500, 500, 500} }; //初始化position和idensity
    auto l2 = light{ {-20, 20, 0}, {500, 500, 500} };

    std::vector<light> lights = { l1, l2 };
    Eigen::Vector3f amb_light_intensity{ 10.0, 10.0, 10.0 };
    Eigen::Vector3f eye_pos{ 0, 0, 10 };

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal.normalized();

    Eigen::Vector3f result_color = { 0, 0, 0 };
    Eigen::Vector3f ambient = Eigen::Vector3f(0, 0, 0);

    for (auto& light : lights)//计算所有光源
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f diffuse = Eigen::Vector3f(0, 0, 0);
        Eigen::Vector3f specular = Eigen::Vector3f(0, 0, 0);
        float r_squared = (point - light.position).squaredNorm();
        Eigen::Vector3f vector_l = (light.position - point).normalized();
        Eigen::Vector3f vector_v = (eye_pos - point).normalized();
        Eigen::Vector3f vector_h = (vector_v + vector_l).normalized();
        diffuse = kd.cwiseProduct(light.intensity / r_squared) * std::max(0.0f, normal.dot(vector_l));
        specular = ks.cwiseProduct(light.intensity / r_squared) * std::max(0.0f, pow(normal.dot(vector_h), p));
        result_color += (diffuse + specular); //光照模型公式
    }
    // 环境光照只需要加一次
    ambient = ka.cwiseProduct(amb_light_intensity);
    result_color += ambient; 
    return result_color * 255.f;
}
```

displacement shader改变的是视点位置（view point）
