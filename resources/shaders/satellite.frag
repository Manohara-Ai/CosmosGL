#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 sunPos;
uniform vec3 satelliteColor;

void main() {
    vec3 lightDir = normalize(sunPos - FragPos);
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    float ambient = 0.05;
    
    vec3 result = (ambient + diff) * satelliteColor;
    FragColor = vec4(result, 1.0);
}