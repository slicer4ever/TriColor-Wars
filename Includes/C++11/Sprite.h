#ifndef SPRITE_H
#define SPRITE_H
#include <LWCore/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <LWCore/LWVector.h>

struct Sprite{
	LWTexture *m_Texture;
	LWVector4f m_TextureBounds;
	LWVector2f m_Size;

	Sprite(LWTexture *Tex, const LWVector2i &Pos, const LWVector2i &Size);
};

#endif