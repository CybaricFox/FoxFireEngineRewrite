#version 450

struct directional_light {
    vec3 direction;
    vec4 color;
};

struct point_light {
    vec3 position;
    vec4 color;
    float constant;
    float linear;
    float quadratic;
};

directional_light hard_light = {
    vec3(-0.57735, -0.57735, -0.57735),
    vec4(0.8, 0.8, 0.8, 1.0)
};

point_light point_light_0 = {
    vec3(-5.5, 0.0, -5.5),
    vec4(0.0, 1.0, 0.0, 1.0),
    1.0,
    0.35,
    0.44
};

point_light point_light_1 = {
    vec3(5.5, 0.0, -5.5),
    vec4(1.0, 0.0, 0.0, 1.0),
    1.0,
    0.35,
    0.44
};

mat3 TBN;

vec4 Calculate_Directional_Light(directional_light light, vec3 normal, vec3 view_direction);
vec4 Calculate_Point_Light(point_light light, vec3 normal, vec3 frag_position, vec3 view_direction);

layout(location = 0) flat in int in_mode;
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
    vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);
    vec3 normal = in_dto.normal;
    vec3 tangent = in_dto.tangent.xyz;
    vec3 bitangent = cross(in_dto.normal, in_dto.tangent.xyz) * in_dto.tangent.w;
    vec3 localNormal = 2.0* texture(samplers[SAMPLER_NORMAL], in_dto.tex_coord).rgb - 1.0;

    tangent = (tangent - dot(tangent, normal) * normal);
    TBN = mat3(tangent, bitangent, normal);
    normal = normalize(TBN * localNormal);

    if(in_mode == 0 || in_mode == 1) {
        vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);
        out_color = Calculate_Directional_Light(hard_light, normal, view_direction);
        out_color += Calculate_Point_Light(point_light_0, normal, in_dto.frag_position, view_direction);
        out_color += Calculate_Point_Light(point_light_1, normal, in_dto.frag_position, view_direction);
    } else if(in_mode == 2) {
        out_color = vec4(abs(normal), 1.0);
    }
}

vec4 Calculate_Directional_Light(directional_light light, vec3 normal, vec3 view_direction) {
    float diffuse_factor = max(dot(normal, -light.direction), 0);
    vec3 half_direction = normalize(view_direction - light.direction);
    float specular_factor = pow(max(dot(half_direction, normal), 0.0), object_ubo.shine);

    vec4 samp = texture(samplers[SAMPLER_DIFFUSE], in_dto.tex_coord);
    vec4 ambient = vec4(vec3(in_dto.ambient * object_ubo.diffuse_color), samp.a);
    vec4 diffuse = vec4(vec3(light.color * diffuse_factor), samp.a);
    vec4 specular = vec4(vec3(light.color * specular_factor), samp.a);

    if(in_mode == 0) {
        diffuse *= samp;
        ambient *= samp;
        specular *= vec4(texture(samplers[SAMPLER_SPECULAR], in_dto.tex_coord).rgb, diffuse.a);
    }

    return (ambient + diffuse + specular);
}

vec4 Calculate_Point_Light(point_light light, vec3 normal, vec3 frag_position, vec3 view_direction) {
    vec3 light_direction = normalize(light.position - frag_position);
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), object_ubo.shine);
    float distance = length(light.position - frag_position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec4 ambient = in_dto.ambient;
    vec4 diffuse = light.color * diff;
    vec4 specular = light.color * spec;

    if(in_mode == 0) {
        vec4 samp = texture(samplers[SAMPLER_DIFFUSE], in_dto.tex_coord);
        diffuse *= samp;
        ambient *= samp;
        specular *= vec4(texture(samplers[SAMPLER_SPECULAR], in_dto.tex_coord).rgb, diffuse.a);
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}