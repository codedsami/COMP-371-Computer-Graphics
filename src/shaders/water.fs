#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform sampler2D waterTexture;
uniform float time;

void main()
{
    // Animate texture coordinates for flowing water effect
    vec2 animatedTexCoord = TexCoord + vec2(sin(time * 0.1), cos(time * 0.15)) * 0.02;
    
    vec3 waterColor = texture(waterTexture, animatedTexCoord).rgb;
    
    // Basic lighting
    vec3 ambient = 0.3 * waterColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * waterColor;
    
    // Simple specular reflection for water shimmer
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = spec * lightColor * 0.8;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 0.85); // Slightly transparent
}