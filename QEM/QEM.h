#pragma once
#include <OpenMesh\Core\IO\MeshIO.hh>
#include <OpenMesh\Core\Mesh\PolyMesh_ArrayKernelT.hh>
#include <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>
#include <vector>
#include <queue>
#include <unordered_set>
//#include <priority_queue>
//#include <pair>

#include <set>
#include <ctime>
#include <string>
#include <Eigen\Dense>
#include <Eigen\Core>
using namespace std;
using namespace Eigen;
using namespace OpenMesh;

struct MyTraits : public OpenMesh::DefaultTraits
{
	typedef Matrix<float, 4, 4> FaceK;//ÿ��face��Ӧ��һ����������
	typedef Matrix<float, 4, 4> VertexQ;//ÿ��face��Ӧ��һ����������
	typedef vector<VertexHandle> vector_pairs;//ÿ��vertex��Ӧһ��vector �����뵱ǰ���㹹�ɵ�pairs��vertex
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> MyMesh;

struct my_set_element
{
	float cost;
	OpenMesh::VertexHandle vh1;
	OpenMesh::VertexHandle vh2;
	OpenMesh::Vec3f v;

	my_set_element(float cost_, OpenMesh::VertexHandle vh1_, OpenMesh::VertexHandle vh2_, OpenMesh::Vec3f v_) :vh1(vh1_), vh2(vh2_), cost(cost_), v(v_) {}
	my_set_element() :cost(-1.0), vh1(-1), vh2(-1), v() {}

};

struct Cmp
{
	bool operator()(const my_set_element& x, const my_set_element& y) const
	{
		return x.cost > y.cost;
	}
};



class QEM
{
public:


	QEM(string file_path,string save_file_path,float r);
	void Simplify();
	void loadMesh();
	void saveMesh();
	void initQ();
	void calculateK();
	void basicOpenMesh();
	void initPairs();
	void calNewVertex();
	void pairContraction(my_set_element pair);
	my_set_element updateQAndV(const MyMesh::VertexHandle vh1, const MyMesh::VertexHandle vh2);


private:
	OpenMesh::IO::Options opt;
	OpenMesh::FPropHandleT< Eigen::Matrix<float, 4, 4> > face_k_prop;//face���Ե�handle
	OpenMesh::VPropHandleT< Eigen::Matrix<float, 4, 4> > vertex_q_prop;//�������Ե�handle
	OpenMesh::VPropHandleT< vector<VertexHandle> > vertex_pairs_prop;//vertex���Ե�handle

	MyMesh mesh;
	string file_path;
	string save_file_path;

	float parameter_t = 0.0f;
	float simpilification_ratio = 0.0f;
	int all_faces = 0; //��¼���е�faces ���� ��Ϊֹͣ�������ж�

	priority_queue<my_set_element,vector<my_set_element>, Cmp> Costs; //���ȶ���
	std::set<MyMesh::VertexHandle> deletePoints;//�����ɾ������


};

