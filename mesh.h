#ifndef MESH_H
#define MESH_H

#include <algebra3.h>
#include <vector>

class Vertex {
  public:
    Vertex();
    ~Vertex();
    vec3 pos;
    vec3 norm;
};

class Edge {
  public:
    Edge();
    ~Edge();
    Edge* sibling;
    Edge* next;
    Vertex* vert;
};

class Triangle {
  public:
    Triangle();
    ~Triangle();
    Edge* edge;
};

class Mesh {
  public:
    Mesh();
    ~Mesh();
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::vector<Triangle> triangles;
  private:
    std::vector<float> pos_buf;
    std::vector<float> n_buf;
    std::vector<float> index_buf;
};

#endif
