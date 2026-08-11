#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

//layout(location = 0) out int out_mode;
layout(location = 1) out struct dto {
    vec4 ambient;
    vec2 tex_coord;
    vec3 normal;
    vec3 view_position;
    vec3 frag_position;
} out_dto;

layout(set = 0, binding = 0) uniform global_uniform {
    mat4 projection;
    mat4 view;
    vec4 ambient_color;
    vec3 view_position;
} global_ubo;

layout(push_constant) uniform push_constants {
    mat4 model;
} uniform_push_constants;

void main() {
    out_dto.tex_coord = in_texcoord;
    out_dto.frag_position = vec3(uniform_push_constants.model * vec4(in_position, 1.0));
    out_dto.normal = mat3(uniform_push_constants.model) * in_normal;
    out_dto.ambient = global_ubo.ambient_color;
    out_dto.view_position = global_ubo.view_position;
    gl_Position = global_ubo.projection * global_ubo.view * uniform_push_constants.model * vec4(in_position, 1.0);
}