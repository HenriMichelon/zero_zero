#pragma once

// Macro to use the concatenation macro
#define Z0_CONCAT_IMPL(x, y) x##y
// Macro to use the __COUNTER__ macro
#define Z0_MACRO_CONCAT(x, y) Z0_CONCAT_IMPL(x, y)
/**
  Register a new class.
  Each registered type have an unique name in the form z0_Type_XX
*/
#define Z0_REGISTER_TYPE(TYP) static _TypeRegister<TYP> Z0_MACRO_CONCAT(z0_Type_, __COUNTER__)(#TYP);

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
/**
 * Application startup macro. Must be used outside any namespace.
 * @param CONFIG An ApplicationConfig object
 * @param ROOTNODE The root Node of the startup scene
 */
#define Z0_APP(CONFIG, ROOTNODE) \
z0::VulkanApplication _z0_app(CONFIG, ROOTNODE); \
int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) { \
 if (z0::Application::_instance == nullptr) z0::die("No Application object found"); \
 z0::Application::get()._mainLoop(); \
 return 0; \
};
#endif