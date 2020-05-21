#ifndef WINDOW_H
#define WINDOW_H
#include<string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "stb_image.h"

#include "mesh.h"
static const int house_triangles_no = 58;

using namespace glm;

using namespace std;

class Window
{
    public:
        Window(int width, int height, const string& title);
        void Clear(float r, float g, float b,float a);

        void TransformCube();
        void RenderCube();
        void TransformLight();
        void RenderLight();
        void TransformTetrahedron();
        void RenderTetrahedron();
        void TransformOctahedron();
        void RenderOctahedron();
        void TransformDodecahedron();
        void RenderDodecahedron();
        void TransformIcosahedron();
        void RenderIcosahedron();
        void TransformHouse();
        void RenderHouse();
        void TransformSphere();
        void RenderSphere();
        void drawNormls();
    
        void TransformNanosui();
        void RenderNanosui();
        void TransformIronMan();
        void RenderIranMan();
    
//        void TransformHelicopter();
//        void RenderHelicopter();


        void RenderWorld();

    void setMat4(const GLuint ID, std::string name, const glm::mat4 &mat) const;

    void setVec3(const GLuint ID,  const std::string &name, const glm::vec3 &value) const;
    void setFloat(const GLuint ID,const std::string &name, float value) const;
    void setInt(const GLuint ID,const std::string &name, int value) const;
    glm::vec3 computeSphereVertices(float theta, float phi);
    void configureLights();

  
    /*  Model Data */
    vector<Texture> textures_loaded_Nanosuit;
    vector<Texture> textures_loaded_IronMan;
    vector<Mesh> meshesIronMan;
    vector<Mesh> meshesNanosuit;
    string directoryIranMan;
    string directoryNanosuit;
    bool gammaCorrection;
    /*  Functions   */
    void loadModelIronMan(string path);
    void loadModel(string path);
    void drawModelNanosui();
    void drawModelIronMan();
    void processNode(aiNode *node, const aiScene *scene);
    void processNodeIronMan(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                         string typeName);
    vector<Texture> loadMaterialTexturesIronMan(aiMaterial *mat, aiTextureType type,
                                                string typeName);
    unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);


    virtual ~Window();


    protected:

    private:
        Window(const Window& other){}
        void operator=(const Window& other){}
        GLFWwindow* window;
        GLuint worldVAO; // VAO
        GLuint worldProgramID; //shader
        GLuint lightVAO;
        GLuint lightProgramID;
        GLuint phongProgramID;
        GLuint gouraudProgramID;
        GLuint modelProgramID;

        GLuint cube_vertexbuffer; //VBo
        GLuint tetrahedron_vertexbuffer; //VBo
        GLuint octahedron_vertexbuffer;
        GLuint dodecahedron_vertexbuffer;
        GLuint icosahedron_vertexbuffer;
        GLuint house_vertexbuffer;
        GLuint sphere_vertexbuffer;
        GLuint house_norms_vertexbuffer;
        GLuint icosahedron_norms_buffer;
        GLuint octahedron_norm_buffer;
        GLuint tetrahedron_norms_buffer;
        GLuint cube_norms_buffer;
        GLuint dodecahedron_norms_buffer;
        GLuint sphere_norms_buffer;


    
    
        GLuint light_vertexbuffer;

        GLuint MatrixID;
        glm::mat4 Projection;
        glm::mat4 Model;
        glm::mat4 MVP;
        glm::mat4 View;
        glm::vec3 lightPos;
        float angle;
        float speed;
        float bgColor[4];
    
        // for sphere:
        float degToRad;
        float r;
};








#endif // WINDOW_H
