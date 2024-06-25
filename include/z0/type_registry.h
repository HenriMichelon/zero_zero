#pragma once

namespace z0 {

    template<typename T> Object* _createNewObjectInstance() { return new T; }
    
    class TypeRegistry {
    public:
        template<typename T> static shared_ptr<T> makeShared(const string&clazz) {
            return shared_ptr<T>(reinterpret_cast<T*>(TypeRegistry::typeMap[clazz]()));
        }
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

    #define Z0_CONCAT_IMPL(x, y) x##y
    #define Z0_MACRO_CONCAT(x, y) Z0_CONCAT_IMPL(x, y)
    #define Z0_REGISTER_TYPE(TYP) static _TypeRegister<TYP> Z0_MACRO_CONCAT(z0_Type_, __COUNTER__)(#TYP); 
}