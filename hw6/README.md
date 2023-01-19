
# hw6 BVH加速结构

```C++ 

Ray 的数据结构 
struct Ray{
    //Destination = origin + t*direction
    Vector3f origin;
    Vector3f direction, direction_inv;
    double t;//transportation time,
    double t_min, t_max;
};


BVH结构与光线求交

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    Intersection result;
    //如果没有和BVH求交，则一定不会和物体求交，直接返回减少计算量
    if (! (node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg)))//判断与bounding box的交点
    {
        return result;
    }
    // 只在叶子节点存储物体
    if (node->left == nullptr && node->right == nullptr)
    {
        return node->object->getIntersection(ray);// 在判断与bounding box相交之后在判断是否与物体相交
    }
    // 后序遍历过程
    Intersection inter_left = getIntersection(node->left, ray);
    Intersection inter_right = getIntersection(node->right, ray);
    return inter_left.distance < inter_right.distance ? inter_left : inter_right;
}



```

## Obejct

Object类

Triangle 和 TriangleMesh都继承自这里

TriangleMesh中，通过BVHAccel::BVHAccel 中的 primitives ，获得所有图元（三角形），node中存储的是Triangle