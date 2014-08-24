#include <iostream>
#include "Texture.h"

#include <cmath>
#include <algorithm>

double Log2( double n ) {  
    // log(n)/log(2) ==  log2
    return log( n ) / log( 2 );  
}

bool Texture::loadFromFile(bool buildMipmaps) {	
	ILboolean success;	
	ilInit(); 

	ILuint imageId;
	ilGenImages(1, &imageId);

	ilBindImage(imageId); /* Binding of DevIL image name */
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 

	//success = ilLoadImage(std::wstring(m_fileName.begin(), m_fileName.end()).c_str);
	success = ilLoadImage((ILstring)m_filename.c_str());

	if (success) {
			/* Convert image to RGBA */
			ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE); 
			
			GLsizei width  = ilGetInteger(IL_IMAGE_WIDTH);
			GLsizei height = ilGetInteger(IL_IMAGE_HEIGHT);
			//GLint num_mipmaps = floor(Log2((width>height?width:height)))+1;
			
			glGenTextures(1, &m_textureId);
			glBindTexture(GL_TEXTURE_2D, m_textureId);

			if (buildMipmaps) {
				GLint num_mipmaps = floor(Log2(std::max(width,height)))+1;			
				glTexStorage2D(GL_TEXTURE_2D, num_mipmaps, GL_RGBA8, width, height);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
				//glTexSubImage2D(GL_TEXTURE_2D, 0​, 0, 0, width​, height​, GL_BGRA, GL_UNSIGNED_BYTE, ilGetData())
				glGenerateMipmap(GL_TEXTURE_2D);  //Generate num_mipmaps number of mipmaps here.
				printf("tex %d - Loaded image: %s (%dx%d, %d mipmap levels) \n", m_textureId, m_filename.c_str(), width, height, num_mipmaps);
			
				/////////////////////
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // LINEAR | NEAREST
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  		
			} else {
				glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
				printf("tex %d - Loaded image: %s (%dx%d, no mipmaps) \n", m_textureId, m_filename.c_str(), width, height);
				//////////////////////
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}	
			/*bool enableFiltering = true;
			if (enableFiltering) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // LINEAR | NEAREST
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  		
			} else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}*/
			m_hasMipmaps = buildMipmaps;
	} else {
			printf("Couldn't load Image: %s\n", m_filename.c_str());
			ILenum Error;
			while ((Error = ilGetError()) != IL_NO_ERROR) { 
				//printf("%d: %s/n", Error, iluErrorString()); 
				printf("%d: \n", Error); 
			}
			//TODO throw exception
	}
	ilDeleteImages(1, &imageId);
	return success;	
}



void Texture::setWrapMode(GLenum textureUnit, TextureClampMode_t clampMode) {
	glActiveTexture(textureUnit);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
}

//MAG
// NEAREST | LINEAR
//MIN
// NEAREST | LINEAR - no mipmapping
// NEAREST_MIPMAP_NEAREST | NEAREST_MIPMAP_LINEAR | LINEAR_MIPMAP_NEAREST | LINEAR_MIPMAP_LINEAR - mipmapping
/*void Texture::setFilteringEnabled(GLenum textureUnit, bool enabled) {
	glActiveTexture(textureUnit);
	if (enabled) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // LINEAR | NEAREST
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  		
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
}*/