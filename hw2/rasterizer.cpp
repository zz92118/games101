// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    // id --> positions 向量的映射
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]

    //向量叉积检测三角形内外算法

    //象素里的点的z坐标应该是0，但判断的三角形是有Z坐标的 这怎么办呢？？ 这里测试了一下把Z都改成0了
    Vector3f point_a = _v[0];
    Vector3f point_b = _v[1];
    Vector3f point_c = _v[2];

    //point_a[2] = 0.0;
    //point_b[2] = 0.0;
    //point_c[2] = 0.0;

    Vector3f vecotr_ab = point_b - point_a;
    Vector3f vecotr_ac = point_c - point_a;
    Vector3f vecotr_bc = point_c - point_b;

    Vector3f point_p = Vector3f( float(x + 0.5), float(y + 0.5), 0.0f);

    Vector3f vector_ap = point_p - point_a;
    Vector3f vector_bp = point_p - point_b;
    Vector3f vector_cp = point_p - point_c;
    // ab x ap ,ca x cp , bc x bp

    Vector3f test_1 = vector_ap.cross(vecotr_ab);
    Vector3f test_2 = vector_bp.cross(vecotr_bc);
    Vector3f test_3 = vector_cp.cross(-vecotr_ac);

    if ((test_1.z() > 0 && test_2.z() > 0 && test_3.z() > 0) || (test_1.z() < 0 && test_2.z() < 0 && test_3.z() < 0))
    {
        return true;
    }

    else
    {
        return false;
    }



}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id]; //得到的是结构体
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model; // 投影之后的 在视见体内部了
    for (auto& i : ind) //最后画出两个三角形
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation 
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t); //经过了视口变换
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4(); //齐次坐标表示
    // TODO : Find out the bounding box of current triangle.

    //四舍五入 最后+1就行

    float offset[4][2] =
    {
        {0.25,0.25},
        {0.75,0.25},
        {0.75,0.25},
        {0.75,0.75}
    };

    int x_min = std::min(std::min(v[0].x(), v[1].x()), v[2].x());
    int x_max = std::max(std::max(v[0].x(), v[1].x()), v[2].x()) + 1;
    int y_min = std::min(std::min(v[0].y(), v[1].y()), v[2].y());
    int y_max = std::max(std::max(v[0].y(), v[1].y()), v[2].y()) + 1;

    // iterate through the pixel and find if the current pixel is inside the triangle

    int i, j, k;
    for (i = x_min; i < x_max; i++)
    {
        for (j = y_min; j < y_max; j++)
        {
            //image [i][j] = inside (Triangle,xor+0.5,y0+0.5);
            // 先划分在判断
            int count = 0;

            for (k = 0; k < 4; k++)
            {
                // 启用MSAA算法
                float x_new = float(i) + offset[k][0];
                float y_new = float(j) + offset[k][1];
                if (insideTriangle(x_new, y_new, t.v)) // sub pix 的三角形内外测试
                {
                    count++;
                        auto tup = computeBarycentric2D(x_new, y_new, t.v);
                        // 在此基础上实现超采样 原本x+0.5 y+0.5 ---> x+0.25,y+0.25
                        float alpha;
                        float beta;
                        float gamma;
                        std::tie(alpha, beta, gamma) = tup;
                        float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                        float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                        z_interpolated *= w_reciprocal;

                        //深度测试
                        int super_buf_index = msaa_get_index(i * 2 + k % 2 , j * 2 + k / 2); //有一个两倍的关系

                        if (msaa_depth_buf[super_buf_index] > z_interpolated)
                        {
                            //count++;
                            msaa_depth_buf[super_buf_index] = z_interpolated; //更新深度缓存
                            msaa_frame_buf[super_buf_index] = t.getColor(); //更新frambuffer 超采杨得到的帧缓存需要额外存储空间

                        }
                }
                // 着色 还是对单个点的着色 这里单个点的color是平均
                    
            }
            
            //if ( (count != 4) && (count != 0))
            //{
            //    std::cout << count;
            //}

            if (count > 0)
            {
                Vector3f point = Vector3f(float(i), float(j), 0);

                Vector3f color = (msaa_frame_buf[msaa_get_index(i * 2, j * 2)] + msaa_frame_buf[msaa_get_index(i * 2 + 1, j * 2)] + msaa_frame_buf[msaa_get_index(i * 2, j * 2 + 1)] + msaa_frame_buf[msaa_get_index(i * 2 + 1, j * 2 + 1)]) / 4.0f;

                //Vector3f color = t.getColor()/(float(count/4)); //写死的颜色代码 不涉及着色中的插值

                set_pixel(point, color);

            }
            // 不采用MSAA算法 
            //if (insideTriangle(i + 0.5, j + 0.5, t.v)) // 在三角形内部 根据zbuffer着色 不在一个平面内部啊？
            //{
            //    auto tup = computeBarycentric2D(i+0.5, j+0.5, t.v);
            //    // 在此基础上实现超采样 原本x+0.5 y+0.5 ---> x+0.25,y+0.25
            //    float alpha;
            //    float beta;
            //    float gamma;
            //    std::tie(alpha, beta, gamma) = tup;

            //    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            //    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            //    z_interpolated *= w_reciprocal;

            //    //zbuffer 深度测试进行着色

            //    int buf_index = get_index(i, j);
            //    if (depth_buf[buf_index] > z_interpolated) //depth_buf 表示当前画面上该位置的深度值 depth_buf[buf_index]越大 说明 i j更近，需要覆盖更新
            //    {
            //        depth_buf[buf_index] = z_interpolated;
            //        Vector3f point = Vector3f(float(i), float(j), z_interpolated );
            //        Vector3f color = t.getColor(); //写死的颜色代码 不涉及着色中的插值

                    //set_pixel(point, color);//着色 
            //    }

            //}
        }
    }

    // If so, use the following code to get the interpolated z value. 
    // 插值Z-value 进行zbuffer深度缓存的更新




    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{ 0, 0, 0 });
        std::fill(msaa_frame_buf.begin(), msaa_frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity()); //初始化zbuffer为正无穷
        std::fill(msaa_depth_buf.begin(), msaa_depth_buf.end(), std::numeric_limits<float>::infinity()); //初始化zbuffer为正无穷
    }

}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    msaa_frame_buf.resize(w * h * 2 * 2);
    msaa_depth_buf.resize(w * h * 2 * 2);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

int rst::rasterizer::msaa_get_index(int x, int y)
{
    return (height * 2 - 1 - y) * 2 * width + x ;

}


void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x(); //一个局部变量
    frame_buf[ind] = color;

}

// clang-format on