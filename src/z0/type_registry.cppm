/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.TypeRegistry;

import z0.Object;
import z0.Tools;

export namespace z0 {

    // template<typename T> Object* _createNewObjectInstance() { return new T(); }

    /**
     * Register custom nodes types used to map the node's description in JSON scene files and real classes
     */
    class TypeRegistry {
    public:
        /**
         * Creates a new shared pointer to a new instance of type `clazz` with casting to the type `T`
         */
        template<typename T> [[nodiscard]] static shared_ptr<T> makeShared(const string&clazz) {
            if (!typeMap->contains(clazz)) { die("Type", clazz, "not registered in TypeRegistry"); }
            return shared_ptr<T>(reinterpret_cast<T*>(typeMap->at(clazz)()));
        }

        /**
         * Register a new class. Use it when registering from code.
         * If you want to register outside a bloc of code use the `Z0_REGISTER_TYPE(class)` macro after your class declaration
         */
        template<typename T> static void registerType(const string&clazz) {
            if (typeMap == nullptr) { typeMap = make_unique<map<string, function<Object*()>>>(); }
            typeMap->emplace(clazz, []{ return new T(); });
        }

    // private:
        static unique_ptr<map<string, function<Object*()>>> typeMap;
    };

    template<typename T>
    struct _TypeRegister {
        explicit _TypeRegister(const string&clazz) {
            TypeRegistry::registerType<T>(clazz);
        }
    };

    unique_ptr<map<string, function<Object*()>>> TypeRegistry::typeMap{nullptr};

}