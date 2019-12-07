#include "ParkKeeper.h"

using namespace NCL::CSC8503;

ParkKeeper::ParkKeeper() {

}

void ParkKeeper::ParkKpeeperMove(void) {

}

void ParkKeeper::ParkKpeeperDetection(void) {
	//int* realData = (int*)data;
	Ray PKRay(ParkKeeper->GetTransform().GetWorldPosition(), (ParkKeeper->GetTransform().GetLocalOrientation() * Vector3(0, 0, 1)).Normalised());
	Debug::DrawLine(PKRay.GetPosition(), PKRay.GetPosition() + PKRay.GetDirection() * 1000.0f, Vector4(1, 0, 0, 1));

	RayCollision closestCollision;

	if (world->Raycast(PKRay, closestCollision, true)) {
		if (CanadaGoose == (GameObject*)closestCollision.node) {
			//(*realData)=10;
		}
	}
}