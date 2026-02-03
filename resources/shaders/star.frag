#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 LocalPos;

uniform vec3 starColor;
uniform vec3 viewPos;
uniform float brightness;

float noise(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
}

void main() {
    vec3 n = normalize(Normal); 
    vec3 v = normalize(viewPos - FragPos); 

    float fresnel = clamp(dot(n, v), 0.0, 1.0); 

    float intensity = pow(fresnel, 0.4);
    
    vec3 finalColor = starColor * brightness * intensity;
    
    finalColor = mix(finalColor * vec3(1.0, 0.4, 0.2), finalColor, fresnel);

    FragColor = vec4(finalColor, 1.0);
}