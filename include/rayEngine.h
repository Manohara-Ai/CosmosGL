#ifndef RAY_ENGINE_H
#define RAY_ENGINE_H

#include <iostream>
#include <vector>
#include <memory>
#include <cerrno>
#include <string>
#include <fstream>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using Clock = std::chrono::high_resolution_clock;

using namespace glm;
using namespace std;

inline double lastPrintTime = 0.0;
inline int    framesCount   = 0;
inline double c = 299792458.0;
inline double G = 6.67430e-11;
inline bool Gravity = false;

struct Camera {
    vec3 target = vec3(0.0f, 0.0f, 0.0f);
    float radius = 6.34194e10f;
    float minRadius = 1e10f, maxRadius = 1e12f;

    float azimuth = 0.0f;
    float elevation = M_PI / 2.0f;

    float orbitSpeed = 0.01f;
    float panSpeed = 0.01f;
    double zoomSpeed = 25e9f;

    bool dragging = false;
    bool panning = false;
    bool moving = false; // For compute shader optimization
    double lastX = 0.0, lastY = 0.0;

    vec3 position() const;
    void update();
    void processMouseMove(double x, double y);
    void processMouseButton(int button, int action, int mods, GLFWwindow* win);
    void processScroll(double xoffset, double yoffset);
    void processKey(int key, int scancode, int action, int mods);
};
inline Camera camera;

struct BlackHole {
    vec3 position;
    double mass;
    double radius;
    double r_s;

    BlackHole(vec3 pos, float m);
    bool Intercept(float px, float py, float pz) const;
};
inline BlackHole SagA(vec3(0.0f, 0.0f, 0.0f), 8.54e36);

struct ObjectData {
    vec4 posRadius; // xyz = position, w = radius
    vec4 color;     // rgb = color, a = unused
    float  mass;
    vec3 velocity; // Initial velocity
};
inline vector<ObjectData> objects = {
    { vec4(4e11f, 0.0f, 0.0f, 4e10f),   vec4(1,1,0,1), 1.98892e30f, vec3(0.0f) },
    { vec4(0.0f, 0.0f, 4e11f, 4e10f),   vec4(1,0,0,1), 1.98892e30f, vec3(0.0f) },
    { vec4(0.0f, 0.0f, 0.0f, SagA.r_s), vec4(0,0,0,1), (float)SagA.mass, vec3(0.0f) }
};

struct Engine {
    GLuint gridShaderID;

    GLFWwindow* window;
    GLuint quadVAO;
    GLuint texture;
    GLuint shaderID;
    GLuint computeShaderID;

    GLuint cameraUBO = 0;
    GLuint diskUBO = 0;
    GLuint objectsUBO = 0;

    GLuint gridVAO = 0;
    GLuint gridVBO = 0;
    GLuint gridEBO = 0;
    int gridIndexCount = 0;

    int WIDTH = 800;  
    int HEIGHT = 600; 
    int COMPUTE_WIDTH  = 200;   
    int COMPUTE_HEIGHT = 150;  
    float width = 1e11f; // Width of the viewport in meters
    float height = 7.5e10f; // Height of the viewport in meters

    Engine();
    void generateGrid(const vector<ObjectData>& objects);
    void drawGrid(const mat4& viewProj);
    void drawFullScreenQuad();
    string getFileContents(const char* filename);
    GLuint createShader(const char* vertexFile="resources/shaders/default.vert", const char* fragmentFile="resources/shaders/default.frag");
    GLuint createComputeShader(const char* computeFile);
    void dispatchCompute(const Camera& cam);
    void uploadCameraUBO(const Camera& cam);
    void uploadObjectsUBO(const vector<ObjectData>& objs);
    void uploadDiskUBO();
    vector<GLuint> QuadVAO();
    void renderScene();
};
inline Engine engine;

void setupCameraCallbacks(GLFWwindow* window);

#endif
