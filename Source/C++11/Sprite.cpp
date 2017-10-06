#include "Sprite.h"
#include <LWVideo/LWTexture.h>


Sprite::Sprite(LWTexture *Tex, const LWVector2i &Pos, const LWVector2i &Size) {
	m_Texture = Tex;
	LWVector2i TexSize = Tex->Get2DSize();
	m_Size = LWVector2f((float)TexSize.x, (float)TexSize.y);
	LWVector2f fTexSize = 1.0f / m_Size;
	m_TextureBounds = LWVector4f(((float)Pos.x + 0.5f)*fTexSize.x, ((float)Pos.y + 0.5f)*fTexSize.y, ((float)(Pos.x + Size.x) - 0.5f)*fTexSize.x, ((float)(Pos.y + Size.y) - 0.5f)*fTexSize.y);
}