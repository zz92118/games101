#include "QEM.h"
using namespace std;
using namespace OpenMesh;

QEM::QEM(string file_path,string save_file_path,float r)
{
	this->file_path = file_path;
	this->save_file_path = save_file_path;
	this->simpilification_ratio = r;
}

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

void QEM::pairContraction(my_set_element pair)
{
	MyMesh::VertexHandle v1_handle = pair.vh1;
	MyMesh::VertexHandle v2_handle = pair.vh2;
	float cost = pair.cost;
	MyMesh::Point new_point = pair.v;
	vector<MyMesh::VertexHandle> v1_pairs = mesh.property(vertex_pairs_prop, v1_handle);
	vector<MyMesh::VertexHandle> v2_pairs = mesh.property(vertex_pairs_prop, v2_handle);
	vector<MyMesh::VertexHandle> vnew_pairs = mesh.property(vertex_pairs_prop, v2_handle);
	MyMesh::VertexHandle vnew_handle = mesh.add_vertex(new_point);//����µĶ���
	// ��ɾ��v1 v2����
	deletePoints.insert(v1_handle);
	deletePoints.insert(v2_handle);
	//1. ���������µĵ�
	//new vertex �� Q����
	mesh.property(vertex_q_prop, vnew_handle) = mesh.property(vertex_q_prop, v1_handle) + mesh.property(vertex_q_prop,v2_handle);
	//new vertex ��pairs����
	for (auto it : v1_pairs)
	{
		auto judge = find(vnew_pairs.begin(), vnew_pairs.end(), v2_handle);
		if (judge == vnew_pairs.end())
		{
			vnew_pairs.push_back(it);
		}
	}
	for (auto it : v2_pairs)
	{
		auto judge = find(vnew_pairs.begin(), vnew_pairs.end(), v2_handle);
		if (judge == vnew_pairs.end())
		{
			vnew_pairs.push_back(it);
		}
	}
	mesh.property(vertex_pairs_prop, vnew_handle) = vnew_pairs;
	std::set<MyMesh::FaceHandle> deleteFaces;
	vector<vector<MyMesh::VertexHandle>> addFaces;
	//2.ɾ��v1�����ڵ���
	for (MyMesh::VertexFaceIter vfit = mesh.vf_begin(v1_handle); vfit.is_valid(); vfit++)
	{
		vector<MyMesh::VertexHandle> aface;//�����
		int afnum = 0;
		for (MyMesh::FaceVertexIter vfvit = mesh.fv_begin(*vfit); vfvit.is_valid(); vfvit++)
		{
			if (*vfvit != v2_handle)//���ܽ�V1,V2��ͬ�ڵ���ӽ���
			{
				if (*vfvit == v1_handle)
					aface.push_back(vnew_handle);
				else
					aface.push_back(*vfvit);
				afnum++;
			}
		}
		deleteFaces.insert(*vfit);
		if (afnum == 3)
			addFaces.push_back(aface);
	}
	//2.ɾ��v2�����ڵ���
	for (MyMesh::VertexFaceIter vfit = mesh.vf_begin(v2_handle); vfit.is_valid(); vfit++)
	{
		vector<MyMesh::VertexHandle> aface;//�����
		int afnum = 0;
		for (MyMesh::FaceVertexIter vfvit = mesh.fv_begin(*vfit); vfvit.is_valid(); vfvit++)
		{
			if (*vfvit != v1_handle)//���ܽ�V1,V2��ͬ�ڵ���ӽ���
			{
				if (*vfvit == v2_handle)
					aface.push_back(vnew_handle);
				else
					aface.push_back(*vfvit);
				afnum++;
			}
		}
		deleteFaces.insert(*vfit);
		if (afnum == 3)
			addFaces.push_back(aface);
	}
	//2.ʵ�ʵ�ɾ������
	if (!mesh.has_vertex_status())
		mesh.request_vertex_status();
	if (!mesh.has_face_status())
		mesh.request_face_status();
	if (!mesh.has_edge_status())
		mesh.request_edge_status();
	for (auto i : deleteFaces)
		mesh.delete_face(i, false);
	for (auto i : addFaces)
		mesh.add_face(i);
	mesh.garbage_collection();//ֻɾ���������û��ɾ������
	// 3. ���������ܵ�Ӱ���pairs �������ȶ���
	// ����ע�� v1 v2�Ѿ�ɾ���ˣ�������ɾ������� 
	// ÿ�������v��������Ҫ���µ� ��Ҫ�Ǳ������еĽڵ㣬�ҵ���Ӧ�İ���v1 v2��pairs����ɾ��
	priority_queue<my_set_element, vector<my_set_element>, Cmp> Coststemp; //���ȶ���
	while (!Costs.empty())
	{
		my_set_element temp = Costs.top();
		my_set_element new_temp;
		Costs.pop();
		Costs.pop();
		if ((temp.vh1 == v1_handle) || temp.vh1==v2_handle)
		{
			Coststemp.push(updateQAndV(vnew_handle, temp.vh2));
			Coststemp.push(updateQAndV(vnew_handle, temp.vh2));
		}
		else if (temp.vh2 == v1_handle || temp.vh2 == v2_handle)
		{
			Coststemp.push(updateQAndV(vnew_handle, temp.vh1));
			Coststemp.push(updateQAndV(vnew_handle, temp.vh1));
		}
		else
		{
			Coststemp.push(temp);
			Coststemp.push(temp);
		}
	}
	Costs = Coststemp;
}

my_set_element QEM::updateQAndV(const MyMesh::VertexHandle vh1, const MyMesh::VertexHandle vh2)
{
	Matrix<float, 4, 4> Q1 = mesh.property(vertex_q_prop, vh1) + mesh.property(vertex_q_prop, vh2);
	Matrix<float, 4, 4> tempQ(Q1);
	Q1(3, 0) = 0;
	Q1(3, 1) = 0;
	Q1(3, 2) = 0;
	Q1(3, 3) = 1;
	//�����µ�v
	Eigen::Vector4f B(0, 0, 0, 1);
	Eigen::Vector4f V = Q1.lu().solve(B);
	Eigen::Matrix<float, 1, 4> v(V);
	Eigen::Matrix<float, 4, 1> vt(V);
	//���浱ǰ�����ڵ���Ϣ
	float cost = v * tempQ * vt;
	return my_set_element(cost, vh2, vh1, OpenMesh::Vec3f(V[0], V[1], V[2]));
}




void QEM::initQ()
{
	calculateK();
	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++)
	{
		Matrix<float, 4, 4> temp_q = Matrix<float, 4, 4>::Zero(4, 4);
		//�����е��1-ring facek�����
		for (MyMesh::VertexFaceIter vf_it = mesh.vf_begin(*v_it); vf_it.is_valid(); vf_it++)
		{
			temp_q += mesh.property(face_k_prop, *vf_it);
		}
		mesh.property(vertex_q_prop, *v_it) = temp_q;
		//cout << temp_q << endl;
		//cout << *v_it<<endl;
		//cout << mesh.point(*v_it) << endl;
		//cout << mesh.property(vertex_q_prop, *v_it) << endl<<endl;
	}
}

void QEM::calculateK()
{
	for (MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); f_it++)
	{
		Matrix<float, 4, 4> temp_k;
		vector<Vec3f> faceVertices;

		for (MyMesh::FaceVertexIter fvit = mesh.fv_iter(*f_it); fvit.is_valid(); fvit++)
		{
			faceVertices.push_back(mesh.point(*fvit));
		}
		Vec3f n0 = faceVertices[1] - faceVertices[0];
		Vec3f n1 = faceVertices[2] - faceVertices[0];
		Vector3f x1;
		Vector3f y1;
		x1 << n0[0], n0[1], n0[2];
		y1 << n1[0], n1[1], n1[2];
		Vector3f norm = x1.cross(y1);

		//auto norm = mesh.normal(*f_it);//�÷���ax+by+cz=d��ʾ��ƽ�棬����(a,b,c)�����䷨�ߡ�
		//normalize(norm);
		float a = norm[0];
		float b = norm[1];
		float c = norm[2];
		float d;//d = -(ax+by+cz)

		MyMesh::FaceVertexIter vf_it;

		vf_it = mesh.fv_iter(*f_it); // �õ���ƽ��������һ��
		float x = (mesh.point(*vf_it))[0];
		float y = (mesh.point(*vf_it))[1];
		float z = (mesh.point(*vf_it))[2];

		d = -(a * x + b * y + c * z);
		temp_k << a * a, a* b, a* c, a* d,
			a* b, b* b, b* c, b* d,
			a* c, b* c, c* c, c* d,
			a* d, b* d, c* d, d* d;
		//cout << a << " " << b << " " << c << " " << d << " " << endl;
		mesh.property(face_k_prop, *f_it) = temp_k;
		//cout << "face " << *f_it << endl << mesh.property(face_k_prop, *f_it) << endl; //���� �õ�property
	}
}




void QEM::initPairs()
{
	int all_pairs = 0;
	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++)
	{
		//���1 �б� 
		MyMesh::VertexHandle v1_handle = *v_it;
		MyMesh::Point p1 = mesh.point(*v_it);
		//cout << p1 << endl;
		for (MyMesh::VertexVertexIter vv_it = mesh.vv_iter(*v_it); vv_it.is_valid(); ++vv_it)
		{
			MyMesh::VertexHandle v2_handle = *vv_it;
			mesh.property(vertex_pairs_prop, v1_handle).push_back(v2_handle);
		}
		all_pairs += mesh.property(vertex_pairs_prop, v1_handle).size();
		//cout << mesh.point(*v_it) <<"		" << mesh.property(vertex_pairs_prop, v1_handle).size() << "		";
		//���2 û��edge ��С����ֵ ��Ҫ�������еĶ��� ���︴�Ӷ��е����
		for (MyMesh::VertexIter v_it2 = mesh.vertices_begin(); v_it2 != mesh.vertices_end(); v_it2++)
		{
			MyMesh::VertexHandle v2_handle = *v_it2;
			MyMesh::Point p2 = mesh.point(*v_it2);
			if (v2_handle == v1_handle)
			{
				continue;
			}
			if (norm(p1 - p2) < parameter_t)//����
			{
				vector<VertexHandle> temp_vector = mesh.property(vertex_pairs_prop, v1_handle);
				auto it = find(temp_vector.begin(), temp_vector.end(), v2_handle);
				if (it==temp_vector.end())
				{
					mesh.property(vertex_pairs_prop, v1_handle).push_back(v2_handle);
				}
			}
		}
		//cout << mesh.property(vertex_pairs_prop, v1_handle).size() << endl;
	}
	////���� Ӧ����һ�� ������ϵ bingo
	//cout << all_pairs << endl;
	//cout << mesh.n_edges() << endl;
}

void QEM::calNewVertex()
{
	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++)
	{
		MyMesh::VertexHandle v1_handle = *v_it;
		vector<MyMesh::VertexHandle> v1_pairs=mesh.property(vertex_pairs_prop, v1_handle);
		for (auto  v2_handle : v1_pairs)//v2
		{
			//Q = Q1 + Q2
			//�����Է���
			//�õ��µ�point
			Matrix<float, 4, 4> Q1 = mesh.property(vertex_q_prop, v1_handle);
			Matrix<float, 4, 4> Q2 = mesh.property(vertex_q_prop, v2_handle);
			Matrix<float, 4, 4> Q_solve = Q1 + Q2;
			Matrix<float, 4, 4> Q(Q_solve);
			//cout << "cannot build" << endl;
			//Q_solve<<
			//	Q(1, 1), Q(1, 2), Q(1, 3), Q(1, 4),
			//	Q(1, 2), Q(2, 2), Q(2, 3), Q(2, 4),
			//	Q(1, 3), Q(2, 3), Q(3, 3), Q(3, 4),
			//	0.0f, 0.0f, 0.0f, 1.0f;
			//cout << "can build" << endl;
			Q_solve(3, 0) = 0.0f;
			Q_solve(3, 1) = 0.0f;
			Q_solve(3, 2) = 0.0f;
			Q_solve(3, 3) = 1.0f;
			Vector4f right(0.0f, 0.0f, 0.0f, 1.0f);
			Vector4f v_new = Q_solve.lu().solve(right);// new point 
			MyMesh::Point v_ = OpenMesh::Vec3f(v_new[0], v_new[1], v_new[2]);
			// ����cost
			Matrix<float, 4, 1> v(v_new);
			Matrix<float, 1, 4> v_transpose(v_new);
			float cost = v_transpose * Q * v;
			//�¶�������ӵı� �ǲ���Ҳ��һ�£� ����һ������
			Costs.push(my_set_element(cost, v1_handle, v2_handle, v_));//�����������ߵ� ������������߶���������
		}
	}
	cout << Costs.size() << endl;
	//while (!Costs.empty())
	//{
	//	cout << Costs.top().cost << endl;
	//	Costs.pop();
	//}
}

void QEM::loadMesh()
{
	if (!OpenMesh::IO::read_mesh(mesh, file_path))
	{
		cerr << "ERROR : Cannot read mesh from:" << file_path << endl;
	}
	//Ҫ��ʹ�÷�����
	mesh.request_face_normals();
	mesh.request_vertex_normals();
	mesh.add_property(face_k_prop);
	mesh.add_property(vertex_q_prop);
	mesh.add_property(vertex_pairs_prop);

	if (!opt.check(OpenMesh::IO::Options::VertexNormal) && mesh.has_face_normals() && mesh.has_vertex_normals())
	{
		cout << "calculate vertex" << endl;
		//mesh.release_face_normals();
		mesh.update_normals();
		mesh.update_face_normals();
	}
	// do sth with your mesh
	mesh.property_stats();
	this->all_faces = mesh.n_faces();
	cout << "error point load" << endl;
}

void QEM::saveMesh()
{
	if (!mesh.has_vertex_status())
		mesh.request_vertex_status();
	if (!mesh.has_face_status())
		mesh.request_face_status();
	if (!mesh.has_edge_status())
		mesh.request_edge_status();
	for (auto i : deletePoints)
		mesh.delete_vertex(i);
	mesh.garbage_collection();
	cout << mesh.n_faces()<<" previous" << all_faces << endl;
	if (!OpenMesh::IO::write_mesh(mesh, save_file_path))
	{
		cerr << "Error:Cannot write mesh to: output.obj" << endl;
	}
}

void QEM::basicOpenMesh()
{
	//cout << mesh.numVer/
	cout << "basic usage" << endl;
	cout << mesh.n_vertices() << endl;
	cout << mesh.n_faces() << endl;
	cout << mesh.n_halfedges() << endl;
	cout << "end basic usage" << endl;

	//for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++)
	//{
		//cout << mesh.point(*v_it)<<endl;
	//	cout << mesh.normal(*v_it)<<endl<<endl;
	//}

}

//void QEM::initPairs_falied()
//{
//	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); v_it++)
//	{
//		//���1 �б� 
//		MyMesh::VertexHandle v1_handle = *v_it;
//		MyMesh::Point p1 = mesh.point(*v_it);
//		//cout << p1 << endl;
//		for (MyMesh::VertexVertexIter vv_it = mesh.vv_iter(*v_it); vv_it.is_valid(); ++vv_it)
//		{
//			//auto v2_handle = *vv_it;
//			MyMesh::VertexHandle v2_handle = *vv_it;
//			//û���ҵ� (v1,v2) && (V2,V1) ��Ϊpair����first second�Ⱥ�˳���
//			if ( (Pair.find(std::make_pair(v1_handle, v2_handle)) == Pair.end() ) && (Pair.find(std::make_pair(v2_handle, v1_handle)) == Pair.end()) )
//			{
//				Pair.insert(std::make_pair(v1_handle, v2_handle));
//			}
//			//if(Pair.find( std::make_pair(v1_handle,v2_handle) ))
//		}
//		//���2 û��edge ��С����ֵ ��Ҫ�������еĶ��� ���︴�Ӷ��е����
//		for (MyMesh::VertexIter v_it2 = mesh.vertices_begin(); v_it2 != mesh.vertices_end(); v_it2++)
//		{
//			MyMesh::VertexHandle v2_handle = *v_it2;
//			MyMesh::Point p2 = mesh.point(*v_it2);
//			if (v2_handle == v1_handle)
//			{
//				continue;
//			}
//			if (norm(p1 - p2) < parameter_t)//����
//			{
//				if ((Pair.find(std::make_pair(v1_handle, v2_handle)) == Pair.end()) && (Pair.find(std::make_pair(v2_handle, v1_handle)) == Pair.end()))
//				{
//					Pair.insert(std::make_pair(v1_handle, v2_handle));
//				}
//			}
//		}
//	}
//	//���ԶԲ���
//	//cout << Pair.size() << endl;
//	//cout << "edges" << mesh.n_edges() << endl;
//}

//�����µĵ� ������ÿ������Ե�cost

//void QEM::basicSTl()
//{
//	Pairtest.insert(std::make_pair(1, 2));
//	Pairtest.insert(std::make_pair(1, 3));
//	Pairtest.insert(std::make_pair(3, 3));
//	Pairtest.insert(std::make_pair(2, 3));
//	Pairtest.insert(std::make_pair(1, 5));
//	Pairtest.insert(std::make_pair(1, 4));
//	auto xx = Pairtest.find(std::make_pair(1, 4));
//
//
//}







