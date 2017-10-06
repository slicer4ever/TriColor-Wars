#ifndef RENDERER_H
#define RENDERER_H
#include <LWEUIManager.h>
#include <LWVideo/LWTypes.h>
#include "Sprite.h"

struct Frame {
	enum {
		MaxTextures = 32
	};
	LWEUIFrame m_UIFrame;
	LWMesh<LWVertexUI> *m_SpriteMesh;
	LWTexture *m_Textures[MaxTextures];
	uint32_t m_VertexCount[MaxTextures];
	uint32_t m_TextureCount;
};

class Renderer {
public:
	enum {
		MaxFrames = 3,
		MaxUIElements = 2048,
		MaxSprites = 2048,
	};

	Frame *BeginFrame(void);

	Renderer &EndFrame(void);

	Renderer &WriteSprite(Frame *F, Sprite *Sprite, const LWVector2f &Position, float Theta, const LWVector2f &Scale, const LWVector4f &ColorMult);

	Renderer &UpdateSize(LWWindow *Window);

	Renderer &UpdateFrame(Frame &Frame);

	Renderer &RenderUIFrame(LWEUIFrame &UIFrame);

	Renderer &RenderFrame(Frame &Frame);

	Renderer &Render(LWWindow *Window);

	Renderer(LWWindow *Window, LWVideoDriver *Driver, LWAllocator &Allocator, LWEAssetManager *AssetMan);

	~Renderer();
private:
	Frame m_Frames[MaxFrames];
	LWVideoBuffer *m_UIUniform;
	LWVideoDriver *m_Driver;
	LWShader *m_FontShader;
	LWShader *m_UITexShader;
	LWShader *m_UIColorShader;
	LWShader *m_SimpleShader;
	LWVideoState *m_DefaultState;
	uint32_t m_WriteFrame;
	uint32_t m_ReadFrame;
	bool m_SizeChanged;
};

#endif