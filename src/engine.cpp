#include "engine.h"

Star::Star(vec3 pos, double m, double r, vec3 c, double b, vec3 v) 
    : position(pos), mass(m), radius(r), color(c), brightness(b), initialVelocity(v) {
    
    int stacks = 50, slices = 50;

    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * M_PI * j / slices;
            float x = (float)radius * sin(phi) * cos(theta);
            float y = (float)radius * cos(phi);
            float z = (float)radius * sin(phi) * sin(theta);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int first = (i * (slices + 1)) + j;
            unsigned int second = first + slices + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

Planet::Planet(vec3 pos, double m, double r, vec3 c, double rS, vec3 v) 
    : position(pos), mass(m), radius(r), color(c), rotationSpeed(rS), initialVelocity(v) {
    
    rotationAngle = 0.0;
    int stacks = 40, slices = 40;

    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * M_PI * j / slices;
            float x = (float)radius * sin(phi) * cos(theta);
            float y = (float)radius * cos(phi);
            float z = (float)radius * sin(phi) * sin(theta);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int first = (i * (slices + 1)) + j;
            unsigned int second = first + slices + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

Engine::Engine() {
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW" << endl;
        exit(EXIT_FAILURE);
    } 

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Solar System", NULL, NULL);
    if (!window) {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Failed to initialize GLAD (OpenGL loader)" << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowUserPointer(window, this);
    
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        auto* eng = (Engine*)glfwGetWindowUserPointer(window);
        eng->WIDTH = width;
        eng->HEIGHT = height;
        glViewport(0, 0, width, height);
    });

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glGenBuffers(1, &uboWindowData);
    glBindBuffer(GL_UNIFORM_BUFFER, uboWindowData);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboWindowData);

    this->starShaderID = createShader("resources/shaders/star.vert", "resources/shaders/star.frag");
    this->planetShaderID = createShader("resources/shaders/planet.vert", "resources/shaders/planet.frag");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

string Engine::getFileContents(const char* filename) {
    ifstream in(filename, ios::binary);
    if (in) {
        string contents;
        in.seekg(0, ios::end);
        contents.resize(in.tellg());
        in.seekg(0, ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    cerr << "FATAL: Could not open shader file at: " << filename << endl;
    throw(errno);
}

GLuint Engine::createShader(const char* vertexFile, const char* fragmentFile) {
    string vertexCode = this->getFileContents(vertexFile);
    string fragmentCode = this->getFileContents(fragmentFile);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cerr << "Vertex Shader Compilation Failed:\n" << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "Fragment Shader Compilation Failed:\n" << infoLog << endl;
    }

    GLuint ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        cerr << "Shader Linking Failed:\n" << infoLog << endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return ID;
}

void Engine::updateMatrices() {
    float aspect = (float)WIDTH / (float)HEIGHT;
    mat4 proj = perspective(radians(45.0f), aspect, 1.0e6f, 1.0e16f);

    vec3 offset(
        distance * cos(radians(pitch)) * cos(radians(yaw)),
        distance * sin(radians(pitch)),
        distance * cos(radians(pitch)) * sin(radians(yaw))
    );

    cameraPos = focusTarget + offset;
    view = lookAt(cameraPos, focusTarget, vec3(0.0f, 1.0f, 0.0f));

    glBindBuffer(GL_UNIFORM_BUFFER, uboWindowData);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(proj));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(view));
}

void Engine::addStar(unique_ptr<Star> st) {
    stars.push_back(move(st));
}

void Engine::addPlanet(float distance, double mass, double radius, vec3 color, double rotSpeed, float orbVel, float incRad) {
    vec3 pos = vec3(
        (float)distance * cos(incRad), 
        (float)distance * sin(incRad), 
        0.0f
    );
    vec3 vel = vec3(0.0f, 0.0f, (float)orbVel);
    planets.push_back(make_unique<Planet>(pos, mass, radius, color, rotSpeed, vel));
}

void Engine::setSimulation() {
    bodies.clear();
    for (auto& s : stars) {
        bodies.emplace_back(&s->position, s->initialVelocity, s->mass);
    }
    for (auto& p : planets) {
        bodies.emplace_back(&p->position, p->initialVelocity, p->mass);
    }
}

void Engine::drawStar(Star& st) {
    mat4 model = mat4(1.0f);
    model = translate(model, st.position);
    glUniformMatrix4fv(glGetUniformLocation(starShaderID, "model"), 1, GL_FALSE, value_ptr(model));
    glUniform3fv(glGetUniformLocation(starShaderID, "viewPos"), 1, value_ptr(cameraPos));
    glUniform3fv(glGetUniformLocation(starShaderID, "starColor"), 1, value_ptr(st.color));
    glUniform1f(glGetUniformLocation(starShaderID, "brightness"), st.brightness);

    glBindVertexArray(st.VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)st.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Engine::drawPlanet(Planet& pt) {
    pt.rotationAngle += pt.rotationSpeed * (double)deltaTime;
    mat4 model = mat4(1.0f);
    model = translate(model, pt.position);
    model = rotate(model, (float)pt.rotationAngle, vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(planetShaderID, "model"), 1, GL_FALSE, value_ptr(model));
    glUniform3fv(glGetUniformLocation(planetShaderID, "sunPos"), 1, value_ptr(stars[0]->position));
    glUniform3fv(glGetUniformLocation(planetShaderID, "planetColor"), 1, value_ptr(pt.color));

    glBindVertexArray(pt.VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)pt.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Engine::step() {
    for (size_t i = 0; i < bodies.size(); ++i) {
        vec3 acceleration(0.0f);
        for (size_t j = 0; j < bodies.size(); ++j) {
            if (i == j) continue;
            vec3 direction = *bodies[j].position - *bodies[i].position;
            float dist = length(direction);
            if (dist < 1e5f) continue;
            float forceMag = (float)((G * bodies[j].mass) / (dist * dist));
            acceleration += normalize(direction) * forceMag;
        }
        bodies[i].velocity += acceleration * (float)deltaTime;
    }
    for (auto& body : bodies) {
        *body.position += body.velocity * (float)deltaTime;
    }
}

bool Engine::run() {
    if (glfwWindowShouldClose(window)) {
        return false;
    }

    currentFrame = (float)glfwGetTime();
    deltaTime = (currentFrame - lastFrame) * timeScale;
    lastFrame = currentFrame;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateMatrices();
    step();

    glUseProgram(this->starShaderID);
    for (const auto& st : this->stars) {
        this->drawStar(*st);
    }

    glUseProgram(this->planetShaderID);
    for (const auto& pt : this->planets) {
        this->drawPlanet(*pt);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

    return true;
}

void Engine::mouse_callback(GLFWwindow* w, double x, double y) {
    auto* e = (Engine*)glfwGetWindowUserPointer(w);
    float dx = (float)(x - e->lastX);
    float dy = (float)(e->lastY - y);
    e->lastX = x; e->lastY = y;

    if (e->isDragging) { 
        e->yaw += dx * 0.2f;
        e->pitch += dy * 0.2f;
        if (e->pitch > 89.0f) e->pitch = 89.0f;
        if (e->pitch < -89.0f) e->pitch = -89.0f;
    } 
    else if (e->isPanning) { 
        vec3 front = normalize(e->focusTarget - e->cameraPos);
        vec3 right = normalize(cross(front, vec3(0, 1, 0)));
        vec3 up = normalize(cross(right, front));
        float panSpeed = e->distance * 0.001f;
        e->focusTarget -= right * dx * panSpeed;
        e->focusTarget -= up * dy * panSpeed;
    }
}

void Engine::mouse_button_callback(GLFWwindow* w, int b, int a, int m) {
    auto* e = (Engine*)glfwGetWindowUserPointer(w);
    if (b == GLFW_MOUSE_BUTTON_LEFT) {
        e->isDragging = (a == GLFW_PRESS);
    } else if (b == GLFW_MOUSE_BUTTON_RIGHT) {
        e->isPanning = (a == GLFW_PRESS);
    }
    glfwGetCursorPos(w, &e->lastX, &e->lastY);
}

void Engine::scroll_callback(GLFWwindow* w, double x, double y) {
    auto* e = (Engine*)glfwGetWindowUserPointer(w);
    if (y > 0) e->distance *= 0.9f; else e->distance *= 1.1f;
    if (e->distance < 6.5e6f) e->distance = 6.5e6f;
}

Engine::~Engine() {
    for (auto& star : stars) {
        glDeleteVertexArrays(1, &star->VAO);
        glDeleteBuffers(1, &star->VBO);
        glDeleteBuffers(1, &star->EBO);
    }

    for (auto& planet : planets) {
        glDeleteVertexArrays(1, &planet->VAO);
        glDeleteBuffers(1, &planet->VBO);
        glDeleteBuffers(1, &planet->EBO);
    }

    glDeleteBuffers(1, &uboWindowData);
    glDeleteProgram(starShaderID);
    glDeleteProgram(planetShaderID);

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}