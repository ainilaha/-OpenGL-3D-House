#ifndef SHADER_H
#define SHADER_H
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>


class Shader
{
    public:
        Shader();
        GLuint Load(const char * vertex_file_path,const char * fragment_file_path);
        void Transform(float angle);
        virtual ~Shader();

    protected:

    private:
        Shader(const Shader& other);
        void operator=(const Shader& other);
};

#endif // SHADER_H
