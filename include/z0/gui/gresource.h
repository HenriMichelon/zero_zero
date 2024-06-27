#pragma once

namespace z0 {

    /**
     * Super class for style resources descriptions
     */
    class GResource: public Object {
    public:
        explicit GResource(string R): res(std::move(R)) {};
        virtual ~GResource() = default;

        [[nodiscard]] const string& Resource() const { return res; };

    private:
        string res;
    };

}