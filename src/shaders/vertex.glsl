#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Outputs to the fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord; // <-- THE MISSING OUTPUT

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Pass world-space position to the fragment shader
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Calculate and pass the normal in world space
    Normal = mat3(transpose(inverse(model))) * aNormal;   
    
    // Pass the texture coordinates through
    TexCoord = aTexCoords; // <-- THE MISSING ASSIGNMENT

    // Calculate the final clip-space position
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
