#include "z0/tools.h"
#include "z0/gui/gresource_vector.h"

namespace z0 {

    GResourceVector::GResourceVector(const string&RES):
        GResource(RES) {
        splitResString(RES);
    }

    void GResourceVector::splitResString(const string&RES) {
        auto res = split(RES, ',');
        if ((!res.empty()) && (!res[0].empty())) {
            width = stoul(string{res[0]});
        } else if (res.size() > 1) {
            height = stoul(string{res[1]});
        } else if (res.size() > 2) {
            if (res[2] == "RAISED") { style = RAISED; }
            if (res[2] == "LOWERED") { style = LOWERED; }
            if (res[2] == "FLAT") { style = FLAT; }
        } else if (res.size() > 3) {
            flat = (res[3] == "FLAT");
        }
    }

}