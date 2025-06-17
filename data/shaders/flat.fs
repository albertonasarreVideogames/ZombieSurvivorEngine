
varying vec4 v_color;
uniform vec4 u_color;
uniform vec3 u_light_direction;
varying vec3 v_normal;
uniform float u_time;

void main()
{

	//changes hear
	vec4 color = v_color * u_color;
	//vec4 color = v_color;

	//Factor tiempo

	float time_factor = mod(u_time,40)/20;
	float night =1;
	if(time_factor>1){
	
	time_factor = 0;
	night=0;
	}

	//cambiamos la altura del light direcction segun el tiempo

	vec3 light_direcction = u_light_direction;

	light_direcction.x = cos(time_factor*3.14);
	light_direcction.y = sin(time_factor*3.14);
	light_direcction.z = 0;

	//normalizamos las variables

	//

	vec3 N=normalize(v_normal);
	vec3 L=normalize(light_direcction);
	//dot product (proyeccion) de la normal sobre la iluminacion
	float NdotL =clamp(dot(N,L),0.0,1.0);

	//Cell shading

	//NdotL = round(NdotL *4.0)/4.0;

	// Naranja
	
	vec3 orange_color = vec3(1.0,112/255,40/255); 

	//Color del sol
	//vec3 Sun_color = vec3(1.0,1.0,1.0)*sin(time_factor*3.14); //naranja 255 112 40


	float desfase2 = 0;

	if(time_factor>0.5){
	
	desfase2 = -3.14/2;
	//desfase2 = 0;
	}

	vec3 Sun_color = (vec3(1.0,1.0,1.0)*sin(time_factor*3.14)) + (((1- sin(time_factor*3.14 + desfase2))*orange_color))*night; 


	

	//luz obtenida de multiplicar por el sol
	vec3 light = NdotL * Sun_color;


	//interpolaed light

	float desfase = 0;

	if(time_factor>0.5){
	
	//desfase = -3.14/2;
	desfase2 = 0;
	}

	//vec3 interpolated_light = (vec3(0.2,0.2,0.2)*sin(time_factor*3.14));
	vec3 interpolated_light = (vec3(0.2,0.2,0.2)*sin(time_factor*3.14)) + (((1- sin(time_factor*3.14 + desfase))*orange_color*0.8)) * night;


	//le sumanos el ambient
	light += interpolated_light + vec3(0.2,0.2,0.2);	//naranja 255 112 40
	//color final
	color.xyz *= light;

	//color pixel final
	gl_FragColor = color;
}
