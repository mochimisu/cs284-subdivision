#include "mesh.h"


#include <iostream>
#include <fstream>
#include <sstream>

Mesh::Mesh()
{
}

void Mesh::init()
{
  //create vbos
  glGenBuffers(1, &vertex_vbo);
  glGenBuffers(1, &face_vbo);
  glGenBuffers(1, &normal_vbo); 

  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void Mesh::loadOBJ(string obj_fname)
{
  vector<vec3> verts;
  vector<vec3> normals;
  
  vector<vec3> raw_faces;
  vector<vec3> raw_faces_normals;

  ifstream inpfile(obj_fname.c_str());
  if (!inpfile.is_open()) {
    cout << "Unable to open file" << endl;
  } else {
    string line;
    while(!getline(inpfile,line).eof()) {
      vector<string> splitline;
      string buf;

      stringstream ss(line);
      while (ss >> buf) {
        splitline.push_back(buf);
      }

      //Ignore blank lines
      if(splitline.size() == 0) {
        continue;
      }
      //Vertex
      if (splitline[0][0] == 'v') {
        //Vertex normal
        if (splitline[0].length() > 1 && splitline[0][1] == 'n'){
          normals.push_back(vec3(atof(splitline[1].c_str()),
                atof(splitline[2].c_str()),atof(splitline[3].c_str())));
        } else {
          verts.push_back(vec3(atof(splitline[1].c_str()),
                atof(splitline[2].c_str()),atof(splitline[3].c_str())));
        }
      } 
      //Face
      else if (splitline[0][0] == 'f') {
        int v1, v2, v3;
        int n1, n2, n3;
        //find "type"
        int numSlash = 0;
        for (int i=0; i<splitline[1].length(); i++) {
          if(splitline[1][i] == '/')
            numSlash++;
        }
        //cout << numSlash << endl;
        if (numSlash == 0) {
          sscanf(line.c_str(), "f %d %d %d", &v1, &v2, &v3);
          raw_faces.push_back(vec3(v1-1,v2-1,v3-1));
        } else if (numSlash == 1) {
          sscanf(line.c_str(), "f %d/%*d %d/%*d %d/%*d", &v1, &v2, &v3);
          raw_faces.push_back(vec3(v1-1,v2-1,v3-1));
        } else if (numSlash == 2) {
          sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3);
          raw_faces.push_back(vec3(v1-1,v2-1,v3-1));
          raw_faces_normals.push_back(vec3(n1-1,n2-1,n3-1));
        } else {
          cout << "Too many slashses in f" << endl;
        }

      }
    }
    inpfile.close();

    //now actually generate our own data structure
    vertices.clear();
    edges.clear();
    triangles.clear();
    int i=0;
    for (vector<vec3>::iterator v = verts.begin(); v != verts.end(); ++v)
    {
      Vertex new_vert = Vertex();
      new_vert.pos = *v;
      new_vert.index = i++;

      vertices.push_back(new_vert);
    }

    // assume normals for now, go back and generate normals for undef normals
    // later
    for (int i = 0; i < raw_faces.size(); ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        edges.push_back(Edge());
      }
      triangles.push_back(Triangle());
    }
    int edge_n = 0;
    for (int i = 0; i < raw_faces.size(); ++i)
    {
      vec3 cur_indices = raw_faces[i];
      vec3 cur_n = raw_faces_normals[i];
      vec3 navg = (normals[(int)cur_n[0]]
          + normals[(int)cur_n[1]]
          + normals[(int)cur_n[2]])/3.;
      vec3 v1 = (verts[(int)cur_indices[1]]
          - verts[(int)cur_indices[0]]);
      vec3 v2 = (verts[(int)cur_indices[2]]
          - verts[(int)cur_indices[0]]);
      vec3 v12n = v1 ^ v2;

      Triangle& tri = triangles[i];
      tri.index = i;
      
      Edge& e0 = edges[edge_n++];
      e0.index = edge_n-1;
      e0.vert = (int)cur_indices[0];
      e0.norm = normals[(int)cur_n[0]];
      e0.tri = tri.index;

      Edge& e1 = edges[edge_n++];
      e1.index = edge_n-1;
      e1.vert = (int)cur_indices[1];
      e1.norm = normals[(int)cur_n[1]];
      e1.tri = tri.index;

      Edge& e2 = edges[edge_n++];
      e2.index = edge_n-1;
      e2.vert = (int)cur_indices[2];
      e2.norm = normals[(int)cur_n[2]];
      e2.tri = tri.index;

      tri.edge = e0.index;

      if ((v12n * navg) > 0)
      {
        //v12n aligned with navg
        //0,1,2
        e0.next = e1.index;
        e1.next = e2.index;
        e2.next = e0.index;
      } else {
        //0,2,1
        e0.next = e2.index;
        e2.next = e1.index;
        e1.next = e0.index;
      }

      //fill in siblings later
    }

    generateBuffers();

  }
}

void Mesh::generateBuffers()
{
  pos_buf.clear();
  n_buf.clear();
  index_buf.clear();

  int index = 0;
  for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); ++v)
  {
    pos_buf.push_back(v->pos[0]);
    pos_buf.push_back(v->pos[1]);
    pos_buf.push_back(v->pos[2]);
    v->index = index++;
  }

  for (vector<Triangle>::iterator t = triangles.begin(); t != triangles.end();
      ++t) 
  {
    Edge * cur_edge = &edges[t->edge];

    //loop around edges
    //assuming triangle
    for (int i = 0; i < 3; ++i)
    {
      index_buf.push_back(vertices[cur_edge->vert].index);
      cur_edge = &edges[cur_edge->next];
    }
  }
}

void Mesh::draw()
{
  /*
  // fix this later...
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*pos_buf.size(),
      &pos_buf[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*n_buf.size(),
      &n_buf[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_vbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float)*index_buf.size(),
      &index_buf[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_vbo);
  glDrawElements(GL_TRIANGLES, index_buf.size(), GL_UNSIGNED_INT, 0);
  */

  //alternative while i figure out whats wrong with loading the buffers
  glBegin(GL_TRIANGLES);
  for (vector<int>::iterator i = index_buf.begin(); i != index_buf.end(); ++i)
  {
    float* cur_vert = &pos_buf[(*i)*3];
    glVertex3f(cur_vert[0], cur_vert[1], cur_vert[2]);
  }
  glEnd();
}

Mesh Mesh::subdivide()
{
  Mesh n_mesh;
  n_mesh.init();

  //go through each triangle, and split each edge
  for (int i = 0; i < triangles.size(); ++i)
  {
    Triangle& old_tri = triangles[i];

    int cur_old_edge_ind = old_tri.edge;
    for (int j = 0; j < 3; ++j) 
    {
      Edge& cur_old_edge = edges[cur_old_edge_ind];
      Edge& next_old_edge = edges[cur_old_edge.next];
      Edge& next_next_old_edge = edges[next_old_edge.next];

      Triangle new_tri;
      new_tri.index = n_mesh.triangles.size();

      //share between siblings later
      Vertex new_v0;
      new_v0.pos = (vertices[cur_old_edge.vert].pos\
          + vertices[next_old_edge.vert].pos)/2;
      new_v0.index = n_mesh.vertices.size();
      n_mesh.vertices.push_back(new_v0);

      Vertex new_v1;
      new_v1.pos = vertices[next_old_edge.vert].pos;
      new_v1.index = n_mesh.vertices.size();
      n_mesh.vertices.push_back(new_v1);

      Vertex new_v2;
      new_v2.pos = (vertices[next_old_edge.vert].pos\
          + vertices[next_next_old_edge.vert].pos)/2;
      new_v2.index = n_mesh.vertices.size();
      n_mesh.vertices.push_back(new_v2);

      Edge new_e0;
      new_e0.vert = new_v0.index;
      new_e0.index = n_mesh.edges.size();
      new_e0.tri = new_tri.index;
      new_e0.next = new_e0.index+1;
      n_mesh.edges.push_back(new_e0);

      Edge new_e1;
      new_e1.vert = new_v1.index;
      new_e1.index = n_mesh.edges.size();
      new_e1.tri = new_tri.index;
      new_e1.next = new_e1.index+1;
      n_mesh.edges.push_back(new_e1);

      Edge new_e2;
      new_e2.vert = new_v2.index;
      new_e2.index = n_mesh.edges.size();
      new_e2.tri = new_tri.index;
      new_e2.next = new_e0.index;
      n_mesh.edges.push_back(new_e2);

      new_tri.edge = new_e0.index;
      n_mesh.triangles.push_back(new_tri);

      cur_old_edge_ind = cur_old_edge.next;
    }
  }

  cout << "orig:" << endl;
  cout << "vertices: " << vertices.size() << endl;
  cout << "edges: " << edges.size() << endl;
  cout << "triangles: " << triangles.size() << endl;

  cout << "subdiv:" << endl;
  cout << "vertices: " << n_mesh.vertices.size() << endl;
  cout << "edges: " << n_mesh.edges.size() << endl;
  cout << "triangles: " << n_mesh.triangles.size() << endl;




  n_mesh.generateBuffers();

  return n_mesh;
}
