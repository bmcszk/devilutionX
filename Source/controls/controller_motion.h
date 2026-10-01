#pragma once

// Processes and stores mouse and joystick motion.

#ifdef USE_SDL3
#include <SDL3/SDL_events.h>
#else
#include <SDL.h>
#endif

#include "controls/axis_direction.h"
#include "controls/controller.h"

namespace devilution {

// Whether we're currently simulating the mouse with SELECT + D-Pad.
extern bool SimulatingMouseWithPadmapper;

// Raw axis values.
extern float leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;

// Axis values scaled to [-1, 1] range and clamped to a deadzone.
extern float leftStickX, leftStickY, rightStickX, rightStickY;

// Whether stick positions have been updated and need rescaling.
extern bool leftStickNeedsScaling, rightStickNeedsScaling;

// Left-stick movement mechanism: from standstill, below the turn threshold the player turns
// in place, at/above it they walk. Once walking, the player keeps walking until the stick is
// fully released (easing off never cuts the walk mid-stride).
constexpr float StickTurnMargin = 0.3F; // added to the deadzone to form the turn threshold

// Minimum scaled stick magnitude to register a direction (firm-push gate for dpad/padmapper
// movement and fake-attack suppression; a separate mechanism from the turn/walk thresholds above).
constexpr float StickDirectionThreshold = 0.4F;

// Current scaled left-stick magnitude.
float GetLeftStickMagnitude();

// Left-stick turn/walk boundary: deadzone + StickTurnMargin (deadzone-dependent, hence not constexpr).
float GetStickTurnThreshold();

// Updates motion state for mouse and joystick sticks.
void ProcessControllerMotion(const SDL_Event &event);

// Indicates whether the event represents movement of an analog thumbstick.
bool IsControllerMotion(const SDL_Event &event);

// Returns direction of the left thumb stick or DPad (if allow_dpad = true).
AxisDirection GetLeftStickOrDpadDirection(bool usePadmapper);

// Direction of the left thumb stick.
AxisDirection GetLeftStickDirection();

// Simulates right-stick movement based on input from padmapper mouse movement actions.
void SimulateRightStickWithPadmapper(ControllerButtonEvent ctrlEvent);

} // namespace devilution
