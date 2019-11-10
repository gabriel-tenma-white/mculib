#pragma once
#include <type_traits>
#include <utility>
#include <assert.h>
#include <new>

// an implementation of std::function using no dynamic memory allocations.
// the size of the stored functor object must be no larger than the size of
// two pointers. For example, a lambda function that captures two pointers
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



// denotes an empty functor
static void* NULLSTATE = (void*)-1;

// set before setting functor, so that a size 0 functor
// is still recognized as non-null
static void* DEFAULTSTATE = (void*)1;

template<class R, class... Args>
class small_function<R(Args...)>
{
	// allocate enough space for an object with the size of 3 pointers.
	// one of the pointers is the vtable pointer.
	void* _state[3];


public:
    small_function() { _state[0] = NULLSTATE; };

    template<class Callable, class = decltype(R(std::declval<typename std::decay<Callable>::type>()(std::declval<Args>()...)))>
    small_function(Callable&& t) {
		using StateType = small_function_state<typename std::decay<Callable>::type, R, Args...>;
		static_assert(sizeof(Callable) <= (sizeof(void*) * 2),
						"functor too large for small_function (up to two pointers' size allowed)");
		static_assert(sizeof(StateType) <= sizeof(_state),
						"internal error: small_function_state is too large!");
		_state[0] = DEFAULTSTATE;
		new ((void*) _state) StateType(static_cast<Callable&&>(t));
	}

    ~small_function() {
		if(*this) ptr()->destruct();
    }

    small_function(small_function& rhs) { rhs.clone((void*) _state); }
    small_function(const small_function& rhs) { rhs.clone((void*) _state); }
    small_function(small_function&& rhs) noexcept {
		_state[0] = rhs._state[0];
		_state[1] = rhs._state[1];
		_state[2] = rhs._state[2];
		rhs._state[0] = NULLSTATE;
	}
    small_function(const small_function&& rhs) = delete;

    void operator=(small_function rhs) noexcept {
        std::swap(_state[0], rhs._state[0]);
        std::swap(_state[1], rhs._state[1]);
        std::swap(_state[2], rhs._state[2]);
    }

    inline R operator()(Args... args) const {
        return ptr()->call(std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept {
		// assume first member of the state object is the vtable pointer, which should not equal NULLSTATE
        return _state[0] != NULLSTATE;
    }
    inline small_function_state_base<R, Args...>* ptr() const {
		return (small_function_state_base<R, Args...>*) _state;
	}
    void clone(void* buf) const {
		if(*this) ptr()->clone(buf);
	}
};

