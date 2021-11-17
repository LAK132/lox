#ifndef LOX_RESOLVER_HPP
#define LOX_RESOLVER_HPP

#include "expr.hpp"
#include "interpreter.hpp"
#include "stmt.hpp"
#include "string_map.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace lox
{
	enum struct function_type
	{
		NONE,
		FUNCTION,
	};

	struct resolver
	{
		lox::interpreter &interpreter;
		std::vector<lox::string_map<char8_t, bool>> scopes;
		lox::function_type current_function;

		inline resolver(lox::interpreter &i)
		: interpreter(i), scopes(), current_function(function_type::NONE)
		{
		}

		std::nullopt_t error(const lox::token &token, std::u8string_view message);

		std::optional<std::monostate> resolve(
		  std::span<const lox::stmt_ptr> statements);

		std::optional<std::monostate> resolve_function(
		  const lox::stmt::function_ptr &func, lox::function_type type);

		template<typename T>
		void resolve_local(const T &expr, const lox::token &name);

		std::optional<std::monostate> declare(const lox::token &name);
		void define(const lox::token &name);

		std::optional<std::monostate> operator()(const lox::expr::assign &expr);
		std::optional<std::monostate> operator()(const lox::expr::binary &expr);
		std::optional<std::monostate> operator()(const lox::expr::call &expr);
		std::optional<std::monostate> operator()(const lox::expr::grouping &expr);
		std::optional<std::monostate> operator()(const lox::expr::literal &expr);
		std::optional<std::monostate> operator()(const lox::expr::logical &expr);
		std::optional<std::monostate> operator()(const lox::expr::unary &expr);
		std::optional<std::monostate> operator()(const lox::expr::variable &expr);

		std::optional<std::monostate> operator()(const lox::stmt::block &stmt);
		std::optional<std::monostate> operator()(const lox::stmt::expr &stmt);
		std::optional<std::monostate> operator()(const lox::stmt::branch &stmt);
		std::optional<std::monostate> operator()(const lox::stmt::print &stmt);
		std::optional<std::monostate> operator()(const lox::stmt::var &stmt);
		std::optional<std::monostate> operator()(const lox::stmt::loop &stmt);
		std::optional<std::monostate> operator()(
		  const lox::stmt::function_ptr &stmt);
		std::optional<std::monostate> operator()(const lox::stmt::ret &stmt);
	};
}

template<typename T>
void lox::resolver::resolve_local(const T &expr, const lox::token &name)
{
	for (size_t i = scopes.size(); i-- > 0U;)
		if (auto &scope = scopes[i]; scope.find(name.lexeme) != scope.end())
			return interpreter.resolve(expr, (scopes.size() - 1U) - i);
}

#endif
