//
// Created by Fis on 09.12.2016.
//

#ifndef CAMVIS_AFACE_H
#define CAMVIS_AFACE_H
#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <windows.h>
#include <vector>
#include <list>
#include <map>
#include "AVertex.h"

using namespace std;

class AMesh;


class AFace {
public:
    list<int> Indices;
    float Normal[3];
    float d;
    char mark;
    AFace(): Indices() {
        for (int i=0; i<3; i++) Normal[i] = 0;
        d = 0;
        mark = NORMAL;
    }

    void clipByPlane(AMesh &BaseMesh, AFace &plane, MapOfList &OpenEdges, SparseTable &ComputedEdges, list<int> &InIndices,  list<int> &OutputIndices, list<AFace*> &removeFromOpen);
    void inverseClipping(AMesh &BaseMesh, list<int> &ClippedInds);
    void draw(AMesh &mesh, list<int> &Inds);
    void makeConvex(AMesh &BaseMesh, bool isTmp);
    void print();
    void print(list<int> Inds);
    void printN();
    void setNormals(AMesh &mesh);
    void resetNormals(AMesh &mesh);
};


#endif //CAMVIS_AFACE_H
