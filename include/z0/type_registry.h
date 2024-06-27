#pragma once

namespace z0 {

    template<typename T> Object* _createNewObjectInstance() { return new T; }
    
    /**
     * Register custom nodes type used to map the node's description in JSON scene files and real classes
     */
    class TypeRegistry {
    public:
        /**
         * Creates a new shared pointer to a new instance of type `clazz` with casting to the type `T`
         */
        template<typename T> [[nodiscard]] static shared_ptr<T> makeShared(const string&clazz) {
            if (!typeMap.contains(clazz)) die("Type", clazz, "not registered in TypeRegistry");
            return shared_ptr<T>(reinterpret_cast<T*>(TypeRegistry::typeMap[clazz]()));
        }

        /**
         * Register a new class. Use it when registering from code.
         * If you want to register outside a bloc of code use the `Z0_REGISTER_TYPE(class)` macro after your class declaration
         */
        template<typename T> static void registerType(const string&clazz) {
            typeMap[clazz] = &_createNewObjectInstance<T>;
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

    // Macro to use the concatenation macro
    #define Z0_CONCAT_IMPL(x, y) x##y
    // Macro to use the __COUNTER__ macro
    #define Z0_MACRO_CONCAT(x, y) Z0_CONCAT_IMPL(x, y)
    /**
      Register a new class.
      Each type register object have an unique name in the form z0_Type_XX
    */
    #define Z0_REGISTER_TYPE(TYP) static _TypeRegister<TYP> Z0_MACRO_CONCAT(z0_Type_, __COUNTER__)(#TYP);  
}