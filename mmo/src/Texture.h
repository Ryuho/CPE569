#ifndef _TEXTURE_H_
#define _TEXTURE_H_

struct TexOptions {
   TexOptions();
   TexOptions &wrap();
   TexOptions &clamp();

   int level;
   int wrap_s, wrap_t, min_filter, mag_filter;
   int internal_format, external_format, type;
};

struct Texture {
   Texture() { tex = -1; }
   Texture(int width, int height, void *data, TexOptions opt = TexOptions());

   void init(int width, int height, void *data, TexOptions opt = TexOptions());
   void bind(int slot = 0);

   unsigned tex;
   int width, height;
};

Texture fromTGA(const char *fn, TexOptions opt = TexOptions());

#endif