#pragma once

#include <string>
using namespace std;

namespace z0 {

    class GResource {
    public:
        explicit GResource(const string&R): res(R) {};
        virtual ~GResource() = default;

        const string& Resource() const { return res; };

    private:
        string res;
    };

}