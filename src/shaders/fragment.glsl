#version 330 core
out vec4 FragColor;

// Inputs from the Vertex Shader
in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoord;

// Uniforms from main.cpp
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;

// The texture sampler
uniform sampler2D texture_diffuse1;

void main()
{
    // Get the base color from the texture
    vec3 objectColor = texture(texture_diffuse1, TexCoord).rgb;

    // Calculate lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
  	
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Combine lighting with the texture color
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}