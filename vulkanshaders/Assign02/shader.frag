#version 450

layout(location = 0) out vec4 out_color;
layout(set = 0, binding = 0) uniform UBO {
    float u_time;
} ubo;

void main()
{
    float r = 0.6 + 0.4 * sin(ubo.u_time * 0.8);
    float g = 0.3 + 0.3 * sin(ubo.u_time * 1.2 + 2.0);
    float b = 0.4 + 0.4 * sin(ubo.u_time * 1.0 + 4.0);
    out_color = vec4(r, g * 0.3, b * 0.6, 1.0);
}