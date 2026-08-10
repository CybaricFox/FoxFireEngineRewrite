#version 450

struct directional_light {
    vec3 direction;
    vec4 color;
};

directional_light hard_light = {
    vec3(-0.57735, -0.57735, -0.57735),
    vec4(0.8, 0.8, 0.8, 1.0)
};

vec4 Calculate_Directional_Light(directional_light light, vec3 normal);

layout(location = 1) in struct dto {
    vec4 ambient;
    vec2 tex_coord;
    vec3 normal;
} in_dto;

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 0) uniform localUniformObject {
    vec4 diffuse_color;
} object_ubo;

layout(set = 1, binding = 1) uniform sampler2D diffuse_sampler;

void main() {
    out_color = Calculate_Directional_Light(hard_light, in_dto.normal);
}

vec4 Calculate_Directional_Light(directional_light light, vec3 normal) {
    float diffuse_factor = max(dot(normal, -light.direction), 0);
    vec4 samp = texture(diffuse_sampler, in_dto.tex_coord);
    vec4 ambient = vec4(vec3(in_dto.ambient * object_ubo.diffuse_color), samp.a);
    vec4 diffuse = vec4(vec3(light.color * diffuse_factor), samp.a);

    diffuse *= samp;
    ambient *= samp;

    return (ambient + diffuse);
}