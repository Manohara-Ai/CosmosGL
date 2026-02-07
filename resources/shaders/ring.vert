#version 460 core
layout (location = 0) in vec3 aPos;

layout (std140, binding=0) uniform WindowData {
    mat4 projection;
    mat4 view;  
};

uniform mat4 model;

out vec3 FragPos;
out vec3 LocalPos; // Used to calculate distance from center

void main() {
    LocalPos = aPos; 
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
}