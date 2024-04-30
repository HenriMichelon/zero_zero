
#include "z0/application.h"
#include "z0/window.h"

#include <cassert>

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszArgument, int nCmdShow)
{
    if (z0::Application::_instance == nullptr) z0::die("No Application object found");
    z0::Application& application = z0::Application::get();
    application._window = make_unique<z0::Window>(hThisInstance);
    application._mainLoop();
    return application._messages.wParam;
}
#endif

namespace z0 {

    Application* Application::_instance = nullptr;

    Application::Application(const z0::ApplicationConfig &appConfig):
        applicationConfig{appConfig} {
        assert(_instance == nullptr);
        _instance = this;
    }

#ifdef _WIN32
    void Application::_mainLoop() {
        while (GetMessage(&_messages, nullptr, 0, 0))
        {
            TranslateMessage(&_messages);
            DispatchMessage(&_messages);
        }
    }
#endif

    Window &Application::getWindow() const {
        assert(_window != nullptr);
        return *_window;
    }

    Application &Application::get() {
        assert(_instance != nullptr);
        return *_instance;
    }

}
