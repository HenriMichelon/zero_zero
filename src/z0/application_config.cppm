module;
#include "z0/libraries.h"

export module z0:ApplicationConfig;

import :Constants;

export namespace z0 {

    /**
     * Global application configuration, given to the Application object at startup
     * via the Z0_APP macro
     */
    struct ApplicationConfig {
        //! Name to display in the window title bar
        string           appName           = "MyApp";
        //! Directory to search for resources and compiled shaders
        filesystem::path appDir            = ".";
        //! Initial state of the display window
        WindowMode       windowMode        = WINDOW_MODE_WINDOWED;
        //! Initial width in pixels of the display window
        uint32_t         windowWidth       = 1920;
        //! Initial height in pixels of the display window
        uint32_t         windowHeight      = 1080;
        //! Default font name, the file must exist in the path
        string           defaultFontName   = "DefaultFont.ttf";
        //! Default font size. See the Font class for the details.
        uint32_t         defaultFontSize   = 20;
        //! Where to log message using log()
        uint32_t        loggingMode        = LOGGING_NONE;
        //! MSAA samples. Note that MSAA is mandatory
        MSAA             msaa              = MSAA_AUTO;
        //! Depth frame buffer format
        DEPTH_FORMAT     depthFormat       = DEPTH_FORMAT_AUTO;
        //! Gamma correction value for the tone mapping renderer
        float            gamma             = 1.0f;
        //! Exposure correction value for the tone mapping renderer
        float            exposure          = 1.0f;
        //! Window clear color
        vec3             clearColor        = WINDOW_CLEAR_COLOR;
        //! Number of simultaneous frames during rendering
        uint32_t         framesInFlight    = 3;
    };
}