/**
 * class Engine
 * brief Orchestrates the OpenGL context, N-body physics simulation, and 3D rendering.
 * * The Engine class acts as the central hub for the solar system simulation. It handles:
 * - Window Management: Initializes GLFW and GLAD context for OpenGL 4.6.
 * - Physics: Implements Newtonian gravitation for all celestial bodies using Euler integration.
 * - Camera Control: Manages an orbital camera with dragging (rotation), panning, and scroll-based zooming.
 * - Scene Graph: Stores and manages unique pointers to Star and Planet objects, ensuring proper GPU resource cleanup.
 * * note Physics calculations use a gravitational constant G = 6.67430e-11 and support time scaling.
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <vector>
#include <memory>
#include <cerrno> 
#include <string> 
#include <fstream> 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef G
#define G 6.67430e-11
#endif

using namespace std;
using namespace glm;

struct Star {
    vec3 position;
    double mass;
    double radius;
    vec3 color;
    double brightness;
    vec3 initialVelocity;
    vector<GLfloat> vertices;
    vector<GLuint> indices;
    GLuint VAO, VBO, EBO;

    Star(vec3 pos, double m, double r, vec3 c, double b, vec3 v);
};

struct Planet {
    vec3 position;
    double mass;
    double radius;
    vec3 color;
    double rotationAngle;
    double rotationSpeed;
    vec3 initialVelocity;
    vector<GLfloat> vertices;
    vector<GLuint> indices;
    GLuint VAO, VBO, EBO;

    Planet(vec3 pos, double m, double r, vec3 c, double rS, vec3 v);
};

struct CelestialBody {
    vec3* position;
    vec3 velocity;
    double mass;

    CelestialBody(vec3* pos, vec3 vel, double m) : position(pos), velocity(vel), mass(m) {}
};

class Engine {
private:
    int WIDTH = 800;
    int HEIGHT = 600;

    vec3 cameraPos = vec3(0.0f, 0.0f, 2.0e7f);
    vec3 focusTarget = vec3(5.0f);
    mat4 projection;
    mat4 view;

    string getFileContents(const char* filename);
    GLuint createShader(const char* vertexFile, const char* fragmentFile);

    GLuint uboWindowData, starShaderID, planetShaderID;

    vector<unique_ptr<Star>> stars;
    vector<unique_ptr<Planet>> planets;
    vector<CelestialBody> bodies;

public:
    float distance = 5.0e10f; 
    float yaw = -90.0f, pitch = 0.0f;
    bool isDragging = false;
    double lastX, lastY;
    bool isPanning = false;
    vec3 cameraFront;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;
    float deltaTime = 0.0f;
    float timeScale = 86400.0f;

    GLFWwindow* window;

    Engine();
    ~Engine();

    void updateMatrices();
    void addStar(unique_ptr<Star> st);
    void addPlanet(float distance, double mass, double radius, vec3 color, double rotSpeed, float orbVel, float incRad);
    void setSimulation();
    void drawStar(Star& st);
    void drawPlanet(Planet& pt);
    void step();
    bool run();

    // Static Callbacks
    static void mouse_callback(GLFWwindow* w, double x, double y);
    static void mouse_button_callback(GLFWwindow* w, int b, int a, int m);
    static void scroll_callback(GLFWwindow* w, double x, double y);
};

#endif