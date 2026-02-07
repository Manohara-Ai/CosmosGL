#include "rasterEngine.h"

int main() {
    Engine engine;
    
    // The Sun 
    Star* sun = engine.addStar(make_unique<Star>(
        vec3(0.0f), 1.989e30, 6.96e8, vec3(1.0f, 0.7f, 0.3f), 2.0, vec3(0.0f)
    ));
    
    // Mercury
    Planet* mercury = engine.addPlanet(5.79e10f,  3.30e23, 2.44e6, vec3(0.7f), 1.24e-6, 47360.0f, 0.1222f);

    // Venus
    Planet* venus = engine.addPlanet(1.082e11f, 4.87e24, 6.05e6, vec3(0.9f, 0.7f, 0.4f), -2.99e-7, 35020.0f, 0.0592f);

    // Earth
    Planet* earth = engine.addPlanet(1.496e11f, 5.97e24, 6.37e6, vec3(0.2f, 0.5f, 1.0f), 7.29e-5,  29780.0f, 0.0000f);
    Satellite* moon = engine.addSatellite(earth, 3.84e8f, 7.34e22, 1.73e6, vec3(0.7f, 0.7f, 0.7f), 0.0f, 1022.0f);

    // Mars
    Planet* mars = engine.addPlanet(2.279e11f, 6.39e23, 3.39e6, vec3(0.9f, 0.3f, 0.2f), 7.08e-5,  24070.0f, 0.0323f);

    // Jupiter
    Planet*Jupiter = engine.addPlanet(7.785e11f, 1.89e27, 6.99e7, vec3(0.8f, 0.7f, 0.6f), 1.76e-4,  13070.0f, 0.0227f);

    // Saturn
    Planet* Saturn = engine.addPlanet(1.433e12f, 5.68e26, 5.82e7, vec3(0.9f, 0.8f, 0.5f), 1.63e-4,   9680.0f, 0.0435f);
    engine.addRing(Saturn, 7.0e7, 6.5e7, 0.45, vec3(0.8f, 0.7f, 0.5f));

    // Uranus
    Planet* Uranus = engine.addPlanet(2.871e12f, 8.68e25, 2.53e7, vec3(0.6f, 0.8f, 0.9f), -1.04e-4,  6800.0f, 0.0134f);

    // Neptune
    Planet* neptune = engine.addPlanet(4.495e12f, 1.02e26, 2.46e7, vec3(0.3f, 0.5f, 0.9f), 1.08e-4,   5430.0f, 0.0309f);

    engine.setSimulation();

    while (engine.run()) {
        static bool tabPressed = false;
        if (glfwGetKey(engine.window, GLFW_KEY_TAB) == GLFW_PRESS) {
            if (!tabPressed) {
                engine.cycleFocus();
                tabPressed = true;
            }
        } else {
            tabPressed = false;
        }

        engine.updateCameraFocus();
    };

    return 0;
}