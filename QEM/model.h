#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//#include <learnopengl/mesh.h>
//#include <learnopengl/shader.h>

#include"mesh.h"
#include"shader.h"
#include "camera.h"
#include"HalfEdge.h"


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;


class Model 
{
public:
    // model data 
    TriMesh* half_edge_meshes = new TriMesh(); //����������ݽṹ
    vector<Texture> textures_loaded;	// �������Ӧ�ÿ�����private ��stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once. 
    vector<Mesh>    meshes; //assimp �õ��Ľ����������
    string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
   Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
        convertToHalfEdge();  // ת��Ϊ������ݽṹ
        cout << meshes.size() << endl; //----- meshes size 1
        cout << half_edge_meshes << endl;
    }

    void Draw(Shader& shader);
    void loadModel(string const& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene); //����Ķ��������� һ�δ���positions���� ������
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
    unsigned int TextureFromFile(const char* path, const string& directory, bool gamma=false);
    void convertToHalfEdge();

    
private:

};




//class model
//{
//};
#endif
