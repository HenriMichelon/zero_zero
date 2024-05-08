#include "z0/gui/glayout.h"
namespace z0 {

//----------------------------------------------
    GLayout::GLayout()
    {
        font = nullptr;
    }


//----------------------------------------------
    shared_ptr<GLayout> GLayout::create(const string&NAME)
    {
        shared_ptr<GLayout> tmp;
        /*if (NAME == "vector") {
            tmp = (GLayout*) new GLayoutVector;
        }
        else if (NAME == "pixmap") {
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
    void GLayout :: setOption(const string&NAME, const string&VAL)
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
    string GLayout :: getOption(const string&NAME)
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