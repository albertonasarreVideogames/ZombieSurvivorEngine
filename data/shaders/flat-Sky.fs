
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform float u_time;

void main()
{

	float time_factor = mod(u_time,40)/20;
	float night =1;

	vec4 color_atardecer_sumar_alAzul = vec4(0.9,127 / 255,80 / 255,1);

	if(time_factor>1){
	
	time_factor = 0;
	night=0;
	}

	vec4 color_luminosidad= vec4(1,1,1,time_factor * night);

	vec2 uv = v_uv;

	//color final

	vec4 color_final = vec4(0,0,0,0);

	if(time_factor<0.5){

	color_final = (time_factor*2*u_color) + ((1-time_factor*2)*color_atardecer_sumar_alAzul);
	}else{
	
	color_final = (time_factor*color_atardecer_sumar_alAzul) + ((1-time_factor)*u_color);

	}

	gl_FragColor = color_final * night * texture2D( u_texture, uv );
}
