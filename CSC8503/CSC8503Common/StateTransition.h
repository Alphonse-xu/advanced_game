#pragma once

namespace NCL {
	namespace CSC8503 {

		class State;

		class StateTransition
		{
		public:
			virtual bool CanTransition() const = 0;

			State* GetDestinationState()  const {
				return destinationState;
			}

			State* GetSourceState() const {
				return sourceState;
			}

		protected:
			State * sourceState;
			State * destinationState;
		};

		template <class T, class U>
		class GenericTransition : public StateTransition
		{
		public:
			typedef bool(*GenericTransitionFunc)(T, U); //
			GenericTransition(GenericTransitionFunc f, T testData, U otherData, State* srcState, State* destState) :
				dataA(testData), dataB(otherData) //实例化dataA和dataB变量。初始化程序列表
			{
				func				= f;
				sourceState			= srcState;		//
				destinationState	= destState;
			}
			~GenericTransition() {}

			virtual bool CanTransition() const override{//确定是否应该从一种状态更改为另一种状态
				if (func) {
					return func(dataA, dataB);
				}
				return false;
			}
			static bool GreaterThanTransition(T dataA, U dataB) {
				return dataA > dataB;
			}

			static bool LessThanTransition(T dataA, U dataB) {
				return dataA < dataB;
			}

			static bool EqualsTransition(T dataA, U dataB) {
				return dataA == dataB;
			}

			static bool NotEqualsTransition(T dataA, U dataB) {
				return dataA != dataB;
			}

		protected:
			GenericTransitionFunc  func;
			T dataA;
			U dataB;
		};
	}
}

