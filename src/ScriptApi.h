#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/appdomain.h>
#include "Application.h"

static void printInfo()           { return Application::instance()->printInfo(); }
static float deltaTime()           { return 1 / 60.0f; }
static void Debug_LogFloat(float delta_time) { printf("%f\n", delta_time); }

static void RegisterCallbacks()
{
    mono_add_internal_call("Simengine.Debug::PrintInfo", printInfo);
    mono_add_internal_call("Simengine.Time::Deltatime", deltaTime);

    mono_add_internal_call("Simengine.Debug::Log(single)", Debug_LogFloat);
}