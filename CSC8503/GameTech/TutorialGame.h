#pragma once

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "ParkKeeper.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "../CSC8503Common/PositionConstraint.h"



namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			//状态机
			void ParkKeeperMachine();
			void ManualMachine();

			int GetScore() { return score; }
			void PrintScore() {
				Debug::Print("score is " + score, Vector2(10, 80));
			}

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void AddGooseConstraint(GameObject* FollowingObject);
			GameObject* previous;
			PositionConstraint* constraint = nullptr;
			void SimpleGJKTest();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();


			//菜单
			void AddGameManual();
			
			GameObject* AddWaterToWorld(const Vector3& position);
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, string CubeName,float inverseMass = 10.0f);
			//IT'S HAPPENING
			GameObject* AddIslandToWorld(const Vector3& position , string IslandName);
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddParkKeeperToWorld(const Vector3& position);
			GameObject* AddCharacterToWorld(const Vector3& position);
			GameObject* AddAppleToWorld(const Vector3& position);
			void AddMapToWorld();
			Vector3 PKPos;
			Vector3 myGoosePos;
			Vector3 yourGoosePos;
			Vector3 ApplePos;
			Vector3 myIslandPos;
			Vector3 yourIslandPos;
			int score;


			NavigationGrid* grid;
			vector<Vector3> testNodes;
			vector<Vector3> enemyNodes;
			static TutorialGame* This_TutorialGame;

			GameObject* ButtonSingle;
			GameObject* ButtonDouble;
			GameObject* ButtonReplay;
			GameObject* ButtonExit;

			GameObject* CanadaGoose;
			GameObject* EnemyGoose;
			GameObject* ManualCube;
			GameObject* ParkKeeper;
			GameObject* RedApple;
			GameObject* enemy;

			Ray* GooseRay;
			float parkkeeper_goose_range;

			void WaterDetection();
			void AppleDetection();
			void CanadaGooseMove();
			void enemyMove();
			static void ParkKpeeperMove(void*);
			static void ParkKpeeperDetection(void*);
			static void ParkKpeeperBack(void*);
			static void ParkKpeeperTouch(void*);
			static void SinglePlayer(void*);
			static void DoublePlayer(void*);
			static void ManualSelect(void*);
			static void ExitSelect(void*);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;
			bool developmod;
			bool DoubleMod;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;
			OGLTexture*	manualTex = nullptr;
			OGLTexture*	singleTex = nullptr;
			OGLTexture*	doubleTex = nullptr;
			OGLTexture*	startTex = nullptr;
			OGLTexture*	quitTex = nullptr;


			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLMesh*	charA		= nullptr;
			OGLMesh*	charB		= nullptr;

			//状态机
			StateMachine* PKMachine = nullptr;
			int PKflag;
			StateMachine* UIMachine = nullptr;
			int Manualflag;

			float TimerDT;
			float SingleplayTimer = 0;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 100, 0);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
		};
	}
}

