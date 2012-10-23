#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glut.h>   
#include "imageSaver.h"
#include <algebra3.h>
#include "mesh.h"
#include <vector>

class Renderer {

  public:
    Renderer();
    ~Renderer();

    void init(int argc, char** argv);
    void draw();
    void mainLoop();
    void saveFrame();
    void toggleDrawNormal();
    void toggleDrawWireframe();

    int width, height;
    vec2 mousePos;
    mat4 orientation;
    ImageSaver * imgSaver;
    int fCount;

    vector<Mesh> meshes;

  protected:
    GLuint vertex_vbo;
    GLuint normal_vbo;
    GLuint face_vbo;

    bool draw_normal;
    bool draw_wireframe;

};

void setActiveRenderer(Renderer * newRenderer);

#endif
