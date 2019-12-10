#pragma once

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "ParkKeeper.h"
#include "../CSC8503Common/NavigationGrid.h"


namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			//状态机
			void ParkKeeperMachine();

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
			void BridgeConstraintTest();
			void SimpleGJKTest();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();

			GameObject* AddWaterToWorld(const Vector3& position);
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, string CubeName,float inverseMass = 10.0f);
			//IT'S HAPPENING
			GameObject* AddIslandToWorld(const Vector3& position);
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddParkKeeperToWorld(const Vector3& position);
			GameObject* AddCharacterToWorld(const Vector3& position);
			GameObject* AddAppleToWorld(const Vector3& position);
			void AddMapToWorld();
			Vector3 PKPos;
			Vector3 GoosePos;
			Vector3 ApplePos;

			NavigationGrid* grid;
			vector<Vector3> testNodes;
			static TutorialGame* This_TutorialGame;

			GameObject* CanadaGoose;
			GameObject* ManualCube;
			GameObject* ParkKeeper;
			GameObject* RedApple;
			void AppleDetection();
			void CanadaGooseMove();
			static void ParkKpeeperMove(void*);
			static void ParkKpeeperDetection(void*);


			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;
			bool useGoose;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLMesh*	charA		= nullptr;
			OGLMesh*	charB		= nullptr;

			//状态机
			StateMachine* PKmachine = nullptr;
			int PKflag = 0;

			//菜单
			void GameManual();

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 100, 0);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
		};
	}
}

