#pragma once

#include "z0/gui/gpanel.h"

namespace z0 {

    class GText: public GPanel {
    public:
        GText(string);

        const string& getText() const { return text; }
        void setText(const string&);

        // get automatic sizing flag
        bool isAutoSize() const { return autoSize; };

        // set automatic sizing flag
        //	bool	: TRUE = auto size when text is changed
        void setAutoSize(bool);

        void computeSize();

    protected:
        string	text;

        virtual void eventCreate();

    private:
        bool	autoSize{true};
    };

}
