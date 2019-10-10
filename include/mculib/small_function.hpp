#pragma once
#include <type_traits>
#include <utility>
#include <assert.h>
#include <new>

// an implementation of std::function using no dynamic memory allocations.
// the size of the stored functor object must be no larger than the size of
// one pointer. For example, a lambda function that captures a single pointer
// can be used with small_function.

template<class Sig>
class small_function;

template<class R, class... Args>
struct small_function_state_base {
    virtual R call(Args...) const = 0;
    virtual void clone(void*) const = 0;
    virtual void destruct() = 0;
    ~small_function_state_base() {}
};

template<class T, class R, class... Args>
struct small_function_state : small_function_state_base<R, Args...> {
    using Self = small_function_state<T, R, Args...>;
    static void operator delete(void* ptr) = delete;
    T t_;
    explicit small_function_state(T t) : t_(std::move(t)) {}
    R call(Args... args) const override {
        return const_cast<T&>(t_)(std::forward<Args>(args)...);
    }
    void clone(void* buf) const override {
        new (buf) Self(*this);
    }
    virtual void destruct() {
		t_.~T();
	}
};



template<class R, class... Args>
class small_function<R(Args...)>
{
	// allocate enough space for an object with the size of 2 pointers.
	// one of the pointers is the vtable pointer.
    void* _state[2];
public:
    small_function() { _state[0] = nullptr; };

    template<class Callable, class = decltype(R(std::declval<typename std::decay<Callable>::type>()(std::declval<Args>()...)))>
    small_function(Callable&& t) {
		static_assert(sizeof(Callable) <= sizeof(void*),
						"functor too large for small_function (up to one pointer's size allowed)");
		new ((void*) _state) small_function_state<typename std::decay<Callable>::type, R, Args...>(static_cast<Callable&&>(t));
	}

    ~small_function() {
		if(*this) ptr()->destruct();
    }

    small_function(small_function& rhs) { rhs.clone((void*) _state); }
    small_function(const small_function& rhs) { rhs.clone((void*) _state); }
    small_function(small_function&& rhs) noexcept {
		_state[0] = rhs._state[0]; 
		_state[1] = rhs._state[1];
		rhs._state[0] = nullptr;
	}
    small_function(const small_function&& rhs) = delete;

    void operator=(small_function rhs) noexcept {
        std::swap(_state[0], rhs._state[0]);
        std::swap(_state[1], rhs._state[1]);
    }

    inline R operator()(Args... args) const {
        return ptr()->call(std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept {
		// assume first member of the state object is the vtable pointer, which is non-null
        return _state[0] != nullptr;
    }
    inline small_function_state_base<R, Args...>* ptr() const {
		return (small_function_state_base<R, Args...>*) _state;
	}
    void clone(void* buf) const {
		if(*this) ptr()->clone(buf);
	}
};

