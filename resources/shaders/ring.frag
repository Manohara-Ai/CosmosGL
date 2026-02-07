#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 LocalPos;

uniform vec3 sunPos;
uniform vec3 ringColor;
uniform float innerRadius;
uniform float outerRadius;

void main() {
    // Calculate radial distance from center (0,0,0) in local space
    float dist = length(LocalPos);
    
    // Create a "banded" texture effect using a sine wave
    float bands = sin(dist * 0.0000005) * 0.5 + 0.5; // Adjust multiplier for band density
    
    // Simple lighting: Rings are usually bright, so we use a high ambient value
    vec3 lightDir = normalize(sunPos - FragPos);
    // Since rings are flat, we treat the "up" vector (0,1,0) as the normal
    float diff = max(dot(vec3(0.0, 1.0, 0.0), lightDir), 0.4); 
    
    // Calculate transparency (alpha)
    // Fade out slightly at the very edges of the inner and outer radius
    float alpha = smoothstep(innerRadius, innerRadius + 1e6, dist) * (1.0 - smoothstep(outerRadius - 1e6, outerRadius, dist));
    
    // Mix the base color with the bands and apply alpha
    vec3 finalColor = ringColor * bands * diff;
    
    FragColor = vec4(finalColor, alpha * 0.8); // 0.8 for slight transparency
}