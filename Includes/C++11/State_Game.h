#ifndef STATE_GAME_H
#define STATE_GAME_H
#include "State.h"
#include <LWETypes.h>
#include <unordered_map>
#include <LWCore/LWVector.h>
#include "Sprite.h"
#include <random>

struct Player;

struct GameObjectDescriptor {
	enum {
		RedPlayer,
		BluePlayer,
		Bullet,
		ShieldFountain,
		AsteroidA,
		AsteroidB,
		AsteroidC,
		Mine,
		EnemyA,
		Count,
		SkippedIDs=4,
		MaxFrames=4
	};
	Sprite *m_SpriteList[MaxFrames];
	uint32_t m_SpriteCount;
	uint32_t m_Value;
	float m_Health;
	float m_Power;
};

struct GameObject {
	enum {
		ShieldNone,
		ShieldRed=1,
		ShieldGreen=2,
		ShieldBlue=4,
	};
	Sprite *m_Sprite;
	Player *m_Owner;
	LWVector2f m_Position;
	LWVector2f m_Velocity;
	uint32_t m_DescriptorID;
	uint32_t m_ShieldType;
	uint32_t m_SpawnTick;
	uint32_t m_DataTickA;
	uint32_t m_DataTickB;
	uint32_t m_Index;
	float m_Theta;
	float m_Health;
	float m_Radius;
};

struct Particle {
	LWVector4f m_Color;
	LWVector2f m_Position;
	LWVector2f m_Velocity;
	float m_Theta;
	uint32_t m_Life;
};

struct Player {
	GameObject *m_Object;
	float m_TargetDir;
	float m_ShootDir;
	uint32_t m_FireTick;
	uint32_t m_Score;
	uint32_t m_Lives;
	uint32_t m_InvCntr;
	bool m_DropShields;
};

struct Wavelet {
	uint32_t m_ID;
	uint32_t m_Count;
	uint32_t m_SpawnRate;
	uint32_t m_SpawnCounter;
	uint32_t m_SpawnShield;
};

struct Wave {
	enum {
		MaxWavelets = 32
	};
	Wavelet m_Waves[MaxWavelets];
	uint32_t m_WaveletCount;
};

struct Stars {
	LWVector2f m_Position;
	float m_Size;
};

class State_Game : public State {
public:
	enum {
		MaxObjects = 1024,
		MaxParticles = 1024,
		MaxWaves = 32,
		MaxStars = 128
	};

	static bool XMLParse(LWEXMLNode *Node, void *UserData, LWEXML *X);

	float Random(float Min, float Max);

	uint32_t Random(uint32_t Min, uint32_t Max);

	int32_t Random(int32_t Min, int32_t Max);

	LWVector2f MapScreenToField(const LWVector2f &ScreenCoord, const LWVector2f &WndSize);

	LWVector2f MapFieldToScreen(const LWVector2f &FieldCoord, const LWVector2f &WndSize);

	bool UpdatePlayer(Player &P, App *A);

	bool UpdateBullet(GameObject *B);

	bool UpdateAsteroid(GameObject *A);

	bool UpdateMine(GameObject *M);

	bool UpdateWave(void);

	bool ProcessCollision(GameObject *A, GameObject *B, App *Ap);

	bool PushWave(Wave &w);

	State &Update(bool Tick, App *A, uint64_t lCurrentTime);

	State &DrawFrame(App *A, Frame *F, Renderer *R);

	State &ProcessInput(App *A, LWWindow *Window);

	State &Activated(App *A);

	State &Deactivated(App *A);

	void RestartBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	void MenuBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	bool RemoveObject(GameObject *Obj);

	GameObject *Spawn(uint32_t DescriptorID, const LWVector2f &Pos, float Theta, bool TempInvulnerable);

	State_Game &SpawnParticles(const LWVector2f &Pos, float Radius, const LWVector4f &Color, uint32_t ParticleCnt);

	GameObjectDescriptor &GetDescriptor(uint32_t ID);

	SpriteManager *GetSpriteManager(void);

	State_Game(App *A, LWEUIManager *UIMan, SpriteManager *SpriteMan);
private:
	std::mt19937 m_Random;
	LWVector2f m_FieldSize;
	GameObjectDescriptor m_DescriptorList[GameObjectDescriptor::Count];
	GameObject m_ObjectPool[MaxObjects];
	GameObject *m_ObjectList[MaxObjects];
	Particle m_ParticlePool[MaxParticles];
	Wave m_WaveList[MaxWaves];
	Stars m_Stars[MaxStars];
	Wave m_ActiveWave;
	Sprite *m_ParticleSprite;
	Sprite *m_ShieldBaseSprite;
	Sprite *m_ShieldGreenBlueSprite;
	Sprite *m_ShieldGreenRedSprite;
	Sprite *m_ShieldBlueRedSprite;
	Sprite *m_ShieldAllSprite;
	Sprite *m_StarSprite;
	Player m_PlayerOne;
	Player m_PlayerTwo;
	
	LWEUI *m_GameMenu;
	LWEUI *m_PauseMenu;
	LWEUILabel *m_ScoreLbl;
	LWEUILabel *m_WaveLbl;

	SpriteManager *m_SpriteManager;
	uint32_t m_ObjectCount;
	uint32_t m_ParticleCount;
	uint32_t m_WaveCount;
	uint32_t m_WaveTimer;
	uint32_t m_Tick;
	uint32_t m_Wave;

};

#endif