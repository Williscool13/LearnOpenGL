#version 330 core

layout (location = 0) in vec3 pos; // position is in clip space, dont need to be transformed
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 directionaluv;

void main()
{
    gl_Position = proj * view * model * vec4(pos, 1);
    //gl_Position = vec4(pos, 1);
   
    // pos is also the direction of the directional uv
    directionaluv = pos;
}