#ifndef LOX_CALLABLE_HPP
#define LOX_CALLABLE_HPP

#include "interpreter.hpp"
#include "object.hpp"
#include "stmt.hpp"

#include <lak/memory.hpp>
#include <lak/span.hpp>
#include <lak/string.hpp>
#include <lak/string_view.hpp>
#include <lak/tuple.hpp>

#include <vector>

namespace lox
{
	struct type;

	struct callable
	{
	private:
		struct impl;
		using impl_ptr = lak::shared_ref<impl>;

		impl_ptr _impl;

		using native_function_ptr_t = lak::result<lox::object> (*)(
		  lox::interpreter &, std::vector<lox::object> &&);

	public:
		callable() = delete;

		// native
		callable(native_function_ptr_t function, size_t arity);

		// interpreted
		callable(lox::stmt::function_ptr function,
		         lox::environment_ptr closure,
		         bool is_init);

		// constructor
		callable(const lox::type &type);

		[[nodiscard]] callable with_binds(
		  std::initializer_list<lak::pair<lak::u8string_view, lox::object>> binds)
		  const;

		[[nodiscard]] callable with_binds(
		  lak::span<const lak::pair<lak::u8string_view, lox::object>> binds) const;

		size_t arity() const;

		lak::u8string to_string() const;

		bool operator==(const callable &rhs) const;

		lak::result<lox::object> operator()(
		  lox::interpreter &interpreter,
		  std::vector<lox::object> &&arguments) const;
	};

	template<typename FUNC>
	struct function_signature;

	template<typename R, typename... ARGS>
	struct function_signature<R (*)(ARGS...)>
	{
		using return_type                      = R;
		using arguments                        = lak::tuple<ARGS...>;
		static constexpr size_t argument_count = sizeof...(ARGS);
	};

	template<typename R, typename... ARGS>
	struct function_signature<R(ARGS...)>
	{
		using return_type                      = R;
		using arguments                        = lak::tuple<ARGS...>;
		static constexpr size_t argument_count = sizeof...(ARGS);
	};

	template<typename FUNC>
	using function_return_t = typename function_signature<FUNC>::return_type;

	template<typename FUNC>
	using function_arguments_t = typename function_signature<FUNC>::arguments;

	template<typename FUNC>
	inline constexpr size_t function_argument_count_v =
	  function_signature<FUNC>::argument_count;

	template<typename... ARGS>
	using callable_function_ptr_t =
	  lak::result<lox::object> (*)(lox::interpreter &, ARGS...);

	template<typename... ARGS, size_t... I>
	inline lak::result<lox::object> call_native(
	  callable_function_ptr_t<ARGS...> function,
	  lox::interpreter &interpreter,
	  std::vector<lox::object> &&arguments,
	  lak::index_sequence<I...>)
	{
		static_assert(sizeof...(ARGS) == sizeof...(I));
		ASSERT_EQUAL(sizeof...(ARGS), arguments.size());
		return function(interpreter, lak::move(arguments[I])...);
	}
}

#define LOX_CALLABLE_MAKE_NATIVE(...)                                         \
	::lox::callable(                                                            \
	  [](::lox::interpreter &interpreter,                                       \
	     ::std::vector<::lox::object> &&arguments)                              \
	  {                                                                         \
			using arguments_t = ::lox::function_arguments_t<decltype(__VA_ARGS__)>; \
			constexpr size_t argument_count =                                       \
			  ::lox::function_argument_count_v<decltype(__VA_ARGS__)>;              \
			static_assert(argument_count >= 1, "Requires at least 1 argument");     \
			static_assert(                                                          \
			  ::lak::is_same_v<                                                     \
			    ::lak::remove_cvref_t<::lak::tuple_element_t<0, arguments_t>>,      \
			    ::lox::interpreter>,                                                \
			  "First argument must be an interpreter");                             \
			return ::lox::call_native(                                              \
			  (__VA_ARGS__),                                                        \
			  interpreter,                                                          \
			  ::lak::move(arguments),                                               \
			  ::lak::make_index_sequence<argument_count - 1>{});                    \
	  },                                                                        \
	  ::lox::function_argument_count_v<decltype(__VA_ARGS__)> - 1)

#endif
