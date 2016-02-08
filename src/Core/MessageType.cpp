
#include "Core/MessageType.h"

#include "Core/CommandSystem.h"

namespace BitEngine {
    CREATE_MESSAGE_TYPE(WindowCreated);
    CREATE_MESSAGE_TYPE(WindowClosed);
	CREATE_MESSAGE_TYPE(InputReceiver::KeyboardInput);
	CREATE_MESSAGE_TYPE(InputReceiver::MouseInput);
	CREATE_MESSAGE_TYPE(CommandSystem::CommandInput);
}
