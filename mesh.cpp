#include "mesh.h"


#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <map>

Mesh::Mesh()
{
}

void Mesh::init()
{
  vertices.clear();
  edges.clear();
  triangles.clear();

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
        for (unsigned int i=0; i<splitline[1].length(); i++) {
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

    // vector to maintain siblings
    // tuple is ("from" vertex, "to" vertex)
    // value is edge num
    map<pair<int,int>,int> edge_map;

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
    for (unsigned int i = 0; i < raw_faces.size(); ++i)
    {
      for (unsigned int j = 0; j < 3; ++j)
      {
        edges.push_back(Edge());
      }
      triangles.push_back(Triangle());
    }
    int edge_n = 0;
    for (unsigned int i = 0; i < raw_faces.size(); ++i)
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
      e0.sibling = -1;

      Edge& e1 = edges[edge_n++];
      e1.index = edge_n-1;
      e1.vert = (int)cur_indices[1];
      e1.norm = normals[(int)cur_n[1]];
      e1.tri = tri.index;
      e1.sibling = -1;

      Edge& e2 = edges[edge_n++];
      e2.index = edge_n-1;
      e2.vert = (int)cur_indices[2];
      e2.norm = normals[(int)cur_n[2]];
      e2.tri = tri.index;
      e2.sibling = -1;

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

      edge_map[make_pair(e0.vert, edges[e0.next].vert)] = e0.index;
      edge_map[make_pair(e1.vert, edges[e1.next].vert)] = e1.index;
      edge_map[make_pair(e2.vert, edges[e2.next].vert)] = e2.index;

      pair<int,int> rev_edge = make_pair(edges[e0.next].vert, e0.vert);
      if (edge_map.count(rev_edge) > 0)
      {
        e0.sibling = edge_map[rev_edge];
        edges[e0.sibling].sibling = e0.index;
      }

      rev_edge = make_pair(edges[e1.next].vert, e1.vert);
      if (edge_map.count(rev_edge) > 0)
      {
        e1.sibling = edge_map[rev_edge];
        edges[e1.sibling].sibling = e1.index;
      }

      rev_edge = make_pair(edges[e2.next].vert, e2.vert);
      if (edge_map.count(rev_edge) > 0)
      {
        e2.sibling = edge_map[rev_edge];
        edges[e2.sibling].sibling = e2.index;
      }

    }

    generateBuffers();

  }
}

void Mesh::generateBuffers()
{
  pos_buf.clear();
  n_buf.clear();
  index_buf.clear();

  //int index = 0;
  for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); ++v)
  {
    pos_buf.push_back(v->pos[0]);
    pos_buf.push_back(v->pos[1]);
    pos_buf.push_back(v->pos[2]);
    //v->index = index++;
  }

  for (vector<Triangle>::iterator t = triangles.begin(); t != triangles.end();
      ++t) 
  {
    Edge * cur_edge = &edges[t->edge];

    //loop around edges
    //assuming triangle
    for (unsigned int i = 0; i < 3; ++i)
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

  //copy vertices from old mesh
  n_mesh.vertices = vertices;

  //go through each edge, and compute a new vertex to split on
  map<int, int> edge_split_verts;
  for (unsigned int i = 0; i < edges.size(); ++i)
  {
    if (edge_split_verts.count(edges[i].sibling) > 0)
    {
      //edge_split_vertss[i] = edge_split_verts[edges[i].sibling];
      continue;
    }
    // for loop subdivision, be smarter about this
    // new edge vertex = 3/8*(sum of two endpts of edge)+1/8*(sum of other
    //   two points that form the triangles w/ this edge)
    Vertex v_split;
    vec3 sum_endpts = (vertices[edges[i].vert].pos \
        + vertices[edges[edges[i].next].vert].pos);
    if (edges[i].sibling >= 0)
    {
      vec3 sum_conntri = (vertices[edges[edges[edges[i].next].next].vert].pos \
          +vertices[edges[edges[edges[edges[i].sibling].next].next].vert].pos);
      v_split.pos = (0.375*sum_endpts) + (0.125*sum_conntri);
    } else {
      vec3 conntri = vertices[edges[edges[edges[i].next].next].vert].pos;
      v_split.pos = (3./7.*sum_endpts) + (conntri/7.);
    }

    v_split.index = n_mesh.vertices.size();
    n_mesh.vertices.push_back(v_split);
    edge_split_verts[i] = v_split.index;
  }

  //split each edge, aling w their sibling edge
  // key: old edge index, value: <n_mesh index, n_mesh index>
  map<int, pair<int,int> > edge_splits;
  for (unsigned int i = 0; i < edges.size(); ++i)
  {
    if (edge_split_verts.count(i) == 0)
      continue;

    // for loop subdivision, average all connected edges in a map
    // moved vertex = (1-n)um neighbor verts*s*old + s*sum of neighbor vertices
    // n=3 -> s=3/16
    // n>3 -> s=1/n*(5/8 - (3/8 + 1/4*cos(2pi/n))^2)
    Edge cur_edge = edges[i];
    Edge new_e0 = Edge();
    new_e0.vert = cur_edge.vert;
    new_e0.index = n_mesh.edges.size();
    new_e0.sibling = -1;
    new_e0.next = -1;
    new_e0.tri = -1;
    n_mesh.edges.push_back(new_e0);

    Edge new_e1 = Edge();
    new_e1.vert = edge_split_verts[i];
    new_e1.index = n_mesh.edges.size();
    new_e1.sibling = -1;
    new_e1.next = -1;
    new_e1.tri = -1;
    n_mesh.edges.push_back(new_e1);

    if (edges[i].sibling >= 0) 
    {
      Edge sibling_edge = edges[edges[i].sibling];
      Edge new_e2 = Edge();
      new_e2.vert = sibling_edge.vert;
      new_e2.index = n_mesh.edges.size();
      n_mesh.edges.push_back(new_e2);

      Edge new_e3 = Edge();
      new_e3.vert = edge_split_verts[i];
      new_e3.index = n_mesh.edges.size();
      n_mesh.edges.push_back(new_e3);

      n_mesh.edges[new_e0.index].sibling = new_e3.index;
      n_mesh.edges[new_e3.index].sibling = new_e0.index;
      n_mesh.edges[new_e1.index].sibling = new_e2.index;
      n_mesh.edges[new_e2.index].sibling = new_e1.index;
      edge_splits[sibling_edge.index] = make_pair(new_e2.index, new_e3.index);
    }

    edge_splits[i] = make_pair(new_e0.index, new_e1.index);
  }

  // go through each triangle and make our 4 new triangles
  for (unsigned int i = 0; i < triangles.size(); ++i)
  {
    Triangle& old_tri = triangles[i];

    Edge old_edge0 = edges[old_tri.edge];
    Edge old_edge1 = edges[old_edge0.next];
    Edge old_edge2 = edges[old_edge1.next];

    pair<int,int> split_edge[3] = {
      edge_splits[old_edge0.index],
      edge_splits[old_edge1.index],
      edge_splits[old_edge2.index] };

    //create new edges
    // 0: 0->1; 1: 1->2; 2: 2->0
    // 3: 0->2; 4: 1->0; 5: 2->1
    int new_edge[6];
    for (unsigned int j = 0; j < 6; ++j)
    {
      Edge n_edge = Edge();
      n_edge.index = n_mesh.edges.size();
      n_edge.vert = n_mesh.edges[split_edge[j%3].second].vert;
      n_mesh.edges.push_back(n_edge);
      new_edge[j] = n_edge.index;
    }

    n_mesh.edges[new_edge[0]].sibling = new_edge[4];
    n_mesh.edges[new_edge[4]].sibling = new_edge[0];
    n_mesh.edges[new_edge[1]].sibling = new_edge[5];
    n_mesh.edges[new_edge[5]].sibling = new_edge[1];
    n_mesh.edges[new_edge[2]].sibling = new_edge[3];
    n_mesh.edges[new_edge[3]].sibling = new_edge[2];

    // make the triangles!
    //0: s2.1->s0.0->n3
    //1: s0.1->s1.0->n4
    //2: s1.1->s2.0->n5
    //3: n0->n1->n2
    Triangle tri0 = Triangle();
    tri0.index = n_mesh.triangles.size();
    tri0.edge = split_edge[2].second;
    n_mesh.edges[split_edge[2].second].next = split_edge[0].first;
    n_mesh.edges[split_edge[0].first].next = new_edge[3];
    n_mesh.edges[new_edge[3]].next = split_edge[2].second;
    n_mesh.edges[split_edge[2].second].tri = tri0.index;
    n_mesh.edges[split_edge[0].second].tri = tri0.index;
    n_mesh.edges[new_edge[3]].tri = tri0.index;
    n_mesh.triangles.push_back(tri0);

    for (unsigned int j = 0; j < 3; ++j)
    {
      int sfirst = (2+j)%3;
      int ssecond = j%3;
      int nthird = j+3;
      Triangle n_tri = Triangle();
      n_tri.index = n_mesh.triangles.size();
      n_tri.edge = split_edge[sfirst].second;
      n_mesh.edges[split_edge[sfirst].second].next = split_edge[ssecond].first;
      n_mesh.edges[split_edge[ssecond].first].next = new_edge[nthird];
      n_mesh.edges[new_edge[nthird]].next = split_edge[sfirst].second;

      n_mesh.edges[split_edge[sfirst].second].tri = n_tri.index;
      n_mesh.edges[split_edge[ssecond].first].tri = n_tri.index;
      n_mesh.edges[new_edge[nthird]].tri = n_tri.index;
      n_mesh.triangles.push_back(n_tri);
    }

    // 4th tri
    Triangle n_tri = Triangle();
    n_tri.index = n_mesh.triangles.size();
    n_tri.edge = new_edge[0];
    n_mesh.edges[new_edge[0]].next = new_edge[1];
    n_mesh.edges[new_edge[1]].next = new_edge[2];
    n_mesh.edges[new_edge[2]].next = new_edge[0];

    n_mesh.edges[new_edge[0]].tri = n_tri.index;
    n_mesh.edges[new_edge[1]].tri = n_tri.index;
    n_mesh.edges[new_edge[2]].tri = n_tri.index;
    n_mesh.triangles.push_back(n_tri);

  }

  //keep this to debug later
#ifdef DEBUG
  cout << "orig:" << endl;
  cout << "vertices: " << vertices.size() << endl;
  cout << "edges: " << edges.size() << endl;
  cout << "triangles: " << triangles.size() << endl;

  cout << "subdiv:" << endl;
  cout << "vertices: " << n_mesh.vertices.size() << endl;
  cout << "edges: " << n_mesh.edges.size() << endl;
  cout << "triangles: " << n_mesh.triangles.size() << endl;
  cout << "=========" << endl;

  for (unsigned int i = 0; i < n_mesh.edges.size(); ++i)
  {
    cout << "e: " \
      << "|" << n_mesh.edges[i].index \
      << "|" << n_mesh.edges[i].sibling \
      << "|" << n_mesh.edges[i].next \
      << "|" << n_mesh.edges[i].vert \
      << "|" << n_mesh.edges[i].tri \
      << endl;
  }
#endif



  n_mesh.generateBuffers();

  return n_mesh;
}
