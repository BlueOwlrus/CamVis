
#ifndef CAMVIS_AMESH_H
#define CAMVIS_AMESH_H
#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <windows.h>
#include <vector>
#include <list>
#include <map>
#include "AFace.h"
#include "AVertex.h"
#include <iterator>

using namespace std;


class AMesh {
public:
    vector <AVertex*> Vertices;
    vector <AFace*> Faces;
    vector <AVertex*> tmpVertices;
    vector <AFace*> tmpFaces;
    vector <list<int>*> Indices;
    vector <list<int>*> tmpIndices;
    float Center[3];

    AMesh() {
        Center[0] = 0;
        Center[1] = 0;
        Center[2] = 0;
    }

    AVertex &getVertex(int i);

    void addVertex(float a, float b, float c, float Scale);
    int addVertex(AVertex *in);

    int addNegVertex(float a, float b, float c); //mark = CLIPPED

    AFace &addTmpFace(AFace &base); //mark = NORMAL
    AFace &addTmpFace(AFace &base, list<int> &points); //mark = NORMAL

    AFace &addFace(AFace &base, list<int> &points);
    void addFace(vector<int> &points);
    void addFace(AFace *f);

    void move(float x, float y, float z); //TODO also rotate

    void clipByMesh(AMesh &clipper);

    void reset();

    void closeMesh(MapOfList &OpenEdges, AFace &clipper); //closing holes in mesh based on OpenEdges data

    void clipByPlane(AFace &plane);

    void print();

    void dprint();

    bool loadObj(const char *path, float Scale);

    void draw(unsigned char BaseColor[]);
    void drawTmp(unsigned char BaseColor[]);

    void modCutter(AMesh &cutter, float *last, float *current);

    void copyFrom(AMesh &base);
};


#endif //CAMVIS_AMESH_H
