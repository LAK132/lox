#ifndef LOX_STMT_HPP
#define LOX_STMT_HPP

#include "expr.hpp"
#include "token.hpp"

#include <lak/memory.hpp>
#include <lak/optional.hpp>
#include <lak/visit.hpp>

#include <vector>

namespace lox
{
	struct stmt;

	using stmt_ptr = lak::unique_ref<lox::stmt>;

	struct stmt
	{
		struct block
		{
			std::vector<lox::stmt_ptr> statements;
		};

		struct expr
		{
			lox::expr_ptr expression;
		};

		struct branch
		{
			lox::expr_ptr condition;
			lox::stmt_ptr then_branch;
			lak::optional<lox::stmt_ptr> else_branch;
		};

		struct print
		{
			lox::expr_ptr expression;
		};

		struct var
		{
			lox::token name;
			lak::optional<lox::expr_ptr> init;
		};

		struct loop
		{
			lox::expr_ptr condition;
			lox::stmt_ptr body;
		};

		struct function
		{
			lox::token name;
			std::vector<lox::token> parameters;
			std::vector<lox::stmt_ptr> body;
		};
		using function_ptr = lak::shared_ref<function>;

		struct type
		{
			lox::token name;
			lak::optional<lox::expr::variable> superclass;
			std::vector<lox::stmt::function_ptr> methods;
		};

		struct ret
		{
			lox::token keyword;
			lak::optional<lox::expr_ptr> value;
		};

		using value_type = lak::
		  variant<block, type, expr, branch, print, var, loop, function_ptr, ret>;

		value_type value;

		static lox::stmt_ptr make_block(block &&stmt);
		static lox::stmt_ptr make_type(type &&stmt);
		static lox::stmt_ptr make_expr(expr &&stmt);
		static lox::stmt_ptr make_branch(branch &&stmt);
		static lox::stmt_ptr make_print(print &&stmt);
		static lox::stmt_ptr make_var(var &&stmt);
		static lox::stmt_ptr make_loop(loop &&stmt);
		static function_ptr make_function_ptr(function &&stmt);
		static lox::stmt_ptr make_function_from_ptr(function_ptr ptr);
		static lox::stmt_ptr make_function(function &&stmt);
		static lox::stmt_ptr make_ret(ret &&stmt);

		template<typename F>
		inline auto visit(F &&f)
		{
			return lak::visit(value, f);
		}

		template<typename F>
		inline auto visit(F &&f) const
		{
			return lak::visit(value, f);
		}
	};
}

#endif
