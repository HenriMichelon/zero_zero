#pragma once

namespace z0 {

    class GStyleClassicResource: public GResource {
    public:
        enum Style {
            FLAT,
            RAISED,
            LOWERED
        };

        Style	style{FLAT};
        float	width{0};
        float	height{0};
        bool	flat{false};

        explicit GStyleClassicResource(const string&);

    private:
        void splitResString(const string&);
    };
    
}