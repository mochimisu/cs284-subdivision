//Referencing njoubert's 184 save frame code

#include "imageSaver.h"

#include <FreeImage.h>


ImageSaver::ImageSaver(std::string directory, std::string pref) : frameCount(0) {
  dir = directory;
  if (dir[dir.size()-1] != '/')
    dir.append("/");
  prefix = pref;
}

ImageSaver::~ImageSaver() {
}

void ImageSaver::saveFrame() {
  int w = glutGet(GLUT_WINDOW_WIDTH);
  int h = glutGet(GLUT_WINDOW_HEIGHT);

  frameCount++;
  std::stringstream filename(std::stringstream::in | std::stringstream::out);
  filename << dir << prefix;
  filename << std::setfill('0') << std::setw(6) << frameCount << ".png";
  std::cout << "Saving frame " << frameCount << std::endl;

  /******************************
   * Here we draw!
   ******************************/
  unsigned char *image;

  /* Allocate our buffer for the image */
  try {
    image = new unsigned char[3*w*h];
  } catch (std::bad_alloc&) {
    std::cout << "Couldn't allocate memory!" << std::endl;
    return;
  }

  glPixelStorei(GL_PACK_ALIGNMENT,1);
  glReadBuffer(GL_BACK_LEFT);
  //glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,image);
  
  glReadPixels(0,0,w,h,GL_BGR,GL_UNSIGNED_BYTE,image);

  FIBITMAP * fImage = FreeImage_ConvertFromRawBits(image, w, h, 3*w, 
      24, 0xFF0000, 0x00FF00, 0x0000FF, false);
  FreeImage_Save(FIF_PNG, fImage, filename.str().c_str(), 0);


  delete [] image;

}
