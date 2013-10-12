#ifndef CALLBACK_HPP_
#define CALLBACK_HPP_

#include <utility>

namespace CB {

template<typename Signature>
class Callback;

template<typename R, typename ... ARGS>
class Callback<R(ARGS...)> {
public:

	typedef R (sig)(ARGS...);
	typedef R (*function)(void *, ARGS...);

	Callback() :
			m_obj(0),
			m_func(0) {
	}

	Callback(void *obj, function func) :
			m_obj(obj),
			m_func(func) {
	}

	Callback(const Callback &o) :
			m_obj(o.m_obj),
			m_func(o.m_func) {
	}

	Callback& operator=(const Callback &o) {
		m_obj = o.m_obj;
		m_func = o.m_func;
		return *this;
	}

	explicit operator bool() {
		return m_func != 0 ? m_obj : 0;
	}

	R operator()(ARGS ... args) {
		return m_func(m_obj, args...);
	}

private:

	void *m_obj;
	function m_func;
};

namespace Factory {

template<class T, typename R, typename ... ARGS>
struct MemberFunc {
	template<R (T::*Func)(ARGS...)>
	static inline Callback<R(ARGS...)> bind(T *t) {
		struct _ {
			static R wrapper(void *obj, ARGS ... args) {
				return (static_cast<T*>(obj)->*Func)(
						std::forward<ARGS>(args)...);
			}
		};
		return Callback<R(ARGS...)>(t, _::wrapper);
	}
};

template<class T, typename R, typename ... ARGS>
struct ConstMemberFunc {
	template<R (T::*Func)(ARGS...)>
	static inline Callback<R(ARGS...)> bind(T *t) {
		struct _ {
			static R wrapper(void *obj, ARGS ... args) {
				return (static_cast<T*>(obj)->*Func)(
						std::forward<ARGS>(args)...);
			}
		};
		return Callback<R(ARGS...)>(t, _::wrapper);
	}
};

template<typename R, typename ... ARGS>
struct FreeFunc {
	template<R (*Func)(ARGS...)>
	static inline Callback<R(ARGS...)> bind() {
		struct _ {
			static R wrapper(void *obj, ARGS ... args) {
				return Func(std::forward<ARGS>(args)...);
			}
		};
		return Callback<R(ARGS...)>(nullptr, _::wrapper);
	}
};

template<class T, typename R, typename ... ARGS>
inline MemberFunc<T, R, ARGS...> get(R (T::*)(ARGS...)) {
	return MemberFunc<T, R, ARGS...>();
}

template<class T, typename R, typename ... ARGS>
inline ConstMemberFunc<T, R, ARGS...> get(R (T::*)(ARGS...) const) {
	return ConstMemberFunc<T, R, ARGS...>();
}

template<typename R, typename ... ARGS>
inline FreeFunc<R, ARGS...> get(R (*)(ARGS...)) {
	return FreeFunc<R, ARGS...>();
}

}

}

#define CD_BIND_MEM_IMPL(instance, func) \
(CB::Factory::get(func).template bind<func>(instance))
#define CB_BIND_MEM CD_BIND_MEM_IMPL

#define CD_BIND_FREE_IMPL(func) \
(CB::Factory::get(func).template bind<func>())
#define CB_BIND_FREE CD_BIND_FREE_IMPL

#endif /* CALLBACK_HPP_ */
