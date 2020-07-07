#version 440

layout(location = 0) in vec2 v_texcoord;
layout(location = 1) flat in uint v_idx;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2DArray tex;

void main()
{
    fragColor = texture(tex, vec3(v_texcoord, v_idx));
}