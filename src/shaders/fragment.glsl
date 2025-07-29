#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// --- NEW: Hemisphere lighting uniforms ---
uniform vec3 skyColor;
uniform vec3 groundColor;

uniform sampler2D texture_diffuse1;

void main()
{
    // Get the base color from the texture
    vec3 objectColor = texture(texture_diffuse1, TexCoord).rgb;

    // --- NEW: Hemisphere Ambient Calculation ---
    // Calculate how much the surface normal is pointing "up"
    float skyBlend = Normal.y * 0.5 + 0.5;
    // Blend between the ground and sky color based on the normal
    vec3 ambient = mix(groundColor, skyColor, skyBlend);

    // Diffuse (Direct light from the sun)
    float diffuseStrength = 2.0; // Make the sun's direct light twice as bright
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * diffuseStrength;

    // Combine lighting with the texture color
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}