#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/gui/gresource.h"
#endif
#include "gstyle_classic_resource.h"

namespace z0 {

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