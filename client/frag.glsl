uniform float shift;

void main()
{
   gl_Color.r *= shift;
   gl_Color.g *= abs(shift*2.0-1.0);
   gl_Color.b *= 1.0 - shift;
	gl_FragColor = gl_Color;
}