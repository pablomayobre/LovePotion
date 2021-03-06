#include "common/runtime.h"
#include "modules/joystick/joystick.h"

using namespace love;

Joystick::Joystick()
{
    hidScanInput();

    for (u32 index = 0; index < MAX_GAMEPADS; index++)
    {
        if (Joystick::IsConnected(index))
            this->AddGamepad(index);
    }
}

Joystick::~Joystick()
{
    for (auto stick : this->gamepads)
        stick->Release();
}

bool Joystick::IsConnected(u32 id)
{
    #if defined (__SWITCH__)
        HidControllerID controller = (id == 0) ? CONTROLLER_P1_AUTO : static_cast<HidControllerID>(id);

        return hidIsControllerConnected(controller);
    #endif

    return true;
}

size_t Joystick::GetJoystickCount() const
{
    return this->gamepads.size();
}

Gamepad * Joystick::GetJoystickFromID(size_t index)
{
    for (auto stick : this->gamepads)
    {
        if (stick->GetID() == index)
            return stick;
    }

    return nullptr;
}

Gamepad * Joystick::AddGamepad(size_t index)
{
    if (index < 0 || index > MAX_GAMEPADS)
        return nullptr;

    Gamepad * joystick = new Gamepad(index);
    this->gamepads.push_back(joystick);

    return joystick;
}

void Joystick::RemoveGamepad(Gamepad * gamepad)
{
    if (!gamepad)
        return;

    auto iterator = std::find(this->gamepads.begin(), this->gamepads.end(), gamepad);

    if (iterator != this->gamepads.end())
    {
        (*iterator)->Release();
        delete *iterator;
        this->gamepads.erase(iterator);
    }
}
