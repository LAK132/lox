#ifndef LOX_EVALUATOR_HPP
#define LOX_EVALUATOR_HPP

#include "environment.hpp"
#include "expr.hpp"
#include "interpreter.hpp"
#include "object.hpp"
#include "stmt.hpp"

#include <lak/optional.hpp>
#include <lak/span.hpp>
#include <lak/string.hpp>

#include <source_location>

namespace lox
{
	struct evaluator
	{
		lox::interpreter &interpreter;
		lox::environment_ptr environment;
		lak::optional<lox::object> block_ret_value;

		inline evaluator(lox::interpreter &i)
		: interpreter(i),
		  environment(i.global_environment),
		  block_ret_value(lak::nullopt)
		{
		}

		lak::err_t<> error(
		  const lox::token &token,
		  lak::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		lak::result<lak::u8string> execute_block(
		  lak::span<const lox::stmt_ptr> statements,
		  const lox::environment_ptr &env);

		template<typename T>
		lak::result<lox::object> find_variable(const lox::token &name,
		                                       const T &expr);

		lak::result<lox::object> operator()(const lox::expr::assign &expr);
		lak::result<lox::object> operator()(const lox::expr::binary &expr);
		lak::result<lox::object> operator()(const lox::expr::call &expr);
		lak::result<lox::object> operator()(const lox::expr::get &expr);
		lak::result<lox::object> operator()(const lox::expr::grouping &expr);
		lak::result<lox::object> operator()(const lox::expr::literal &expr);
		lak::result<lox::object> operator()(const lox::expr::logical &expr);
		lak::result<lox::object> operator()(const lox::expr::set &expr);
		lak::result<lox::object> operator()(const lox::expr::super_keyword &expr);
		lak::result<lox::object> operator()(const lox::expr::this_keyword &expr);
		lak::result<lox::object> operator()(const lox::expr::unary &expr);
		lak::result<lox::object> operator()(const lox::expr::variable &expr);

		lak::result<lak::u8string> operator()(const lox::stmt::block &stmt);
		lak::result<lak::u8string> operator()(const lox::stmt::type &stmt);
		lak::result<lak::u8string> operator()(const lox::stmt::expr &stmt);
		lak::result<lak::u8string> operator()(const lox::stmt::branch &stmt);
		lak::result<lak::u8string> operator()(const lox::stmt::print &stmt);
		lak::result<lak::u8string> operator()(const lox::stmt::var &stmt);
		lak::result<lak::u8string> operator()(const lox::stmt::loop &stmt);
		lak::result<lak::u8string> operator()(const lox::stmt::function_ptr &stmt);
		lak::result<lak::u8string> operator()(const lox::stmt::ret &stmt);
	};
}

template<typename T>
lak::result<lox::object> lox::evaluator::find_variable(const lox::token &name,
                                                       const T &expr)
{
	return interpreter.find(expr).visit(lak::overloaded{
	  [&](size_t distance) -> lak::result<lox::object>
	  {
		  return lak::copy_result_from_pointer(environment->find(name, distance))
		    .or_else(
		      [&](auto &&) -> lak::result<lox::object>
		      {
			      return error(name,
			                   u8"Undefined local variable '"_str +
			                     name.lexeme.to_string() + u8"'.");
		      });
	  },
	  [&](lak::monostate) -> lak::result<lox::object>
	  {
		  return lak::copy_result_from_pointer(
		           interpreter.global_environment->find(name))
		    .or_else(
		      [&](auto &&) -> lak::result<lox::object>
		      {
			      return error(name,
			                   u8"Undefined global variable '"_str +
			                     name.lexeme.to_string() + u8"'.");
		      });
	  },
	});
}

#endif
