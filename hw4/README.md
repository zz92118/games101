# hw4 贝塞尔曲线
进行了两种实现

* 递归实现形式
* 迭代实现形式 （参考了《The NURBS BOOK》的开源代码）

```C++

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    // 迭代形式 公式递推
    std::vector<cv::Point2f> Q = control_points;
    int n = control_points.size();
    //std::cout << Q[0] << std::endl;
    for (int k = 1; k < n; k++)
    {
        for (int i = 0; i < n - k; i++)
        {
            Q[i] = (1.0 - t) * Q[i] + t * Q[i + 1];
        }
    }
    //std::cout << Q[0] << std::endl;
    return Q[0];
}

cv::Point2f recursive_bezier2(const std::vector<cv::Point2f>& control_points, float t)
{
    // TODO: Implement de Casteljau's algorithm
    // 递归形式 直观的通过算法的思想进行计算
    int n = control_points.size();
    if (n == 1)// 递归出口
    {
        return control_points[0];
    }
    std::vector<cv::Point2f> temp_points;
    for (int i = 1; i < n; i++)
    {
        temp_points.push_back(control_points[i - 1] * (1.0 - t) + control_points[i] * t);
    }
    return recursive_bezier2(temp_points, t);//往下一层
}

```

<div align=center> <image src="bezeir.webp"  width="50%" height="50%"> </div>

## 反走样

根据到像素中心的距离来考虑与它相邻的像素的颜色。

* 想法1：设置一个4邻域，简单的使用距离平方反比进行填充颜色

* 想法2：[最近距离比](https://zhuanlan.zhihu.com/p/474776825) 参考的代码放在这了