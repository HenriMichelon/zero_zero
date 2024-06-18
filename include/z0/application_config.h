#pragma once

namespace z0 {

    /**
     * Global application configuration, given to the Application object at startup
     * via the Z0_APP macro
     */
    struct ApplicationConfig {
        //! Name to display in the window title bar
        string           appName         = "MyApp";
        //! Directory to search for resources and compiled shaders
        filesystem::path appDir          = ".";
        //! Initial state of the display window
        WindowMode       windowMode      = WINDOW_MODE_WINDOWED;
        //! Initial width in pixels of the display window
        uint32_t         windowWidth     = 800;
        //! Initial height in pixels of the display window
        uint32_t         windowHeight    = 600;
        //! Default font name, the file must exists in the path
        string           defaultFontName = "DefaultFont.ttf";
        //! Default font size (not in pixels)
        uint32_t         defaultFontSize = 16;
        //! Where to log message using log()
        LoggingMode      loggingMode     = LOGGING_WINDOW;
        //! Default MSAA parameter. Note that MSAA is mandatory
        MSAA             msaa            = MSAA_AUTO;
        //! Gamma correction value for the tone mapping renderer
        float            gamma           = 1.0f;
        //! Exposure correction value for the tone mapping renderer
        float            exposure        = 1.0f;
    };
}