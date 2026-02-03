#version 460 core
layout (location = 0) in vec3 aPos;

layout (std140) uniform WindowData {
    mat4 projection;
};
uniform mat4 model;
uniform int uTotalPoints;

out vec4 vColor;

void main() {
    gl_Position = projection * model * vec4(aPos, 1.0);
    float alpha = float(gl_VertexID) / float(uTotalPoints);
    vColor = vec4(1.0, 1.0, 1.0, alpha);
}