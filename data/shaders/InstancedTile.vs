attribute vec3 a_vertex;
attribute mat4 u_model;
//attribute vec2 a_uv;

uniform vec3 u_maxpos;
uniform mat4 u_viewprojection;

//this will store the color for the pixel shader
varying vec3 v_position;
varying vec3 v_world_position;
varying vec4 v_color;
//varying vec2 v_uv;
void main()
{
	//calcule the vertex in object space
	v_position = a_vertex;
	v_world_position = (u_model * vec4( a_vertex, 1.0) ).xyz;
	//v_uv = a_uv;
	v_color = vec4(u_model[3].xyz / u_maxpos, 0.8);
	
	//calcule the position of the vertex using the matrices
	gl_Position = u_viewprojection * vec4( v_world_position, 1.0 );
}