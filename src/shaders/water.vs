#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    vec3 pos = aPos;
    
    // Add gentle wave animation
    pos.y += sin(pos.x * 0.5 + time * 2.0) * 0.5;
    pos.y += cos(pos.z * 0.3 + time * 1.5) * 0.3;
    
    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoords;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}