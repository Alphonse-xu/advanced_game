#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/PositionConstraint.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame* TutorialGame::This_TutorialGame = nullptr;

TutorialGame::TutorialGame()	{
	This_TutorialGame = this;
	myGoosePos = Vector3(0, 2, -85);
	yourGoosePos = Vector3(-85, 2, -85);
	ApplePos = Vector3(80, 0, -85);
	PKPos = Vector3(85, 0, -85);
	myIslandPos = Vector3(0, -7, -85);
	yourIslandPos = Vector3(-85, -7, -85);
	DoubleMod = false;

	grid = new NavigationGrid("TestGrid1.txt");

	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;
	developmod = false;

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("goose.msh"	 , &gooseMesh);
	loadFunc("CharacterA.msh", &keeperMesh);
	loadFunc("CharacterM.msh", &charA);
	loadFunc("CharacterF.msh", &charB);
	loadFunc("Apple.msh"	 , &appleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	manualTex = (OGLTexture*)TextureLoader::LoadAPITexture("goose.png");
	singleTex = (OGLTexture*)TextureLoader::LoadAPITexture("Single.png");
	doubleTex = (OGLTexture*)TextureLoader::LoadAPITexture("double.png");
	startTex = (OGLTexture*)TextureLoader::LoadAPITexture("start.png");
	quitTex = (OGLTexture*)TextureLoader::LoadAPITexture("quit.png");

	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete gooseMesh;
	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;

	delete PKMachine;
	delete UIMachine;
}

void TutorialGame::UpdateGame(float dt) {
	TimerDT = dt;
	if (developmod) {
		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}
		lockedObject = nullptr;

		UpdateKeys();

		if (useGravity) {
			Debug::Print("(G)ravity on", Vector2(10, 40));
		}
		else {
			Debug::Print("(G)ravity off", Vector2(10, 40));
		}
		SelectObject();
		MoveSelectedObject();
		CanadaGooseMove();

		world->UpdateWorld(dt);
		renderer->Update(dt);
		physics->Update(dt);
		constraint->UpdateConstraint(dt);
		Debug::FlushRenderables();
		renderer->Render();

		//BridgeConstraintTest();
	}
	else {
		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}
		if (lockedObject != nullptr) {
			LockedCameraMovement();
		}

		UpdateKeys();

		useGravity = true;
		physics->UseGravity(useGravity);

		SelectObject();
		MoveSelectedObject();
		CanadaGooseMove();
		enemyMove();
		PKMachine->Update();
		world->UpdateWorld(dt);
		renderer->Update(dt);
		physics->Update(dt);
		constraint->UpdateConstraint(dt);
		Debug::FlushRenderables();
		renderer->Render();

		UIMachine->Update();
	}

}

void TutorialGame::UpdateKeys() {

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}
	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F11)) {
		developmod = !developmod;
	}
	

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		selectionObject->GetPhysicsObject()->AddForce(-rightAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		selectionObject->GetPhysicsObject()->AddForce(rightAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}
}

void  TutorialGame::LockedCameraMovement() {
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetWorldPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x-90);
		world->GetMainCamera()->SetYaw(angles.y + 180);
	}
}


void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(10, 0));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->SetCollisionPos(closestCollision.collidedAt);
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
			
		}

		if (selectionObject) { //要选中物体才能显示坐标
			renderer->DrawString("click position:" + selectionObject->PrintCollisionPos(), Vector2(10, 60)); //显示选中物体位置和方向
			string CosPos = "x = " + std::to_string(selectionObject->GetConstTransform().GetLocalPosition().x) + " y = " + std::to_string(selectionObject->GetConstTransform().GetLocalPosition().y) + "z = " + std::to_string(selectionObject->GetConstTransform().GetLocalPosition().z);
			renderer->DrawString("object position:" + CosPos, Vector2(10, 80)); 

		}

	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(10, 0));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	renderer->DrawString(" Click Force :" + std::to_string(forceMagnitude),Vector2(10, 20)); // Draw debug text at 10 ,20
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	
		if (!selectionObject) {
		return;// we haven 't selected anything !
		
	}
	// Push the selected object !
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(* world->GetMainCamera());
		
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(
				ray.GetDirection()* forceMagnitude,closestCollision.collidedAt );
			}
		}
		
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;

}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	score = 0;
	PKflag = 4;
	Manualflag = 1;

	CanadaGoose = AddGooseToWorld(myGoosePos);
	RedApple = AddAppleToWorld(ApplePos);
	ParkKeeper = AddParkKeeperToWorld(PKPos);
	enemy = AddCharacterToWorld(Vector3(45, 0, 0));
	AddFloorToWorld(Vector3(-55, -9, 0));
	AddFloorToWorld(Vector3(55, -9, 0));
	AddWaterToWorld(Vector3(0, -9, 0));
	AddIslandToWorld(myIslandPos, "myisland");
	AddMapToWorld();
	AddGameManual();


	previous = CanadaGoose;
	constraint = new PositionConstraint(previous, nullptr, 3);

	if (DoubleMod)
	{
		AddIslandToWorld(yourIslandPos, "yourisland");
		EnemyGoose = AddGooseToWorld(myGoosePos);
	}
}

//From here on it's functions to add in objects to the world!

/*

A single function to add a large immoveable cube to the bottom of our world

*/

void TutorialGame::AddMapToWorld() {
	Vector3 cubeDims = Vector3(5, 5, 5);
	Vector3 CubePos;  
	for (int i = 0; i < grid->GetCubeNum(); i++)
	{
		if ((grid->GetAllnodes())[i].type == 'x')
		{
			CubePos = Vector3((grid->GetAllnodes())[i].position.x-95, -3, (grid->GetAllnodes())[i].position.z-95);
			AddCubeToWorld(CubePos, cubeDims,"realwall",0);
		}
		
	}
	
}

GameObject* TutorialGame::AddIslandToWorld(const Vector3& position, string IslandName) {
	Vector3 islandDims = Vector3(3, 1, 3);

	GameObject* Island = new GameObject(IslandName);
	AABBVolume* volume = new AABBVolume(islandDims);
	Island->SetBoundingVolume((CollisionVolume*)volume);
	Island->GetTransform().SetWorldScale(islandDims);
	Island->GetTransform().SetWorldPosition(position);

	Island->SetRenderObject(new RenderObject(&Island->GetTransform(), cubeMesh, nullptr, basicShader));
	Island->SetPhysicsObject(new PhysicsObject(&Island->GetTransform(), Island->GetBoundingVolume()));

	Island->GetPhysicsObject()->SetInverseMass(0);
	Island->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Island);
	Island->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));





	return Island;
}

GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor");

	Vector3 floorSize = Vector3(35, 1, 100);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(floorSize);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);
	floor->GetRenderObject()->SetColour(Vector4(0, 0.8, 0, 1));

	return floor;
}

GameObject* TutorialGame::AddWaterToWorld(const Vector3& position) {
	GameObject* water = new GameObject("water");

	Vector3 waterSize = Vector3(20, 1, 100);
	AABBVolume* volume = new AABBVolume(waterSize);
	water->SetBoundingVolume((CollisionVolume*)volume);
	water->GetTransform().SetWorldScale(waterSize);
	water->GetTransform().SetWorldPosition(position);

	water->SetRenderObject(new RenderObject(&water->GetTransform(), cubeMesh, nullptr, basicShader));
	water->SetPhysicsObject(new PhysicsObject(&water->GetTransform(), water->GetBoundingVolume()));

	water->GetPhysicsObject()->SetInverseMass(0);
	water->GetPhysicsObject()->InitCubeInertia();
	//water->isWall = 1;
	world->AddGameObject(water);
	water->GetRenderObject()->SetColour(Vector4(0, 0.8, 1, 1));

	return water;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions,string CubeName, float inverseMass) {
	GameObject* cube = new GameObject(CubeName);

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position)
{
	float size			= 1.0f;
	float inverseMass	= 1.0f;

	GameObject* goose = new GameObject("goose");


	SphereVolume* volume = new SphereVolume(size);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetWorldScale(Vector3(size,size,size) );
	goose->GetTransform().SetWorldPosition(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(goose);


	return goose;
}

void TutorialGame::CanadaGooseMove() {

		GooseRay = new Ray(CanadaGoose->GetTransform().GetWorldPosition(), (CanadaGoose->GetTransform().GetLocalOrientation() * Vector3(0, 0, 1)).Normalised());
		Debug::DrawLine(GooseRay->GetPosition(), GooseRay->GetPosition() + GooseRay->GetDirection() * 1000.0f, Vector4(1, 1, 0, 1));


		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::U)) {
			CanadaGoose->GetPhysicsObject()->AddForce(GooseRay->GetDirection() * 50.0f);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::J)) {
			CanadaGoose->GetPhysicsObject()->AddForce(-GooseRay->GetDirection() * 50.0f);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::K)) {
			CanadaGoose->GetPhysicsObject()->AddTorque(Vector3(0, -3, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::H)) {
			CanadaGoose->GetPhysicsObject()->AddTorque(Vector3(0, 3, 0));
		}
		
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
			CanadaGoose->GetPhysicsObject()->AddForce(Vector3(0, 50, 0));
		}

		AppleDetection();

		WaterDetection();
	
}

GameObject* TutorialGame::AddParkKeeperToWorld(const Vector3& position)
{
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	GameObject* keeper = new GameObject("parkkeeper");

	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	keeper->SetBoundingVolume((CollisionVolume*)volume);

	keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	keeper->GetTransform().SetWorldPosition(position);

	keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), keeperMesh, nullptr, basicShader));
	keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume()));

	keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	keeper->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(keeper);

	ParkKeeperMachine();

	return keeper;
}

void TutorialGame::ParkKeeperMachine() {

	PKMachine = new StateMachine();
	//干什么
	StateFunc AFunc = &ParkKpeeperMove;
	StateFunc BFunc = &ParkKpeeperDetection;
	StateFunc CFunc = &ParkKpeeperBack;
	StateFunc DFunc = &ParkKpeeperTouch;

	GenericState* stateA = new GenericState(AFunc, (void*)&PKflag);
	GenericState* stateB = new GenericState(BFunc, (void*)&PKflag);
	GenericState* stateC = new GenericState(CFunc, (void*)&PKflag);
	GenericState* stateD = new GenericState(DFunc, (void*)&PKflag);
	PKMachine->AddState(stateA);
	PKMachine->AddState(stateB);
	PKMachine->AddState(stateC);
	PKMachine->AddState(stateD);

	//转换条件
	GenericTransition <int&, int >* transitionA =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			PKflag, 0, stateB, stateA); // if greater than 1, A to B

	GenericTransition <int&, int >* transitionB =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			PKflag, 1, stateA, stateD); // if equals 0, B to A

	GenericTransition <int&, int >* transitionC =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			PKflag, 2, stateA, stateC); // if equals 0, B to A

	GenericTransition <int&, int >* transitionD =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			PKflag, 3, stateD, stateC); // if equals 0, B to A

	GenericTransition <int&, int >* transitionE =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			PKflag, 4, stateC, stateB); // if equals 0, B to A

	PKMachine->AddTransition(transitionA);
	PKMachine->AddTransition(transitionB);
	PKMachine->AddTransition(transitionC);
	PKMachine->AddTransition(transitionD);
	PKMachine->AddTransition(transitionE);

	PKMachine->SetActiveState(stateB);
}

void TutorialGame::ParkKpeeperTouch(void*) {
	if (false) //鹅有苹果
	{
		This_TutorialGame->RedApple->GetTransform().SetWorldPosition(This_TutorialGame->ApplePos);
		This_TutorialGame->PKflag = 3;
	}


}

void TutorialGame::ParkKpeeperBack(void*) {
	This_TutorialGame->ParkKeeper->GetTransform().SetWorldPosition(This_TutorialGame->PKPos);
	This_TutorialGame->PKflag = 4;

}

void TutorialGame::ParkKpeeperMove(void*) {
	NavigationPath outPath;
	Vector3 startPos = This_TutorialGame->ParkKeeper->GetTransform().GetWorldPosition(); 
	
	startPos.x += 100;
	startPos.y = 0;
	startPos.z += 100;
	Vector3 endPos = This_TutorialGame->CanadaGoose->GetTransform().GetWorldPosition();
	endPos.x += 100;
	endPos.y = 0;
	endPos.z += 100;
	//PKpos This_TutorialGame->PKPos goosepos This_TutorialGame->GoosePos Vector3(0, 0, -85)

	This_TutorialGame->testNodes.clear();

	bool found = This_TutorialGame->grid->FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pos.x -= 95;
		pos.z -= 95;

		This_TutorialGame->testNodes.push_back(pos);

		//std::cout << pos << std::endl;
	}

	for (int i = 1; i < This_TutorialGame->testNodes.size(); ++i) {
		Vector3 a = This_TutorialGame->testNodes[i - 1];
		Vector3 b = This_TutorialGame->testNodes[i];
		Debug::DrawLine(a, b, Vector4(1, 0, 0, 1));
	}


	for (int i = 1; i < This_TutorialGame->testNodes.size() ; i++)
	{
		Vector3 a = This_TutorialGame->testNodes[i];

			This_TutorialGame->ParkKeeper->GetPhysicsObject()->AddForce((a - This_TutorialGame->ParkKeeper->GetTransform().GetWorldPosition()) * 0.5f);
			//std::cout << a << std::endl;
		
		
	}

	if (This_TutorialGame->physics->apple_island_detection == true)
	{
		This_TutorialGame->score += 10;
		This_TutorialGame->physics->apple_island_detection = false;
		This_TutorialGame->PKflag = 1;
	}

}

void TutorialGame::ParkKpeeperDetection(void* ) {
	Vector3 PGRANGE = This_TutorialGame->ParkKeeper->GetTransform().GetWorldPosition() - This_TutorialGame->CanadaGoose->GetTransform().GetWorldPosition();
	This_TutorialGame->parkkeeper_goose_range = PGRANGE.x * PGRANGE.x + PGRANGE.z * PGRANGE.z;
	
		if (This_TutorialGame->parkkeeper_goose_range < 1000) {
			This_TutorialGame->PKflag = 0;
		}
	
}

GameObject* TutorialGame::AddCharacterToWorld(const Vector3& position) {
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	auto pos = keeperMesh->GetPositionData();

	Vector3 minVal = pos[0];
	Vector3 maxVal = pos[0];

	for (auto& i : pos) {
		maxVal.y = max(maxVal.y, i.y);
		minVal.y = min(minVal.y, i.y);
	}

	GameObject* character = new GameObject();

	float r = rand() / (float)RAND_MAX;


	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	character->GetTransform().SetWorldPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), r > 0.5f ? charA : charB, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);

	return character;
}

void TutorialGame::enemyMove() {

	NavigationPath outPath;
	Vector3 startPos = CanadaGoose->GetTransform().GetWorldPosition();
	startPos.x += 100;
	startPos.y = 0;
	startPos.z += 100;
	Vector3 endPos = enemy->GetTransform().GetWorldPosition();
	endPos.x += 100;
	endPos.y = 0;
	endPos.z += 100;
	//PKpos This_TutorialGame->PKPos goosepos This_TutorialGame->GoosePos Vector3(0, 0, -85)

	This_TutorialGame->enemyNodes.clear();

	bool found = This_TutorialGame->grid->FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pos.x -= 95;
		pos.z -= 95;

		This_TutorialGame->enemyNodes.push_back(pos);

		//std::cout << pos << std::endl;
	}

	for (int i = 1; i < This_TutorialGame->enemyNodes.size(); ++i) {
		Vector3 a = This_TutorialGame->enemyNodes[i - 1];
		Vector3 b = This_TutorialGame->enemyNodes[i];
		Debug::DrawLine(a, b, Vector4(1, 0, 0, 1));
	}


	for (int i = 1; i < This_TutorialGame->enemyNodes.size(); i++)
	{
		Vector3 a = This_TutorialGame->enemyNodes[i];

		enemy->GetPhysicsObject()->AddForce((a - enemy->GetTransform().GetWorldPosition()) * 0.5f);
		//std::cout << a << std::endl;


	}

}

GameObject* TutorialGame::AddAppleToWorld(const Vector3& position) {
	GameObject* apple = new GameObject("apple");

	SphereVolume* volume = new SphereVolume(0.7f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(4, 4, 4));
	apple->GetTransform().SetWorldPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();
	apple->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::AppleDetection() {



	if (physics->apple_goose_detection == true) {
		AddGooseConstraint(RedApple);
		physics->apple_goose_detection = false;

	}

}

void TutorialGame::WaterDetection() {
	if (physics->goose_water_detection == true) {
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::I)) {
			CanadaGoose->GetPhysicsObject()->AddForce(GooseRay->GetDirection() * 1000.0f);
		}

	}


}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims,"");
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
	//AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, "",1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}


void TutorialGame::AddGooseConstraint(GameObject* FollowingObject) {
	float    maxDistance = 3;
	FollowingObject->GetTransform().SetWorldPosition(CanadaGoose->GetTransform().GetWorldPosition() + Vector3(0, 10, 0));
	constraint = new PositionConstraint(previous, FollowingObject, maxDistance);
	world->AddConstraint(constraint);
	previous = FollowingObject;

}

void TutorialGame::SimpleGJKTest() {
	Vector3 dimensions		= Vector3(5, 5, 5);
	Vector3 floorDimensions = Vector3(100, 2, 100);

	GameObject* fallingCube = AddCubeToWorld(Vector3(0, 20, 0), dimensions, "",10.0f);
	GameObject* newFloor	= AddCubeToWorld(Vector3(0, 0, 0), floorDimensions,"", 0.0f);

	delete fallingCube->GetBoundingVolume();
	delete newFloor->GetBoundingVolume();

	fallingCube->SetBoundingVolume((CollisionVolume*)new OBBVolume(dimensions));
	newFloor->SetBoundingVolume((CollisionVolume*)new OBBVolume(floorDimensions));

}

void TutorialGame::AddGameManual() {
	ManualCube=AddCubeToWorld(Vector3(-300, 0, -300), Vector3(100, 1, 100), "manual", 0.0f);
	ManualCube->GetRenderObject()->SetDefaultTexture(manualTex);
	//按钮
	ButtonSingle = AddCubeToWorld(Vector3(-300, 10, -315), Vector3(10, 0.1, 5), "single", 0.0f);
	ButtonDouble = AddCubeToWorld(Vector3(-300, 10, -285), Vector3(10, 0.1, 5), "double", 0.0f);
	ButtonReplay = AddCubeToWorld(Vector3(-300, -10, -315), Vector3(10, 0.1, 5), "replay", 0.0f);
	ButtonExit = AddCubeToWorld(Vector3(-300, -10, -285), Vector3(10, 0.1, 5), "exit", 0.0f);

	ButtonReplay->GetRenderObject()->SetDefaultTexture(startTex);
	ButtonExit->GetRenderObject()->SetDefaultTexture(quitTex);
	ButtonSingle->GetRenderObject()->SetDefaultTexture(singleTex);
	ButtonDouble->GetRenderObject()->SetDefaultTexture(doubleTex);

	ButtonSingle->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	ButtonReplay->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	ButtonDouble->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	ButtonExit->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));

	ManualMachine();
}



void TutorialGame::ManualMachine() {

	UIMachine = new StateMachine();
	//干什么
	StateFunc AFunc = &SinglePlayer;
	StateFunc BFunc = &DoublePlayer;
	StateFunc CFunc = &ManualSelect;
	StateFunc DFunc = &ExitSelect;



	GenericState* stateA = new GenericState(AFunc, (void*)&Manualflag);
	GenericState* stateB = new GenericState(BFunc, (void*)&Manualflag);
	GenericState* stateC = new GenericState(CFunc, (void*)&Manualflag);
	GenericState* stateD = new GenericState(DFunc, (void*)&Manualflag);

	UIMachine->AddState(stateA);
	UIMachine->AddState(stateB);
	UIMachine->AddState(stateC);
	UIMachine->AddState(stateD);

	//转换条件
	GenericTransition <int&, int >* transitionA =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			Manualflag, 0, stateC, stateA); // , manual to single 

	GenericTransition <int&, int >* transitionB =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			Manualflag, 1, stateA, stateC); // if equals 0, single to manual
	
	GenericTransition <int&, int >* transitionC =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			Manualflag, 2, stateA, stateD); // if equals 0, single to exit
	
	GenericTransition <int&, int >* transitionD =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			Manualflag, 3, stateD, stateA); // if equals 0, exit to single

	GenericTransition <int&, int >* transitionE =
		new GenericTransition <int&, int >(
			GenericTransition <int&, int >::EqualsTransition,
			Manualflag, 4, stateC, stateB); // if equals 0, manual to double

	UIMachine->AddTransition(transitionA);
	UIMachine->AddTransition(transitionB);
	UIMachine->AddTransition(transitionC);
	UIMachine->AddTransition(transitionD);
	UIMachine->AddTransition(transitionE);

	UIMachine->SetActiveState(stateC);
}

//cfunc
void TutorialGame::ManualSelect(void*) {
	This_TutorialGame->lockedObject = This_TutorialGame->ManualCube;
	Window::GetWindow()->ShowOSPointer(true);
	Window::GetWindow()->LockMouseToWindow(false);

	GameObject* ManualSelectionObject;
	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*This_TutorialGame->world->GetMainCamera());

		RayCollision closestCollision;

		if (This_TutorialGame->world->Raycast(ray, closestCollision, true)) {
			ManualSelectionObject = (GameObject*)closestCollision.node;
			if (ManualSelectionObject == This_TutorialGame->ButtonSingle)
			{
				//This_TutorialGame->InitWorld();
				This_TutorialGame->lockedObject = This_TutorialGame->CanadaGoose;
				This_TutorialGame->Manualflag = 0;
			}
		
			if (ManualSelectionObject == This_TutorialGame->ButtonDouble)
			{
				This_TutorialGame->Manualflag = 4;
			}
		}
	}


}

//afunc
void TutorialGame::SinglePlayer(void*) {

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE))
	{
		This_TutorialGame->Manualflag = 1;
	}
	This_TutorialGame->SingleplayTimer += This_TutorialGame->TimerDT;
	
	if (This_TutorialGame->SingleplayTimer > 180 )
	{
		This_TutorialGame->Manualflag = 2;

	}

	//if (false)
	//{
	//	This_TutorialGame->Manualflag = 2;
	//}
}

void TutorialGame::DoublePlayer(void*) {
	This_TutorialGame->DoubleMod = true;
	This_TutorialGame->InitWorld();
	This_TutorialGame->lockedObject = This_TutorialGame->CanadaGoose;



}

void TutorialGame::ExitSelect(void*) {
	This_TutorialGame->lockedObject = This_TutorialGame->ManualCube;
	Window::GetWindow()->ShowOSPointer(true);
	Window::GetWindow()->LockMouseToWindow(false);

	This_TutorialGame->ButtonReplay->GetTransform().SetWorldPosition(Vector3(-300, 10, -315));
	This_TutorialGame->ButtonExit->GetTransform().SetWorldPosition(Vector3(-300, 10, -285));

	Debug::Print("score is " + This_TutorialGame->score, Vector2(10, 80));

	GameObject* ManualSelectionObject;
	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*This_TutorialGame->world->GetMainCamera());

		RayCollision closestCollision;

		if (This_TutorialGame->world->Raycast(ray, closestCollision, true)) {
			ManualSelectionObject = (GameObject*)closestCollision.node;
			if (ManualSelectionObject == This_TutorialGame->ButtonReplay)
			{
				
				This_TutorialGame->ButtonReplay->GetTransform().SetWorldPosition(Vector3(-300, -10, -315));
				This_TutorialGame->ButtonExit->GetTransform().SetWorldPosition(Vector3(-300, -10, -285));
				This_TutorialGame->Manualflag = 3;
			}

			if (ManualSelectionObject == This_TutorialGame->ButtonExit)
			{
				exit(0);
			}
		}
	}

}