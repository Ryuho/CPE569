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


int Animation::dirFace(vec2 dir)
{
   int adir = 0;
   if (fabs(dir.x) >= fabs(dir.y)) {
      if(dir.x > 0)
         adir = 1; //right
      else
         adir = 3; //left
   } else {
      if(dir.y > 0)
         adir = 0; // up
      else
         adir = 2; //down
   }
   return adir;
}

int Animation::dirFaceLR(vec2 dir)
{
   int adir = 0;
   if(dir.x > 0)
      adir = 1; //right
   else
      adir = 3; //left
   return adir;
}

void Animation::init(Sprite *sprite, Type type, bool alwaysAnim)
{
   this->sprite = sprite;
   this->type = type;
   this->alwaysAnim = alwaysAnim;
}

void Animation::draw(vec2 dir, bool moving)
{
   int dirIndex = 2; //down by default
   switch(this->type) {
      case Animation::Normal :
         dirIndex = dirFace(dir);
         break;
      case Animation::LeftRight :
         dirIndex = dirFaceLR(dir);
         break;
      case Animation::Forward :
         //printf("anim::draw forward todo\n");
         dirIndex = Animation::Down;
         break;
      default:
         printf("Error: anim::draw invalid animation type");
   }
   int frames = dirs[dirIndex].size();
   if(frames > 0) {
      int frame;
      if(alwaysAnim)
         frame = ((SDL_GetTicks() - animStart) / speed) % frames;
      else if(!moving)
         frame = 0;
      else
         frame = 1 + ((SDL_GetTicks() - animStart) / speed) % (frames - 1);
      sprite->draw(dirs[dirIndex][frame].x, 
         dirs[dirIndex][frame].y);
   } else
      printf("Error: unable to draw animation\n");
}

void Animation::draw()
{
   draw(vec2(0.1,-0.9), true);
}
