#include "il2cpp_dump.h"
#include <Windows.h>
#include <cassert>
#include <chrono>

#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <vcruntime_string.h>
#include <stringapiset.h>
#include "log.h"
#include "il2cpp-tabledefs.h"
#include "il2cpp-types.h"

#define DO_API(r, n, p) r (*n) p

#include "il2cpp-api-functions.h"

#undef DO_API

static uint64_t il2cpp_base = 0;

struct il2cppString : Il2CppObject // Credits: il2cpp resolver (https://github.com/sneakyevil/IL2CPP_Resolver/blob/main/Unity/Structures/System_String.hpp)
{
    int m_iLength;
    wchar_t m_wString[1024];

    void Clear()
    {
        if (!this) return;

        memset(m_wString, 0, static_cast<size_t>(m_iLength) * 2);
        m_iLength = 0;
    }

    std::string ToString()
    {
        if (!this) return "";

        std::string sRet(static_cast<size_t>(m_iLength) * 3 + 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, m_wString, m_iLength, &sRet[0], static_cast<int>(sRet.size()), 0, 0);
        return sRet;
    }
};

using StaticString_t = il2cppString*(*)();


std::string GetProtectedExportName() {
    /* SXITXMA
    const std::string suffix = "_wasting_your_life";
    std::string fullName;
    std::ifstream file("GameAssembly.dll", std::ios::binary);
    assert(!file && "Error Occured when trying to open the GameAssembly dll file!");
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    size_t pos = fileContent.find(suffix);
    if (pos != std::string::npos) {
        size_t start = pos;
        while (start > 0 && (isalnum(fileContent[start - 1]) || fileContent[start - 1] == '_')) { --start; }
        fullName = fileContent.substr(start, pos - start + suffix.length());
    }
    return fullName.empty() ? "il2cpp_domain_get_assemblies" : fullName;
    */
    // NULLBIT
    HMODULE pe_base = LoadLibraryExA(MODULENAME, NULL, DONT_RESOLVE_DLL_REFERENCES);
    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)pe_base;
    PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((BYTE*)dos_header + dos_header->e_lfanew);

    PIMAGE_OPTIONAL_HEADER optional_header = (PIMAGE_OPTIONAL_HEADER)&nt_headers->OptionalHeader;
    PIMAGE_DATA_DIRECTORY export_data_directory = &(optional_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
    PIMAGE_EXPORT_DIRECTORY export_directory = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)pe_base + export_data_directory->VirtualAddress);

    DWORD name_count = export_directory->NumberOfNames;
    PDWORD export_name_table = (PDWORD)((BYTE*)pe_base + export_directory->AddressOfNames);

    std::string protected_export = "_wasting_your_life";

    for (DWORD i = 0; i < export_directory->NumberOfNames; i++) {
        char* name = (char*)((BYTE*)pe_base + export_name_table[i]);
        std::string name_buf = std::string(name);
        if (name_buf.find(protected_export) != std::string::npos) {
            return name_buf;
        }
    }
    return "il2cpp_domain_get_assemblies";
}

void init_il2cpp_api() {
#define DO_API(r, n, p) n = (r (*) p)GetProcAddress((HMODULE)il2cpp_base, (#n != "il2cpp_domain_get_assemblies") ? #n : GetProtectedExportName().c_str())
    #include "il2cpp-api-functions.h"
    #undef DO_API
}

HMODULE get_module_base(const char *module_name)
{
    return GetModuleHandleA(module_name);
}

std::string get_method_modifier(uint32_t flags) {
    std::stringstream outPut;
    auto access = flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK;
    switch (access) {
        case METHOD_ATTRIBUTE_PRIVATE:
            outPut << "private ";
            break;
        case METHOD_ATTRIBUTE_PUBLIC:
            outPut << "public ";
            break;
        case METHOD_ATTRIBUTE_FAMILY:
            outPut << "protected ";
            break;
        case METHOD_ATTRIBUTE_ASSEM:
        case METHOD_ATTRIBUTE_FAM_AND_ASSEM:
            outPut << "internal ";
            break;
        case METHOD_ATTRIBUTE_FAM_OR_ASSEM:
            outPut << "protected internal ";
            break;
    }
    if (flags & METHOD_ATTRIBUTE_STATIC) {
        outPut << "static ";
    }
    if (flags & METHOD_ATTRIBUTE_ABSTRACT) {
        outPut << "abstract ";
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT) {
            outPut << "override ";
        }
    } else if (flags & METHOD_ATTRIBUTE_FINAL) {
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_REUSE_SLOT) {
            outPut << "sealed override ";
        }
    } else if (flags & METHOD_ATTRIBUTE_VIRTUAL) {
        if ((flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) == METHOD_ATTRIBUTE_NEW_SLOT) {
            outPut << "virtual ";
        } else {
            outPut << "override ";
        }
    }
    if (flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) {
        outPut << "extern ";
    }
    return outPut.str();
}

bool _il2cpp_type_is_byref(const Il2CppType *type) {
    auto byref = type->byref;
    if (il2cpp_type_is_byref) {
        byref = il2cpp_type_is_byref(type);
    }
    return byref;
}

std::string GetFullType(const Il2CppType* type) { // Shows stuff like Dictionary<TKey, TValue>, List<T> etc. 
    std::string _ = "";
    if (type->type == IL2CPP_TYPE_GENERICINST) {
        Il2CppGenericClass* genericClass = type->data.generic_class;
        _ = il2cpp_class_get_name(genericClass->cached_class);
        if (_[_.size() - 2] == '`') {
            _.pop_back(); _.pop_back();
        }
        _ += "<";
        const Il2CppGenericInst* classInst = genericClass->context.class_inst;
        if (classInst) {
            for (uint32_t i = 0; i < classInst->type_argc; ++i) {
                const Il2CppType* argType = classInst->type_argv[i];
                Il2CppClass* argClass = il2cpp_class_from_type(argType);
                if (argClass) {
                    _ += GetFullType(argType);
                }
                else {
                    _ += "UnknownType";
                }

                if (i < classInst->type_argc - 1) {
                    _ += ", ";
                }
            }
        }
        _ += ">";
    }
    else {
        Il2CppClass* typeClass = il2cpp_class_from_type(type);
        if (typeClass) {
            _ += il2cpp_class_get_name(typeClass);
        }
    }
    return _;
}

std::string Field_ReturnType(FieldInfo* field)
{
    auto field_type = il2cpp_field_get_type(field);
    auto field_class = il2cpp_class_from_type(field_type);
    return std::string(il2cpp_class_get_name(field_class));
}

StaticString_t get_version_m{ nullptr };
std::string dump_method(Il2CppClass *klass, const char* ns, const std::string& klassName) {
    std::stringstream outPut;
    outPut << "\n\t// Methods\n\n";
    void *iter = nullptr;
    while (auto method = il2cpp_class_get_methods(klass, &iter)) {
        //TODO attribute
        const char* methodName = il2cpp_method_get_name(method);
        if (method->methodPointer) {
            if (!get_version_m
                && klassName == "Application"
                && (strcmp(ns, "UnityEngine") == 0)
                && (strcmp(methodName, "get_version") == 0)
                )
                get_version_m = (StaticString_t)method->methodPointer;

            outPut << "\t// RVA: 0x";
            outPut << std::hex << (uint64_t) method->methodPointer - il2cpp_base;
            outPut << " VA: 0x";
            outPut << std::hex << (uint64_t) method->methodPointer;
        } else {
            outPut << "\t// RVA: 0x VA: 0x0";
        }
        outPut << "\n\t";
        uint32_t iflags = 0;
        auto flags = il2cpp_method_get_flags(method, &iflags);
        outPut << get_method_modifier(flags);
        auto return_type = il2cpp_method_get_return_type(method);
        if (_il2cpp_type_is_byref(return_type)) {
            outPut << "ref ";
        }
        auto return_class = il2cpp_class_from_type(return_type);
        outPut << GetFullType(return_type) << " " << methodName << "(";
        auto param_count = il2cpp_method_get_param_count(method);
        for (int i = 0; i < (int)param_count; ++i) {
            auto param = il2cpp_method_get_param(method, i);
            auto attrs = param->attrs;
            if (_il2cpp_type_is_byref(param)) {
                if (attrs & PARAM_ATTRIBUTE_OUT && !(attrs & PARAM_ATTRIBUTE_IN)) {
                    outPut << "out ";
                } else if (attrs & PARAM_ATTRIBUTE_IN && !(attrs & PARAM_ATTRIBUTE_OUT)) {
                    outPut << "in ";
                } else {
                    outPut << "ref ";
                }
            } else {
                if (attrs & PARAM_ATTRIBUTE_IN) {
                    outPut << "[In] ";
                }
                if (attrs & PARAM_ATTRIBUTE_OUT) {
                    outPut << "[Out] ";
                }
            }
            auto parameter_class = il2cpp_class_from_type(param);
            if (param->type == IL2CPP_TYPE_GENERICINST) outPut << GetFullType(param);
            else outPut << il2cpp_class_get_name(parameter_class);
            outPut << " " << il2cpp_method_get_param_name(method, i) << ", ";
        }
        if (param_count > 0) {
            outPut.seekp(-2, outPut.cur);
        }
        outPut << ") { }\n\n";
    }
    return outPut.str();
}

std::string dump_property(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Properties\n";
    void *iter = nullptr;
    while (auto prop_const = il2cpp_class_get_properties(klass, &iter)) {
        auto prop = const_cast<PropertyInfo *>(prop_const);
        auto get = il2cpp_property_get_get_method(prop);
        auto set = il2cpp_property_get_set_method(prop);
        auto prop_name = il2cpp_property_get_name(prop);
        outPut << "\t";
        Il2CppClass *prop_class = nullptr;
        uint32_t iflags = 0;
        if (get) {
            outPut << get_method_modifier(il2cpp_method_get_flags(get, &iflags));
            prop_class = il2cpp_class_from_type(il2cpp_method_get_return_type(get));
        } else if (set) {
            outPut << get_method_modifier(il2cpp_method_get_flags(set, &iflags));
            auto param = il2cpp_method_get_param(set, 0);
            prop_class = il2cpp_class_from_type(param);
        }
        if (prop_class) {
            const Il2CppType* skibidi = il2cpp_class_get_type(prop_class);
            outPut << GetFullType(skibidi) << " " << prop_name << " { ";            
            if (get) {
                outPut << "get; ";
            }
            if (set) {
                outPut << "set; ";
            }
            outPut << "}\n";
        } else {
            if (prop_name) {
                outPut << " // unknown property " << prop_name;
            }
        }
    }
    return outPut.str();
}

#define INIT_CONST_FIELD(type) \
        outPut << " = ";        \
        type data;               \
		il2cpp_field_static_get_value(field, &data)

#define FieldIs(typeName) FieldType == typeName

#define INIT_CONST_NUMBER_FIELD(type, typeName, stdtype) \
        if (FieldIs(typeName)) {                          \
            INIT_CONST_FIELD(type);                        \
            outPut << stdtype << data;                      \
        }

std::string dump_field(Il2CppClass *klass) {
    std::stringstream outPut;
    outPut << "\n\t// Fields\n";
    auto is_enum = il2cpp_class_is_enum(klass);
    void *iter = nullptr;
    while (auto field = il2cpp_class_get_fields(klass, &iter)) {
        outPut << "\t";
        auto attrs = il2cpp_field_get_flags(field);
        auto access = attrs & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK;
        switch (access) {
            case FIELD_ATTRIBUTE_PRIVATE:
                outPut << "private ";
                break;
            case FIELD_ATTRIBUTE_PUBLIC:
                outPut << "public ";
                break;
            case FIELD_ATTRIBUTE_FAMILY:
                outPut << "protected ";
                break;
            case FIELD_ATTRIBUTE_ASSEMBLY:
            case FIELD_ATTRIBUTE_FAM_AND_ASSEM:
                outPut << "internal ";
                break;
            case FIELD_ATTRIBUTE_FAM_OR_ASSEM:
                outPut << "protected internal ";
                break;
        }
        if (attrs & FIELD_ATTRIBUTE_LITERAL) {
            outPut << "const ";
        } else {
            if (attrs & FIELD_ATTRIBUTE_STATIC) {
                outPut << "static ";
            }
            if (attrs & FIELD_ATTRIBUTE_INIT_ONLY) {
                outPut << "readonly ";
            }
        }
        auto field_type = il2cpp_field_get_type(field);
        auto field_class = il2cpp_class_from_type(field_type);
        outPut << GetFullType(field_type) << " " << il2cpp_field_get_name(field);
        if (attrs & FIELD_ATTRIBUTE_LITERAL && is_enum) {
            uint64_t val = 0;
            il2cpp_field_static_get_value(field, &val);
            outPut << " = " << std::dec << val;
        }

        if (il2cpp_field_is_literal(field)) { // field_is_const
            std::string FieldType = Field_ReturnType(field);

            if (FieldIs("String")) {
                INIT_CONST_FIELD(il2cppString*);
                if (data != nullptr) {
                    std::string skibidi = data->ToString().c_str();
                    if (skibidi.size() == 1) {
                        outPut << "'" << skibidi.c_str() << "'";
                    }
                    else {
                        outPut << "\"" << skibidi.c_str() << "\"";
                    }
                }
            }

            if (FieldIs("Boolean")) {
                INIT_CONST_FIELD(bool);
                if (data) outPut << "true";
                else outPut << "false";
            }

            INIT_CONST_NUMBER_FIELD(int16_t, "Int16", std::dec)
            INIT_CONST_NUMBER_FIELD(int, "Int32", std::dec)
            INIT_CONST_NUMBER_FIELD(int64_t, "Int64", std::dec)

            INIT_CONST_NUMBER_FIELD(double, "Double", std::showpoint)
            INIT_CONST_NUMBER_FIELD(float, "Single", std::showpoint)

            INIT_CONST_NUMBER_FIELD(int16_t, "UInt16", std::dec)
            INIT_CONST_NUMBER_FIELD(uint32_t, "UInt32", std::dec)
            INIT_CONST_NUMBER_FIELD(int64_t, "UInt64", std::dec)
        }

        outPut << "; // 0x" << std::hex << il2cpp_field_get_offset(field) << "\n";
    }
    return outPut.str();
}

std::string dump_type(const Il2CppType *type) {
    std::stringstream outPut;
    
    auto *klass = il2cpp_class_from_type(type);
    std::string klassName = GetFullType(type);
    const char* klassNamespace = il2cpp_class_get_namespace(klass);

    outPut << "\n// Namespace: " << klassNamespace << "\n";
    auto flags = il2cpp_class_get_flags(klass);
    if (flags & TYPE_ATTRIBUTE_SERIALIZABLE) {
        outPut << "[Serializable]\n";
    }
    //TODO attribute
    auto is_valuetype = il2cpp_class_is_valuetype(klass);
    auto is_enum = il2cpp_class_is_enum(klass);
    auto visibility = flags & TYPE_ATTRIBUTE_VISIBILITY_MASK;
    switch (visibility) {
        case TYPE_ATTRIBUTE_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_PUBLIC:
            outPut << "public ";
            break;
        case TYPE_ATTRIBUTE_NOT_PUBLIC:
        case TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM:
        case TYPE_ATTRIBUTE_NESTED_ASSEMBLY:
            outPut << "internal ";
            break;
        case TYPE_ATTRIBUTE_NESTED_PRIVATE:
            outPut << "private ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAMILY:
            outPut << "protected ";
            break;
        case TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM:
            outPut << "protected internal ";
            break;
    }
    if (flags & TYPE_ATTRIBUTE_ABSTRACT && flags & TYPE_ATTRIBUTE_SEALED) {
        outPut << "static ";
    } else if (!(flags & TYPE_ATTRIBUTE_INTERFACE) && flags & TYPE_ATTRIBUTE_ABSTRACT) {
        outPut << "abstract ";
    } else if (!is_valuetype && !is_enum && flags & TYPE_ATTRIBUTE_SEALED) {
        outPut << "sealed ";
    }
    if (flags & TYPE_ATTRIBUTE_INTERFACE) {
        outPut << "interface ";
    } else if (is_enum) {
        outPut << "enum ";
    } else if (is_valuetype) {
        outPut << "struct ";
    } else {
        outPut << "class ";
    }
    outPut << klassName;
    std::vector<std::string> extends;
    auto parent = il2cpp_class_get_parent(klass);
    if (!is_valuetype && !is_enum && parent) {
        auto parent_type = il2cpp_class_get_type(parent);
        if (parent_type->type != IL2CPP_TYPE_OBJECT) {
            extends.emplace_back(GetFullType(parent_type));
        }
    }
    void *iter = nullptr;
    while (auto itf = il2cpp_class_get_interfaces(klass, &iter)) {
        extends.emplace_back(GetFullType(il2cpp_class_get_type(itf)));
    }
    if (!extends.empty()) {
        outPut << " : " << extends[0];
        for (int i = 1; i < extends.size(); ++i) {
            outPut << ", " << extends[i];
        }
    }
    outPut << "\n{";
    outPut << dump_field(klass);
    outPut << dump_property(klass);
    outPut << dump_method(klass, klassNamespace, klassName);
    //TODO EventInfo
    outPut << "}\n";
    return outPut.str();
}

void il2cpp_dump(void *handle, char *outDir, const char* il2cppModuleName) {

    while (get_module_base(il2cppModuleName) == nullptr)
    {
        LOGI("%s isn't initialized, waiting for 2 sec.", il2cppModuleName);
        Sleep(2000);
    }

    il2cpp_base = (uint64_t)get_module_base(il2cppModuleName);

    if (il2cpp_base) {
        LOGD("%s at %" PRIx64"", il2cppModuleName, il2cpp_base);
        LOGI("Loading...");
        Sleep(2000);
        init_il2cpp_api();
    } else {
        LOGE("Failed to get %s module.", il2cppModuleName);
        return;
    }

    auto domain = il2cpp_domain_get();
    il2cpp_thread_attach(domain);

    //start dump
    LOGI("dumping...");

    auto StartTimer = std::chrono::high_resolution_clock::now();

    size_t size;
    auto assemblies = il2cpp_domain_get_assemblies(domain, &size);
    std::stringstream imageOutput;
    for (int i = 0; i < size; ++i) {
        auto image = il2cpp_assembly_get_image(assemblies[i]);
        imageOutput << "// Image " << i << ": " << il2cpp_image_get_name(image) << "\n";
    }
    std::vector<std::string> outPuts;
    if (il2cpp_image_get_class) {
        LOGI("Version greater than 2018.3");
        for (int i = 0; i < size; ++i) {
            auto image = il2cpp_assembly_get_image(assemblies[i]);
            std::stringstream imageStr;
            imageStr << "\n// Dll : " << il2cpp_image_get_name(image);
            auto classCount = il2cpp_image_get_class_count(image);
            for (int j = 0; j < classCount; ++j) {
                auto klass = il2cpp_image_get_class(image, j);
                auto type = il2cpp_class_get_type(const_cast<Il2CppClass *>(klass));
                LOGD("type name : %s", il2cpp_type_get_name(type));
                auto outPut = imageStr.str() + dump_type(type);
                outPuts.push_back(outPut);
            }
        }
    } else {
        LOGI("Version less than 2018.3");
        //使用反射
        auto corlib = il2cpp_get_corlib();
        auto assemblyClass = il2cpp_class_from_name(corlib, "System.Reflection", "Assembly");
        auto assemblyLoad = il2cpp_class_get_method_from_name(assemblyClass, "Load", 1);
        auto assemblyGetTypes = il2cpp_class_get_method_from_name(assemblyClass, "GetTypes", 0);
        if (assemblyLoad && assemblyLoad->methodPointer) {
            LOGI("Assembly::Load: %p", assemblyLoad->methodPointer);
        } else {
            LOGI("miss Assembly::Load");
            return;
        }
        if (assemblyGetTypes && assemblyGetTypes->methodPointer) {
            LOGI("Assembly::GetTypes: %p", assemblyGetTypes->methodPointer);
        } else {
            LOGI("miss Assembly::GetTypes");
            return;
        }
        typedef void *(*Assembly_Load_ftn)(void *, Il2CppString *, void *);
        typedef Il2CppArray *(*Assembly_GetTypes_ftn)(void *, void *);
        for (int i = 0; i < size; ++i) {
            auto image = il2cpp_assembly_get_image(assemblies[i]);
            std::stringstream imageStr;
            auto image_name = il2cpp_image_get_name(image);
            imageStr << "\n// Dll : " << image_name;
            LOGD("image name : %s", image_name);
            auto imageName = std::string(image_name);
            auto pos = imageName.rfind('.');
            auto imageNameNoExt = imageName.substr(0, pos);
            auto assemblyFileName = il2cpp_string_new(imageNameNoExt.c_str());
            auto reflectionAssembly = ((Assembly_Load_ftn) assemblyLoad->methodPointer)(nullptr,
                                                                                        assemblyFileName,
                                                                                        nullptr);
            auto reflectionTypes = ((Assembly_GetTypes_ftn) assemblyGetTypes->methodPointer)(
                    reflectionAssembly, nullptr);
            auto items = reflectionTypes->vector;
            for (int j = 0; j < reflectionTypes->max_length; ++j) {
                auto klass = il2cpp_class_from_system_type((Il2CppReflectionType *) items[j]);
                auto type = il2cpp_class_get_type(klass);
                LOGD("type name : %s", il2cpp_type_get_name(type));
                auto outPut = imageStr.str() + dump_type(type);
                outPuts.push_back(outPut);
            }
        }
    }
    LOGI("write dump file");
    
    auto outPath = std::string(outDir).append("dump");
    if (FORMATED_FILE_NAME)
    {
        outPath.push_back('-');
        outPath.append(get_version_m()->ToString().c_str());
    }
    outPath.append(".cs");

    LOGI("File Name: %s", outPath.c_str());

    std::ofstream outStream(outPath);
    outStream << imageOutput.str();
    auto count = outPuts.size();
    for (int i = 0; i < count; ++i) {
        outStream << outPuts[i];
    }
    outStream.close();
    auto EndTimer = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> Timer = EndTimer - StartTimer;
    LOGI("done!\ndumping took %f seconds!", Timer.count());
}
