# 光栅化和MSAA超采样技术

## 首先要对源码进行一定的阅读理解

## 额外注意图形学编程中float和int的问题

整个光栅化的过程大概就是

1. 在真实空间中定义一个三角形 

```C++
    std::vector<Eigen::Vector3f> pos //位置
    std::vector<Eigen::Vector3i> ind //id
    std::vector<Eigen::Vector3f> cols //colors
```

2. 帧缓存和深度缓存清空 设置MVP矩阵

```C++ 
r.clear(rst::Buffers::Color | rst::Buffers::Depth);

r.set_model(get_model_matrix(angle));
r.set_view(get_view_matrix(eye_pos));
r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

```

3. draw traiangle
```C++

    r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle); //绘制三角形？？

```

4. draw traiangle 的内部实现 先进性视口变化，在进行光栅化三角形



因此解决这个问题的办法就是把每个像素的四个样本的深度和颜色都记录下来，说白了就是直接将其看作四倍的图像进行处理，然后再进行着色。

在之前的基础上需要添加的数据结构和代码

```C++
    std::vector<Eigen::Vector3f> frame_buf;
    std::vector<Eigen::Vector3f> super_frame_buf;

    std::vector<float> depth_buf;
    std::vector<float> super_depth_buf;

    int rst::rasterizer::get_super_index(int x, int y)
    void rst::rasterizer::clear(rst::Buffers buff)
    rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
```