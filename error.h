#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED
#include<iostream>
#include<string>

#define reportError(s) _ReportError(__LINE__, (s))

// Note that it uses gluErrorString, so you must inlcude glu32.lib in your visual studio projects and -lGLU on linux
void _ReportError(int ln, const std::string str) {
    GLuint err = glGetError();
    if (!err) return;
    const GLubyte* glerr = gluErrorString(err);
    printf("******************************************\n%d: %s: GLError %d: %s\n", ln, str.c_str(), err, glerr);
}

void glfwErrorCB(int error, const char* description) {
    fputs(description, stderr);
}

#endif // ERROR_H_INCLUDED
