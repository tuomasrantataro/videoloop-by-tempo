#version 130

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 a_TexCoord;

uniform mat4 MV;
uniform mat3 N;
uniform mat4 MVP;

out vec3 normal;
out vec3 position;
out vec2 texCoord;

void main()
{
    // Transform to eye coordinates
    normal = normalize( N * vertexNormal );
    position = vec3( MV * vec4( vertexPosition, 1.0 ) );

    gl_Position = MVP * vec4( vertexPosition, 1.0 );

    texCoord = a_TexCoord;
}
