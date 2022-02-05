#ifndef LOX_RESOLVER_HPP
#define LOX_RESOLVER_HPP

#include "expr.hpp"
#include "interpreter.hpp"
#include "stmt.hpp"
#include "string_map.hpp"

#include <vector>

namespace lox
{
	enum struct function_type
	{
		NONE,
		FUNCTION,
		INIT,
		METHOD,
	};

	enum struct class_type
	{
		NONE,
		CLASS,
		SUBCLASS,
	};

	struct resolver
	{
		lox::interpreter &interpreter;
		std::vector<lox::string_map<char8_t, bool>> scopes;
		lox::function_type current_function;
		lox::class_type current_class;

		inline resolver(lox::interpreter &i)
		: interpreter(i),
		  scopes(),
		  current_function(function_type::NONE),
		  current_class(lox::class_type::NONE)
		{
		}

		lak::err_t<> error(
		  const lox::token &token,
		  lak::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		lak::result<> resolve(lak::span<const lox::stmt_ptr> statements);

		lak::result<> resolve_function(const lox::stmt::function_ptr &func,
		                               lox::function_type type);

		template<typename T>
		void resolve_local(const T &expr, const lox::token &name);

		lak::result<> declare(const lox::token &name);
		void define(const lox::token &name);

		lak::result<> operator()(const lox::expr::assign &expr);
		lak::result<> operator()(const lox::expr::binary &expr);
		lak::result<> operator()(const lox::expr::call &expr);
		lak::result<> operator()(const lox::expr::get &expr);
		lak::result<> operator()(const lox::expr::grouping &expr);
		lak::result<> operator()(const lox::expr::literal &expr);
		lak::result<> operator()(const lox::expr::logical &expr);
		lak::result<> operator()(const lox::expr::set &expr);
		lak::result<> operator()(const lox::expr::super_keyword &expr);
		lak::result<> operator()(const lox::expr::this_keyword &expr);
		lak::result<> operator()(const lox::expr::unary &expr);
		lak::result<> operator()(const lox::expr::variable &expr);

		lak::result<> operator()(const lox::stmt::block &stmt);
		lak::result<> operator()(const lox::stmt::type &stmt);
		lak::result<> operator()(const lox::stmt::expr &stmt);
		lak::result<> operator()(const lox::stmt::branch &stmt);
		lak::result<> operator()(const lox::stmt::print &stmt);
		lak::result<> operator()(const lox::stmt::var &stmt);
		lak::result<> operator()(const lox::stmt::loop &stmt);
		lak::result<> operator()(const lox::stmt::function_ptr &stmt);
		lak::result<> operator()(const lox::stmt::ret &stmt);
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
