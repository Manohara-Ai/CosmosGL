#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 LocalPos;

uniform vec3 sunPos;
uniform vec3 ringColor;
uniform float innerRadius;
uniform float outerRadius;

void main() {
    float dist = length(LocalPos);
    
    float bands = sin(dist * 0.0000005) * 0.5 + 0.5; // Adjust multiplier for band density
    
    vec3 lightDir = normalize(sunPos - FragPos);
    float diff = max(dot(vec3(0.0, 1.0, 0.0), lightDir), 0.4); 

    float alpha = smoothstep(innerRadius, innerRadius + 1e6, dist) * (1.0 - smoothstep(outerRadius - 1e6, outerRadius, dist));
    
    vec3 finalColor = ringColor * bands * diff;
    
    FragColor = vec4(finalColor, alpha * 0.8); // 0.8 for slight transparency
}