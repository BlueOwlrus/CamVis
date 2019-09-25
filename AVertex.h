//
// Created by Fis on 09.12.2016.
//

#ifndef CAMVIS_AVERTEX_H
#define CAMVIS_AVERTEX_H
#include "AMesh.h"
#include "AFace.h"

using namespace std;


//float epsilon = 0.001; //fuck cpp linker!
enum {NORMAL=0, //basic
    OUTSIDE=1, //positive side of a clipping plane
    INSIDE=2, //negative side of a clipping plane
    ONTO=3, //lays on a clipping plane (for vertex only)
    ONTO_I=-2, //lays in a clipping plane, inside of clipping polygon (at least we suggest so)
    ONTO_C=-3, //lays in a clipping plane (and was clipped) or touchs it with an edge
    CLIPPED=4, //vertex divides original edge (lays between 2 original vertices)
    CLOSED=5, //for faces only, it marks result of closeMesh func
    CLOSED_N=6, //for freshly added closed face (for copying tmpIndices)
    DEAD=-1 //for faces only
};

class AVertex {
public:
    float x, y, z;
    char mark;
    float d; //distance from a plane;
    AVertex(): x(0), y(0), z(0), mark(NORMAL), d(0) {}
    AVertex(float a, float b, float c): x(a), y(b), z(c), mark(NORMAL), d(0) {}
    AVertex(float a, float b, float c, char m): x(a), y(b), z(c), mark(m), d(0) {}
    void print();
};

class SparseTable {
public:
    map<int , map<int, int>> tbl;
    void write(int x, int y, int val);
    bool read(int x, int y, int &out); //returns false if element is not found
};

struct MapOfList {
    map< int , list<int>> tbl;
    void add(int key, int val);
};

bool getNormal(AVertex a, AVertex b, AVertex c, float *n);
float Dot(float *a, AVertex &b);
float Dot(float *a, float *b);
bool isAngleConvex(float *a, float *b, float *c);
void panic(string func, string s);
bool SameDirection(float *a, float *b);
bool isParallel(float *a, float *b);
void vectorAB(AVertex &a, AVertex &b, float *res);
void dprint(string s);
bool normalize(float *n);
float norm(float *V);
float length(float *V);


#endif //CAMVIS_AVERTEX_H
