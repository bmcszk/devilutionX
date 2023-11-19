#include "engine/events.hpp"

#include <cstdint>

#include "controls/input.h"
#include "engine.h"
#include "engine/demomode.h"
#include "interfac.h"
#include "movie.h"
#include "options.h"
#include "panels/console.hpp"
#include "utils/log.hpp"

#ifdef USE_SDL1
#include "utils/display.h"
#else
#include "controls/touch/event_handlers.h"
#endif

#ifdef __vita__
#include "diablo.h"
#include "platform/vita/touch.h"
#endif

#ifdef __SWITCH__
#include "platform/switch/docking.h"
#include <switch.h>
#endif

namespace devilution {

namespace {

bool FalseAvail(const char *name, int value)
{
	LogVerbose("Unhandled SDL event: {} {}", name, value);
	return true;
}

bool FetchMessage_Real(SDL_Event *event, uint16_t *modState)
{
#ifdef __SWITCH__
	HandleDocking();
#endif

	SDL_Event e;
	if (PollEvent(&e) == 0) {
		return false;
	}

	event->type = static_cast<SDL_EventType>(0);
	*modState = SDL_GetModState();

#ifdef __vita__
	HandleTouchEvent(&e, MousePosition);
#elif !defined(USE_SDL1)
	HandleTouchEvent(e);
#endif

	if (e.type == SDL_QUIT || IsCustomEvent(e.type)) {
		*event = e;
		return true;
	}

	if (IsAnyOf(e.type, SDL_KEYUP, SDL_KEYDOWN) && e.key.keysym.sym == SDLK_UNKNOWN) {
		// Erroneous events generated by RG350 kernel.
		return true;
	}

#if !defined(USE_SDL1) && !defined(__vita__)
	if (!movie_playing) {
		// SDL generates mouse events from touch-based inputs to provide basic
		// touchscreeen support for apps that don't explicitly handle touch events
		if (IsAnyOf(e.type, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP) && e.button.which == SDL_TOUCH_MOUSEID)
			return true;
		if (e.type == SDL_MOUSEMOTION && e.motion.which == SDL_TOUCH_MOUSEID)
			return true;
		if (e.type == SDL_MOUSEWHEEL && e.wheel.which == SDL_TOUCH_MOUSEID)
			return true;
	}
#endif

#ifdef USE_SDL1
	if (e.type == SDL_MOUSEMOTION) {
		OutputToLogical(&e.motion.x, &e.motion.y);
	} else if (IsAnyOf(e.type, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP)) {
		OutputToLogical(&e.button.x, &e.button.y);
	}
#endif

	if (HandleControllerAddedOrRemovedEvent(e))
		return true;

	switch (e.type) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	case SDL_CONTROLLERAXISMOTION:
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
	case SDL_FINGERDOWN:
	case SDL_FINGERUP:
	case SDL_TEXTEDITING:
	case SDL_TEXTINPUT:
	case SDL_WINDOWEVENT:
#else
	case SDL_ACTIVEEVENT:
#endif
	case SDL_JOYAXISMOTION:
	case SDL_JOYHATMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		*event = e;
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		if (e.key.keysym.sym == -1)
			return FalseAvail(e.type == SDL_KEYDOWN ? "SDL_KEYDOWN" : "SDL_KEYUP", e.key.keysym.sym);
		*event = e;
		break;
#ifndef USE_SDL1
	case SDL_MOUSEWHEEL:
#ifdef _DEBUG
		if (IsConsoleOpen()) {
			*event = e;
			break;
		}
#endif
		// This is a hack, mousewheel events should be handled directly by their consumers instead.
		event->type = SDL_KEYDOWN;
		if (e.wheel.y > 0) {
			event->key.keysym.sym = (SDL_GetModState() & KMOD_CTRL) != 0 ? SDLK_KP_PLUS : SDLK_UP;
		} else if (e.wheel.y < 0) {
			event->key.keysym.sym = (SDL_GetModState() & KMOD_CTRL) != 0 ? SDLK_KP_MINUS : SDLK_DOWN;
		} else if (e.wheel.x > 0) {
			event->key.keysym.sym = SDLK_LEFT;
		} else if (e.wheel.x < 0) {
			event->key.keysym.sym = SDLK_RIGHT;
		}
		break;
#if SDL_VERSION_ATLEAST(2, 0, 4)
	case SDL_AUDIODEVICEADDED:
		return FalseAvail("SDL_AUDIODEVICEADDED", e.adevice.which);
	case SDL_AUDIODEVICEREMOVED:
		return FalseAvail("SDL_AUDIODEVICEREMOVED", e.adevice.which);
	case SDL_KEYMAPCHANGED:
		return FalseAvail("SDL_KEYMAPCHANGED", 0);
#endif
#endif
	default:
		return FalseAvail("unknown", e.type);
	}
	return true;
}

} // namespace

EventHandler CurrentEventHandler;

EventHandler SetEventHandler(EventHandler eventHandler)
{
	sgOptions.Padmapper.ReleaseAllActiveButtons();

	EventHandler previousHandler = CurrentEventHandler;
	CurrentEventHandler = eventHandler;
	return previousHandler;
}

bool FetchMessage(SDL_Event *event, uint16_t *modState)
{
	const bool available = demo::IsRunning() ? demo::FetchMessage(event, modState) : FetchMessage_Real(event, modState);

	if (available && demo::IsRecording())
		demo::RecordMessage(*event, *modState);

	return available;
}

void HandleMessage(const SDL_Event &event, uint16_t modState)
{
	assert(CurrentEventHandler != nullptr);

	CurrentEventHandler(event, modState);
}

} // namespace devilution
