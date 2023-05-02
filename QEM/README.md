# QEM ������㷨

<image src="pictures/00.png">


����򻯵�Ŀ�����ڲ�������ʧģ�;�ϸ�ȵ�����£������ܼ���ʹ�õ������θ��������ڲ�ͬ����Ҫ�����ɲ�ͬϸ�ڳ̶ȣ�Level Of Detail��LOD����ģ�͡�

���õ�����򻯲�������

* Vertex Decimation
* Vertex Clustering
* Edge Contraction

Ȼ��������ʹ��Edge Contraction�����һϵ�����⣬���籾�����ںܽ��ĵ�ᱻ��Զ����ʱ���ܻ��γɱ������еĿն���

���ġ�Surface Simplification Using Quadric Error Metrics�������һ�ֻ���Pair Contraction�ķ��������Ժܺõ���������û�бߵ��Ǿ���Ͻ��ĵ㣬�Ӷ�������һ���⣬ʹ��Pair Contraction��Edge Contraction���������£�

<div align=center>
<image src="pictures/01.png">
</div>


�����ж��㷨�����������������£�
<div align=center>
<image src="pictures/02.png"  width="50%" height="50%">
</div>

## ����ʵ��
���㷨�ľ���ʵ���ϣ���Ϊ��Ҫ���浽����м���������ݽṹ����Ʒǳ��ؼ���

���ҵ�ʵ���У�ʹ����OpenMesh�������ߣ�ʹ����***������ݽṹ Half Edge data structure***��ԭʼ������д���

��***OpenGL***��ʾ�У�ʹ��Assimp��������д�����Ϊ���Ĳ��õ�ģ��û��ʹ�õ��������꣬�������Ⱦ��Shader��û���������ӳ�䣬�õ���ͼ�ǵ�һ����ɫ

<div align=center>
<image src="pictures/opengl.png"  width="50%" height="50%">
</div>

ͨ������Զ�������������ÿ��face��Kֵ��ÿ��Vertex��Qֵ�Լ�ÿ�����������ɵ����кϷ�����ԣ�Valid Pairs����



```C++
struct MyTraits : public OpenMesh::DefaultTraits
{
	typedef Matrix<float, 4, 4> FaceK;//ÿ��face��Ӧ��һ����������
	typedef Matrix<float, 4, 4> VertexQ;//ÿ��vertex��Ӧ��һ����������
	typedef vector<VertexHandle> vector_pairs;//ÿ��vertex��Ӧһ��vector �����뵱ǰ���㹹�ɵ�pairs��vertex
};
```


### 1. ���㶥���Qֵ
����һ�α��������̣��������Ƕ���v1,v2������һ������v����С�����µ������������򻯺�ĵ���ԭ������Ӧ�ֲ��ľ����ƽ����

<div align=center>
<image src="pictures/03.png" width="50%" height="50%">
</div>

ʹ�ö�����ʽ���б�ʾ������a,b,c,dΪ����ƽ��P�Ĳ������Զ���Qֵ�ļ���ת��Ϊĳ����������1����ƽ���Kֵ��͡�

<div align=center>
<image src="pictures/04.png" width="50%" height="50%">
</div>

��OpenMesh�У�����ͨ��������ʵ㡢�ߵ����Ե�����1������Ϣ��

```C++
	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++)
		//�����е��1-ring facek�����
		for (MyMesh::VertexFaceIter vf_it = mesh.vf_begin(*v_it); vf_it.is_valid(); vf_it++)

```

### 2.ѡ��valid pairs
��Ч��Ե�ѡ����Ҫ������������֮һ��

* $ (v1-v2)$ ��һ����
* $||v1-v2||<t$

�ڱ���ʵ���У�ʹ����Vertex Handle���ɵ��������洢ÿ�����������valid pairs

```C++
typedef vector<VertexHandle> vector_pairs;//ÿ��vertex��Ӧһ��vector �����뵱ǰ���㹹�ɵ�pairs��vertex
```

### 3.����ÿ���¶���Ե�����λ�ã����õ�cost���������ȶ���

����ÿһ�ԺϷ������(v1,v2)��ͨ�������󷽳̣���ȡ�¶��� $\bar{v} $ �����꣬����Q=Q1+Q2

<div align=center>
<image src="pictures/05.png">
</div>

���� $\bar{v} (Q1+Q2) v $ �õ�ÿ������Ե�Cost����������Լ���һ�����ȶ�����

�洢����Ե����ݽṹΪ��

```C++
priority_queue<my_set_element,vector<my_set_element>, Cmp> Costs; //���ȶ���

```

### 4. ��������

ÿ�����������У�ѡ��Ѷ��Ķ����(v1,v2)�������������Ͻ������������¶����������v1��v2��pairs�������¼���

* $ \bar{v} $ ��Q����ΪQ1+Q2
* �����漰��v1,v2�Ķ���ԣ�ʹ��$ \bar{v} $ ���棬���޸Ķ�Ӧ��Costs���µ� $ \bar{v}_{new} $

�������£�

```C++
void QEM::Simplify()
{
	initQ();
	initPairs();
	calNewVertex();
	while (!Costs.empty() && mesh.n_faces() > simpilification_ratio * all_faces)
	{
		my_set_element temp= Costs.top();
		Costs.pop();
		Costs.pop();
		pairContraction(temp);
	}
}
```

## ʵ����

ʹ��QEM�㷨��bunny.obj�������񻯼򣬼򻯱�������Ϊ100%��ԭͼ����80%��50%��20%���õ���ʵ��������:

<div align=center> <image src="pictures/ԭͼ.png"  width="50%" height="50%"> </div>

<div align=center> <image src="pictures/80.png"  width="50%" height="50%"></div>

<div align=center> <image src="pictures/50.png"  width="50%" height="50%"> </div>

<div align=center>  <image src="pictures/20.png"  width="50%" height="50%"> </div>


���Ĳ���t���۲�򻯱���Ϊ20%��ʵ���������Է��ֲ���t�����ö�����Ļ�����������һ����Ӱ��

<div align=center> <image src="pictures/10.png" > </div>

## ������Ľ�����

* ����ʵ�ֲ�ȡ��C++���õ�priority_queue�ṹ�洢ÿ������Ե�cost��Ȼ����Ҳ��������ÿ�θ��µ����Ĺ����У��漰�����Ҷ���������v1 v2��ض���Ĳ�����ʱ����Ҫ�����������е�Ԫ�أ��������漰���ϴ�����ģ��ʱ�����ٶȺ���������������һ�¸��ӷ���Ĳ��Һ͸��½ṹ��

<div align=center> <image src="pictures/cow.png"  width="50%" height="50%"> </div>


* ��������typedef vector<VertexHandle> vector_pairs���ڴ洢���ж���ԣ�Ҳ�����˴洢�ռ�����࣬�Ӵ��˳���Ŀռ俪����
