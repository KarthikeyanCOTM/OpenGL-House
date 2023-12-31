#version 400 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBiTangent;

uniform mat4 proj_matrix;
uniform mat4 camera_matrix;
uniform mat4 normal_matrix;
uniform mat4 model_matrix;
uniform mat4 light_proj_matrix;
uniform mat4 light_cam_matrix;

uniform vec3 EyePosition;
out vec4 Position;
out vec3 Normal;
out vec3 View;
out vec4 LightPosition;
out vec2 texCoord;
out vec3 Tangent;
out vec3 BiTangent;

void main( )
{
    // Compute transformed vertex position in view space
    gl_Position = proj_matrix*(camera_matrix*(model_matrix*vPosition));

    // Compute n (transformed by normal matrix) (passed to fragment shader)
    Normal = vec3(normalize(normal_matrix * normalize(vec4(vNormal,0.0f))));

    // Compute vertex position in world coordinates (passed to fragment shader)
    Position = model_matrix*vPosition;

    // Compute v (camera location - transformed vertex) (passed to fragment shader)
    View = normalize(EyePosition - Position.xyz);

    // Pass texture coordinate to frag shader
    texCoord = vTexCoord;

    // Compute vertex in light space
    LightPosition = light_proj_matrix*(light_cam_matrix*Position);

    // Compute tangent space vectors
    Tangent = vec3(normalize(normal_matrix*normalize(vec4(vTangent, 0.0))));
    BiTangent = cross(Normal, Tangent);
}
