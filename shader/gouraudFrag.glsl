#version 330 core

out vec4 color;
in vec3 LightingColor;

uniform int draw_norm_flag; // draw norm
void main(){
    
    color = vec4(LightingColor,1.0);
    if(draw_norm_flag==1)
        color = vec4(0.5,0.7,0.2,9.0);
    
}
