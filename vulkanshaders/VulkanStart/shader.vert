#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor; 

layout(location = 0) out vec4 fragColor;

void main() {
    // Convert pass-in vertex position to a vec4 (w = 1)
    // Pass through unchanged
    gl_Position = vec4(inPosition, 1.0);

    // Pass through vertex color
    fragColor = inColor;
} 
