#ifndef SPRITE_H
#define SPRITE_H
#include <LWCore/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWText.h>
#include <LWEXML.h>
#include <unordered_map>

struct Sprite{
	LWTexture *m_Texture;
	LWVector4f m_TextureBounds;
	LWVector2f m_Size;
	LWVector2f m_Anchor;

	Sprite(LWTexture *Tex, const LWVector2i &Pos, const LWVector2i &Size, const LWVector2f &Anchor);
};

class SpriteManager {
public:

	static bool XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X);

	bool InsertSprite(const LWText &Name, LWTexture *Tex, const LWVector2i &Pos, const LWVector2i &Size, const LWVector2f &Anchor);

	Sprite *Find(const LWText &Name);

	LWEAssetManager *GetAssetManager(void);

	SpriteManager(LWEAssetManager *AssetMan);
private:
	std::unordered_map<uint32_t, Sprite> m_SpriteMap;
	LWEAssetManager *m_AssetMan;
};

#endif