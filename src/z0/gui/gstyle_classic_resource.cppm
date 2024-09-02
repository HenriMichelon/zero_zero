module;
#include "z0/modules.h"

export module Z0:GStyleClassicResource;

import :Tools;
import :GResource;

export namespace z0 {

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


    GStyleClassicResource::GStyleClassicResource(const string&RES):
        GResource(RES) {
        splitResString(RES);
    }

    void GStyleClassicResource::splitResString(const string&RES) {
        auto res = split(RES, ',');
        if ((!res.empty()) && (!res[0].empty())) {
            width = stof(string{res[0]});
        }
        if ((res.size() > 1) && (!res[1].empty())) {
            height = stof(string{res[1]});
        }
        if ((res.size() > 2) && (!res[2].empty())) {
            if (res[2] == "RAISED") { style = RAISED; }
            if (res[2] == "LOWERED") { style = LOWERED; }
            if (res[2] == "FLAT") { style = FLAT; }
        }
        if ((res.size() > 3) && (!res[3].empty())) {
            flat = (res[3] == "FLAT");
        }
    }
    
}