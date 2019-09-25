//
// Created by Fis on 09.12.2016.
//

#include "AMesh.h"
#include "AVertex.h"

using namespace std;

void AVertex::print() {
    printf("%.2f %.2f %.2f mark = %d, d = %.2f\n", x, y, z, mark, d);
}

void panic(string func, string s) {
    cout << "---PANIC---" << endl << "Error in func: " + func << endl;
    cout << s << endl << "---PANIC---" << endl;
    exit(-1);
}

void dprint(string s) {
    cout << s << endl;
}


bool getNormal(AVertex a, AVertex b, AVertex c, float *n) {
    float epsilon = 0.001;
    n[0]=(b.y-a.y)*(c.z-a.z)-(b.z-a.z)*(c.y-a.y);
    n[1]=(c.x-a.x)*(b.z-a.z)-(b.x-a.x)*(c.z-a.z);
    n[2]=(b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y);
    for (int i=0; i<3; i++) if (n[i]<epsilon && n[i]>-epsilon) n[i]=0;
    return normalize(n);
}

bool normalize(float *n) {
    float epsilon = 0.001;
    float len = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
    if (len<epsilon && len>-epsilon) return false;
    n[0] /= len;
    n[1] /= len;
    n[2] /= len;
    return true;
}

float Dot(float *a, AVertex &b) {
    return a[0]*b.x + a[1]*b.y + a[2]*b.z;
}

float Dot(float *a, float *b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

bool isAngleConvex(float *a, float *b, float *c) { //ir returns true if right triple or angle < 180
    float res = a[0]*b[1]*c[2] + a[1]*b[2]*c[0] + a[2]*b[0]*c[1];
    res -= (a[2]*b[1]*c[0] + a[1]*b[0]*c[2] + a[0]*b[2]*c[1]);
    //printf("isConvex: %.2f %.2f %.2f and %.2f %.2f %.2f and %.2f %.2f %.2f = %.5f\n", a[0], a[1], a[2], b[0], b[1], b[2], c[0], c[1], c[2], res);
    return res > 0;
}

bool SameDirection(float *a, float *b) {
    for (int i=0; i<3; i++) {
        if (a[i]==0 &&  b[i]==0) continue;
        return (a[i] > 0 && b[i] > 0) || (a[i] < -0 && b[i] < -0);
    }
    panic("SameDirection","Null vectors given on input");
    return true;
}

bool isParallel(float *a, float *b) {
    //printf("isParallel:  ");
    //printf("%.2f %.2f %.2f and %.2f %.2f %.2f", a[0], a[1], a[2], b[0], b[1], b[2]);
    float epsilon = 0.001;
    float res[3];
    res[0] = a[1]*b[2] - a[2]*b[1];
    res[1] = a[2]*b[0] - a[0]*b[2];
    res[2] = a[0]*b[1] - a[1]*b[0];
    for (int i=0; i<3; i++) if (res[i] > epsilon || res[i] < -epsilon) {
            //printf("   false\n");
            return false;
        }
    //printf("   true\n");
    return true; //if res == {0,0,0}
}

void vectorAB(AVertex &a, AVertex &b, float *res) {
    res[0] = b.x - a.x;
    res[1] = b.y - a.y;
    res[2] = b.z - a.z;
}

float norm(float *V) {
    float len = sqrt(V[0]*V[0]+V[1]*V[1]+V[2]*V[2]);
    V[0] /= len;
    V[1] /= len;
    V[2] /= len;
    return len;
}

float length(float *V) {
    float len = sqrt(V[0]*V[0]+V[1]*V[1]+V[2]*V[2]);
    return len;
}

void SparseTable::write(int x, int y, int val) {
    //printf("write: %d %d\n", x, y);
    if (x<y) tbl[x][y] = val;
    else tbl[y][x] = val;
}

bool SparseTable::read(int x, int y, int &out) {
    if (x<y) {
        auto search = tbl.find(x);
        if (search == tbl.end()) return false;
        auto search1 = (search->second).find(y);
        if (search1 == (search->second).end()) return false;
        out = search1->second;
        return true;
    } else {
        auto search = tbl.find(y);
        if (search == tbl.end()) return false;
        auto search1 = (search->second).find(x);
        if (search1 == (search->second).end()) return false;
        out = search1->second;
        return true;
    }
}

void MapOfList::add(int key, int val) {
    //printf("Adding: %d -> %d\n", key, val);
    tbl[key].push_back(val);
}


