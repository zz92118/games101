# hw7 ���ؿ������׷��

```C++
// ÿ��shaddingpoint��һ������ ��ָֹ����ը
// ��ÿ��pixel���ж�β��� �����ı���Ϊ render.cpp �е�spp
Vector3f Scene::castRay(const Ray& ray, int depth) const
{
    Intersection point_intersection = intersect(ray); //��ó���������Ľ��� object�󽻵�
    // 1��special case
    //���û�д����壬�򷵻ر�����ɫ
    //������˹�Դ����Դ��һ�������object����ֱ�ӷ��ع�Դ��ɫ
    if (point_intersection.m->hasEmission())
    {
        return point_intersection.m->getEmission();
    }
    if (!point_intersection.happened)
    {
        return Vector3f();
    } 
    //����ֱ�ӹ��պͼ����� �����ڹ�Դ�������
    Vector3f L_dir, L_indir;
    Intersection light_intersection;
    float light_pdf;
    sampleLight(light_intersection, light_pdf);// �ڹ�Դ������в��� ��һ������
    Ray sample_ray(p, ws);
    Intersection check = intersect(sample_ray);//�����ж���ֱ�ӹ��ջ��Ǽ�ӹ���

    // 2, ����ֱ�ӹ���
    if ((check.distance - distance) > -EPSILON) // ֱ�ӹ���
    {
        //m->eval Ϊbrdf����
        L_dir = emit * m->eval(wo, ws, N) * dotProduct(N, ws) * dotProduct(-ws, NN) * pow(distance, 2) / light_pdf;
    }
    // 3, ��������� ʹ��RR�㷨
    float random_pro = get_random_float();
    if (random_pro < RussianRoulette)
    {
        return L_dir;// �޷����������
    }

    // �ڲ��ʵı�����в��� sampleһ���������
    Vector3f wi = (m->sample(wo, N)).normalized();
    Ray reflect_ray(p, wi);
    Intersection i = intersect(reflect_ray);// �����մ򵽵�λ��
    if (i.happened && !i.m->hasEmission())// �벻�ǹ�Դ�������ཻ����÷���⣩
    {
        L_indir = castRay(reflect_ray, depth + 1) * m->eval(wo, wi, N) * dotProduct(wi, N) / m->pdf(wo, wi, N) / RussianRoulette;
    }
    //4. ����ֱ�ӹ������ӹ��� ���
    return L_dir + L_indir;

}

```

