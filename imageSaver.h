//Referencing njoubert's 184 save frame code
#ifndef _IMAGESAVER_H_
#define _IMAGESAVER_H_

#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

class ImageSaver {
  public:
    ImageSaver(std::string directory, std::string pref);
    ~ImageSaver();

    void saveFrame();

  private:
    int frameCount;
    std::string dir;
    std::string prefix;
};

#endif
