#version 420 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 MV;
uniform mat3 N;
uniform mat4 MVP;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec3 position;
layout (location = 2) out vec2 texCoord;

void main()
{
    // Transform to eye coordinates
    normal = normalize( N * vertexNormal );
    position = vec3( MV * vec4( vertexPosition, 1.0 ) );

    gl_Position = MVP * vec4( vertexPosition, 1.0 );

    texCoord = a_TexCoord;
}
