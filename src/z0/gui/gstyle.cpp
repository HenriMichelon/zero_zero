#include "z0/gui/gstyle.h"
#include "z0/gui/gstyle_classic.h"

namespace z0 {

//----------------------------------------------
    GStyle::GStyle()
    {
        font = nullptr;
    }


//----------------------------------------------
    shared_ptr<GStyle> GStyle::create(const string&NAME)
    {
        shared_ptr<GStyle> tmp;
        if (NAME == "vector") {
            tmp = make_shared<GStyleClassic>();
        }
        /*else if (NAME == "pixmap") {
            tmp = (GLayout*) new GLayoutPixmap;
        }*/
        if (tmp != nullptr) {
            if (!tmp->init()) {
                tmp = nullptr;
            }
        }
        return tmp;
    }


//----------------------------------------------
    void GStyle :: setOption(const string&NAME, const string&VAL)
    {
        shared_ptr<GLayoutOption> option;
        for(auto& opt: options) {
            if (opt->name == NAME) {
                option = opt;
                break;
            }
        }
        if (option == nullptr) {
            option = make_shared<GLayoutOption>(NAME);
            options.push_back(option);
        }
        option->value = VAL;
        updateOptions();
    }


//----------------------------------------------
    string GStyle :: getOption(const string&NAME)
    {
        shared_ptr<GLayoutOption> option;
        for(auto& opt: options) {
            if (opt->name == NAME) {
                option = opt;
                break;
            }
        }
        if (option == nullptr) { return ""; }
        return option->value;
    }

}