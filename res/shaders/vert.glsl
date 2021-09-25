#version 330 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tc_in;

uniform mat4 transform;

out vec2 tc;

void main() {
    tc = tc_in;

    gl_Position = transform * pos;
}