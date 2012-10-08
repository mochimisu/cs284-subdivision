#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glut.h>   
#include "imageSaver.h"
#include <algebra3.h>

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
};

void setActiveRenderer(Renderer * newRenderer);

#endif
