#ifndef LOX_STMT_HPP
#define LOX_STMT_HPP

#include "expr.hpp"
#include "token.hpp"

#include <memory>
#include <vector>

namespace lox
{
	struct stmt
	{
		using stmt_ptr = std::unique_ptr<lox::stmt>;

		struct block
		{
			std::vector<stmt_ptr> statements;
		};

		struct expr
		{
			lox::expr_ptr expression;
		};

		struct branch
		{
			lox::expr_ptr condition;
			stmt_ptr then_branch;
			stmt_ptr else_branch;
		};

		struct print
		{
			lox::expr_ptr expression;
		};

		struct var
		{
			lox::token name;
			lox::expr_ptr init;
		};

		struct loop
		{
			lox::expr_ptr condition;
			stmt_ptr body;
		};

		struct function
		{
			lox::token name;
			std::vector<lox::token> parameters;
			std::vector<stmt_ptr> body;
		};
		using function_ptr = std::shared_ptr<function>;

		struct klass
		{
			lox::token name;
			lox::expr_ptr superclass;
			std::vector<lox::stmt::function_ptr> methods;
		};

		struct ret
		{
			lox::token keyword;
			lox::expr_ptr value;
		};

		using value_type = std::
		  variant<block, klass, expr, branch, print, var, loop, function_ptr, ret>;
		value_type value;

		static stmt_ptr make_block(block &&stmt);
		static stmt_ptr make_klass(klass &&stmt);
		static stmt_ptr make_expr(expr &&stmt);
		static stmt_ptr make_branch(branch &&stmt);
		static stmt_ptr make_print(print &&stmt);
		static stmt_ptr make_var(var &&stmt);
		static stmt_ptr make_loop(loop &&stmt);
		static stmt_ptr make_function(function &&stmt);
		static stmt_ptr make_ret(ret &&stmt);

		template<typename F>
		inline auto visit(F &&f)
		{
			return std::visit(f, value);
		}

		template<typename F>
		inline auto visit(F &&f) const
		{
			return std::visit(f, value);
		}
	};

	using stmt_ptr = stmt::stmt_ptr;
}

#endif
