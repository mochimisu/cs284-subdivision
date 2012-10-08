#include "mesh.h"


#include <iostream>
#include <fstream>
#include <sstream>

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

    //just for ex, load into the bufs directly
    pos_buf.clear();
    n_buf.clear();
    index_buf.clear();

    cout << "verts" << endl;

    for(vector<vec3>::iterator v = verts.begin(); v != verts.end(); ++v)
    {
      vec3 cur_v = *v;
      pos_buf.push_back(cur_v[0]);
      pos_buf.push_back(cur_v[1]);
      pos_buf.push_back(cur_v[2]);
      cout << cur_v << endl;
    }
    cout << "norms" << endl;

    for(vector<vec3>::iterator n = normals.begin(); n != normals.end(); ++n)
    {
      vec3 cur_n = *n;
      n_buf.push_back(cur_n[0]);
      n_buf.push_back(cur_n[1]);
      n_buf.push_back(cur_n[2]);
      cout << cur_n << endl;
    }
    cout << "faces" << endl;

    for(vector<vec3>::iterator f = raw_faces.begin(); f != raw_faces.end(); ++f)
    {
      vec3 cur_f = *f;
      index_buf.push_back(cur_f[0]);
      index_buf.push_back(cur_f[1]);
      index_buf.push_back(cur_f[2]);
      cout << cur_f << endl;

    }

    //now actually generate our own data structure

  }
}

void Mesh::loadBuffers(GLuint& vertex_vbo, GLuint& normal_vbo, 
    GLuint& index_vbo)
{
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*pos_buf.size(),
      &pos_buf[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*n_buf.size(),
      &n_buf[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float)*index_buf.size(),
      &index_buf[0], GL_STATIC_DRAW);
}

int Mesh::numElements()
{
  return index_buf.size();
}
