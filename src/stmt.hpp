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
			stmt_ptr then_brach;
			stmt_ptr else_brach;
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

		std::variant<block, expr, branch, print, var, loop> value;

		static stmt_ptr make_block(block &&stmt);
		static stmt_ptr make_expr(expr &&stmt);
		static stmt_ptr make_branch(branch &&stmt);
		static stmt_ptr make_print(print &&stmt);
		static stmt_ptr make_var(var &&stmt);
		static stmt_ptr make_loop(loop &&stmt);

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
