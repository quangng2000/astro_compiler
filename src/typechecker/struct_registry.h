#pragma once

#include "ast/type_nodes.h"
#include "type_env.h"
#include <string>
#include <unordered_map>
#include <vector>

struct FieldInfo {
    std::string name;
    TypeRef type;
    bool is_pub;
};

struct MethodInfo {
    std::string name;
    FnSignature signature;
    bool is_pub;
    bool has_self;
};

struct StructInfo {
    std::string name;
    std::vector<FieldInfo> fields;
    std::vector<MethodInfo> methods;
};

class StructRegistry {
public:
    void register_struct(const std::string& name, StructInfo info) {
        structs_[name] = std::move(info);
    }

    const StructInfo* lookup(const std::string& name) const {
        auto it = structs_.find(name);
        return it != structs_.end() ? &it->second : nullptr;
    }

    const FieldInfo* lookup_field(const std::string& sname,
                                  const std::string& fname) const {
        auto* s = lookup(sname);
        if (!s) return nullptr;
        for (auto& f : s->fields) if (f.name == fname) return &f;
        return nullptr;
    }

    const MethodInfo* lookup_method(const std::string& sname,
                                    const std::string& mname) const {
        auto* s = lookup(sname);
        if (!s) return nullptr;
        for (auto& m : s->methods) if (m.name == mname) return &m;
        return nullptr;
    }

private:
    std::unordered_map<std::string, StructInfo> structs_;
};
