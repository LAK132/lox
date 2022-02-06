#ifndef LOX_ERROR_HPP
#define LOX_ERROR_HPP

#include <lak/result.hpp>
#include <lak/stdint.hpp>
#include <lak/string.hpp>
#include <lak/string_literals.hpp>

#include "token.hpp"

namespace lox
{
	/* --- positional_error --- */

	template<typename TAG>
	struct positional_error
	{
		size_t line;
		lak::u8string where;
		lak::u8string message;

		static positional_error at(size_t line, lak::u8string msg)
		{
			return positional_error{
			  .line = line, .where{}, .message = lak::move(msg)};
		}

		static positional_error at(const lox::token &token, lak::u8string msg)
		{
			positional_error result;
			result.line = token.line;
			if (token.type == lox::token_type::EOF_TOK)
				result.where = u8" at end"_str;
			else if (token.type != lox::token_type::ERROR_TOK)
				result.where = u8" at '"_str + lak::u8string(token.lexeme) + u8"'";
			result.message = lak::move(msg);
			return result;
		}
	};

	template<typename TAG>
	lak::u8string to_string(const lox::positional_error<TAG> &err)
	{
		return u8"[line "_str +
		       lak::as_u8string(std::to_string(err.line)).to_string() +
		       u8"] Error"_str + err.where + u8": "_str + err.message;
	}

	/* --- result_set --- */

	template<typename... T>
	struct result_set
	{
		using value_type = lak::variant<T...>;
		value_type value;

		result_set() = default;

		template<lak::concepts::one_of<T...> U>
		result_set(const U &val) : value(lak::forward<U>(val))
		{
		}

		template<lak::concepts::one_of<T...> U>
		result_set(U &&val) : value(lak::forward<U>(val))
		{
		}

		template<lak::concepts::one_of<T...>... U>
		result_set(const result_set<U...> &other)
		: value(other.visit([]<typename V>(V &&v) -> value_type
		                    { return value_type{lak::forward<V>(v)}; }))
		{
		}

		template<lak::concepts::one_of<T...>... U>
		result_set(result_set<U...> &&other)
		: value(lak::move(other).visit([]<typename V>(V &&v) -> value_type
		                               { return value_type{lak::forward<V>(v)}; }))
		{
		}

		template<typename F>
		auto visit(F &&f) &
		{
			return lak::visit(value, lak::forward<F>(f));
		}

		template<typename F>
		auto visit(F &&f) const &
		{
			return lak::visit(value, lak::forward<F>(f));
		}

		template<typename F>
		auto visit(F &&f) &&
		{
			return lak::visit(lak::move(value), lak::forward<F>(f));
		}
	};

}

#endif
