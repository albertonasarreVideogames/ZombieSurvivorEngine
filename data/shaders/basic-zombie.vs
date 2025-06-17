attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;
attribute vec4 a_color;

uniform vec3 u_camera_pos;

uniform mat4 u_model;
uniform mat4 u_viewprojection;
uniform mat4 u_model_rotate_torse;
uniform mat4 u_model_rotate_leg_r;
uniform mat4 u_model_rotate_leg_l;
uniform mat4 u_model_rotate;
uniform mat4 u_model_rotate_head;

uniform vec4 u_color;
//this will store the color for the pixel shader
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;
varying int vertexID;
varying vec4 m_color;

void main()
{	
	mat4 model = u_model  ;
	vertexID = gl_VertexID;
	m_color = u_color;

	// torso

	 if( vertexID >= 0 && vertexID < 228){
	
	 model = u_model * u_model_rotate_torse * u_model_rotate;
	
	 }

	//cabeza

	 else if( vertexID > 227 && vertexID < 612){
	
	 model =  u_model * u_model_rotate_head * u_model_rotate;
	
	 }

	 //pierna izquierda

	  else if( vertexID > 611 && vertexID < 708){
	
	   model =  u_model * u_model_rotate_leg_l * u_model_rotate;
	
	  }

	 //pierna derecha

	 else if( vertexID > 707 && vertexID < 20000){
	
	  model =  u_model * u_model_rotate_leg_r * u_model_rotate;
	
	 }

	//calcule the normal in camera space (the NormalMatrix is like ViewMatrix but without traslation)
	v_normal = (model * vec4( a_normal, 0.0) ).xyz;
	
	
	//calcule the vertex in object space
	v_position = a_vertex;
	v_world_position = (model * vec4( v_position, 1.0) ).xyz;
	
	//store the color in the varying var to use it from the pixel shader
	v_color = a_color;

	//store the texture coordinates
	v_uv = a_uv;

	//calcule the position of the vertex using the matrices
	gl_Position = u_viewprojection * vec4( v_world_position, 1.0 );
}