#pragma once

#ifndef RENDER_TO_TEXTURE
#define RENDER_TO_TEXTURE

#include "includes.h"
#include "framework.h"
#include "texture.h"

class RenderToTexture : public Texture
{
public:
	bool generate_mipmaps;

	GLuint fbo;
	GLuint depthbuffer;

	static bool init();

	RenderToTexture();
	~RenderToTexture();

	bool create(int width, int height, bool generate_mipmaps = false);
	void enable();
	void disable();
};

#endif