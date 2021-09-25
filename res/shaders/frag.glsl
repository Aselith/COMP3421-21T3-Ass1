
/*#version 330 core

out vec4 fs_color;

void main() {
    fs_color = vec4(0, 0, 0, 1);
}*/

#version 330 core

uniform sampler2D tex0;

in vec2 tc;

out vec4 fs_color;

void main() {
    fs_color = texture(tex0, tc);
}