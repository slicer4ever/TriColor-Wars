#include "Sprite.h"
#include <LWVideo/LWTexture.h>
#include <LWEAsset.h>

Sprite::Sprite(LWTexture *Tex, const LWVector2i &Pos, const LWVector2i &Size, const LWVector2f &Anchor) {
	m_Texture = Tex;
	LWVector2i TexSize = Tex->Get2DSize();
	LWVector2f fTexSize = 1.0f / LWVector2f((float)TexSize.x, (float)TexSize.y);
	m_Size = LWVector2f((float)Size.x, (float)Size.y);
	m_Anchor = Anchor;
	m_TextureBounds = LWVector4f(((float)Pos.x + 0.5f)*fTexSize.x, ((float)Pos.y + 0.5f)*fTexSize.y, ((float)(Pos.x + Size.x) - 0.5f)*fTexSize.x, ((float)(Pos.y + Size.y) - 0.5f)*fTexSize.y);
}


bool SpriteManager::XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X) {
	SpriteManager *SMan = (SpriteManager*)UserData;
	LWEAssetManager *AMan = SMan->GetAssetManager();
	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = LWText::CompareMultiple(C->m_Name, 1, "Sprite");
		if (i == 0xFFFFFFFF) continue;
		LWXMLAttribute *NameAttr = C->FindAttribute("Name");
		LWXMLAttribute *TexNameAttr = C->FindAttribute("TexName");
		LWXMLAttribute *PosAttr = C->FindAttribute("Pos");
		LWXMLAttribute *SizeAttr = C->FindAttribute("Size");
		LWXMLAttribute *AnchorAttr = C->FindAttribute("Anchor");
		if (!NameAttr || !TexNameAttr) continue;
		LWTexture *Tex = AMan->GetAsset<LWTexture>(TexNameAttr->m_Value);
		if(!Tex){
			std::cout << "Could not find texture: " << TexNameAttr->m_Value << std::endl;
			continue;
		}
		LWVector2i Pos = LWVector2i(0);
		LWVector2f Anchor = LWVector2f(0.0f);
		LWVector2i Size = Tex->Get2DSize();
		if (PosAttr) sscanf(PosAttr->m_Value, "%d | %d", &Pos.x, &Pos.y);
		if (SizeAttr) sscanf(SizeAttr->m_Value, "%d | %d", &Size.x, &Size.y);
		if (AnchorAttr) sscanf(AnchorAttr->m_Value, "%f | %f", &Anchor.x, &Anchor.y);
		SMan->InsertSprite(NameAttr->m_Value, Tex, Pos, Size, Anchor);
	}
	return true;
}

LWEAssetManager *SpriteManager::GetAssetManager(void) {
	return m_AssetMan;
}

bool SpriteManager::InsertSprite(const LWText &Name, LWTexture *Tex, const LWVector2i &Pos, const LWVector2i &Size, const LWVector2f &Anchor) {
	auto Res = m_SpriteMap.emplace(std::pair<uint32_t, Sprite>(Name.GetHash(), Sprite(Tex, Pos, Size, Anchor)));
	if (!Res.second) std::cout << "Error name collision with: " << Name.GetCharacters() << std::endl;
	return Res.second;
}

Sprite *SpriteManager::Find(const LWText &Name) {
	auto Iter = m_SpriteMap.find(Name.GetHash());
	return Iter == m_SpriteMap.end() ? nullptr : &Iter->second;
}

SpriteManager::SpriteManager(LWEAssetManager *AssetMan) : m_AssetMan(AssetMan) {
}