#include "Renderer.h"
#include <LWVideo/LWVideoDriver.h>
#include <LWPlatform/LWWindow.h>
#include <LWEAsset.h>
#include <LWCore/LWMatrix.h>

Frame *Renderer::BeginFrame(void) {
	if (m_WriteFrame - m_ReadFrame >= MaxFrames-1) return nullptr;
	uint32_t WFrame = m_WriteFrame%MaxFrames;
	m_Frames[WFrame].m_UIFrame.m_TextureCount = 0;
	m_Frames[WFrame].m_UIFrame.m_Mesh->SetActiveVertexCount(0);
	m_Frames[WFrame].m_TextureCount = 0;
	m_Frames[WFrame].m_SpriteMesh->SetActiveVertexCount(0);
	return &m_Frames[WFrame];
}

Renderer &Renderer::EndFrame(void) {
	m_WriteFrame++;
	return *this;
}

Renderer &Renderer::WriteSprite(Frame *F, Sprite *Sprite, const LWVector2f &Position, float Theta, const LWVector2f &Scale, const LWVector4f &ColorMult) {
	LWMesh<LWVertexUI> *M = F->m_SpriteMesh;
	if (!F->m_TextureCount || F->m_Textures[F->m_TextureCount - 1] != Sprite->m_Texture) {
		if (F->m_TextureCount >= Frame::MaxTextures) return *this;
		F->m_Textures[F->m_TextureCount] = Sprite->m_Texture;
		F->m_VertexCount[F->m_TextureCount] = 0;
		F->m_TextureCount++;
	}
	if (!M->CanWriteVertices(6)) return *this;
	uint32_t o = M->WriteVertices(6);
	LWVector2f Rot = LWVector2f::MakeTheta(Theta);
	LWVector2f hSize = Sprite->m_Size*0.5f*Scale;
	LWVector2f TopLeft = LWVector2f(-hSize.x, hSize.y) - Sprite->m_Anchor;
	LWVector2f TopRight = LWVector2f(hSize.x, hSize.y) - Sprite->m_Anchor;
	LWVector2f BtmLeft = LWVector2f(-hSize.x, -hSize.y) - Sprite->m_Anchor;
	LWVector2f BtmRight = LWVector2f(hSize.x, -hSize.y) - Sprite->m_Anchor;
	TopLeft = LWVector2f(TopLeft.x*Rot.x - TopLeft.y*Rot.y, TopLeft.x*Rot.y + TopLeft.y*Rot.x) + Position;
	TopRight = LWVector2f(TopRight.x*Rot.x - TopRight.y*Rot.y, TopRight.x*Rot.y + TopRight.y*Rot.x) + Position;
	BtmLeft = LWVector2f(BtmLeft.x*Rot.x - BtmLeft.y*Rot.y, BtmLeft.x*Rot.y + BtmLeft.y*Rot.x) + Position;
	BtmRight = LWVector2f(BtmRight.x*Rot.x - BtmRight.y*Rot.y, BtmRight.x*Rot.y + BtmRight.y*Rot.x) + Position;
	
	LWVertexUI *V = M->GetVertexAt(o);
	*(V + 0) = { LWVector4f(TopLeft.x, TopLeft.y, 0.0f, 1.0f), ColorMult, LWVector4f(Sprite->m_TextureBounds.x, Sprite->m_TextureBounds.y, 0.0f, 0.0f) };
	*(V + 1) = { LWVector4f(BtmLeft.x, BtmLeft.y, 0.0f, 1.0f), ColorMult, LWVector4f(Sprite->m_TextureBounds.x, Sprite->m_TextureBounds.w, 0.0f, 0.0f) };
	*(V + 2) = { LWVector4f(BtmRight.x, BtmRight.y, 0.0f, 1.0f), ColorMult, LWVector4f(Sprite->m_TextureBounds.z, Sprite->m_TextureBounds.w, 0.0f, 0.0f) };
	*(V + 3) = { LWVector4f(TopLeft.x, TopLeft.y, 0.0f, 1.0f), ColorMult, LWVector4f(Sprite->m_TextureBounds.x, Sprite->m_TextureBounds.y, 0.0f, 0.0f) };
	*(V + 4) = { LWVector4f(BtmRight.x, BtmRight.y, 0.0f, 1.0f), ColorMult, LWVector4f(Sprite->m_TextureBounds.z, Sprite->m_TextureBounds.w, 0.0f, 0.0f) };
	*(V + 5) = { LWVector4f(TopRight.x, TopRight.y, 0.0f, 1.0f), ColorMult, LWVector4f(Sprite->m_TextureBounds.z, Sprite->m_TextureBounds.y, 0.0f, 0.0f) };
	F->m_VertexCount[F->m_TextureCount - 1] += 6;
	return *this;
}


Renderer &Renderer::UpdateSize(LWWindow *Window) {
	LWVector2i WndSize = Window->GetSize();
	LWVector2f WndSizef = LWVector2f((float)WndSize.x, (float)WndSize.y);
	LWMatrix4f Ortho = LWMatrix4f::Ortho(0.0f, WndSizef.x, 0.0f, WndSizef.y, 0.0f, 1.0f);
	memcpy(m_UIUniform->GetLocalBuffer(), &Ortho, sizeof(LWMatrix4f));
	m_UIUniform->SetEditLength(sizeof(LWMatrix4f)).MarkUpdated();
	m_Driver->ViewPort();
	m_SizeChanged = false;
	return *this;
}

Renderer &Renderer::UpdateFrame(Frame &Frame) {
	Frame.m_UIFrame.m_Mesh->MarkUploadable();
	Frame.m_SpriteMesh->MarkUploadable();
	return *this;
}

Renderer &Renderer::RenderUIFrame(LWEUIFrame &UIFrame) {
	uint32_t o = 0;
	for (uint32_t i = 0; i < UIFrame.m_TextureCount; i++) {
		LWShader *Shader = UIFrame.m_Textures[i] ? (UIFrame.m_FontTexture[i] ? m_FontShader : m_UITexShader) : m_UIColorShader;
		Shader->SetTexture(0, UIFrame.m_Textures[i]);
		m_Driver->DrawMesh(Shader, LWVideoDriver::Triangle, UIFrame.m_Mesh, UIFrame.m_VertexCount[i], o);
		o += UIFrame.m_VertexCount[i];
	}
	return *this;
}

Renderer &Renderer::RenderFrame(Frame &F) {
	uint32_t o = 0;
	for (uint32_t i = 0; i < F.m_TextureCount; i++) {
		m_SimpleShader->SetTexture(0, F.m_Textures[i]);
		m_Driver->DrawMesh(m_SimpleShader, LWVideoDriver::Triangle, F.m_SpriteMesh, F.m_VertexCount[i], o);
		o += F.m_VertexCount[i];
	}
	RenderUIFrame(F.m_UIFrame);
	return *this;
}

Renderer &Renderer::Render(LWWindow *Window) {
	if (Window->GetFlag()&LWWindow::SizeChanged) m_SizeChanged = true;
	if (!m_Driver->Update()) return *this;
	if (m_SizeChanged) UpdateSize(Window);
	if (!m_WriteFrame) return *this; //A frame hasn't been written yet!
	if (m_ReadFrame != m_WriteFrame){
		UpdateFrame(m_Frames[m_ReadFrame%MaxFrames]);
		m_ReadFrame++;
	}
	uint32_t CurrFrame = (m_ReadFrame - 1)%MaxFrames;
	m_Driver->Clear(LWVideoDriver::Color, 0xFF, 0.0f, 0);
	RenderFrame(m_Frames[CurrFrame]);
	m_Driver->Present(nullptr, 1);
	return *this;
}

Renderer::Renderer(LWWindow *Window, LWVideoDriver *Driver, LWAllocator &Allocator, LWEAssetManager *AssetMan) : m_Driver(Driver), m_ReadFrame(0), m_WriteFrame(0), m_SizeChanged(true) {

	for (uint32_t i = 0; i < MaxFrames; i++) {
		LWVideoBuffer *UIBuffer = m_Driver->CreateVideoBuffer(LWVideoBuffer::Vertex, sizeof(LWVertexUI)*MaxUIElements * 6, Allocator, LWVideoBuffer::LocalCopy | LWVideoBuffer::WriteDiscardable, nullptr);
		LWVideoBuffer *SpriteBuffer = m_Driver->CreateVideoBuffer(LWVideoBuffer::Vertex, sizeof(LWVertexUI)*MaxSprites * 6, Allocator, LWVideoBuffer::LocalCopy | LWVideoBuffer::WriteDiscardable, nullptr);
		m_Frames[i].m_UIFrame.m_Mesh = LWVertexUI::MakeMesh(Allocator, UIBuffer, 0);
		m_Frames[i].m_SpriteMesh = LWVertexUI::MakeMesh(Allocator, SpriteBuffer, 0);
	}
	m_DefaultState = m_Driver->CreateVideoState( LWVideoState::CULL_CW | LWVideoState::BLENDING, LWVideoState::BLEND_SRC_ALPHA, LWVideoState::BLEND_ONE_MINUS_SRC_ALPHA, 0, Allocator);
	m_UIUniform = m_Driver->CreateVideoBuffer(LWVideoBuffer::Uniform, sizeof(LWMatrix4f), Allocator, LWVideoBuffer::LocalCopy | LWVideoBuffer::WriteDiscardable, nullptr);

	m_FontShader = AssetMan->GetAsset("FontShader")->AsShader();
	m_UITexShader = AssetMan->GetAsset("UITextureShader")->AsShader();
	m_UIColorShader = AssetMan->GetAsset("UIColorShader")->AsShader();
	m_SimpleShader = AssetMan->GetAsset("SimpleShader")->AsShader();

	m_FontShader->SetUniformBlock(LWSHADER_BLOCKUNIFORM, m_UIUniform);
	m_UITexShader->SetUniformBlock(LWSHADER_BLOCKUNIFORM, m_UIUniform);
	m_UIColorShader->SetUniformBlock(LWSHADER_BLOCKUNIFORM, m_UIUniform);
	m_SimpleShader->SetUniformBlock(LWSHADER_BLOCKUNIFORM, m_UIUniform);

	m_Driver->SetVideoState(m_DefaultState);
}

Renderer::~Renderer() {
	for (uint32_t i = 0; i < MaxFrames; i++) {
		m_Driver->DestroyVideoBuffer(m_Frames[i].m_UIFrame.m_Mesh->GetMeshBuffer());
		m_Driver->DestroyVideoBuffer(m_Frames[i].m_SpriteMesh->GetMeshBuffer());
		LWAllocator::Destroy(m_Frames[i].m_UIFrame.m_Mesh);
		LWAllocator::Destroy(m_Frames[i].m_SpriteMesh);
	}
	m_Driver->DestroyVideoState(m_DefaultState);
	m_Driver->DestroyVideoBuffer(m_UIUniform);
}