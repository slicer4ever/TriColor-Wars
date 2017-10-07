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
		AsteroidA,
		AsteroidB,
		AsteroidC,
		Count,
		MaxFrames=4
	};
	Sprite *m_SpriteList[MaxFrames];
	uint32_t m_SpriteCount;
	uint32_t m_Value;
	float m_Health;
	float m_Power;
};

struct GameObject {
	Sprite *m_Sprite;
	Player *m_Owner;
	LWVector2f m_Position;
	LWVector2f m_Velocity;
	uint32_t m_DescriptorID;
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
};

struct Wave {
	enum {
		MaxWavelets = 32
	};
	uint32_t m_SpawnIDs[MaxWavelets];
	uint32_t m_SpawnCnt[MaxWavelets];
	uint32_t m_SpawnRates[MaxWavelets];
	uint32_t m_SpawnCounter[MaxWavelets];
	uint32_t m_WaveletCount;
};

class State_Game : public State {
public:
	enum {
		MaxObjects = 1024,
		MaxParticles = 1024,
		MaxWaves = 32
	};

	static bool XMLParse(LWEXMLNode *Node, void *UserData, LWEXML *X);

	float Random(float Min, float Max);

	uint32_t Random(uint32_t Min, uint32_t Max);

	int32_t Random(int32_t Min, int32_t Max);

	bool UpdatePlayer(Player &P);

	bool UpdateBullet(GameObject *B);

	bool UpdateAsteroid(GameObject *A);

	bool UpdateWave(void);

	bool ProcessCollision(GameObject *A, GameObject *B);

	bool PushWave(Wave &w);

	State &Update(bool Tick, App *A, uint64_t lCurrentTime);

	State &DrawFrame(App *A, Frame *F, Renderer *R);

	State &ProcessInput(App *A, LWWindow *Window);

	State &Activated(App *A);

	State &Deactivated(App *A);

	bool RemoveObject(GameObject *Obj);

	GameObject *Spawn(uint32_t DescriptorID, const LWVector2f &Pos, float Theta);

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
	Wave m_ActiveWave;
	Sprite *m_ParticleSprite;
	Player m_PlayerOne;
	Player m_PlayerTwo;
	
	LWEUI *m_GameMenu;
	LWEUILabel *m_ScoreLbl;

	SpriteManager *m_SpriteManager;
	uint32_t m_ObjectCount;
	uint32_t m_ParticleCount;
	uint32_t m_WaveCount;
	uint32_t m_Tick;
	uint32_t m_Wave;

};

#endif