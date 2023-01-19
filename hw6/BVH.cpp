#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))//这里放的几何图元
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;
    std::cout << primitives.size() << std::endl;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

//这里划分的方式是每个BVHnode中只有一个三角形
BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
    {
        //std::cout << objects.size() << std::endl;
        bounds = Union(bounds, objects[i]->getBounds());
    }
    // 递归终止条件
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds; // 中心区域
        for (int i = 0; i < objects.size(); ++i)
        {
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        }
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }
        // 递归构造
        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)// 可以看作一种保护机制
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

//这里要判断是否和包围盒求交点 在bound3中的IntersectP 函数 
// 先判断和包围盒求交点 在判断和三角形求交点

//Intersect(Ray ray, BVH node) {
//    if (ray misses node.bbox) return;
// 
//    if (node is a leaf node)
//        test intersection with all objs;
//    return closest intersection;
// 
//    hit1 = Intersect(ray, node.child1); hit2 = Intersect(ray, node.child2);
//    return the closer of hit1, hit2;
//}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    // 求出 invDir 和 DirIsNeg 变量传递给IntersectionP
    //invDir = (1.0 / x, 1.0 / y, 1.0 / z), use this because Multiply is faster that Division
    //dirIsNeg = [int(x > 0), int(y > 0), int(z > 0)], use this to simplify your logic
    std::array<int, 3> dirIsNeg;//判断某个分量是否为负数
    dirIsNeg[0] = ray.direction[0] < 0;
    dirIsNeg[1] = ray.direction[1] < 0;
    dirIsNeg[2] = ray.direction[2] < 0;
    Intersection result;
    if (! (node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg)))//判断与bounding box的交点
    {
        return result;
    }

    //if((getIntersection(node),ray).)
    // 只在叶子节点
    if (node->left == nullptr && node->right == nullptr)
    {
        return node->object->getIntersection(ray);// 在判断与bounding box相交之后在判断是否与物体相交
        //return result;
    }

    Intersection inter_left = getIntersection(node->left, ray);
    Intersection inter_right = getIntersection(node->right, ray);
    return inter_left.distance < inter_right.distance ? inter_left : inter_right;
}