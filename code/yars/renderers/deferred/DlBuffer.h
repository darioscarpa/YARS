#ifndef DLBUFFER_H
#define	DLBUFFER_H


class DlBuffer {
public:
    enum DLBUFFER_TEXTURE_TYPE {
        DLBUFFER_TEXTURE_TYPE_DIFFUSE,
        DLBUFFER_TEXTURE_TYPE_SPECULAR,			
        DLBUFFER_NUM_TEXTURES
    };    
	static const int DLBUFFER_TEXTURE_AVAILABLE  = DLBUFFER_NUM_TEXTURES+1;
   
};

#endif	

