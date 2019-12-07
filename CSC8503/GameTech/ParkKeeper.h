#pragma once
#include "../CSC8503Common/GameObject.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/Ray.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class ParkKeeper : public GameObject {
		public:
			ParkKeeper();


			static void ParkKpeeperMove(void* t);
			void ParkKpeeperDetection();

			void Update(float dt) override {

			}

		protected:




		};




	}



}
