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
   g++ src/solarSystem.cpp src/rasterEngine.cpp -lglad -ldl -lGL -lglfw -Iinclude -o build/solarSystem
   ```

4. **Execute**

   ```bash
   ./build/solarSystem
   ```

### OR

Run the bash script:

```bash
chmod +x solar_system.bash
./solar_system.bash
```

---

## Technical Overview

### Physics Engine
The simulation utilizes Newtonian mechanics to dictate the movement of all celestial bodies:
* **Gravitational Constant ($G$):** Defined as $6.67430 \times 10^{-11}$.
* **N-Body Interaction:** The engine calculates the gravitational pull between every pair of objects in the simulation, excluding self-interaction and enforcing a distance threshold of $10^5$m to prevent singularities.
* **Integration:** Implements Velocity Verlet-style Euler integration for position and velocity updates.
* **Time Scaling:** Supports variable time scales, defaulting to 86,400 (one Earth day per real-time second).
* **Satellite Mechanics:** Moons calculate acceleration based on their parent planet's mass and position while maintaining relative orbital velocity.



### Rendering Architecture
The graphical pipeline is built for efficiency and visual fidelity using OpenGL 4.6:
* **Uniform Buffer Objects (UBO):** Uses a shared `WindowData` block (binding 0) to store `projection` and `view` matrices, accessible by all shaders.
* **Procedural Geometry:** * **Spheres:** Generated via stacks and slices (50x50) for all celestial bodies.
    * **Rings:** Procedurally generated bands using a `sin` function on the local distance to create realistic planetary rings.
* **Visual Effects:**
    * **Star Glow:** Implements a Fresnel effect to create a glowing atmospheric rim based on the view angle.
    * **Orbital Trails:** Dynamic `std::deque` storage for historical positions, rendered as `GL_LINE_STRIP` with alpha blending.

---

## Controls

The engine features an interactive orbital camera and focus system:

| Input | Action |
| :--- | :--- |
| **Left Click + Drag** | Rotates the camera (Yaw/Pitch) around the focus target. |
| **Right Click + Drag** | Pans the camera target across the orbital plane. |
| **Scroll Wheel** | Logarithmic zoom (adjusts camera distance). |
| **TAB Key** | Cycles camera focus through registered stars, planets, and moons. |

---

![Saturn](resources/images/img1.png "Saturn")
*Saturn from the simulation of our Solar System.*

![Solar System](resources/images/img2.png "Solar System")
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

- **Rendering Enhancements**

- **Simulation Improvements**

---
