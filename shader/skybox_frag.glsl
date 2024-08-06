#version 330

in vec3 fragPosition;

uniform samplerCube environmentMap;
uniform bool vflipped;
uniform bool doGamma;

out vec4 finalColor;

void main()
{
    vec3 color = vec3(0.0);

    color = texture(environmentMap, vec3(fragPosition.x, -fragPosition.y, fragPosition.z)).rgb;
    // color = texture(environmentMap, fragPosition).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    finalColor = vec4(color, 1.0);
}
