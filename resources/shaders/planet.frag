#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 LocalPos;

uniform vec3 sunPos;
uniform vec3 planetColor;

void main() {
    vec3 lightDir = normalize(sunPos - FragPos);
    vec3 norm = normalize(Normal);
    
    float diff = max(dot(norm, lightDir), 0.0);
    float ambient = 0.02;
    
    vec3 result = (ambient + diff) * planetColor;
    
    FragColor = vec4(result, 1.0);
}