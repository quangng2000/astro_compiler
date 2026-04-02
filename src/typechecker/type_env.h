#pragma once

#include "ast/type_nodes.h"
#include <string>
#include <unordered_map>
#include <vector>

struct FnSignature {
    std::vector<AstType> param_types;
    AstType return_type;
};

class TypeEnv {
public:
    explicit TypeEnv(TypeEnv* parent = nullptr) : parent_(parent) {}

    void define(const std::string& name, AstType type) {
        vars_[name] = type;
    }

    AstType lookup(const std::string& name) const {
        auto it = vars_.find(name);
        if (it != vars_.end()) return it->second;
        if (parent_) return parent_->lookup(name);
        return AstType::Unknown;
    }

    void define_fn(const std::string& name, FnSignature sig) {
        fns_[name] = std::move(sig);
    }

    const FnSignature* lookup_fn(const std::string& name) const {
        auto it = fns_.find(name);
        if (it != fns_.end()) return &it->second;
        if (parent_) return parent_->lookup_fn(name);
        return nullptr;
    }

private:
    TypeEnv* parent_;
    std::unordered_map<std::string, AstType> vars_;
    std::unordered_map<std::string, FnSignature> fns_;
};
