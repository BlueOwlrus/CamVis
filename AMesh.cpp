//
// Created by Fis on 09.12.2016.
//

#include "AMesh.h"
#include "AFace.h"
#include "AVertex.h"

using namespace std;

AVertex &AMesh::getVertex(int i) {
    if (i >= 0) return *Vertices[i];
    else return *tmpVertices[abs(i)-1];
}

void AMesh::addVertex(float a, float b, float c, float Scale) {
    Vertices.push_back(new AVertex(Scale*a, Scale*b, Scale*c));
}

int AMesh::addVertex(AVertex *in) {
    Vertices.push_back(in);
    return (int)Vertices.size()-1;
}

int AMesh::addNegVertex(float a, float b, float c) {
    //mark it CLIPPED, returns index
    tmpVertices.push_back(new AVertex(a, b, c, CLIPPED));
    int res = (int)tmpVertices.size();
    return -res;
}

AFace &AMesh::addTmpFace(AFace &base) {
    AFace *fp = new AFace();
    tmpFaces.push_back(fp);
    for (int i=0; i<3; i++) fp->Normal[i] = base.Normal[i];
    fp->d = base.d;
    fp->mark = CLOSED_N;
    return *fp;
} //marked CLOSED_N, used in closeMesh

AFace &AMesh::addTmpFace(AFace &base, list<int> &points) {
    AFace *fp = new AFace();
    tmpFaces.push_back(fp);
    AFace &face = *fp;
    face.Indices = points;
    for (int i=0; i<3; i++) face.Normal[i] = base.Normal[i];
    face.d = base.d;
    face.mark = base.mark;
    return face;
} //used in makeConvex

AFace &AMesh::addFace(AFace &base, list<int> &points) {
    AFace *fp = new AFace();
    Faces.push_back(fp);
    AFace &face = *fp;
    face.Indices = points;
    for (int i=0; i<3; i++) face.Normal[i] = base.Normal[i];
    face.d = base.d;
    face.mark = base.mark;
    return face;
} //used in inverseClipping and makeConvex

void AMesh::addFace(vector<int> &points) {
    AFace *fp = new AFace();
    Faces.push_back(fp);
    AFace &face = *fp;
    for (int p : points) face.Indices.push_back(p);
    getNormal(*Vertices[points[0]], *Vertices[points[1]], *Vertices[points[2]], face.Normal);
    normalize(face.Normal);
    face.d = -Dot(face.Normal, *Vertices[points[0]]);
} //used in loadObj

void AMesh::addFace(AFace *f) {
    Faces.push_back(f);
} //used in clipByMesh (final part)

void AMesh::move(float x, float y, float z) {
    for (AVertex *v : Vertices) {
        v->x += x;
        v->y += y;
        v->z += z;
    }
    Center[0] += x; Center[1] += y; Center[2] += z;
    for (AFace *f : Faces) f->setNormals(*this);
}

void AMesh::clipByMesh(AMesh &clipper) { //TODO do not start closeMesh from vertices that belong to face which lays in clipping plane
    for (AFace *cl : clipper.Faces) clipByPlane(*cl);
    //dprint();
    for (int i=0 ,n = Faces.size(); i<n; i++) { //'n' is needed because size of Faces can vary during the cycle
        AFace &f = *Faces[i];
        if (f.mark == INSIDE || f.mark == ONTO_I) f.mark = DEAD;
        if (f.mark == CLIPPED || f.mark == ONTO_C) f.inverseClipping(*this, *Indices[i]);
    }
    for (int i=0, n = tmpFaces.size(); i<n; i++) {
        AFace &f = *tmpFaces[i];
        if (f.mark == CLIPPED || f.mark == ONTO_C) f.inverseClipping(*this, *tmpIndices[i]);
        else if (f.mark == CLOSED) f.Indices = *tmpIndices[i];
    }

    map<int, int> TranslateTable;
    vector<bool> toDelete(tmpVertices.size(), true);

    for (auto it = Faces.begin(); it != Faces.end();) {
        if ((*it)->mark == DEAD) {
            delete *it;
            it = Faces.erase(it);
        }
        else {
            (*it)->mark = NORMAL;
            for (int &k : (*it)->Indices) if (k<0) {
                    int &p = TranslateTable[k];
                    if (p == 0) {
                        p = addVertex(&getVertex(k));
                        //printf("%d -> %d\n", k, p); //dprint
                        toDelete[-(k+1)] = false;
                    }
                    k = p;
                }
            ++it;
        }
    }
    for (auto it = tmpFaces.begin(); it != tmpFaces.end(); ++it) {
        AFace &f = **it;
        if (f.mark == CLIPPED || f.mark == CLOSED || f.mark == ONTO_C) {
            for (int &k : f.Indices) if (k<0) {
                    int &p = TranslateTable[k];
                    if (p == 0) {
                        p = addVertex(&getVertex(k));
                        //printf("%d -> %d\n", k, p); //dprintf
                        toDelete[-(k+1)] = false;
                    }
                    k = p;
                }
            (*it)->mark = NORMAL;
            addFace(*it);
        } else delete *it;
    }
    for (AVertex *v : Vertices) v->mark = NORMAL;
    for (int i=0; i<tmpVertices.size(); i++) if (toDelete[i]) delete tmpVertices[i];
    for (auto lst : Indices) delete lst;
    for (auto lst : tmpIndices) delete lst;
    Indices.clear();
    tmpIndices.clear();
    tmpVertices.clear();
    tmpFaces.clear();
}

void AMesh::clipByPlane(AFace &plane) {
    //printf("Cutting Plane:\n");
    //plane.print(); plane.printN();
    //printf("Normal: %.2f %.2f %.2f   d = %.2f\n", plane.Normal[0], plane.Normal[1], plane.Normal[2], plane.d);
    float epsilon = 0.001;
    for (AVertex *vp : Vertices) { //marking vertices
        AVertex &v = *vp;
        v.d = Dot(plane.Normal, v) + plane.d; //side relatively to the plane
        if (v.d < epsilon && v.d > -epsilon) v.d = 0.0; //v.d updates with every new plane
        if (v.d > 0) v.mark = OUTSIDE; //v.mark saves for the whole cycle of clipping
        else if (v.d < 0 && v.mark == NORMAL) v.mark = INSIDE;
        else if(v.d == 0 && v.mark != OUTSIDE) v.mark = ONTO;
    }
    for (AVertex *vp : tmpVertices) { //marking tmp vertices
        AVertex &v = *vp;
        v.d = Dot(plane.Normal, v) + plane.d;
        if (v.d < epsilon && v.d > -epsilon) v.d = 0.0;
        if (v.d > 0) v.mark = OUTSIDE;
        else if (v.d < 0 && v.mark == NORMAL) v.mark = INSIDE;
        else if(v.d == 0 && (v.mark==NORMAL || v.mark==INSIDE)) v.mark = ONTO; //it differs from Vertices cycle because tmp vertex may be CLIPPED
    }

    SparseTable ComputedEdges;
    MapOfList OpenEdges;
    list<AFace*> DeleteIt; //TODO delete it
    for (int i=0; i<Faces.size(); i++) {
        AFace &f = *Faces[i];
        list<int> out;
        if (Indices.size() < Faces.size()) Indices.push_back(new list<int>());
        if (f.mark == NORMAL) *Indices[i] = f.Indices;
        //if (f.mark == CLOSED_N) {
        //    *Indices[i] = f.Indices;
        //    f.mark = CLOSED;
        //}
        //char mrk = f.mark;
        if (f.mark != OUTSIDE) f.clipByPlane(*this, plane, OpenEdges, ComputedEdges, *Indices[i], out, DeleteIt);
        if (f.mark == CLIPPED || f.mark == ONTO_C) *Indices[i] = out;
        //if (mrk == CLOSED) f.mark = CLOSED;
    }
    for (int i=0; i<tmpFaces.size(); i++) {
        AFace &f = *tmpFaces[i];
        list<int> out;
        if (tmpIndices.size() < tmpFaces.size()) tmpIndices.push_back(new list<int>());
        if (f.mark == NORMAL) *tmpIndices[i] = f.Indices;
        if (f.mark == CLOSED_N) {
            *tmpIndices[i] = f.Indices;
            f.mark = CLOSED;
        }
        char mrk = f.mark;
        if (f.mark != OUTSIDE) f.clipByPlane(*this, plane, OpenEdges, ComputedEdges, *tmpIndices[i], out, DeleteIt);
        if (f.mark == CLIPPED || f.mark == ONTO_C) *tmpIndices[i] = out;
        if (mrk == CLOSED && f.mark != OUTSIDE) f.mark = CLOSED;
    }
    /*
    printf("Vertices:\n");
    for (int i=0; i<Vertices.size(); i++) {
        AVertex &v = *Vertices[i];
        printf("%.2d: %.2f %.2f %.2f   %d\n", i, v.x, v.y, v.z, v.mark);
    }
    printf("tmpVertices:\n");
    for (int i=0; i<tmpVertices.size(); i++) {
        AVertex &v = *tmpVertices[i];
        printf("%.2d: %.2f %.2f %.2f   %d\n", -i-1, v.x, v.y, v.z, v.mark);
    }
    printf("Computed Edges: \n");
    for (auto it = ComputedEdges.tbl.begin(); it != ComputedEdges.tbl.end(); ++it) {
        for (auto iter = it->second.begin(); iter != it->second.end(); ++iter) {
            printf("%d-(%.2d)-%d;  ", it->first, iter->second, iter->first);
        }
        printf("\n");
    }*/
    //printf("Before closeMesh "); plane.printN();
    closeMesh(OpenEdges, plane);
}

void AMesh::closeMesh(MapOfList &OpenEdges, AFace &clipper) {
    //printf("\nOpenEdges:\n");
    /*for (auto it = OpenEdges.tbl.begin(); it != OpenEdges.tbl.end();) {
        if (it->second.size() !=2 ) {
            //printf("Size of list isn't 2   ");
            //printf("%.2d:  ", it->first);
            //for (int k : it->second) printf("%.2d , ", k);
            //printf("\n");
            //it =  OpenEdges.tbl.erase(it);
            ++it;
            //panic("closeMesh", "Size of list isn't 2");
        } else {
            //printf("%.2d , %.2d , %.2d\n", it->second.front(), it->first, it->second.back());
            ++it;
        }
    }*/
    while (!OpenEdges.tbl.empty()) {
        AFace &f = addTmpFace(clipper);
        auto current_it = OpenEdges.tbl.begin();
        int next, prev = current_it->first, start = current_it->first;
        for (;;) {
            f.Indices.push_back(current_it->first);
            if (current_it->second.empty()) panic("closeMesh", "Reached point with empty list");
            next = current_it->second.back();
            if (next == prev) { //if it points back to first
                next = current_it->second.front();
            }
            prev = current_it->first;
            OpenEdges.tbl.erase(current_it);
            if (next == start) break; //if face is finished
            //f.Indices.push_back(next);
            current_it = OpenEdges.tbl.find(next);
        }
        if (f.Indices.size() < 3) {
            f.Indices.clear();
            continue;
        }
        if (f.Indices.size() < 3) panic("closeMesh", "Size of f.Indices is less than 3");
        for (auto tmp = f.Indices.begin(), t = tmp;; ++tmp) {
            int n[3];
            t = tmp;
            for (int i=0; i<3; i++, ++t) n[i] = *t;
            if (getNormal(getVertex(n[0]), getVertex(n[1]), getVertex(n[2]), f.Normal)) break;
        }
        if (SameDirection(clipper.Normal, f.Normal)) {
            //printf("Reversed\n");
            f.Indices.reverse();
            for (int i=0; i<3; i++) f.Normal[i] = -f.Normal[i];
        }
        //printf("Closed "); f.print();
        f.makeConvex(*this, true);
    }
    //printf("Finished closeMesh\n\n");
}

void AMesh::print() {
    for (AVertex *v : Vertices) printf("v  %.3f %.3f %.3f\n", v->x, v->y, v->z);
    for (AFace *f : Faces) {
        printf("f ");
        for (int i : f->Indices) printf("%d ", i+1);
        printf("\n");
    }
}

void AMesh::dprint() {
    printf("Vertices:\n");
    for (int i=0; i<Vertices.size(); i++) {
        AVertex *v = Vertices[i];
        printf("%d:  %.2f %.2f %.2f\n", i, v->x, v->y, v->z);
    }
    for (int i=0; i<tmpVertices.size(); i++) {
        AVertex *v = tmpVertices[i];
        printf("%d:  %.2f %.2f %.2f\n", -i-1, v->x, v->y, v->z);
    }
    printf("Faces:\n");
    for (int i=0; i<Faces.size(); i++) {
        printf("%d ", i);
        Faces[i]->print();
    }
    for (int i=0; i<tmpFaces.size(); i++) {
        printf("%d ", -i-1);
        tmpFaces[i]->print();
    }
    printf("\n");
}

void AMesh::reset() {
    for (AVertex *v : Vertices) v->mark = NORMAL;
    tmpVertices.clear();
    tmpFaces.clear();
}

bool AMesh::loadObj(const char *path, float Scale) {
    FILE * file = fopen(path, "r");
    if( file == NULL ) return false;

    bool read = true;
    char lineHeader[128];

    for (;;) {
        int res = 1;
        if (read) res = fscanf(file, "%s", lineHeader);
        read = true;
        if (res == EOF) break;
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            addVertex(vertex.x, vertex.y, vertex.z, Scale);
        } else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            //texture coords
        } else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            //normals
        } else if (strcmp(lineHeader, "f") == 0) {
            vector<int> vertexIndex;
            //int uvIndex[3], normalIndex[3];
            for (;;) {
                fscanf(file, "%s", lineHeader);
                if (!isdigit(lineHeader[0])) {
                    read = false;
                    break;
                }
                int count = 0;
                string str;
                int number;
                for (char a : lineHeader) {
                    if (a == '/') {
                        if (!str.empty()) number = stoi(str) - 1;
                        else number = -1;
                        if (count == 0) vertexIndex.push_back(number);
                        //if (count == 1) uvIndex[i] = number;
                        count++;
                        str.clear();
                    }
                    str += a;
                }
                if (!str.empty()) number = stoi(str) - 1;
                else number = -1;
                if (count == 0) vertexIndex.push_back(number);
                //if (count == 2) normalIndex[i] = number;
            }
            addFace(vertexIndex);
            //texture coords here
            //normals here
        }
    }
    return true;
}

void AMesh::draw(unsigned char Color[]) {
    float Nl = 100.0;
    for (AFace *face : Faces) {
        if (face->mark == DEAD) continue;
        float mid[3] = {0,0,0};
        glBegin(GL_POLYGON);
        glColor3ub(Color[0], Color[1], Color[2]);
        glNormal3f(face->Normal[0], face->Normal[1], face->Normal[2]);
        for (int k : face->Indices) {
            AVertex *v = Vertices[k];
            mid[0]+=v->x; mid[1]+=v->y; mid[2]+=v->z;
            glVertex3f(Vertices[k]->x, Vertices[k]->y, Vertices[k]->z);
        }
        glEnd();

        /*
        int k = face->Indices.size();
        mid[0]/=k; mid[1]/=k; mid[2]/=k;
        glBegin(GL_LINES);
        glColor3ub(0,255,0);
        glNormal3f(mid[0], mid[1], mid[2]);
        for (int i=0; i<3; i++) mid[i] += Nl*face->Normal[i];
        glNormal3f(mid[0], mid[1], mid[2]);
        glEnd();*/
    }
}

void AMesh::drawTmp(unsigned char Color[]) {
    for (AFace *face : Faces) {
        if (face->mark == DEAD) continue;
        glBegin(GL_POLYGON);
        glColor3ub(Color[0], Color[1], Color[2]);
        glNormal3f(face->Normal[0], face->Normal[1], face->Normal[2]);
        for (int k : face->Indices) {
            AVertex &v = getVertex(k);
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }
    for (AFace *face : tmpFaces) {
        if (face->mark == DEAD) continue;
        glBegin(GL_POLYGON);
        glColor3ub(Color[0], Color[1], Color[2]);
        glNormal3f(face->Normal[0], face->Normal[1], face->Normal[2]);
        for (int k : face->Indices) {
            AVertex &v = getVertex(k);
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }
}

void AMesh::modCutter(AMesh &cutter, float *last, float *current) {
    //for (AVertex *v : Vertices) delete v;
    //for (AFace *f : Faces) delete f;
    Vertices.clear();
    Faces.clear();

    for (AVertex *v : cutter.Vertices) {
        addVertex(v->x, v->y, v->z, 1.0);
    }
    AFace plane;
    float vec_back[3];
    for (int i=0; i<3; i++) {
        plane.Normal[i] = current[i] - last[i];
        vec_back[i] = last[i] - current[i];
    }
    AVertex C(cutter.Center[0], cutter.Center[1], cutter.Center[2]);
    plane.d = -Dot(plane.Normal, C);

    map<int, int> mp;
    float epsilon = 0.001;
    for (int i=0; i<Vertices.size(); i++) { //marking vertices
        AVertex &v = *Vertices[i];
        v.d = Dot(plane.Normal, v) + plane.d; //side relatively to the plane
        if (v.d < epsilon && v.d > -epsilon) v.d = 0.0; //v.d updates with every new plane
        if (v.d > 0) v.mark = OUTSIDE; //v.mark saves for the whole cycle of clipping
        else if (v.d < 0) v.mark = INSIDE;
        else if(v.d == 0) v.mark = ONTO;
        mp[i] = 0;
    }

    for (AFace *face : cutter.Faces) {
        bool hasOutside=false, hasInside = false;
        int start = *face->Indices.begin();
        AVertex *f = &getVertex(start);
        int f_i = start, s_i;

        for (auto it = face->Indices.begin();;) {
            f_i = *it;
            if (++it == face->Indices.end()) it = face->Indices.begin();
            AVertex *s = &getVertex(*it);
            s_i = *it;
            if (f->d > 0) {
                mp[f_i] = 1;
                hasOutside = true;
                if (s->d < 0) mp[s_i] = 1;
            } else if (f->d < 0) {
                hasInside = true;
                if (s->d > 0)  {
                    mp[f_i] = 1;
                    mp[s_i] = 1;
                }
            } else mp[f_i] = 1;
            f = s;
            if (*it == start) break;
        }

        if (hasOutside) {
            if (hasInside) {
                list<int> points;
                for (int k : face->Indices) {
                    if (mp[k] == 1) points.push_back(k);
                }
                addFace(*face, points);
            } else addFace(face);
        }
    }

    vector<int> points;
    for (int i=0; i<Vertices.size(); i++) {
        AVertex &v = getVertex(i);
        if (mp[i] == 1 && v.d <= 0.0) {
            points.push_back(i);
            v.x += vec_back[0];
            v.y += vec_back[1];
            v.z += vec_back[2];
        }
    }
    addFace(points);
    for (AFace *f : Faces) f->resetNormals(*this);
    AFace &lastFace = *Faces[Faces.size()-1];
    if (!SameDirection(lastFace.Normal, vec_back)) {
        lastFace.Indices.reverse();
        lastFace.resetNormals(*this);
    }
}

void AMesh::copyFrom(AMesh &base) {
    //for (AVertex *v : Vertices) delete v;
    //for (AFace *f : Faces) delete f;
    Vertices.clear();
    Faces.clear();

    for (AVertex *v : base.Vertices) {
        addVertex(v->x, v->y, v->z, 1.0);
    }
    for (AFace *f : base.Faces) {
        addFace(*f, f->Indices);
    }
    for (int i=0; i<3; i++) Center[i] = base.Center[i];
}