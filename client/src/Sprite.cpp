#include "Sprite.h"
#include "GLUtil.h"

Sprite::Sprite(Texture tex, int sWidth, int sHeight)
   : tex(tex), sWidth(sWidth), sHeight(sHeight)
{

}

void Sprite::draw(int x, int y)
{
   float s1, s2, t1, t2;
   float sinc = 1.0/(tex.width/(float)sWidth);
   float tinc = 1.0/(tex.height/(float)sHeight);
   s1 = x * sinc;
   s2 = (x+1) * sinc;
   t1 = 1.0 - y * tinc;
   t2 = 1.0 - (y+1) * tinc;

   glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
   tex.bind();
   glBegin(GL_QUADS);
   
   glTexCoord2f(s1, t2);
   glVertex2f(-sWidth/2.0, -sHeight/2.0);

   glTexCoord2f(s2, t2);
   glVertex2f(sWidth/2.0, -sHeight/2.0);

   glTexCoord2f(s2, t1);
   glVertex2f(sWidth/2.0, sHeight/2.0);

   glTexCoord2f(s1, t1);
   glVertex2f(-sWidth/2.0, sHeight/2.0);

   glEnd();
  
}