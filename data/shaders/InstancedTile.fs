
//varying vec2 v_uv;
//uniform sampler2D u_texture;
varying vec4 v_color;

void main()
{
	//vec2 uv = v_uv;
	///gl_FragColor = vec4(1.0,1.0,1.0,0.8);
	gl_FragColor = v_color;
}
