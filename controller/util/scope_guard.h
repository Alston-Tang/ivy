//
// Created by tang on 5/26/19.
//

#ifndef IVY_SCOPE_GUARD_H
#define IVY_SCOPE_GUARD_H

#include <functional>


class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void(void)> &&f);
    ~ScopeGuard();

private:
    std::function<void(void)> onScopeExit;
};


#endif //IVY_SCOPE_GUARD_H
