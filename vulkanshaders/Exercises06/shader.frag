#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    // Final color is just the passed in fragment color
    vec4 finalColor = fragColor;
    
    // Output final color
    outColor = vec4(1.0, 0, 0, 1.0);
} 
