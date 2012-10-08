#ifndef MESH_H
#define MESH_H

#include <algebra3.h>
#include <vector>
#include <GL/glu.h>


class Vertex {
  public:
    Vertex() {};
    ~Vertex() {};
    vec3 pos;
    //Edge* edge;
};

class Edge {
  public:
    Edge() {};
    ~Edge() {};
    Edge* sibling;
    Edge* next;
    Vertex* vert;
    vec3 norm;
};

class Triangle {
  public:
    Triangle() {};
    ~Triangle() {};
    Edge* edge;
};

class Mesh {
  public:
    Mesh() {};
    Mesh(string obj_fname);
    ~Mesh() {};

    void loadOBJ(string obj_fname);
    void loadBuffers(GLuint& vertex_vbo, GLuint& normal_vbo,
        GLuint& index_vbo);
    int numElements();

    vector<Vertex> vertices;
    vector<Edge> edges;
    vector<Triangle> triangles;
  private:
    vector<float> pos_buf;
    vector<float> n_buf;
    vector<int> index_buf;
};

#endif
