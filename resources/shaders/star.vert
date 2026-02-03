#version 460 core
layout (location = 0) in vec3 aPos;

layout (std140, binding=0) uniform WindowData {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out vec3 FragPos;
out vec3 Normal;
out vec3 LocalPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));

    Normal = mat3(transpose(inverse(model))) * aPos; 
    gl_Position = projection * view * vec4(FragPos, 1.0);
}