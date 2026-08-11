#version 450

struct directional_light {
    vec3 direction;
    vec4 color;
};

directional_light hard_light = {
    vec3(-0.57735, -0.57735, -0.57735),
    vec4(0.8, 0.8, 0.8, 1.0)
};

mat3 TBN;

vec4 Calculate_Directional_Light(directional_light light, vec3 normal, vec3 view_direction);

layout(location = 1) in struct dto {
    vec4 ambient;
    vec2 tex_coord;
    vec3 normal;
    vec3 view_position;
    vec3 frag_position;
    vec4 color;
    vec4 tangent;
} in_dto;

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 0) uniform localUniformObject {
    vec4 diffuse_color;
    float shine;
} object_ubo;

const int SAMPLER_DIFFUSE = 0;
const int SAMPLER_SPECULAR = 1;
const int SAMPLER_NORMAL = 2;
layout(set = 1, binding = 1) uniform sampler2D samplers[3];

void main() {
    vec3 normal = in_dto.normal;
    vec3 tangent = in_dto.tangent.xyz;
    tangent = (tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(in_dto.normal, in_dto.tangent.xyz) * in_dto.tangent.w;
    TBN = mat3(tangent, bitangent, normal);
    vec3 localNormal = 2.0* texture(samplers[SAMPLER_NORMAL], in_dto.tex_coord).rgb - 1.0;
    normal = normalize(TBN * localNormal);

    vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);
    out_color = Calculate_Directional_Light(hard_light, normal, view_direction);
}

vec4 Calculate_Directional_Light(directional_light light, vec3 normal, vec3 view_direction) {
    float diffuse_factor = max(dot(normal, -light.direction), 0);
    vec3 half_direction = normalize(view_direction - light.direction);
    float specular_factor = pow(max(dot(half_direction, normal), 0.0), object_ubo.shine);

    vec4 samp = texture(samplers[SAMPLER_DIFFUSE], in_dto.tex_coord);
    vec4 ambient = vec4(vec3(in_dto.ambient * object_ubo.diffuse_color), samp.a);
    vec4 diffuse = vec4(vec3(light.color * diffuse_factor), samp.a);
    vec4 specular = vec4(vec3(light.color * specular_factor), samp.a);

    diffuse *= samp;
    ambient *= samp;
    specular *= vec4(texture(samplers[SAMPLER_SPECULAR], in_dto.tex_coord).rgb, diffuse.a);

    return (ambient + diffuse + specular);
}