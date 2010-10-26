#include <fstream>
#include "Texture.h"
#include "GLUtil.h"
#include <boost/shared_ptr.hpp>

using namespace std;
using boost::shared_ptr;

TexOptions::TexOptions()
{
   level = 0;
   wrap_s = GL_REPEAT;
   wrap_t = GL_REPEAT;
   min_filter = GL_LINEAR;
   mag_filter = GL_LINEAR;
   internal_format = GL_RGB8;
   external_format = GL_RGB8;
   type = GL_UNSIGNED_BYTE;
}

TexOptions &TexOptions::wrap()
{
   wrap_s = GL_REPEAT;
   wrap_t = GL_REPEAT;
   return *this;
}

TexOptions &TexOptions::clamp()
{
   wrap_s = GL_CLAMP_TO_EDGE;
   wrap_t = GL_CLAMP_TO_EDGE;
   return *this;
}

Texture::Texture(int width, int height, void *data, TexOptions opt)
{
   init(width, height, data, opt);
}

void Texture::init(int width, int height, void *data, TexOptions opt)
{
   this->width = width;
   this->height = height;

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, opt.min_filter);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, opt.mag_filter);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, opt.wrap_s);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, opt.wrap_t);

   glTexImage2D(GL_TEXTURE_2D, opt.level, opt.internal_format, 
      width, height, 0, opt.external_format, opt.type, data);

   glUtilPrintError("Texture Init");
}

void Texture::bind(int slot)
{
   if (tex >= 0) {
      glActiveTexture(GL_TEXTURE0 + slot);
      glBindTexture(GL_TEXTURE_2D, tex);
      glUtilPrintError("Texture Bind");
   }
}

namespace {
   // Copied from http://github.com/jckarter/ch4-flag/blob/master/file-util.c
   struct TGA_Header {
       char  id_length;
       char  color_map_type;
       char  data_type_code;
       unsigned char  color_map_origin[2];
       unsigned char  color_map_length[2];
       char  color_map_depth;
       unsigned char  x_origin[2];
       unsigned char  y_origin[2];
       unsigned char  width[2];
       unsigned char  height[2];
       char  bits_per_pixel;
       char  image_descriptor;
   };

   short makeShort(unsigned char *bytes)
   {
      return bytes[0] | ((char)bytes[1] << 8);
   }
}

// Based on read_tga at http://github.com/jckarter/ch4-flag/blob/master/file-util.c
// but modified to use streams
Texture fromTGA(const char *fn, TexOptions opt)
{
   ifstream file;
   TGA_Header header;
   int width, height;

   file.exceptions(ifstream::failbit | ifstream::badbit | ifstream::eofbit);
   try {
      file.open(fn, ios::binary);
      file.read((char*)&header, sizeof(TGA_Header));
   
      if (file.eof())
         throw "Texture %s has an incomplete header\n";
      if (header.data_type_code != 2)
         throw "Texture %s is not an uncompressed TGA file\n";
      if (header.bits_per_pixel != 24 && header.bits_per_pixel != 32)
         throw "Texture %s does not have 24 or 32 bits per pixel\n";

      file.ignore(makeShort(header.color_map_length) * (header.color_map_depth/8));
      width = makeShort(header.width);
      height = makeShort(header.height);

      int pixelSize = width * height * (header.bits_per_pixel/8);
      shared_ptr<char> pixels(new char[pixelSize]);
      file.read(pixels.get(), pixelSize);

      if (header.bits_per_pixel == 24) {
         opt.external_format = GL_BGR;
         opt.internal_format = GL_RGB;
         return Texture(width, height, pixels.get(), opt);
      } else if (header.bits_per_pixel == 32) {
         opt.external_format = GL_BGRA;
         opt.internal_format = GL_RGBA;
         return Texture(width, height, pixels.get(), opt);
      } else {
         assert(0 && "Should not get here?");
         return Texture();
      }

   } catch (ifstream::failure e) {
      printf("Error reading texture: %s\n", fn);
      return Texture();
   } catch (const char *e) {
      printf(e, fn);
      return Texture();
   }
}