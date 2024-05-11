#include "z0/tools.h"
#include "z0/gui/glayout_vector_resource.h"

namespace z0 {

    GLayoutVectorResource::GLayoutVectorResource(const string&RES):
        GResource(RES) {
        splitResString(RES);
    }

    void GLayoutVectorResource::splitResString(const string&RES) {
        auto res = split(RES, ',');
        if ((!res.empty()) && (!res[0].empty())) {
            width = stoul(string{res[0]});
        }
        if ((res.size() > 1) && (!res[1].empty())) {
            height = stoul(string{res[1]});
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