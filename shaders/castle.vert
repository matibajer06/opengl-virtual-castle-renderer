#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform float uTime;
uniform int uObjectType;

void main() {
    vec3 localPos = aPos;

    if (uObjectType == 1) {
        float distanceFromPole = clamp(localPos.x, 0.0, 1.0);
        float primaryWave = sin(localPos.x * 8.0 + uTime * 4.2) * 0.08;
        float secondaryWave = sin(localPos.x * 17.0 + uTime * 6.0 + localPos.y * 4.0) * 0.025;
        localPos.z += (primaryWave + secondaryWave) * distanceFromPole;
    }

    vec4 worldPos = model * vec4(localPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = normalize(normalMatrix * aNormal);
    TexCoord = aTexCoord;
    gl_Position = projection * view * worldPos;
}
