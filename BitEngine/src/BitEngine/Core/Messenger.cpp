
#include "bitengine/Core/Messenger.h"

using namespace BitEngine;

u32 Messenger::MessageType::getNextType() {
	static u32 X = 0;
	return ++X;
}