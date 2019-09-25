#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <windows.h>
#include <vector>
#include <sstream>
#include <string>
#include "AMesh.h"
#include <fstream>

using namespace std;

//OUTSIDE - outside of clipping plane
//DEAD - inside of clipping plane
//NORMAL - not processed yet

//Variables for visual part:
GLFWwindow *window;
int WWidth = 800, WHeight = 600;
bool isFrame = 0;
bool lightOn = 1;
bool isPaused = false;
double PCFreq = 0.0;
__int64 CounterStart = 0;
double rotate_x, rotate_y, rotate_z;
double scaleX=1, scaleY=1, scaleZ=1;
double moveX, moveY, moveZ;
GLfloat L0_diff[4] = {0.98, 0.98, 0.76, 1.0};
GLfloat L0_amb[4] = {0.01, 0.01, 0.01, 1.0};
GLfloat L0_pos[4] = {10000, 5000, 5000, 0};
GLfloat L1_diff[4] = {1.0, 1.0, 1.0, 1.0};
GLfloat L1_amb[4] = {0.01, 0.01, 0.01, 1.0};
GLfloat L1_pos[4] = {10000, 5000, 5000, 0};
unsigned char BaseColor[3] = {120, 120, 140};

//Other variables:
bool toClip = false, drawCutter = true, drawBase = false;
bool moveCamera = false;
double xpos, ypos, xposL=0, yposL=0;
double CameraSensitivity = 5.0;
AMesh cutter;
bool doAnim = true;
float Scale = 1; //1 in obj = 1mm [1 default]


void window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(130, 1, 50, 0);
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //gluLookAt(150, 150, 150, 0, 0, 0, 0, 100, 0);
    //glFrustum (-100, 100, -100, 100, 0, 1000);
    glOrtho(-width/3, width/3, -height/3, height/3, -10000, 10000.0);
    //glRotatef( 35.26-90, 1.0, 0.0, 0.0 );
    //glRotatef( -180+45, 0.0, 0.0, 1.0 );
    glRotatef( 35.26-90, 1.0, 0.0, 0.0 );
    glRotatef( -180+45, 0.0, 0.0, 1.0 );
}

void LightInit() {
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, L0_diff);
    glLightfv(GL_LIGHT0, GL_POSITION, L0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, L0_amb);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

void turnLight() {
    if (lightOn) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        lightOn = !lightOn;
        turnLight();
    }
    if (key == GLFW_KEY_SPACE  && action == GLFW_PRESS) doAnim = !doAnim;
    if (key == GLFW_KEY_LEFT) cutter.move(0,-3,0);
    if (key == GLFW_KEY_RIGHT) cutter.move(0,3,0);
    if (key == GLFW_KEY_UP) cutter.move(-3,0,0);
    if (key == GLFW_KEY_DOWN) cutter.move(3,0,0);
    if (key == GLFW_KEY_Q) cutter.move(0,0,3);
    if (key == GLFW_KEY_E) cutter.move(0,0,-3);
    if (key == GLFW_KEY_C && action == GLFW_PRESS) toClip = true;
    if (key == GLFW_KEY_T && action == GLFW_PRESS) drawCutter = !drawCutter;
    if (key == GLFW_KEY_B && action == GLFW_PRESS) drawBase = !drawBase;
    if (key == GLFW_KEY_W) {
        moveX-=3;
        moveY-=3;
    }
    if (key == GLFW_KEY_S) {
        moveX+=3;
        moveY+=3;
    }
    if (key == GLFW_KEY_A) {
        moveX+=3;
        moveY-=3;
    }
    if (key == GLFW_KEY_D) {
        moveX-=3;
        moveY+=3;
    }
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        isFrame = !isFrame;
        if (isFrame == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }/*
    if (key == GLFW_KEY_Q) {
        scaleX+=0.1; scaleY+=0.1; scaleZ+=0.1;
    }
    if (key == GLFW_KEY_E) {
        scaleX-=0.1; scaleY-=0.1; scaleZ-=0.1;
    }*/
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            moveCamera = true;
            glfwGetCursorPos(window, &xposL, &yposL);
        } else if (action == GLFW_RELEASE) {
            moveCamera = false;
        }
    }
}

void scroll_callback(GLFWwindow* window, double x, double y) {
    scaleX += y/CameraSensitivity;
    scaleY += y/CameraSensitivity;
    scaleZ += y/CameraSensitivity;
}

void drawCoords() {
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3ub(0,255,0);
    glVertex3f(0.0, 0.0, 0.0);
    glColor3ub(0,255,0); //green Y
    glVertex3f(0.0, 100000.0, 0.0);
    glColor3ub(0,0,255);
    glVertex3f(0.0, 0.0, 0.0);
    glColor3ub(0,0,255); //blue Z
    glVertex3f(0.0, 0.0, 100000.0);
    glColor3ub(255,0,0);
    glVertex3f(0.0, 0.0, 0.0);
    glColor3ub(255,0,0); //red X
    glVertex3f(100000.0, 0.0, 0.0);
    glEnd();
    turnLight();
}

void StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
        cout << "QueryPerformanceFrequency failed!\n";

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}
double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}

int init() {
    if(!glfwInit()) {
        cout << "Init error" << endl;
        return -1;
    }
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    window = glfwCreateWindow( WWidth, WHeight, "CamVis", NULL, NULL);
    if (!window) {
        cout << "Creation error" << endl;
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, window_size_callback);
    window_size_callback(window, WWidth, WHeight);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    //glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glLineWidth(5.0f);
    glPointSize(7.0f);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glfwSwapInterval(1);

    glLineWidth(2);
    LightInit();
}

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

float start[3] = {0,0,60};
float target[3]  = {0,0,60};
float current[3] = {0,0,60};
float C[3], V[3], R, current_rad, target_rad;
AMesh cutterVis;

bool eq(float a, float b) {
    //printf("Compare: %f and %f\n", a, b);
    float epsilon = 0.2;
    float ab = a - b;
    return (ab < epsilon && ab > -epsilon);
}

bool G00(AMesh &mesh, float mm_per_frame) {
    //printf("Doing G00\n");
    bool finished = true;
    for (int i=0; i<3; i++) {
        current[i] += V[i]*mm_per_frame;
        if (!eq(current[i], target[i])) finished = false;
    }
    mesh.move(V[0]*mm_per_frame, V[1]*mm_per_frame, V[2]*mm_per_frame);
    return finished;
}

bool G01(AMesh &mesh, float mm_per_frame, int &frame, AMesh &VisMesh) {
    bool finished = true;
    for (int i=0; i<3; i++) {
        current[i] += V[i]*mm_per_frame;
        if (!eq(current[i], target[i])) finished = false;
    }
    cutter.move(V[0]*mm_per_frame, V[1]*mm_per_frame, V[2]*mm_per_frame);

    if (frame%30 == 0) {
        frame = 0;
        printf("Modding the cutter: %.2f %.2f %.2f -> %.2f %.2f %.2f\n", start[0], start[1], start[2], current[0], current[1], current[2]);
        cutterVis.modCutter(cutter, start, current);
        printf("Copying...\n");
        VisMesh.copyFrom(mesh);
        printf("Trying to clip...\n");
        VisMesh.clipByMesh(cutterVis);
        printf("Clipped!\n");
    }
    if (finished) {
        printf("Modding cutter for clipping on real mesh...\n");
        cutterVis.modCutter(cutter, start, current);
        printf("Clipping the real mesh...\n");
        mesh.clipByMesh(cutterVis);
        printf("Clipped the real mesh!\n");
    }
    return finished;
}

bool G02(AMesh &mesh, float rad_per_frame, int &frame) {
}

bool G03(AMesh &mesh, float rad_per_frame, int &frame) {
}

int readCommand(string in) {
    cout << "Reading: " << in << endl;
    vector<string> com;
    split(in, ' ', com);
    for (int i=1; i<com.size(); i++) {
        string &s = com[i];
        if (s[0] == 'X') target[0] = stof(s.substr(1), nullptr);
        if (s[0] == 'Y') target[1] = stof(s.substr(1), nullptr);
        if (s[0] == 'Z') target[2] = stof(s.substr(1), nullptr);
        if (s[0] == 'I') V[0] = stof(s.substr(1), nullptr);
        if (s[0] == 'J') V[1] = stof(s.substr(1), nullptr);
        if (s[0] == 'K') V[2] = stof(s.substr(1), nullptr);
    }
    int ret = -2;
    if (com[0] == "G00") ret = 0;
    if (com[0] == "G01") ret = 1;
    if (com[0] == "G02") ret = 2;
    if (com[0] == "G03") ret = 3;
    if (com[0] == "M30") ret = -1;
    if (ret == 0 || ret == 1) {
        for (int i=0; i<3; i++) {
            V[i] = target[i] - start[i];
        }
        norm(V);
    }
    if (ret == 2 || ret == 3) {
        R = length(V);
        for (int i=0; i<3; i++) {
            C[i] = V[i] + start[i]; //center of the radius
            V[i] = -V[i];
            current[i] = target[i] - C[i];
        }
        current_rad = Dot(V, current);
    }
    return ret;
}

vector<string> loadComms(const char *path) {
    vector<string> ret;

    ifstream file(path);
    string str;
    while (getline(file, str)) {
        ret.push_back(string(str));
    }
    return ret;
}

int main() {
    AMesh BaseMesh, VisMesh, *mesh;
    bool commandFinished = true;
    int Gcode = 0;
    BaseMesh.loadObj("D:\\Study\\Kursach\\models\\base_100_100_50.obj", 1);
    //BaseMesh.loadObj("D:\\Study\\Kursach\\models\\Sculpt1.obj", 1);
    //cutter.loadObj("D:\\Study\\Kursach\\models\\tool_12.obj", 1);
    cutter.loadObj("D:\\Study\\Kursach\\models\\sphere.obj", 1);
    cutter.move(0, 0, 60);
    //string coms[] = {"G00 Y50", "G00 X60", "G00 Z25", "G01 X-50", "G01 Y-50", "M30"};
    vector<string> coms = loadComms("D:\\Study\\Kursach\\models\\M30.txt");
    printf("Commands:\n");
    for (string s : coms) printf("%s\n", s.c_str());
    int Icomm = 0, frame = 1;

    VisMesh.copyFrom(BaseMesh);

    init();
    double FPSlimit = 60.0;
    double FrameTime = 0, tmp = 0, FrameTimeSum=0.0;
    int FramesCounter = 0;
    StartCounter();

    do{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (toClip) {
            BaseMesh.clipByMesh(cutter);
            toClip = false;
            BaseMesh.dprint();
            BaseMesh.print();
        }
        if (doAnim) {
            if (commandFinished) {
                for (int i=0; i<3; i++) start[i] = target[i];
                Gcode = readCommand(coms[Icomm]);
                Icomm++;
                commandFinished = false;
            }
            if (Gcode == 0) {
                commandFinished = G00(cutter, 0.2);
            } else if (Gcode == 1) {
                commandFinished = G01(BaseMesh, 0.2, frame, VisMesh);
                frame++;
            } else if (Gcode == 2) {
                commandFinished = G02(BaseMesh, 0.01, frame);
                frame++;
            } else if (Gcode == 3) {
                commandFinished = G03(BaseMesh, 0.01, frame);
                frame++;
            }
        }

        if (moveCamera) {
            glfwGetCursorPos(window, &xpos, &ypos);
            rotate_z += (xpos - xposL)/CameraSensitivity;
            xposL = xpos;
            rotate_x -= (ypos - yposL)/CameraSensitivity;
            yposL = ypos;
        }

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glScalef(scaleX, scaleY, scaleZ);
        glTranslatef(moveX, moveY, moveZ);
        glRotatef( rotate_x, 1.0, -1.0, 0.0 );
        glRotatef( rotate_z, 0.0, 0.0, 1.0 );
        if (drawBase) BaseMesh.draw(BaseColor);
        else VisMesh.draw(BaseColor);
        if (drawCutter) cutter.draw(BaseColor);
        if (!doAnim) cutterVis.draw(BaseColor);
        drawCoords();

        tmp = GetCounter();
        FrameTime = tmp - FrameTime;
        if (FrameTime < 2000.0/FPSlimit) Sleep((int)round(2000.0/FPSlimit - FrameTime)); //FPS doubles for some reason
        FramesCounter++;
        FrameTimeSum += FrameTime;
        if (FramesCounter >= 60) {
            cout << "FPS: " << 60000.0/FrameTimeSum << endl;
            FramesCounter = 0;
            FrameTimeSum = 0;
        }
        FrameTime = tmp;

        glfwSwapBuffers(window);
        //glfwWaitEvents();
        glfwPollEvents();
    }
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}