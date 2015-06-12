const char* onlyColorFS =
	"uniform sampler2D inSampler0;						"
	"uniform sampler2D inSampler1;						"
	"varying mediump vec2 v_UV0;						"
	"varying mediump vec2 v_UV1;						"
	"varying lowp vec4 v_Col;							"
	"													"
	"void main(void)									"
	"{													"
	"	mediump vec4 c0 = texture2D(inSampler0, v_UV0);	"
	"	mediump vec4 c = v_Col;							"
	"	c.w = c.w * c0.w;								"
	"	gl_FragColor = c;								"
	"}													";