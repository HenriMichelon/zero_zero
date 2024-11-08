/*
 * Copyright (c) 2024 Henri Michelon
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

    template<typename T> Object* _createNewObjectInstance() { return new T(); }
    
    /**
     * Register custom nodes types used to map the node's description in JSON scene files and real classes
     */
    class TypeRegistry {
    public:
        /**
         * Creates a new shared pointer to a new instance of type `clazz` with casting to the type `T`
         */
        template<typename T> [[nodiscard]] static shared_ptr<T> makeShared(const string&clazz) {
            if (!typeMap.contains(clazz)) die("Type", clazz, "not registered in TypeRegistry");
            return shared_ptr<T>(reinterpret_cast<T*>(typeMap[clazz]()));
        }

        /**
         * Register a new class. Use it when registering from code.
         * If you want to register outside a bloc of code use the `Z0_REGISTER_TYPE(class)` macro after your class declaration
         */
        template<typename T> static void registerType(const string&clazz) {
            typeMap.emplace(clazz, &_createNewObjectInstance<T>);
        }
    private:
        static map<std::string, Object*(*)()> typeMap;
    };

    template<typename T>
    struct _TypeRegister {
        _TypeRegister(const string&clazz) {
            TypeRegistry::registerType<T>(clazz);
        }
    };

    map<std::string, Object*(*)()> TypeRegistry::typeMap;

}