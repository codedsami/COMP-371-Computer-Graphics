#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// --- NEW: Hemisphere lighting uniforms ---
uniform vec3 skyColor; // which is sky blue for now
uniform vec3 groundColor; // a bit browny this time

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

float CalculateShadow(vec4 fragPosLightSpace) // Shadow Calculation Function
{
    // Perform perspective divide to get normalized device coordinates [ -1, 1 ]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to texture coordinates [ 0, 1 ]
    projCoords = projCoords * 0.5 + 0.5;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Add a small bias to prevent "shadow acne" artifacts
    float bias = 0.005;

    // Sample the shadow map with Percentage-Closer Filtering (PCF) for softer shadow edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // calculates the size of a single texel
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            if(pcfDepth < currentDepth - bias)
                shadow += 1.0;        
        }
    }
    shadow /= 9.0;
    
    // Don't cast shadows into the void
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
    // Get the base color from the texture
    vec3 objectColor = texture(texture_diffuse1, TexCoord).rgb;

    // Hemisphere Ambient Calculation (your code is perfect, no changes here)
    float skyBlend = Normal.y * 0.5 + 0.5;
    vec3 ambient = mix(groundColor, skyColor, skyBlend);

    // Diffuse (your code is perfect, no changes here)
    float diffuseStrength = 2.0;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * diffuseStrength;

    // --- APPLY THE SHADOW ---
    float shadow = CalculateShadow(FragPosLightSpace); // <-- ADD THIS
    
    // Combine lighting: The shadow only affects the direct (diffuse) light, not the ambient light.
    vec3 result = (ambient + (1.0 - shadow) * diffuse) * objectColor; // <-- MODIFY THIS
    FragColor = vec4(result, 1.0);
}