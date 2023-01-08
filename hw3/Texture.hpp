//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
public:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
        std::cout << width << std::endl;
        std::cout << height << std::endl;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        //访问越界问题解决方法
        if (u < 0) u = 0;
        if (v < 0) v = 0;
        if (u > 1) u = 1;
        if (v > 1) v = 1;
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        if (u_img <= 0)
        {
            u_img += 1;
        }
        if (v_img <= 0)
        {
            v_img += 1;
        }
        if (u_img >= width)
        {
            u_img -= 1;
        }
        if (v_img >= height)
        {
            v_img -= 1;
        }

        //std::cout << u_img << " " << v_img << std::endl;

        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
