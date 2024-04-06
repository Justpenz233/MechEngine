//
// Created by MarvelLi on 2024/3/8.
//

#pragma once
#include <type_traits>


template <class T>
concept IsObject = std::is_base_of_v<Object, T>;

template <class T, class... Args>
concept ObjectConstructBy = IsObject<T> && std::constructible_from<T, Args...>;


/**
 * Get the name of a template type, only support GCC, Clang and MSVC
 * @see https://stackoverflow.com/questions/35941045/can-i-obtain-c-type-names-in-a-constexpr-way
 */
template<typename T>
struct TypeName {
	constexpr static std::string_view fullname_intern() {
		#if defined(__clang__) || defined(__GNUC__)
		return __PRETTY_FUNCTION__;
		#elif defined(_MSC_VER)
		return __FUNCSIG__;
		#else
		#error "Unsupported compiler"
		#endif
	}
	constexpr static std::string_view name() {
		size_t prefix_len = TypeName<void>::fullname_intern().find("void");
		size_t multiple   = TypeName<void>::fullname_intern().size() - TypeName<int>::fullname_intern().size();
		size_t dummy_len  = TypeName<void>::fullname_intern().size() - 4*multiple;
		size_t target_len = (fullname_intern().size() - dummy_len)/multiple;
		std::string_view rv = fullname_intern().substr(prefix_len, target_len);
		if (rv.rfind(' ') == rv.npos)
			return rv;
		return rv.substr(rv.rfind(' ')+1);
	}

	using type = T;
	constexpr static std::string_view value = name();
};
