//
// Created by tang on 5/26/19.
//

#include "scope_guard.h"

ScopeGuard::ScopeGuard(std::function<void(void)> &&f) {
    onScopeExit = f;
}

ScopeGuard::~ScopeGuard() {
    onScopeExit();
}

