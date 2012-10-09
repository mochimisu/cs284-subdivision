#ifndef MESH_H
#define MESH_H

#include <algebra3.h>
#include <vector>
#include <GL/glu.h>

class Vertex;
class Edge;
class Triangle;
class Mesh;

class Vertex {
  public:
    Vertex() {};
    ~Vertex() {};
    int index;
    vec3 pos;
    //Edge* edge;
};

class Edge {
  public:
    Edge() {};
    ~Edge() {};
    int index;
    int sibling;
    int next;
    int vert;
    vec3 norm;
    int tri;
};

class Triangle {
  public:
    Triangle() {};
    ~Triangle() {};
    int index;
    int edge;
};

class Mesh {
  public:
    Mesh();
    Mesh(string obj_fname);
    ~Mesh() {};

    void init();
    void loadOBJ(string obj_fname);
    void generateBuffers();
    void draw();

    Mesh subdivide();

    vector<Vertex> vertices;
    vector<Edge> edges;
    vector<Triangle> triangles;
  private:
    vector<float> pos_buf;
    vector<float> n_buf;
    vector<int> index_buf;

    GLuint vertex_vbo;
    GLuint face_vbo;
    GLuint normal_vbo;
};

#endif
