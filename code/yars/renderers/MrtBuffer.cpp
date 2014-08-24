#include "MrtBuffer.h"

const std::vector<MrtBufferType> MrtBuffer::bufTypePresets = MrtBuffer::createBufTypePresets();

void MrtBuffer::destroy() {
	printf("MrtBuffer::destroy()\n");
	if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_textures[0] != 0) {
		glDeleteTextures( m_textures.size(), m_textures.data());
    }

	if (m_depthTexture != 0) {
		glDeleteTextures(1, &m_depthTexture);
	}
}



void MrtBuffer::init(unsigned int winWidth, unsigned int winHeight,  int firstTexOffset, 
					 MrtBufferType::OptionValues colorBufFmt, MrtBufferType::OptionValues depthBufFmt) {
					 //GLint colorBufInternalFormat, GLenum colorBufFormat, GLenum colorBufType,
					 //GLint depthBufInternalFormat, GLenum depthBufFormat, GLenum depthBufType) {
	printf("MrtBuffer::init(%u, %u, %d)\n", winWidth, winHeight, firstTexOffset);

	m_startTextureUnit = firstTexOffset;
	m_winWidth = winWidth;
	m_winHeight = winHeight;
	m_colorBufType = colorBufFmt; //getDepthBufferType()[colorBufFmt];//MrtBufferType("defaultcolorBufInternalFormat, colorBufFormat, colorBufType);
	m_depthBufType = depthBufFmt; //getDepthBufferType()[depthBufFmt];//MrtBufferType(depthBufInternalFormat, depthBufFormat, depthBufType);

	MrtBufferType colorBufType = bufTypePresets[colorBufFmt];
	MrtBufferType depthBufType = bufTypePresets[depthBufFmt];

	// Create the FBO
    glGenFramebuffers(1, &m_fbo);    
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    // Create the gbuffer textures
    glGenTextures(m_numBuffers, &m_textures[0]);	
	
    for (int i = 0 ; i < m_numBuffers ; i++) {
		printf("MrtBuffer texture %d: %d\n", i, m_textures[i]);
    	glBindTexture(GL_TEXTURE_2D, m_textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, colorBufType.internalFormat, winWidth, winHeight, 0, colorBufType.format, colorBufType.type, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, winWidth, winHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, winWidth, winHeight, 0, GL_RGB, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, winWidth, winHeight, 0, GL_RGB, GL_FLOAT, NULL);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// minimize need for guard bands
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i], 0);
    }

	GLenum *drawBuffers = new GLenum[m_numBuffers];
	for (int i = 0; i < m_numBuffers; ++i) {
		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(m_numBuffers, drawBuffers);
	delete[] drawBuffers;
	
	if (m_hasDepthStencil || m_hasDepth) {
		glGenTextures(1, &m_depthTexture);    
		glBindTexture(GL_TEXTURE_2D, m_depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, depthBufType.internalFormat, winWidth, winHeight, 0, depthBufType.format, depthBufType.type, NULL);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// minimize need for guard bands
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLenum depthAttachment = m_hasDepthStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, depthAttachment, GL_TEXTURE_2D, m_depthTexture, 0);
	} 
		

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
		//todo throw       
    }

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);    
}


void MrtBuffer::printFramebufferInfo(GLenum target) {
 
    int res, i = 0;
    GLint buffer;
 
    glBindFramebuffer(target, m_fbo);
 
    do {
        glGetIntegerv(GL_DRAW_BUFFER0+i, &buffer);
 
        if (buffer != GL_NONE) {
 
            printf("Shader Output Location %d - color attachment %d\n", i, buffer - GL_COLOR_ATTACHMENT0);
 
            glGetFramebufferAttachmentParameteriv(target, buffer, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &res);
            printf("\tAttachment Type: %s\n",  res==GL_TEXTURE?"Texture":"Render Buffer");
            glGetFramebufferAttachmentParameteriv(target, buffer,  GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &res);
            printf("\tAttachment object name: %d\n",res);
        }
        ++i; 
    } while (buffer != GL_NONE);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}