#version 330 core
out vec4 FragColor;

// We only need the final color for the sun
uniform vec3 objectColor;

void main()
{
    // Simply output the solid color. No lighting is calculated.
    FragColor = vec4(objectColor, 1.0);
}