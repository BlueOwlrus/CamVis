//
// Created by Fis on 09.12.2016.
//

#include "AMesh.h"
#include "AFace.h"
#include <algorithm>

using namespace std;

void AFace::clipByPlane(AMesh &BaseMesh, AFace &plane, MapOfList &OpenEdges,
                       SparseTable &ComputedEdges, list<int> &InIndices, list<int> &OutputIndices, list<AFace*> &removeFromOpen) {
    bool dp = false; //debug print
    if (this->mark == CLOSED) dp = true;
    int start;
    bool isOutside=true, isInside=true;
    start = *InIndices.begin();
    AVertex *f = &BaseMesh.getVertex(start);
    int f_index = start;
    list<int> ClippedVertices;

    //printf("\nEnter Clip"); this->print(InIndices);
    for (auto it = InIndices.begin();;) {
        if (++it == InIndices.end()) it = InIndices.begin();
        AVertex *s = &BaseMesh.getVertex(*it);

        //printf("f: ");f->print();
        //printf("s: ");s->print();
        if (f->d > 0) {
            isInside = false;
            if (s->d < 0) {
                int IntersectionIndex;
                if (!ComputedEdges.read(f_index, *it, IntersectionIndex)) {
                    float p[3] = {s->x - f->x, s->y - f->y, s->z - f->z}; //vector of line "fs"
                    //printf("p[]: %.2f %.2f %.2f\n", p[0], p[1], p[2]);
                    float t = f->d / (p[0] * plane.Normal[0] + p[1] * plane.Normal[1] +
                                                  p[2] * plane.Normal[2]); //computing intersection
                    //printf("t: %.2f\n", t);
                    IntersectionIndex = BaseMesh.addNegVertex(f->x - t * p[0], f->y - t * p[1], f->z - t * p[2]);
                    ComputedEdges.write(f_index, *it, IntersectionIndex);
                }
                OutputIndices.push_back(IntersectionIndex);
                ClippedVertices.push_back(IntersectionIndex);
            }
        } else if (f->d < 0) {
            isOutside = false;
            OutputIndices.push_back(f_index);
            if (s->d > 0) {
                int IntersectionIndex;
                if (!ComputedEdges.read(f_index, *it, IntersectionIndex)) {
                    float p[3] = {s->x - f->x, s->y - f->y, s->z - f->z}; //vector of line "fs"
                    float t = f->d / (p[0] * plane.Normal[0] + p[1] * plane.Normal[1] +
                                      p[2] * plane.Normal[2]); //computing intersection
                    IntersectionIndex = BaseMesh.addNegVertex(f->x - t * p[0], f->y - t * p[1], f->z - t * p[2]);
                    ComputedEdges.write(f_index, *it, IntersectionIndex);
                }
                OutputIndices.push_back(IntersectionIndex);
                ClippedVertices.push_back(IntersectionIndex);
            }
        } else { //ONTO
            OutputIndices.push_back(f_index);
            ClippedVertices.push_back(f_index);
        }

        f = s;
        f_index = *it;
        if (*it == start) break;
    }

    int res = 0;
    if (isOutside && isInside) { //ONTO (current face lays in the clipping plane)
        //mark = ONTO;
        if (mark != ONTO_C) {
            if (SameDirection(plane.Normal, Normal)) { //check for side of the face relatively to the cutting plane
                if (mark != CLIPPED) mark = INSIDE; //this may double the closing face, if it's not 100% equal to the clipping face
            } else { //current face touches the cutting plane from outside
                mark = OUTSIDE; //used to be marked so
                //if (mark != CLIPPED) mark = ONTO_I;
                //else mark = ONTO_C;
            }
        }
    } else if (isOutside) {
        if (ClippedVertices.size() >= 2 && mark != CLOSED) {
            //printf("Edge touch from OUTSIDE: %d (=2 is ok)\n", OutputIndices.size());
            mark = ONTO_C;
        } else mark = OUTSIDE;
    } else if (isInside) {
        //in case face touches clipper with an edge:
        if (ClippedVertices.size() >= 2 && mark != ONTO_C && mark != ONTO_I) { //pray for it's == 2, not more (maybe 3+ if 3+ points lay on same straight)
            //printf("Edge touch: %d (=2 is ok)\n", ClippedVertices.size());
            OpenEdges.add(ClippedVertices.front(), ClippedVertices.back());
            OpenEdges.add(ClippedVertices.back(), ClippedVertices.front());
        }
        if (mark != CLIPPED && mark != ONTO_C && mark != ONTO_I) mark = INSIDE;
    } else {
        if (mark != ONTO_C && mark != ONTO_I) {
            mark = CLIPPED;
            OpenEdges.add(ClippedVertices.front(), ClippedVertices.back());
            OpenEdges.add(ClippedVertices.back(), ClippedVertices.front());
        } else mark = ONTO_C;
    }
    //printf("Leave Clip"); this->print(OutputIndices);
}

void AFace::inverseClipping(AMesh &mesh, list<int> &ClippedInds) {
    //printf("\nEnterInvers "); this->print();
    //printf("Clipped "); this->print(ClippedInds);
    int start = *Indices.begin(), f = start, s;
    AVertex *sv, *fv;
    fv = &mesh.getVertex(f);
    vector<vector<float>> vecs(ClippedInds.size(), vector<float>(3));
    float as[3]; //for vector a-s
    int i=0;
    for (auto it = ClippedInds.begin(); i<ClippedInds.size(); i++, it++) vectorAB(mesh.getVertex(*it), *fv, &vecs[i][0]);
    map<int, list<int>::iterator> LinksFromIndicesToClipped;

    for (auto it = Indices.begin();;) {
        if (++it == Indices.end()) it = Indices.begin();
        s = *it;
        sv = &mesh.getVertex(s);
        if (sv->mark == ONTO) {
            auto itt = find(ClippedInds.begin(), ClippedInds.end(), s);
            LinksFromIndicesToClipped[s] = itt;
        }
        i=0;
        //printf("Edge %d\n", s);
        for (auto iter = ClippedInds.begin(); i<ClippedInds.size(); i++, iter++) {
            int a = *iter;
            //printf("Checking: %d\n", *iter);
            vectorAB(mesh.getVertex(a), *sv, as);
            bool tmp = isParallel(&vecs[i][0], as);
            if (tmp && f!=a && s!=a) {
                Indices.insert(it, a);
                LinksFromIndicesToClipped[a] = iter;
            }
            //printf("Test "); this->print();
            vecs[i][0] = as[0]; vecs[i][1] = as[1]; vecs[i][2] = as[2];
        }
        f = s;
        if (s == start) break;
    }
    /*printf("Marks Face: "); for (int k : Indices) {
        AVertex &v = mesh.getVertex(k);
        printf("%+.2d ", v.mark);
    }
    printf("\nMixed "); this->print();*/

    if (LinksFromIndicesToClipped.empty()) { //special case TODO (triangle it)
        printf("Special Case Error\n");
        panic("inverseClipping", "Special Case Error");
    }

    list<int> Face, WriteLaterFace, *CurrentNewFace = &WriteLaterFace;
    for (auto it = Indices.begin();;) {
        f = *it;
        //printf("Working on: %d\n", f);
        fv = &mesh.getVertex(f);
        if (fv->mark == OUTSIDE) {
            CurrentNewFace->push_back(f);
            int plus, minus;
            auto iter = it; ++iter;
            for (;;) { //moving forward
                if (iter == Indices.end()) iter = Indices.begin();
                //printf("+Checking: %d\n", *iter);
                if (mesh.getVertex(*iter).mark == OUTSIDE) {
                    CurrentNewFace->push_back(*iter);
                    iter = Indices.erase(iter);
                }
                else { //CLIPPED and ONTO
                    plus = *iter;
                    break;
                }
            }
            for (iter = it;;) { //moving backward
                if (iter == Indices.begin()) iter = Indices.end();
                --iter;
                //printf("-Checking: %d\n", *iter);
                if (mesh.getVertex(*iter).mark == OUTSIDE) {
                    CurrentNewFace->push_front(*iter);
                    iter = Indices.erase(iter);
                } else { //CLIPPED and ONTO
                    minus = *iter;
                    CurrentNewFace->push_front(minus);
                    break;
                }
            }
            //printf("Bounds finished\n");
            Indices.erase(it); //we delete all OUTSIDE points from Indices
            for (auto t = LinksFromIndicesToClipped[plus];;) { //adding clipped edges
                if (*t == minus) break;
                else CurrentNewFace->push_back(*t);
                if (t == ClippedInds.begin()) t = ClippedInds.end();
                --t;
            }
            //new polygon is ready:
            it = Indices.begin();
            if (CurrentNewFace == &WriteLaterFace) CurrentNewFace = &Face;
            else {
                AFace &tmp = mesh.addFace(*this, Face);
                Face.clear();
                //printf("Added new face in inverseClipping "); tmp.print();
                tmp.makeConvex(mesh, false);
            }
        } else ++it;
        if (it == Indices.end()) break; //breaks only if none OUTSIDE points were detected
    }
    Indices = WriteLaterFace; //saving new face replacing the old one
    //printf("Finished inverseClipping "); this->print();
    //printf("     Call for makeConvex from inverseClipping\n");
    this->makeConvex(mesh, false);
}

void AFace::makeConvex(AMesh &mesh, bool isTmp) {
    list<int> Face;
    //printf("EnterConvex:"); this->print();
    auto it = Indices.begin();
    AVertex *back = &mesh.getVertex(*it), *front;
    AVertex *v = &mesh.getVertex(*(++it));
    auto last = it;
    auto start = ++it;
    float vf[3], vb[3], vm[3]; //vectors
    for (;;) {
        if (it == Indices.end()) it = Indices.begin();
        front = &mesh.getVertex(*it);
        //printf("Working on front %d\n", *it);
        vectorAB(*v, *front, vf);
        vectorAB(*v, *back, vb);
        if (!isAngleConvex(vf, vb, Normal)) {
            //printf("Detected non-convex!\n");
            for (auto iter = Indices.begin();;) {
                AVertex *m = &mesh.getVertex(*iter);
                //printf("Checking: %d\n", *iter);
                vectorAB(*v, *m, vm);
                if (m!=v && m!=back && m!=front && (isAngleConvex(vf, vm, Normal) || isAngleConvex(vm, vb, Normal))) {
                    for (auto t = iter;; t++) {
                        if (t == Indices.end()) t = Indices.begin();
                        Face.push_back(*t);
                        if (t == last) break;
                    }
                    AFace *res;
                    if (isTmp) res = &mesh.addTmpFace(*this, Face);
                    else res = &mesh.addFace(*this, Face);
                    //printf("Res:"); res->print();
                    res->makeConvex(mesh, isTmp);
                    for (auto t = ++iter;;) {
                        if (t == Indices.end()) t = Indices.begin();
                        if (t != last) t = Indices.erase(t);
                        else break;
                    }
                    //printf("This:"); this->print();
                    this->makeConvex(mesh, isTmp);
                    return;
                }
                if (++iter == Indices.end()) {
                    break;
                    //mesh.dprint();
                    //panic("makeConvex", "Failed to find suitable point");
                }
            }
        }
        back = v;
        v = front;
        last = it;
        if (++it == start) break;
    }
    //printf("Finished makeConvex\n");
}

void AFace::draw(AMesh &mesh, list<int> &Inds) {
    glBegin(GL_POLYGON);
    glColor3ub(150, 150, 150);
    glNormal3f(Normal[0], Normal[1], Normal[2]);
    for (int k : Inds) {
        AVertex &v = mesh.getVertex(k);
        glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
    glBegin(GL_LINE_LOOP);
    glColor3ub(255, 255, 255);
    for (int k : Inds) {
        AVertex &v = mesh.getVertex(k);
        glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
    glBegin(GL_POINTS);
    for (int k : Inds) {
        AVertex &v = mesh.getVertex(k);
        if (v.mark == NORMAL) glColor3ub(255, 255, 255);
        if (v.mark == CLIPPED) glColor3ub(255, 0, 0);
        if (v.mark == OUTSIDE) glColor3ub(0, 255, 0);
        if (v.mark == INSIDE) glColor3ub(0, 0, 255);
        if (v.mark == ONTO) glColor3ub(150, 0, 0);
        glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
}

void AFace::print() {
    printf("Face: ");
    for (int k : Indices) printf("%+.2d ", k);
    printf("   mark = %d", mark);
    printf("\n");
}

void AFace::print(list<int> Inds) {
    printf("Face: ");
    for (int k : Inds) printf("%+.2d ", k);
    printf("   mark = %d", mark);
    printf("\n");
}

void AFace::printN() {
    printf("Normal: %.2f %.2f %.2f\n", Normal[0], Normal[1], Normal[2]);
}

void AFace::setNormals(AMesh &mesh) {
    auto it = Indices.begin();
    d = -Dot(Normal, mesh.getVertex(*it));
}

void AFace::resetNormals(AMesh &mesh) {
    auto it = Indices.begin();
    int a = *it; ++it;
    int b = *it; ++it;
    int c = *it;
    getNormal(mesh.getVertex(a), mesh.getVertex(b), mesh.getVertex(c), Normal);
    normalize(Normal);
    d = -Dot(Normal, mesh.getVertex(a));
}