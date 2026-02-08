#include "rayEngine.h"

vec3 Camera::position() const {
    float clampedElevation = clamp(elevation, 0.01f, float(M_PI) - 0.01f);
    return vec3(
        radius * sin(clampedElevation) * cos(azimuth),
        radius * cos(clampedElevation),
        radius * sin(clampedElevation) * sin(azimuth)
    );
}

void Camera::update() {
    target = vec3(0.0f, 0.0f, 0.0f);
    if(dragging | panning) {
        moving = true;
    } else {
        moving = false;
    }
}

void Camera::processMouseMove(double x, double y) {
    float dx = float(x - lastX);
    float dy = float(y - lastY);

    if (dragging && panning) {
        // Pan: Shift + Left or Middle Mouse
        // Disable panning to keep camera centered on black hole
    }
    else if (dragging && !panning) {
        // Orbit: Left mouse only
        azimuth   += dx * orbitSpeed;
        elevation -= dy * orbitSpeed;
        elevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);
    }

    lastX = x;
    lastY = y;
    update();
}

void Camera::processMouseButton(int button, int action, int mods, GLFWwindow* win) {
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            dragging = true;
            // Disable panning so camera always orbits center
            panning = false;
            glfwGetCursorPos(win, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            dragging = false;
            panning = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            Gravity = true;
        } else if (action == GLFW_RELEASE) {
            Gravity = false;
        }
    }
}

void Camera::processScroll(double xoffset, double yoffset) {
    radius -= yoffset * zoomSpeed;
    radius = glm::clamp(radius, minRadius, maxRadius);
    update();
}

void Camera::processKey(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_G) {
        Gravity = !Gravity;
        cout << "[INFO] Gravity turned " << (Gravity ? "ON" : "OFF") << endl;
    }
}

BlackHole::BlackHole(vec3 pos, float m) 
    : position(pos), mass(m) {
        r_s = 2.0 * G * mass / (c*c);
}
    
bool BlackHole::Intercept(float px, float py, float pz) const {
    double dx = double(px) - double(position.x);
    double dy = double(py) - double(position.y);
    double dz = double(pz) - double(position.z);
    double dist2 = dx * dx + dy * dy + dz * dz;
    return dist2 < r_s * r_s;
}

Engine::Engine() {
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW" << endl;
        exit(EXIT_FAILURE);
    } 

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Black Hole", NULL, NULL);
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

    cout << "OpenGL " << glGetString(GL_VERSION) << "\n";
    this->shaderID = createShader();
    gridShaderID = createShader("resources/shaders/grid.vert", "resources/shaders/grid.frag");

    computeShaderID = createComputeShader("resources/shaders/geodesic.comp");
    glGenBuffers(1, &cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, cameraUBO);

    glGenBuffers(1, &diskUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, diskUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, diskUBO);

    glGenBuffers(1, &objectsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, objectsUBO);

    GLsizeiptr objUBOSize = sizeof(int) + 3 * sizeof(float)
        + 16 * (sizeof(vec4) + sizeof(vec4))
        + 16 * sizeof(float);
    glBufferData(GL_UNIFORM_BUFFER, objUBOSize, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, objectsUBO);

    auto result = QuadVAO();
    this->quadVAO = result[0];
    this->texture = result[1];
}

void Engine::generateGrid(const vector<ObjectData>& objects) {
    const int gridSize = 25;
    const float spacing = 1e10f;

    vector<vec3> vertices;
    vector<GLuint> indices;

    for (int z = 0; z <= gridSize; ++z) {
        for (int x = 0; x <= gridSize; ++x) {
            float worldX = (x - gridSize / 2) * spacing;
            float worldZ = (z - gridSize / 2) * spacing;

            float y = 0.0f;

            for (const auto& obj : objects) {
                vec3 objPos = vec3(obj.posRadius);
                double mass = obj.mass;
                double radius = obj.posRadius.w;

                double r_s = 2.0 * G * mass / (c * c);
                double dx = worldX - objPos.x;
                double dz = worldZ - objPos.z;
                double dist = sqrt(dx * dx + dz * dz);

                if (dist > r_s) {
                    double deltaY = 2.0 * sqrt(r_s * (dist - r_s));
                    y += static_cast<float>(deltaY) - 3e10f;
                } else {
                    y += 2.0f * static_cast<float>(sqrt(r_s * r_s)) - 3e10f;  // or add a deep pit
                }
            }

            vertices.emplace_back(worldX, y, worldZ);
        }
    }

    for (int z = 0; z < gridSize; ++z) {
        for (int x = 0; x < gridSize; ++x) {
            int i = z * (gridSize + 1) + x;
            indices.push_back(i);
            indices.push_back(i + 1);

            indices.push_back(i);
            indices.push_back(i + gridSize + 1);
        }
    }

    if (gridVAO == 0) glGenVertexArrays(1, &gridVAO);
    if (gridVBO == 0) glGenBuffers(1, &gridVBO);
    if (gridEBO == 0) glGenBuffers(1, &gridEBO);

    glBindVertexArray(gridVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // location = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    gridIndexCount = indices.size();

    glBindVertexArray(0);
}

void Engine::drawGrid(const mat4& viewProj) {
    glUseProgram(gridShaderID);
    glUniformMatrix4fv(glGetUniformLocation(gridShaderID, "viewProj"),
                    1, GL_FALSE, glm::value_ptr(viewProj));
    glBindVertexArray(gridVAO);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawElements(GL_LINES, gridIndexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void Engine::drawFullScreenQuad() {
    glUseProgram(shaderID);
    glBindVertexArray(quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shaderID, "screenTexture"), 0);

    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    glEnable(GL_DEPTH_TEST);
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

GLuint Engine::createComputeShader(const char* computeFile) {
    string computeCode = this->getFileContents(computeFile);
    const char* computeSource = computeCode.c_str();

    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cs, 1, &computeSource, nullptr);
    glCompileShader(cs);
    GLint ok; 
    glGetShaderiv(cs, GL_COMPILE_STATUS, &ok);
    if(!ok) {
        GLint logLen;
        glGetShaderiv(cs, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetShaderInfoLog(cs, logLen, nullptr, log.data());
        std::cerr << "Compute shader compile error:\n" << log.data() << "\n";
        exit(EXIT_FAILURE);
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, cs);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if(!ok) {
        GLint logLen;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetProgramInfoLog(prog, logLen, nullptr, log.data());
        std::cerr << "Compute shader link error:\n" << log.data() << "\n";
        exit(EXIT_FAILURE);
    }

    glDeleteShader(cs);
    return prog;
}

void Engine::dispatchCompute(const Camera& cam) {
    int cw = cam.moving ? COMPUTE_WIDTH  : 200;
    int ch = cam.moving ? COMPUTE_HEIGHT : 150;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D,
                0,                // mip
                GL_RGBA8,         // internal format
                cw,               // width
                ch,               // height
                0, GL_RGBA, 
                GL_UNSIGNED_BYTE, 
                nullptr);

    glUseProgram(computeShaderID);
    uploadCameraUBO(cam);
    uploadDiskUBO();
    uploadObjectsUBO(objects);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    GLuint groupsX = (GLuint)std::ceil(cw / 16.0f);
    GLuint groupsY = (GLuint)std::ceil(ch / 16.0f);
    glDispatchCompute(groupsX, groupsY, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Engine::uploadCameraUBO(const Camera& cam) {
    struct UBOData {
        vec3 pos; float _pad0;
        vec3 right; float _pad1;
        vec3 up; float _pad2;
        vec3 forward; float _pad3;
        float tanHalfFov;
        float aspect;
        bool moving;
        int _pad4;
    } data;
    vec3 fwd = normalize(cam.target - cam.position());
    vec3 up = vec3(0, 1, 0);
    vec3 right = normalize(cross(fwd, up));
    up = cross(right, fwd);

    data.pos = cam.position();
    data.right = right;
    data.up = up;
    data.forward = fwd;
    data.tanHalfFov = tan(radians(60.0f * 0.5f));
    data.aspect = float(WIDTH) / float(HEIGHT);
    data.moving = cam.dragging || cam.panning;

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), &data);
}

void Engine::uploadObjectsUBO(const vector<ObjectData>& objs) {
    struct UBOData {
        int   numObjects;
        float _pad0, _pad1, _pad2;
        vec4  posRadius[16];
        vec4  color[16];
        float  mass[16]; 
    } data;

    size_t count = std::min(objs.size(), size_t(16));
    data.numObjects = static_cast<int>(count);

    for (size_t i = 0; i < count; ++i) {
        data.posRadius[i] = objs[i].posRadius;
        data.color[i] = objs[i].color;
        data.mass[i] = objs[i].mass;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, objectsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
}

void Engine::uploadDiskUBO() {
    float r1 = SagA.r_s * 2.2f;    
    float r2 = SagA.r_s * 5.2f;   
    float num = 2.0;               // number of rays
    float thickness = 1e9f;          // padding for std140 
    float diskData[4] = { r1, r2, num, thickness };

    glBindBuffer(GL_UNIFORM_BUFFER, diskUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(diskData), diskData);
}

vector<GLuint> Engine::QuadVAO() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,  // top left
        -1.0f, -1.0f,  0.0f, 0.0f,  // bottom left
        1.0f, -1.0f,  1.0f, 0.0f,  // bottom right

        -1.0f,  1.0f,  0.0f, 1.0f,  // top left
        1.0f, -1.0f,  1.0f, 0.0f,  // bottom right
        1.0f,  1.0f,  1.0f, 1.0f   // top right
    };
    
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D,
                0,             // mip
                GL_RGBA8,      // internal format
                COMPUTE_WIDTH,
                COMPUTE_HEIGHT,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                nullptr);
    vector<GLuint> VAOtexture = {VAO, texture};
    return VAOtexture;
}

void Engine::renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderID);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
}

void setupCameraCallbacks(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, &camera);

    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->processMouseButton(button, action, mods, win);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double x, double y) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->processMouseMove(x, y);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* win, double xoffset, double yoffset) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->processScroll(xoffset, yoffset);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->processKey(key, scancode, action, mods);
    });
}