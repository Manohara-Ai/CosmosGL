# CosmosGL

A lightweight, C++ N-body gravity simulation and rendering engine powered by OpenGL 4.6.

This project computes the interactions of celestial bodies ranging from terrestrial planets to supermassive black holes using Newton's law of universal gravitation and a modern shader-based pipeline.

---

## Setup

Ensure you have the following libraries installed and linked:

- GLFW3
- GLAD
- GLM

---

## Build & Run
Follow these steps to set up and launch the simulation on your local machine.
1. **Clone the Repository**
   Open your terminal and clone the repo to your local environment:
   ```bash
   git clone https://github.com/Manohara-Ai/CosmosGL.git
   cd CosmosGL
   ```


2. **Verify Shader Paths**

   Ensure the `resources/shaders/` directory contains the required `.vert` and `.frag` files.

3. **Compile the source**

   ```bash
   g++ src/solarSystem.cpp src/engine.cpp -lglad -ldl -lGL -lglfw -Iinclude -o build/solarSystem
   ```

4. **Execute**

   ```bash
   ./build/solarSystem
   ```

### OR

Run the bash script:

```bash
chmod +x run.bash
./run.bash
```

---

![Solar System](resources/images/img1.png "Our Solar System")
*Still from the simulation of our Solar System.*

---

## Contributing

1. Fork the project  
2. Create a feature branch:
   ```bash
   git checkout -b feature/AmazingFeature
   ```
3. Commit changes:
   ```bash
   git commit -m "Add AmazingFeature"
   ```
4. Push the branch:
   ```bash
   git push origin feature/AmazingFeature
   ```
5. Open a Pull Request

---

### Planned Extensions

- **Black Hole Physics**

- **Rendering Enhancements**

- **Simulation Improvements**

---
