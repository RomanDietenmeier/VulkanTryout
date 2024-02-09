#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor; //layout(location = 0) target frame buffer with index 0

void main(){//invoked for every fragment
    outColor = vec4(fragColor, 1.0);
}