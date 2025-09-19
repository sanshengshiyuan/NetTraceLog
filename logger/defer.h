#pragma once

#include <functional>

namespace logger {
class ExecuteOnScopeExit {
public:
    ExecuteOnScopeExit() = default;

    ExecuteOnScopeExit(ExecuteOnScopeExit&& ) = default;

    ExecuteOnScopeExit& operator = (ExecuteOnScopeExit&& ) = default;

    ExecuteOnScopeExit(const ExecuteOnScopeExit& ) = delete;

    ExecuteOnScopeExit& operator = (const ExecuteOnScopeExit& ) = delete;

    template <typename F, typename... Args> 
    ExecuteOnScopeExit(F&& f, Args&&... args) {
        func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    }

    ~ExecuteOnScopeExit() {
        if (func_) {
            func_();
        }
    }
private:
    std::function<void()> func_;
    
};
}

// 宏连接工具
#define CONCAT(x, y) x##y

#define DEFER(lambda) logger::ExecuteOnScopeExit CONCAT(defer_, __LINE__) = lambda