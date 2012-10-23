#include <GL/glut.h>
#include <GL/glu.h>

#include "renderer.h"
#include "shaders.h"
#include "mesh.h"
#include "loadimage.h"

// Constants to set up lighting on the teapot
const GLfloat light_position[] = {0,15,10,1};    // Position of light 0
const GLfloat light_position1[] = {0,15,-10,1};  // Position of light 1
const GLfloat light_specular[] = {0.6,0.6,0.6,1};    // Specular of light 0
const GLfloat light_specular1[] = {0.6,0.6,0.6,1};   // Specular of light 1
const GLfloat one[] = {1,1,1,1};                 // Specular on teapot
const GLfloat medium[] = {1,1,1,1};        // Diffuse on teapot
const GLfloat blue[] = {0,0,1,1};        // Diffuse on teapot
const GLfloat small[] = {0.2,0.2,0.2,1};         // Ambient on teapot
const GLfloat none[] = {0,0,0,1};
const GLfloat red[] = {1,0,0,1};
const GLfloat high[] = {100};                      // Shininess of teapot
GLfloat light0[4],light1[4]; 

GLuint vertexshader, fragmentshader, shaderprogram ; // shaders
GLuint istex; 
GLuint islight; 
GLuint light0posn; 
GLuint light0color; 
GLuint light1posn; 
GLuint light1color; 
GLuint ambient; 
GLuint diffuse; 
GLuint specular; 
GLuint shininess; 

GLuint tex;
bool cur_islight;


Renderer * activeRenderer;

void transformvec (const GLfloat input[4],GLfloat output[4]) {
  GLfloat modelview[16]; // in column major order
  glGetFloatv(GL_MODELVIEW_MATRIX,modelview); 

  for (int i = 0; i < 4; i++) {
    output[i] = 0; 
    for (int j = 0; j < 4; j++) 
      output[i] += modelview[4*j+i] * input[j]; 
  }
}

void applyMat4(mat4 &mat) {
  double glmat[16];
  int k = 0;
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      glmat[k++] = mat[j][i];
    }
  }
  glMultMatrixd(glmat);
}

void setActiveRenderer(Renderer * newRenderer)
{
  activeRenderer = newRenderer;
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(0,0,5, 0,0,-1, 0,1,0);

  applyMat4(activeRenderer->orientation);    
  transformvec(light_position,light0); 
  transformvec(light_position1,light1); 

  glUniform4fv(light0posn,1,light0); 
  glUniform4fv(light0color,1,light_specular); 
  glUniform4fv(light1posn,1,light1); 
  glUniform4fv(light1color,1,light_specular1); 

  //glUniform4fv(ambient,1,small); 
  glUniform4fv(diffuse,1,medium); 
  glUniform4fv(ambient,1,small); 
  //glUniform4fv(diffuse,1,small); 
  glUniform4fv(specular,1,none); 
  glUniform1fv(shininess,1,high); 

  activeRenderer->draw();

  glutSwapBuffers();
  //activeRenderer->saveFrame();
}

void reshape(int w, int h)
{
  activeRenderer->width = w;
  activeRenderer->height = h;
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(-2, 2, -2, 2, 0.1, 99);
  //gluPerspective(60, float(w)/float(h), 0.1, 99);
}

void keyboard(unsigned char key, int x, int y)
{
  switch(key) {
    case 27:
      exit(0);
      break;
    case 'q':
      activeRenderer->meshes.push_back(
          activeRenderer->meshes.back().subdivide());
      break;
    case 'Q':
      if (activeRenderer->meshes.size() > 1)
      {
        activeRenderer->meshes.pop_back();
      }
      break;
    case 'w':
      activeRenderer->toggleDrawWireframe();
      break;
    case 'd':
      activeRenderer->toggleDrawNormal();
      break;
    case 'l':
      cur_islight = !cur_islight;
      glUniform1i(islight,cur_islight);
      break;
  }
  glutPostRedisplay();
}

void specialKeyboard(int key, int x, int y) 
{
}

void activeMotion(int x, int y)
{
  vec2 newMouse = vec2((double)x / activeRenderer->width,(double)y / 
      activeRenderer->height);
  vec2 diff = (newMouse - activeRenderer->mousePos);
  double len = diff.length();
  if (len > .001) {
    vec3 axis = vec3(diff[1]/len, diff[0]/len, 0);
    activeRenderer->orientation = activeRenderer->orientation * 
      rotation3D(axis, -180 * len);
  }

  //Record the mouse location for drawing crosshairs
  activeRenderer->mousePos = newMouse;

  //Force a redraw of the window.
  glutPostRedisplay();
}

void passiveMotion(int x, int y)
{ 
  //Record the mouse location for drawing crosshairs
  activeRenderer-> mousePos = vec2((double)x / activeRenderer->width,
      (double)y / activeRenderer->height);

  //Force a redraw of the window.
  glutPostRedisplay();
}


Renderer::Renderer()
{
  width = 1280;
  height = 1280;
  orientation = identity3D();
  imgSaver = new ImageSaver("images/", "cs284-subdivision_");
  fCount = 0;
  draw_normal = true;
  draw_wireframe = true;
}

Renderer::~Renderer()
{
  glDeleteBuffers(1, &vertex_vbo);
  glDeleteBuffers(1, &face_vbo);
}

float pos[9];
unsigned int v_index[3];
void Renderer::init(int argc,char** argv)
{
  if (argc < 2)
  {
    cerr << "Missing input OBJ file" << endl;
    exit(1);
  }

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_MULTISAMPLE);

  glutInitWindowSize(width, height); 
  glutInitWindowPosition(0,0); 
  glutCreateWindow("CS284 Subdivision");

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMotionFunc(activeMotion);
  glutPassiveMotionFunc(passiveMotion);
  glutSpecialFunc(specialKeyboard);
  glutIdleFunc(display);

  // set some lights
  {
    float ambient[4] = { .1f, .1f, .1f, 1.f };
    float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.f };
    float pos[4] = { 4, 25, 0, 0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
    glEnable(GL_LIGHT0);
  }
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  //glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);


  //enable face culling for removal
  //glEnable(GL_CULL_FACE);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

  //shaders
  vertexshader = initshaders(GL_VERTEX_SHADER,"shaders/light.vert.glsl");
  fragmentshader = initshaders(GL_FRAGMENT_SHADER,"shaders/light.frag.glsl");
  shaderprogram = initprogram(vertexshader,fragmentshader);
  istex = glGetUniformLocation(shaderprogram,"istex");
  islight = glGetUniformLocation(shaderprogram,"islight");
  light0posn = glGetUniformLocation(shaderprogram,"light0posn");
  light0color = glGetUniformLocation(shaderprogram,"light0color");
  light1posn = glGetUniformLocation(shaderprogram,"light1posn");
  light1color = glGetUniformLocation(shaderprogram,"light1color");
  ambient = glGetUniformLocation(shaderprogram,"ambient");
  diffuse = glGetUniformLocation(shaderprogram,"diffuse");
  specular = glGetUniformLocation(shaderprogram,"specular");

  shininess = glGetUniformLocation(shaderprogram,"shininess");
  glUniform1i(islight,true);
  cur_islight = true;

  Mesh mesh;
  mesh.init();
  mesh.loadOBJ(argv[1]);
  meshes.push_back(mesh);

  if (argc == 3)
  {
    //texture
    loadTexture(argv[2], tex);
    glEnable(GL_TEXTURE_2D);
    glUniform1i(istex,true);
  }
}

void Renderer::mainLoop() 
{
  glutMainLoop();
}

void Renderer::draw()
{
  //normal
  if (draw_normal)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    meshes.back().draw();
  }

  //Wireframe
  if (draw_wireframe)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPolygonOffset(1,1);
    glUniform4fv(diffuse,1,blue); 
    meshes.back().draw();
  }
}

void Renderer::toggleDrawNormal()
{
  draw_normal ^= 1;
}

void Renderer::toggleDrawWireframe()
{
  draw_wireframe ^= 1;
}

void Renderer::saveFrame() {
  if(fCount == 0) {
    imgSaver->saveFrame();
      //activeRenderer->orientation = activeRenderer->orientation * 
      rotation3D(vec3(0,1,0), -0.1);
  }
  fCount = (fCount + 1)%20;
}
