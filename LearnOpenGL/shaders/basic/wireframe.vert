layout (location = 0) in vec3 pos;


uniform mat4 mvp;

out VS_OUT {
    vec3 wpos;
} vs_out;



void main(){
    gl_Position = mvp * vec4(pos, 1.0);
}
