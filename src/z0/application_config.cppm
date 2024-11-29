/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ApplicationConfig;

import z0.Constants;
import z0.DebugConfig;

export namespace z0 {

    /**
     * Global application configuration, given to the Application object at startup
     * via the Z0_APP macro
     */
    struct ApplicationConfig {
        //! Name to display in the Window title bar
        string           appName                    = "MyApp";
        //! Directory to search for resources and compiled shaders
        filesystem::path appDir                     = ".";
        //! State of the display Window
        WindowMode       windowMode                 = WindowMode::WINDOWED;
        //! Width in pixels of the display Window
        uint32_t         windowWidth                = 1280;
        //! Height in pixels of the display Window
        uint32_t         windowHeight               = 720;
        //! Monitor index to display the Window
        int32_t          windowMonitor              = 0;
        //! Default font name, the file must exist in the path
        string           defaultFontName            = "DefaultFont.ttf";
        //! Default font size. See the Font class for the details.
        uint32_t         defaultFontSize            = 20;
        //! Where to log message using log()
        int              loggingMode                = LOGGING_MODE_NONE;
        //! Monitor index for the logging Window
        int32_t          loggingMonitor             = 0;
        //! MSAA samples. Note that MSAA is mandatory
        MSAA             msaa                       = MSAA::X4;
        //! Depth frame buffer format
        DepthFormat      depthFormat                = DepthFormat::B24;
        //! Use a depth pre-pass in the main renderer
        bool             useDepthPrepass            = true;
        //! Gamma correction value for the tone mapping renderer
        float            gamma                      = 1.0f;
        //! Exposure correction value for the tone mapping renderer
        float            exposure                   = 1.0f;
        //! Window clear color
        vec3             clearColor                 = WINDOW_CLEAR_COLOR;
        //! Number of simultaneous frames during rendering
        uint32_t         framesInFlight             = 3;
        //! Size (width & height) in pixels of the cascaded (directional lights) shadow maps
        uint32_t         cascadedShadowMapSize      = 4096;
        //! Size (width & height) in pixels of the omni & spotlights shadow maps
        uint32_t         pointLightShadowMapSize    = 1024;
        //! Enable the debug renderer
        bool             debug                      = false;
        //! Configuration for the debug rendering
        DebugConfig      debugConfig                = {};
    };
}