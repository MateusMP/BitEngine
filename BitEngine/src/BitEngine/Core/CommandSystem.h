#pragma once

#include <unordered_map>

#include "BitEngine/Core/Window.h"
#include "BitEngine/Core/Messenger.h"

namespace BitEngine {

/**
 * Command System is a default system used to define commands for inputs.
 * The inputs should be registered to a command, that will be triggered by an event.
 */
class BE_API CommandSystem
{
public:
    /**
     * The body for command events.
     */
    struct BE_API MsgCommandInput
    {
    public:
        MsgCommandInput() { action.other = 0; }
        MsgCommandInput(int _id, float _intensity, int _other);
        MsgCommandInput(int _id, float _intensity, KeyAction _other);
        MsgCommandInput(int _id, float _intensity, MouseAction _other, double x, double y);
        MsgCommandInput(int _id, float _intensity);

        int commandID; // The registered command id
        float intensity; // If it was an analogic input
        double mouse_x; // Invalid unless Mouse input was used
        double mouse_y; // Invalid unless Mouse input was used

        union {
            KeyAction fromButton; // keyboard / mouse / joystick buttons
            MouseAction fromMouse;
            int other;
        } action; // If there was some other action
    };

    CommandSystem(Window* window);
    ~CommandSystem();

    Messenger<MsgCommandInput> commandSignal;

    bool init();
    void shutdown();

    void setCommandState(int state);

    /**
     * Remove all keybinds to given command
     */
    void unregisterCommand(int commandID);

    /**
     * @param commandID	the command ID returned when given key/action/mod combination ocurrs
     * @param commandState		CommandSystem state necessary for processing this command
     *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
     * @param key	keyboard key needed
     * Note that this function will register the commandID for both inputs: RELEASE and PRESS
     */
    bool registerKeyboardCommand(int commandID, int commandState, int key);

    /**
     * @param commandID	the command ID returned when given key/action/mod combination ocurrs
     * @param commandState		CommandSystem state necessary for processing this command
     *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
     * @param key	keyboard key needed
     * Note that this function will register the commandID for both inputs: RELEASE and PRESS and to be accepted with any keymod combination
     *
     * If an ambiguous keybind is found, it wont be overwritten and the command being bind will not be fully binded.
     * In this case, the function returns false.
     * It's recommended that all commands for the requested commandID are cleared if this function fails.
     * Left optional for the caller, so no other binding will be silently removed.
     */
    bool registerKeyCommandForAllMods(int commandID, int commandState, int key);

    /**
     * @param commandID	Command ID returned when given key/action/mod combination ocurrs
     * @param commandState		CommandSystem state necessary for processing this command
     *							Use -1 to assign it as a "Global" command. Useful for debug purposes.
     * @param key	Keyboard key needed
     * @param action	Key state needed
     * @param mod	Key modifiers (Shift, Alt, Ctrl, Super)
     */
    bool registerKeyboardCommand(int commandID, int commandState, int key, KeyAction action, KeyMod mod = KeyMod::NONE);

    /**
     * Same for mouse commands
     */
    bool RegisterMouseCommand(int commandID, int commandState, int button, MouseAction action, KeyMod mod = KeyMod::NONE);

    /**
     *
     * @param commandID
     * @param commandState
     * @param mod
     * @return
     */
    bool RegisterMouseMove(int commandID, int commandState, KeyMod mod = KeyMod::NONE) {
        return RegisterMouseCommand(commandID, commandState, 0, MouseAction::MOVE, mod);
    }

    /**
     * The command system listen to MsgKeyboardInput so it can check for command triggers.
     */
    void onMessage(const MsgKeyboardInput& msg_);

    /**
     * The command system listen to MsgKeyboardInput so it can check for command triggers.
     */
    void onMessage(const MsgMouseInput& msg);

private:
    enum class InputType : char {
        keyboard,
        mouse,
        joystick,
        other,
    };

    struct CommandIdentifier {
        CommandIdentifier()
            : commandState(0), msgCommandInputType(InputType::other)
        {}

        CommandIdentifier(int s, const MsgKeyboardInput& k)
            : commandState(s), msgCommandInputType(InputType::keyboard), keyboard(k) {}

        CommandIdentifier(int s, const MsgMouseInput& m)
            : commandState(s), msgCommandInputType(InputType::mouse), mouse(m) {}

        int commandState;
        InputType msgCommandInputType;

        MsgKeyboardInput keyboard;
        MsgMouseInput mouse;
    };

    // CommandIdentifier Hash and Equal
    class CIHash {
    public:
        std::size_t operator()(const CommandIdentifier& k) const;
    };
    class CIEqual {
    public:
        bool operator() (const CommandIdentifier& t1, const CommandIdentifier& t2) const;
    };

    std::unordered_map<CommandIdentifier, int, CIHash, CIEqual> m_commands;
    int m_commandState;
    Window* window;
    Messenger<MsgKeyboardInput>::ScopedSubscription subKeyboard;
    Messenger<MsgMouseInput>::ScopedSubscription subMouse;
};

}
