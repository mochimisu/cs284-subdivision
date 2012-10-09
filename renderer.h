#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glut.h>   
#include "imageSaver.h"
#include <algebra3.h>
#include "mesh.h"

class Renderer {

  public:
    Renderer();
    ~Renderer();

    void init(int argc, char** argv);
    void draw();
    void mainLoop();
    void saveFrame();

    int width, height;
    vec2 mousePos;
    mat4 orientation;
    ImageSaver * imgSaver;
    int fCount;

    Mesh mesh;

  private:
    GLuint vertex_vbo;
    GLuint normal_vbo;
    GLuint face_vbo;

};

void setActiveRenderer(Renderer * newRenderer);

#endif
