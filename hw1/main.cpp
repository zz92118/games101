#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

//�� main.cpp �У�����ģ����ͼ�ι��ߡ��������ȶ����˹�դ�������ʵ����Ȼ�����������Ҫ�ı�����Ȼ�����ǵõ�һ���������������Ӳ����������(�벻 Ҫ�޸���)��
// 
// ���������ϣ����Ƕ����������ֱ����ģ�͡���ͼ��ͶӰ����ĺ�����
//    ÿһ���������᷵����Ӧ�ľ��󡣽��ţ������������ķ���ֵ�ᱻ set_model(),
//    set_view() �� set_projection() �������������դ�����С���󣬹�դ������
//    ��Ļ����ʾ���任�Ľ����
constexpr double MY_PI = 3.1415926;
inline float GetRadValue(double deg) { return deg * MY_PI / 180; } //30 --- 1/6 pi


Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 
        0, 1, 0, -eye_pos[1], 
        0, 0, 1,-eye_pos[2],
        0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    float rad = float(rotation_angle);

    model << cos(rad), -sin(rad), 0, 0,
        sin(rad), cos(rad), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;


    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    // eye_fov field-of-view
    //float top = -tan(GetRadValue(eye_fov / 2.0f) * abs(zNear));
    //float right = top * aspect_ratio;
    //float left = -right;
    //float button = -top;

    //projection << 2*zNear / right-left, 0, (left+right)/(left-right), 0,
    //    0, 2*zNear / top-button, (button+top)/(button-top), 0,
    //    0, 0, (zNear + zFar) / (zNear - zFar), (2 * zNear * zFar) / (zFar - zNear),
    //    0, 0, 1, 0;

    //projection << zNear / right, 0, (left + right) / (left - right), 0,
    //    0,  zNear / 0, (button + top) / (button - top), 0,
    //    0, 0, (zNear + zFar) / (zNear - zFar), (2 * zNear * zFar) / (zFar - zNear),
    //    0, 0, 1, 0;

    float top = -tan(GetRadValue(eye_fov / 2.0f) * abs(zNear));
    float right = top * aspect_ratio;
    float left = -right;
    float button = -top;

    projection << 2*zNear / (right-left), 0, (left + right) / (left - right), 0,
        0, 2*zNear / (top-button), (button + top) / (button - top), 0,
        0, 0, (zNear + zFar) / (zNear - zFar), (2 * zNear * zFar) / (zFar - zNear),
        0, 0, 1, 0;

    return projection;
    //return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));
        //�õ������������������ռ�
        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}