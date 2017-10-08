#include "State_Game.h"
#include "App.h"
#include <LWPlatform/LWWindow.h>
#include <LWPlatform/LWInputDevice.h>
#include <LWEUIManager.h>
#include <LWEAsset.h>
#include <LWEUI/LWEUILabel.h>
#include "Renderer.h"
#include <algorithm>

bool State_Game::XMLParse(LWEXMLNode *Node, void *UserData, LWEXML *X) {
	State_Game *GState = (State_Game*)UserData;
	SpriteManager *SMan = GState->GetSpriteManager();
	auto ParseDescriptor = [](State_Game *GState, LWEXMLNode *Node, SpriteManager *SMan)->bool {
		const char *TypeNames[] = { "RedPlayer", "BluePlayer", "Bullet", "ShieldFountain", "AsteroidA", "AsteroidB", "AsteroidC", "Mine" };
		const uint32_t TypeCount = sizeof(TypeNames) / sizeof(TypeNames[0]);
		LWXMLAttribute *TypeAttr = Node->FindAttribute("Type");
		LWXMLAttribute *HealthAttr = Node->FindAttribute("Health");
		LWXMLAttribute *PowerAttr = Node->FindAttribute("Power");
		LWXMLAttribute *ValueAttr = Node->FindAttribute("Value");
		if (!TypeAttr) return false;
		uint32_t i = LWText::CompareMultiplea(TypeAttr->m_Value, TypeCount, TypeNames); 
		if (i >= TypeCount) { 
			std::cout << "Could not find type: " << TypeAttr->m_Value << std::endl;
			return false;
		}
		GameObjectDescriptor &Desc = GState->GetDescriptor(i);
		Desc.m_Health = HealthAttr ? (float)atof(HealthAttr->m_Value) : 0.0f;
		Desc.m_Power = PowerAttr ? (float)atof(PowerAttr->m_Value) : 0.0f;
		Desc.m_Value = ValueAttr ? atoi(ValueAttr->m_Value) : 0;
		Desc.m_SpriteCount = 0;
		for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
			uint32_t n = LWText::CompareMultiple(C->m_Name, 1, "Sprite");
			if (n == 0xFFFFFFFF) continue;
			if (Desc.m_SpriteCount >= GameObjectDescriptor::MaxFrames) {
				std::cout << "Frame count exceeded for: " << TypeNames[i] << std::endl;
				continue;
			}
			Desc.m_SpriteList[Desc.m_SpriteCount] = SMan->Find(C->m_Text);
			if (!Desc.m_SpriteList[Desc.m_SpriteCount]) {
				std::cout << "Sprite not found: '" << C->m_Text << "'" << std::endl;
				continue;
			}
			Desc.m_SpriteCount++;
		}
		return true;
	};

	auto ParseWave = [](State_Game *GState, LWEXMLNode *Node) {
		const char *ShieldNames[] = { "None", "Red", "Green", "RedGreen", "Blue", "BlueRed", "BlueGreen", "All" };
		const uint32_t ShieldCount = sizeof(ShieldNames) / sizeof(ShieldNames[0]);
		Wave w;
		w.m_WaveletCount = 0;
		for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
			uint32_t i = LWText::CompareMultiple(C->m_Name, 4, "AsteroidA", "AsteroidB", "AsteroidC", "Mine");
			if (i == 0xFFFFFFFF) continue;
			i += GameObjectDescriptor::SkippedIDs; //skip first four id's.
			if (w.m_WaveletCount >= Wave::MaxWavelets) break;
			LWXMLAttribute *CountAttr = C->FindAttribute("Count");
			LWXMLAttribute *RateAttr = C->FindAttribute("Rate");
			LWXMLAttribute *OffsetAttr = C->FindAttribute("Offset");
			LWXMLAttribute *ShieldAttr = C->FindAttribute("Shield");
			w.m_Waves[w.m_WaveletCount].m_ID = i;
			w.m_Waves[w.m_WaveletCount].m_Count = CountAttr ? atoi(CountAttr->m_Value) : 0;
			w.m_Waves[w.m_WaveletCount].m_SpawnRate = RateAttr ? atoi(RateAttr->m_Value) : 0;
			w.m_Waves[w.m_WaveletCount].m_SpawnCounter = OffsetAttr ? atoi(OffsetAttr->m_Value) : 0;
			w.m_Waves[w.m_WaveletCount].m_SpawnShield = 0;
			w.m_Waves[w.m_WaveletCount].m_SpawnCounter += w.m_Waves[w.m_WaveletCount].m_SpawnRate;
			if (ShieldAttr) {
				uint32_t n = LWText::CompareMultiplea(ShieldAttr->m_Value, ShieldCount, ShieldNames);
				if (n != 0xFFFFFFFF) w.m_Waves[w.m_WaveletCount].m_SpawnShield = n;
			}
			w.m_WaveletCount++;
		}
		GState->PushWave(w);
	};

	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = LWText::CompareMultiple(C->m_Name, 2, "Object", "Wave");
		if(i==0) ParseDescriptor(GState, C, SMan);
		else if (i == 1) ParseWave(GState, C);
	}
	return true;
}

float State_Game::Random(float Min, float Max) {
	uint32_t v = m_Random()%100000;
	float vf = (float)v / 100000.0f;
	return Min + (Max - Min)*vf;
}

uint32_t State_Game::Random(uint32_t Min, uint32_t Max) {
	return Min + m_Random() % (Max - Min);
}

int32_t State_Game::Random(int32_t Min, int32_t Max) {
	return Min + m_Random() % (Max - Min);
}

LWVector2f State_Game::MapScreenToField(const LWVector2f &ScreenCoord, const LWVector2f &WndSize) {
	float Scale = std::min<float>(WndSize.x / m_FieldSize.x, WndSize.y / m_FieldSize.y);
	LWVector2f ScreenOffset = WndSize*0.5f - m_FieldSize*Scale*0.5f;
	return ((ScreenCoord-ScreenOffset) / Scale );
}

LWVector2f State_Game::MapFieldToScreen(const LWVector2f &FieldCoord, const LWVector2f &WndSize) {
	float Scale = std::min<float>(WndSize.x / m_FieldSize.x, WndSize.y / m_FieldSize.y);
	LWVector2f ScreenOffset = WndSize*0.5f - m_FieldSize*Scale*0.5f;
	return FieldCoord*Scale-ScreenOffset;
}

bool State_Game::UpdatePlayer(Player &P, App *A) {
	const float MaxSpeed = 7.0f;
	const float AccelSpeed = 1.0f;
	const uint32_t FireRate = 10;
	if (!P.m_Object) return false;
	P.m_Object->m_Sprite = m_DescriptorList[P.m_Object->m_DescriptorID].m_SpriteList[P.m_TargetDir <= LW_2PI ? 1 : 0];
	P.m_Object->m_Theta = P.m_ShootDir <= LW_2PI ? P.m_ShootDir : (P.m_TargetDir <= LW_2PI ? P.m_TargetDir : P.m_Object->m_Theta);
	
	if (P.m_TargetDir <= LW_2PI) {
		P.m_Object->m_Velocity += LWVector2f::MakeTheta(P.m_TargetDir)*AccelSpeed;
		if (P.m_Object->m_Velocity.LengthSquared() >= MaxSpeed*MaxSpeed) {
			P.m_Object->m_Velocity = P.m_Object->m_Velocity.Normalize()*MaxSpeed;
		}
	}
	if (P.m_FireTick) P.m_FireTick--;
	if (P.m_InvCntr) P.m_InvCntr--;
	if (!P.m_FireTick && P.m_ShootDir <= LW_2PI) {
		GameObject *Obj = Spawn(GameObjectDescriptor::Bullet, P.m_Object->m_Position + LWVector2f::MakeTheta(P.m_Object->m_Theta)*P.m_Object->m_Radius, P.m_Object->m_Theta, false);
		if (Obj) {
			Obj->m_ShieldType = P.m_Object->m_ShieldType;
			Obj->m_Owner = &P;
			P.m_FireTick = FireRate;
			A->DispatchAudio("Fire", 0.1f);
		}
	}
	if (P.m_DropShields) {
		P.m_Object->m_ShieldType = 0;
		P.m_DropShields = false;
	}
	return true;
}

bool State_Game::UpdateBullet(GameObject *B) {
	const float BulletSpeed = 15.0f;
	const float DecayRate = 0.1f;
	B->m_Health -= DecayRate;
	B->m_Velocity = LWVector2f::MakeTheta(B->m_Theta)*BulletSpeed;
	return true;
}

bool State_Game::UpdateAsteroid(GameObject *A) {
	const float InvDecay = 1.0f / 0.9f;
	const float ThetaInc = LW_2PI / 360.0f;
	if (A->m_DescriptorID == GameObjectDescriptor::AsteroidA) {
		if (A->m_Health <= 0.0f) {
			float T = A->m_Velocity.Theta();
			GameObject *ObjA = Spawn(GameObjectDescriptor::AsteroidB, A->m_Position, Random(-LW_PI*0.25f, LW_PI*0.25f) + T, false);
			GameObject *ObjB = Spawn(GameObjectDescriptor::AsteroidB, A->m_Position, Random(-LW_PI*0.25f, LW_PI*0.25f) + T, false);
			ObjA->m_Velocity = LWVector2f::MakeTheta(ObjA->m_Theta)*A->m_Velocity.Length();
			ObjB->m_Velocity = LWVector2f::MakeTheta(ObjB->m_Theta)*A->m_Velocity.Length();
		}
	}
	A->m_Velocity *= InvDecay;
	A->m_Theta += ThetaInc;
	return true;
}

bool State_Game::UpdateMine(GameObject *M) {
	const float InvDecay = 1.0f / 0.9f;
	const float ThetaInc = LW_2PI / 360.0f;
	const float MineRange = 250.0f;
	const float MineAttackSpeed = 5.5f;
	const float MineDriftSpeed = 0.8f;
	const uint32_t InvulnerableRate = 180;
	GameObject *PlayerOne = m_PlayerOne.m_Object;
	GameObject *PlayerTwo = m_PlayerTwo.m_Object;
	M->m_Velocity *= InvDecay;
	M->m_Theta += ThetaInc;
	if (M->m_DataTickA) {
		M->m_DataTickA--;
		return true;
	}
	GameObject *AttackObj = nullptr;
	if (PlayerOne) {
		if (M->m_Position.DistanceSquared(PlayerOne->m_Position) <= MineRange*MineRange) {
			if (m_PlayerOne.m_InvCntr) M->m_DataTickA = InvulnerableRate;
			else AttackObj = PlayerOne;
		}
	}
	if (PlayerTwo && !AttackObj) {
		if (M->m_Position.DistanceSquared(PlayerTwo->m_Position) <= MineRange*MineRange) {
			if (m_PlayerTwo.m_InvCntr) M->m_DataTickA = InvulnerableRate;
			else AttackObj = PlayerTwo;
		}
	}
	if(AttackObj){
		LWVector2f Dir = AttackObj->m_Position - M->m_Position;
		M->m_Velocity = Dir.Normalize()*MineAttackSpeed;
	} else M->m_Velocity = M->m_Velocity.Normalize()*MineDriftSpeed;

	return true;
}

bool State_Game::UpdateWave(void) {
	const uint32_t WaveBannerRate = 120;
	auto SpawnAsteroid = [](State_Game *G, uint32_t DescID, const LWVector2f &FieldSize)->GameObject* {
		LWVector2f hFieldSize = FieldSize*0.5f;
		const float MinSpeed = 4.0f;
		const float MaxSpeed = 5.0f;
		int32_t d = G->Random(0, 4);
		float v = G->Random(0.1f, 0.9f);
		float m = G->Random(2.0f, 30.0f);
		float s = G->Random(MinSpeed, MaxSpeed);
		LWVector2f Pos;
		if (d == 0) Pos = LWVector2f(FieldSize.x*v, m);
		else if (d == 1) Pos = LWVector2f(FieldSize.x*v, FieldSize.y - m);
		else if (d == 2) Pos = LWVector2f(m, FieldSize.y*v);
		else Pos = LWVector2f(FieldSize.x - m, FieldSize.y*v);
		GameObject *Obj = G->Spawn(DescID, Pos, G->Random(0.0f, LW_2PI), true);
		if (Obj) {
			Obj->m_Velocity = LWVector2f::MakeTheta(G->Random(0.0f, LW_2PI))*s;
		}

		return Obj;
	};

	auto SpawnMine = [](State_Game *G, uint32_t DescID, const LWVector2f &FieldSize)->GameObject* {

		const uint32_t InvulnerableRate = 180;
		const float MineDriftSpeed = 0.8f;
		float x = G->Random(0.1f, 0.9f);
		float y = G->Random(0.1f, 0.9f);
		GameObject *Obj = G->Spawn(DescID, FieldSize*LWVector2f(x, y), 0.0f, true);
		if (Obj) {
			Obj->m_Velocity = LWVector2f::MakeTheta(G->Random(0.0f, LW_2PI))*MineDriftSpeed;
			Obj->m_DataTickA = InvulnerableRate;
		}
		return Obj;
	};
	uint32_t RemainSpawn = 0;
	for (uint32_t i = 0; i < m_ActiveWave.m_WaveletCount; i++) {
		if(!m_ActiveWave.m_Waves[i].m_Count) continue;
		if (m_ActiveWave.m_Waves[i].m_SpawnCounter) m_ActiveWave.m_Waves[i].m_SpawnCounter--;
		if (!m_ActiveWave.m_Waves[i].m_SpawnCounter) {
			GameObject *Spawned = nullptr;
			switch (m_ActiveWave.m_Waves[i].m_ID) {
			case GameObjectDescriptor::AsteroidA:
			case GameObjectDescriptor::AsteroidB:
			case GameObjectDescriptor::AsteroidC:
				Spawned = SpawnAsteroid(this, m_ActiveWave.m_Waves[i].m_ID, m_FieldSize);
				break;
			case GameObjectDescriptor::Mine:
				Spawned = SpawnMine(this, m_ActiveWave.m_Waves[i].m_ID, m_FieldSize);
				break;
			}
			if (Spawned) {
				Spawned->m_ShieldType = m_ActiveWave.m_Waves[i].m_SpawnShield;
				m_ActiveWave.m_Waves[i].m_SpawnCounter = m_ActiveWave.m_Waves[i].m_SpawnRate;
				m_ActiveWave.m_Waves[i].m_Count--;
			}
		}
		RemainSpawn += m_ActiveWave.m_Waves[i].m_Count;
	}
	if (!RemainSpawn) {
		bool Finished = true;
		for (uint32_t i = 0; i < m_ObjectCount && Finished; i++) {
			Finished = (m_ObjectList[i]->m_DescriptorID < GameObjectDescriptor::SkippedIDs);
		}
		if (Finished) {
			m_Wave++;
			uint32_t w = m_Wave;
			if (w > 5) {
				if (w >= m_WaveCount) w -= 5;
			} else w = w%m_WaveCount;
			m_ActiveWave = m_WaveList[w];
			m_WaveTimer = WaveBannerRate;
			m_WaveLbl->SetText("Wave Cleared");
		}
	}
	if (m_WaveTimer) {
		m_WaveTimer--;
		if (!m_WaveTimer) m_WaveLbl->SetTextf("Wave: %.2d", (m_Wave+1));
	}


	return true;
}

bool State_Game::ProcessCollision(GameObject *A, GameObject *B, App *Ap) {
	const uint32_t InvunerableRate = 120;
	GameObject *PlayerObj = (A->m_DescriptorID == GameObjectDescriptor::BluePlayer || A->m_DescriptorID == GameObjectDescriptor::RedPlayer) ? A : ((B->m_DescriptorID == GameObjectDescriptor::BluePlayer || B->m_DescriptorID == GameObjectDescriptor::RedPlayer) ? B : nullptr);
	GameObject *BulletObj = A->m_DescriptorID == GameObjectDescriptor::Bullet ? A : (B->m_DescriptorID == GameObjectDescriptor::Bullet ? B : nullptr);
	GameObject *FountainObj = (A->m_DescriptorID == GameObjectDescriptor::ShieldFountain ? A : (B->m_DescriptorID == GameObjectDescriptor::ShieldFountain) ? B : nullptr);
	GameObject *OtherObj = (PlayerObj == A || BulletObj == A) ? B : A;
	if (FountainObj && BulletObj) return false;

	if (PlayerObj && FountainObj) {
		if (!FountainObj->m_SpawnTick) {
			PlayerObj->m_ShieldType = FountainObj->m_ShieldType;
			FountainObj->m_SpawnTick = InvunerableRate;
		}
		return true;
	}

	if (BulletObj && !PlayerObj) {
		Ap->DispatchAudio("Pop", 0.1f);
		BulletObj->m_Health = 0.0f; //Check shields match!
		if((OtherObj->m_ShieldType&BulletObj->m_ShieldType)!=0 || (BulletObj->m_ShieldType==0 && OtherObj->m_ShieldType==0)){
			if (!OtherObj->m_ShieldType) {
				OtherObj->m_Health -= m_DescriptorList[BulletObj->m_DescriptorID].m_Power;
				if (OtherObj->m_Health <= 0.0f) {
					if (BulletObj->m_Owner) {
						BulletObj->m_Owner->m_Score += m_DescriptorList[OtherObj->m_DescriptorID].m_Value;
					}
				}
			} else OtherObj->m_ShieldType ^= BulletObj->m_ShieldType;
		}
	}
	if (PlayerObj) {
		if (OtherObj->m_Owner) return false;
		Player *P = PlayerObj->m_Owner;
		if (P->m_InvCntr) return false;
		Ap->DispatchAudio("Pop", 0.2f);
		if (PlayerObj->m_ShieldType) PlayerObj->m_ShieldType = 0;
		else {
			if (!P->m_Lives) PlayerObj->m_Health = 0.0f;
			else P->m_Lives--;
		}
		P->m_InvCntr = InvunerableRate;
	}
	return true;
}

bool State_Game::PushWave(Wave &w) {
	if (m_WaveCount >= MaxWaves) return false;
	m_WaveList[m_WaveCount++] = w;
	return true;
}

State &State_Game::Update(bool Tick, App *A, uint64_t lCurrentTime) {
	const float VelDecayRate = 0.9f;
	const LWVector4f ShieldColors[] = { LWVector4f(1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 1.0f), LWVector4f(1.0f, 0.0f, 1.0f, 1.0f), LWVector4f(0.0f, 1.0f, 1.0f, 1.0f), LWVector4f(1.0f) };
	if (!Tick) return *this;
	Settings &Set = A->GetSettings();
	if (Set.m_GameMode == Settings::OnePlayer) {
		m_ScoreLbl->SetTextf("%.6d", m_PlayerOne.m_Score);
	} else if (Set.m_GameMode = Settings::TwoPlayer) {
		m_ScoreLbl->SetTextf("%.6d P1 | P2 %.6d", m_PlayerOne.m_Score, m_PlayerTwo.m_Score);
	}

	for (uint32_t i = 0; i < m_ParticleCount; i++) {
		m_ParticlePool[i].m_Position += m_ParticlePool[i].m_Velocity;
		m_ParticlePool[i].m_Life--;
		if (!m_ParticlePool[i].m_Life) {
			std::swap(m_ParticlePool[i], m_ParticlePool[m_ParticleCount - 1]);
			m_ParticleCount--;
			i--;
		}
	}
	if (m_PauseMenu->GetVisible()) return *this;
	for (uint32_t i = 0; i < m_ObjectCount; i++) {
		GameObject *O = m_ObjectList[i];
		O->m_Position += m_ObjectList[i]->m_Velocity;
		O->m_Velocity *= VelDecayRate;
		if (O->m_SpawnTick) O->m_SpawnTick--;
		if (O->m_DescriptorID != GameObjectDescriptor::Bullet) {
			if (O->m_Position.x >= m_FieldSize.x) O->m_Position.x -= m_FieldSize.x;
			if (O->m_Position.x < 0.0f) O->m_Position.x += m_FieldSize.x;
			if (O->m_Position.y >= m_FieldSize.y) O->m_Position.y -= m_FieldSize.y;
			if (O->m_Position.y < 0.0f) O->m_Position.y += m_FieldSize.y;
		}
		switch (O->m_DescriptorID) {
		case GameObjectDescriptor::Bullet:
			UpdateBullet(O);
			break;
		case GameObjectDescriptor::AsteroidA:
		case GameObjectDescriptor::AsteroidB:
		case GameObjectDescriptor::AsteroidC:
			UpdateAsteroid(O);
			break;
		case GameObjectDescriptor::Mine:
			UpdateMine(O);
		}
		if (O->m_Health <= 0.0f) {
			SpawnParticles(O->m_Position, 2.0f, ShieldColors[O->m_ShieldType], 15);
			if (RemoveObject(O)) i--;
		}
	}
	bool oneFinished = UpdatePlayer(m_PlayerOne, A);
	bool twoFinished = UpdatePlayer(m_PlayerTwo, A);
	if (!oneFinished && !twoFinished && !m_PauseMenu->GetVisible()) {
		m_PauseMenu->SetVisible(true);
		//Play sound!
		m_WaveLbl->SetTextf("Game over! Waves cleared: %.2d", m_Wave);
		return *this;
	}
	
	for (uint32_t i = 0; i < m_ObjectCount; i++) {
		if (m_ObjectList[i]->m_SpawnTick) continue;
		for (uint32_t d = i + 1; d < m_ObjectCount; d++) {
			if(m_ObjectList[d]->m_SpawnTick) continue;
			LWVector2f Dir = m_ObjectList[i]->m_Position - m_ObjectList[d]->m_Position;
			float r = m_ObjectList[i]->m_Radius + m_ObjectList[d]->m_Radius;
			if (Dir.LengthSquared() <= r*r) ProcessCollision(m_ObjectList[i], m_ObjectList[d], A);
		}
	}
	UpdateWave();

	m_Tick++;
	return *this;
}

State &State_Game::DrawFrame(App *A, Frame *F, Renderer *R) {
	const float SpriteScale = 0.25f;
	const float ParticleScale = 0.25f;
	const LWVector4f ShieldColors[] = { LWVector4f(1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 1.0f), LWVector4f(1.0f), LWVector4f(1.0f), LWVector4f(1.0f) };
	Sprite *ShieldSprites[] = { nullptr, m_ShieldBaseSprite, m_ShieldBaseSprite, m_ShieldGreenRedSprite, m_ShieldBaseSprite, m_ShieldBlueRedSprite, m_ShieldGreenBlueSprite, m_ShieldAllSprite };
	Sprite *P1Sprite = m_DescriptorList[GameObjectDescriptor::RedPlayer].m_SpriteList[0];
	Sprite *P2Sprite = m_DescriptorList[GameObjectDescriptor::BluePlayer].m_SpriteList[0];
	float P1SpriteY = P1Sprite->m_Size.y*SpriteScale;
	float P2SpriteY = P2Sprite->m_Size.y*SpriteScale;
	LWVector2f WndSizef = A->GetWindow()->GetSizef();
	float Scale = std::min<float>(WndSizef.x/m_FieldSize.x, WndSizef.y/m_FieldSize.y);
	LWVector2f ScreenOffset = WndSizef*0.5f - m_FieldSize*Scale*0.5f;

	Settings &Set = A->GetSettings();

	for (uint32_t i = 0; i < MaxStars; i++) {
		R->WriteSprite(F, m_StarSprite, ScreenOffset + m_Stars[i].m_Position*Scale, 0.0f, LWVector2f(m_Stars[i].m_Size*Scale), LWVector4f(1.0f));
	}

	for (uint32_t i = 0; i < m_ParticleCount; i++) {
		R->WriteSprite(F, m_ParticleSprite, ScreenOffset+m_ParticlePool[i].m_Position*Scale, m_ParticlePool[i].m_Theta, LWVector2f(ParticleScale*Scale), m_ParticlePool[i].m_Color);
	}

	for (uint32_t i = 0; i < m_ObjectCount;i++) { //Draw in reverse so our players are always on top.
		GameObject *Obj = m_ObjectList[i];
		bool Visible = true;
		if (Obj == m_PlayerOne.m_Object) Visible = (m_PlayerOne.m_InvCntr % 30) < 20;
		else if (Obj == m_PlayerTwo.m_Object) Visible = (m_PlayerTwo.m_InvCntr % 30) < 20;
		if (Visible) {
			if (Obj->m_DescriptorID == GameObjectDescriptor::Bullet || Obj->m_DescriptorID==GameObjectDescriptor::ShieldFountain) {
				R->WriteSprite(F, Obj->m_Sprite, ScreenOffset+Obj->m_Position*Scale, Obj->m_Theta, LWVector2f(Scale), ShieldColors[Obj->m_ShieldType]);
			} else {
				Sprite *Shield = ShieldSprites[Obj->m_ShieldType];
				if(Shield){
					float ShieldScale = Obj->m_Radius / (Shield->m_Size.x*0.5f);
					R->WriteSprite(F, Shield, ScreenOffset+Obj->m_Position*Scale, Obj->m_Theta, ShieldScale*Scale, ShieldColors[Obj->m_ShieldType]);
				}
				R->WriteSprite(F, Obj->m_Sprite, ScreenOffset+Obj->m_Position*Scale, Obj->m_Theta, LWVector2f(Scale), LWVector4f(1.0f));
			}
		}
	}

	LWVector2f Pnt = m_ScoreLbl->GetVisiblePosition() - LWVector2f(0.0f, P1SpriteY);
	Pnt.x += P1Sprite->m_Size.x*0.5f*SpriteScale;
	for (uint32_t i = 0; i < m_PlayerOne.m_Lives; i++) {
		R->WriteSprite(F, P1Sprite, Pnt, 0.0f, SpriteScale, LWVector4f(1.0f));
		Pnt.x += P1Sprite->m_Size.x*SpriteScale + 10.0f;
	}
	if (Set.m_GameMode == Settings::TwoPlayer) {
		LWVector2f Pnt = m_ScoreLbl->GetVisiblePosition() - LWVector2f(0.0f, P2SpriteY);
		Pnt.x += m_ScoreLbl->GetVisibleSize().x - P2Sprite->m_Size.x*0.5f*SpriteScale;
		for (uint32_t i = 0; i < m_PlayerTwo.m_Lives; i++) {
			R->WriteSprite(F, P2Sprite, Pnt, LW_PI, SpriteScale, LWVector4f(1.0f));
			Pnt.x -= (P2Sprite->m_Size.x*SpriteScale + 10.0f);
		}
	}
	return *this;
}

State &State_Game::ProcessInput(App *A, LWWindow *Window) {
	Settings &Set = A->GetSettings();
	LWMouse *Mouse = Window->GetMouseDevice();
	LWKeyboard *Keyboard = Window->GetKeyboardDevice();
	Player &P1 = m_PlayerOne;
	Player &P2 = m_PlayerTwo;

	GameObject *PlayerOne = P1.m_Object;
	GameObject *PlayerTwo = P2.m_Object;
	if (Mouse && PlayerOne) {
		LWVector2f MP = MapScreenToField(Mouse->GetPositionf(), A->GetWindow()->GetSizef());
		LWVector2f Dir = MP-PlayerOne->m_Position;
		if (Mouse->ButtonDown(LWMouseKey::Left)) {
			P1.m_TargetDir = Dir.Theta();
		} else P1.m_TargetDir = 100.0f;
		if (Mouse->ButtonDown(LWMouseKey::Right)) {
			P1.m_ShootDir = Dir.Theta();
		} else P1.m_ShootDir = 100.0f;
		if (Mouse->ButtonPressed(LWMouseKey::Middle)) {
			P1.m_DropShields = true;
		}
	}
	if (Keyboard && PlayerTwo) {
		LWVector2f Dir = LWVector2f(0.0f);
		LWVector2f SDir = LWVector2f(0.0f);
		if (Keyboard->ButtonDown(LWKey::A)) Dir.x = -1.0f;
		if (Keyboard->ButtonDown(LWKey::D)) Dir.x = 1.0f;
		if (Keyboard->ButtonDown(LWKey::W)) Dir.y = 1.0f;
		if (Keyboard->ButtonDown(LWKey::S)) Dir.y = -1.0f;
		if (Keyboard->ButtonDown(LWKey::Left)) SDir.x = -1.0f;
		if (Keyboard->ButtonDown(LWKey::Right)) SDir.x = 1.0f;
		if (Keyboard->ButtonDown(LWKey::Up)) SDir.y = 1.0f;
		if (Keyboard->ButtonDown(LWKey::Down)) SDir.y = -1.0f;
		if (Keyboard->ButtonPressed(LWKey::Q)) P2.m_DropShields = true;
		
		if (Dir.LengthSquared() > 0.1f) P2.m_TargetDir = Dir.Theta();
		else P2.m_TargetDir = 100.0f;

		if (SDir.LengthSquared() > 0.1f)  P2.m_ShootDir = SDir.Theta();
		else P2.m_ShootDir = 100.0f;
	}
	if (Keyboard) {
		if (Keyboard->ButtonPressed(LWKey::Esc)) {
			bool Finished = !P1.m_Object && !P2.m_Object;
			if (Finished) MenuBtnPressed(nullptr, 0, A);
			else m_PauseMenu->SetVisible(!m_PauseMenu->GetVisible());
		}
	}
	return *this;
}

State &State_Game::Activated(App *A) {
	const uint32_t DefaultLives = 3;
	const float MinStarSize = 0.2f;
	const float MaxStarSize = 0.4f;
	m_FieldSize = LWVector2f(1600.0f, 1080.0f);
	Settings &Set = A->GetSettings();
	m_ObjectCount = 0;
	m_ParticleCount = 0;
	m_Wave = 0;
	m_ActiveWave = m_WaveList[m_Wave];
	m_PlayerOne.m_Object = m_PlayerTwo.m_Object = nullptr;

	Spawn(GameObjectDescriptor::ShieldFountain, LWVector2f(m_FieldSize.x*0.5f, m_FieldSize.y - 100.0f), 0.0f, true)->m_ShieldType = GameObject::ShieldBlue;
	Spawn(GameObjectDescriptor::ShieldFountain, LWVector2f(100.0f, 100.0f), 0.0f, true)->m_ShieldType = GameObject::ShieldRed;
	Spawn(GameObjectDescriptor::ShieldFountain, LWVector2f(m_FieldSize.x - 100.0f, 100.0f), 0.0f, true)->m_ShieldType = GameObject::ShieldGreen;
	if (Set.m_GameMode == Settings::OnePlayer) {
		m_PlayerOne.m_Object = Spawn(GameObjectDescriptor::RedPlayer, m_FieldSize*0.5f, 0.0f, true);
		m_PlayerOne.m_Object->m_Owner = &m_PlayerOne;
	} else {
		m_PlayerOne.m_Object = Spawn(GameObjectDescriptor::RedPlayer, m_FieldSize*0.5f - LWVector2f(100.0f, 0.0f), 0.0f, true);
		m_PlayerTwo.m_Object = Spawn(GameObjectDescriptor::BluePlayer, m_FieldSize*0.5f + LWVector2f(100.0f, 0.0f), LW_PI, true);
		m_PlayerOne.m_Object->m_Owner = &m_PlayerOne;
		m_PlayerTwo.m_Object->m_Owner = &m_PlayerTwo;
	}
	m_PlayerOne.m_TargetDir = m_PlayerTwo.m_TargetDir = 100.0f;
	m_PlayerOne.m_ShootDir = m_PlayerTwo.m_ShootDir = false;
	m_PlayerOne.m_FireTick = m_PlayerTwo.m_FireTick = 0;
	m_PlayerOne.m_Score = m_PlayerTwo.m_Score = 0;
	m_PlayerOne.m_Lives = m_PlayerTwo.m_Lives = DefaultLives;
	m_PlayerOne.m_InvCntr = m_PlayerTwo.m_InvCntr = 0;
	m_PlayerOne.m_DropShields = m_PlayerTwo.m_DropShields = false;
	m_GameMenu->SetVisible(true);
	m_PauseMenu->SetVisible(false);
	m_WaveTimer = 1;

	//Build starfield.
	for (uint32_t i = 0; i < MaxStars; i++) {
		m_Stars[i].m_Position = LWVector2f(Random(0.0f, m_FieldSize.x), Random(0.0f, m_FieldSize.y));
		m_Stars[i].m_Size = Random(MinStarSize, MaxStarSize);
	}
	return *this;
}

State &State_Game::Deactivated(App *A) {
	m_GameMenu->SetVisible(false);
	m_PauseMenu->SetVisible(false);
	return *this;
}

void State_Game::RestartBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	Activated(A);
	return;
}

void State_Game::MenuBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	A->SetActiveState(State::Menu);
	return;
}

bool State_Game::RemoveObject(GameObject *Obj) {
	if (Obj == m_PlayerOne.m_Object) m_PlayerOne.m_Object = nullptr;
	if (Obj == m_PlayerTwo.m_Object) m_PlayerTwo.m_Object = nullptr;
	uint32_t i = Obj->m_Index;
	uint32_t n = m_ObjectCount - 1;
	m_ObjectList[i]->m_Index = n;
	m_ObjectList[n]->m_Index = i;
	m_ObjectList[i] = m_ObjectList[n];
	m_ObjectList[n] = Obj;
	m_ObjectCount--;
	return true;
}

GameObject *State_Game::Spawn(uint32_t DescriptorID, const LWVector2f &Pos, float Theta, bool TempInvulnerable) {
	const uint32_t SpawnInvulnRate = 60;
	if (m_ObjectCount >= MaxObjects) return nullptr;
	GameObject *Obj = m_ObjectList[m_ObjectCount];
	GameObjectDescriptor &Desc = m_DescriptorList[DescriptorID];
	Obj->m_DescriptorID = DescriptorID;
	Obj->m_Sprite = Desc.m_SpriteList[0];
	Obj->m_Owner = nullptr;
	Obj->m_Position = Pos;
	Obj->m_Velocity = LWVector2f(0.0f);
	Obj->m_Theta = Theta;
	Obj->m_Health = Desc.m_Health;
	Obj->m_Radius = std::max<float>(Desc.m_SpriteList[0]->m_Size.x, Desc.m_SpriteList[0]->m_Size.y)*0.5f;
	Obj->m_ShieldType = GameObject::ShieldNone;
	Obj->m_SpawnTick = TempInvulnerable ?SpawnInvulnRate:0;
	m_ObjectCount++;
	return Obj;
}

State_Game &State_Game::SpawnParticles(const LWVector2f &Pos, float Radius, const LWVector4f &Color, uint32_t ParticleCnt) {
	const float MinSpeed = 1.0f;
	const float MaxSpeed = 2.0f;
	const uint32_t MinLife = 20;
	const uint32_t MaxLife = 40;
	for (uint32_t i = 0; i < ParticleCnt; i++) {
		if (m_ParticleCount >= MaxParticles) return *this;
		float r = Random(0.0f, Radius);
		float t = Random(0.0f, LW_2PI);
		float s = Random(MinSpeed, MaxSpeed);
		LWVector2f Dir = LWVector2f::MakeTheta(t);
		uint32_t l = Random(MinLife, MaxLife);
		m_ParticlePool[m_ParticleCount++] = { Color, Pos + Dir*r, Dir*s, t, l };
	}
	return *this;
}

SpriteManager *State_Game::GetSpriteManager(void) {
	return m_SpriteManager;
}

GameObjectDescriptor &State_Game::GetDescriptor(uint32_t ID) {
	return m_DescriptorList[ID];
}

State_Game::State_Game(App *A, LWEUIManager *UIMan, SpriteManager *SpriteMan) : m_ObjectCount(0), m_WaveCount(0), m_Tick(0), m_SpriteManager(SpriteMan) {
	std::seed_seq sd = { time(NULL) };
	m_Random.seed(sd);
	for (uint32_t i = 0; i < MaxObjects; i++) {
		m_ObjectList[i] = &m_ObjectPool[i];
		m_ObjectList[i]->m_Index = i;
	}
	m_GameMenu = UIMan->GetNamedUI("GameMenu");
	m_PauseMenu = UIMan->GetNamedUI("PauseMenu");
	m_ScoreLbl = (LWEUILabel *)UIMan->GetNamedUI("ScoreLbl");
	m_WaveLbl = (LWEUILabel *)UIMan->GetNamedUI("WaveLbl");
	m_ParticleSprite = SpriteMan->Find("Bullet");
	m_ShieldBaseSprite = m_SpriteManager->Find("ShieldBase");
	m_ShieldBlueRedSprite = m_SpriteManager->Find("ShieldBlueRed");
	m_ShieldGreenBlueSprite = m_SpriteManager->Find("ShieldGreenBlue");
	m_ShieldGreenRedSprite = m_SpriteManager->Find("ShieldGreenRed");
	m_ShieldAllSprite = m_SpriteManager->Find("ShieldAll");
	m_StarSprite = m_SpriteManager->Find("Star");

	UIMan->RegisterMethodEvent("Pause_RestartBtn", LWEUI::Event_Released, &State_Game::RestartBtnPressed, this, A);
	UIMan->RegisterMethodEvent("Pause_MenuBtn", LWEUI::Event_Released, &State_Game::MenuBtnPressed, this, A);
	
}