#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main() {    
    //gl_Position = gl_in[0].gl_Position; 
    for (int i=0;i<3;i++){
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}  