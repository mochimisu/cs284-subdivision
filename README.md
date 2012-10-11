cs284-subdivision
=================

CS284 Fall 2012 Subdivision Homework

This project will take in a triangle mesh in OBJ, and subdivide it using the
Loop subdivision scheme.

Currently, it will read the OBJ and subdivide on midpoints of edges.

The project is written in C++ using OpenGL, and tested only on OSX 10.8

Running it
==========
I have only tested this under OSX 10.8.
To compile, type `make` in the folder
To run, run `subdivision` with an OBJ file as an argument, such as:
`./subdivision models/mug.obj`
