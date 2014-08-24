#ifndef SAAOGBUFFER_H
#define	SAAOGBUFFER_H

class SaAoGBuffer {
public:
    enum GBUFFER_TEXTURE_TYPE {        
         GBUFFER_TEXTURE_TYPE_DIFFUSE,
         GBUFFER_TEXTURE_TYPE_NORMAL,
		 GBUFFER_TEXTURE_TYPE_SPECULAR,			
         GBUFFER_NUM_TEXTURES
    };    
	static const int GBUFFER_TEXTURE_TYPE_DEPTH = GBUFFER_NUM_TEXTURES;
	static const int GBUFFER_TEXTURE_AVAILABLE  = GBUFFER_NUM_TEXTURES+1;   
};

#endif	
