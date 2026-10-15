#version 330 core

struct Material {
    vec3 baseColor;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float cutOff;
    float outerCutOff;
    bool enabled;
};

#define MAX_POINT_LIGHTS 8

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform DirectionalLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int pointLightCount;
uniform SpotLight spotLight;
uniform bool uEmissive;
uniform bool uUseTexture;
uniform sampler2D diffuseTexture;

vec3 textureColor() {
    vec3 tex = uUseTexture ? texture(diffuseTexture, TexCoord).rgb : vec3(1.0);
    return tex * material.baseColor;
}

vec3 applyDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = 0.12 * light.color * albedo;
    vec3 diffuse = diff * light.color * albedo;
    vec3 specular = spec * light.color * (0.20 + albedo * 0.25);
    return (ambient + diffuse + specular) * light.intensity;
}

vec3 applyPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distanceToLight = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distanceToLight + light.quadratic * distanceToLight * distanceToLight);

    vec3 ambient = 0.04 * light.color * albedo;
    vec3 diffuse = diff * light.color * albedo;
    vec3 specular = spec * light.color * (0.25 + albedo * 0.35);
    return (ambient + diffuse + specular) * attenuation * light.intensity;
}

vec3 applySpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 albedo) {
    if (!light.enabled) {
        return vec3(0.0);
    }

    vec3 lightDir = normalize(light.position - FragPos);
    float theta = dot(normalize(-light.direction), lightDir);
    float epsilon = light.cutOff - light.outerCutOff;
    float beam = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 diffuse = diff * light.color * albedo;
    vec3 specular = spec * light.color * (0.18 + albedo * 0.25);
    return (diffuse + specular) * beam * light.intensity;
}

void main() {
    vec3 albedo = textureColor();

    if (uEmissive) {
        FragColor = vec4(min(albedo * 1.7, vec3(1.0)), 1.0);
        return;
    }

    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = applyDirectionalLight(dirLight, normal, viewDir, albedo);

    for (int i = 0; i < pointLightCount && i < MAX_POINT_LIGHTS; ++i) {
        result += applyPointLight(pointLights[i], normal, viewDir, albedo);
    }

    result += applySpotLight(spotLight, normal, viewDir, albedo);
    result = pow(result, vec3(1.0 / 2.2));
    FragColor = vec4(result, 1.0);
}
