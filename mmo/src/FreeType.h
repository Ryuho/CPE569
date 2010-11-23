#ifndef FREE_NEHE_H
#define FREE_NEHE_H

///Wrap everything in a namespace, that we can use common
///function names like "print" without worrying about
///overlapping with anyone else's code.
namespace freetype {

//This holds all of the information related to any
//freetype font that we want to create.  
struct font_data {
	float h;			///< Holds the height of the font.
	unsigned int * textures;	///< Holds the texture id's 
	unsigned int list_base;	///< Holds the first display list id

	//The init function will create a font of
	//of the height h from the file fname.
	void init(const char * fname, unsigned int h);

	//Free all the resources assosiated with the font.
	void clean();
};

//The flagship function of the library - this thing will print
//out text at window coordinates x,y, using the font ft_font.
//The current modelview matrix will also be applied to the text. 
void print(const font_data &ft_font, float x, float y, const char *fmt, ...) ;

}

#endif
