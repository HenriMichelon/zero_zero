/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
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
        //! State of the display window
        WindowMode       windowMode        = WINDOW_MODE_WINDOWED;
        //! Width in pixels of the display window
        uint32_t         windowWidth       = 1280;
        //! Height in pixels of the display window
        uint32_t         windowHeight      = 720;
        //! Monitor index to display the window
        int32_t          windowMonitor     = 0;
        //! Default font name, the file must exist in the path
        string           defaultFontName   = "DefaultFont.ttf";
        //! Default font size. See the Font class for the details.
        uint32_t         defaultFontSize   = 20;
        //! Where to log message using log()
        uint32_t         loggingMode       = LOGGING_NONE;
        //! Monitor index for the logging window
        int32_t          loggingMonitor    = 0;
        //! MSAA samples. Note that MSAA is mandatory
        MSAA             msaa              = MSAA_4X;
        //! Depth frame buffer format
        DepthFormat      depthFormat       = DEPTH_FORMAT_24BIT;
        //! Use a depth pre-pass in the main renderer
        bool             useDepthPrepass   = true;
        //! Gamma correction value for the tone mapping renderer
        float            gamma             = 1.0f;
        //! Exposure correction value for the tone mapping renderer
        float            exposure          = 1.0f;
        //! Window clear color
        vec3             clearColor        = WINDOW_CLEAR_COLOR;
        //! Number of simultaneous frames during rendering
        uint32_t         framesInFlight    = 3;
        //! Size (width & height) in pixels of the cascaded (directional lights) shadow maps
        uint32_t         cascadedShadowMapSize   = 4096;
        //! Size (width & height) in pixels of the omni & spot lights shadow maps
        uint32_t         pointLightShadowMapSize = 1024;
    };
}