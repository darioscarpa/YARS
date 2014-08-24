#ifndef AOBUFFER_H
#define	AOBUFFER_H


class AoBuffer {
public:
    enum AOBUFFER_TEXTURE_TYPE {
        AOBUFFER_TEXTURE_TYPE_AO,        
		AOBUFFER_NUM_TEXTURES
    };    
	static const int AOBUFFER_TEXTURE_AVAILABLE  = AOBUFFER_NUM_TEXTURES;

};

#endif	

