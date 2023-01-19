
# hw6 BVH���ٽṹ

```C++ 

Ray �����ݽṹ 
struct Ray{
    //Destination = origin + t*direction
    Vector3f origin;
    Vector3f direction, direction_inv;
    double t;//transportation time,
    double t_min, t_max;
};


BVH�ṹ�������

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    Intersection result;
    //���û�к�BVH�󽻣���һ������������󽻣�ֱ�ӷ��ؼ��ټ�����
    if (! (node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg)))//�ж���bounding box�Ľ���
    {
        return result;
    }
    // ֻ��Ҷ�ӽڵ�洢����
    if (node->left == nullptr && node->right == nullptr)
    {
        return node->object->getIntersection(ray);// ���ж���bounding box�ཻ֮�����ж��Ƿ��������ཻ
    }
    // �����������
    Intersection inter_left = getIntersection(node->left, ray);
    Intersection inter_right = getIntersection(node->right, ray);
    return inter_left.distance < inter_right.distance ? inter_left : inter_right;
}



```

## Obejct

Object��

Triangle �� TriangleMesh���̳�������

TriangleMesh�У�ͨ��BVHAccel::BVHAccel �е� primitives ���������ͼԪ�������Σ���node�д洢����Triangle