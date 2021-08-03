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

		struct print
		{
			lox::expr_ptr expression;
		};

		struct var
		{
			lox::token name;
			lox::expr_ptr init;
		};

		std::variant<block, expr, print, var> value;

		static stmt_ptr make_block(block &&stmt);
		static stmt_ptr make_expr(expr &&stmt);
		static stmt_ptr make_print(print &&stmt);
		static stmt_ptr make_var(var &&stmt);

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
