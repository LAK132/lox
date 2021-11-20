#ifndef LOX_EXPR_HPP
#define LOX_EXPR_HPP

#include "object.hpp"
#include "token.hpp"

#include <memory>
#include <variant>
#include <vector>

namespace lox
{
	struct expr
	{
		using expr_ptr = std::unique_ptr<lox::expr>;

		struct assign
		{
			lox::token name;
			expr_ptr value;
		};

		struct binary
		{
			expr_ptr left;
			lox::token op;
			expr_ptr right;
		};

		struct call
		{
			expr_ptr callee;
			lox::token paren;
			std::vector<expr_ptr> arguments;
		};

		struct get
		{
			expr_ptr object;
			lox::token name;
		};

		struct grouping
		{
			expr_ptr expression;
		};

		struct literal
		{
			lox::object value;
		};

		struct logical
		{
			expr_ptr left;
			lox::token op;
			expr_ptr right;
		};

		struct set
		{
			expr_ptr object;
			lox::token name;
			expr_ptr value;
		};

		struct super_keyword
		{
			lox::token keyword;
			lox::token method;
		};

		struct this_keyword
		{
			lox::token keyword;
		};

		struct unary
		{
			lox::token op;
			expr_ptr right;
		};

		struct variable
		{
			lox::token name;
		};

		std::variant<assign,
		             binary,
		             call,
		             get,
		             grouping,
		             literal,
		             logical,
		             set,
		             super_keyword,
		             this_keyword,
		             unary,
		             variable>
		  value;

		static expr_ptr make_assign(assign &&expr);
		static expr_ptr make_binary(binary &&expr);
		static expr_ptr make_call(call &&expr);
		static expr_ptr make_get(get &&expr);
		static expr_ptr make_grouping(grouping &&expr);
		static expr_ptr make_literal(literal &&expr);
		static expr_ptr make_logical(logical &&expr);
		static expr_ptr make_set(set &&expr);
		static expr_ptr make_super(super_keyword &&expr);
		static expr_ptr make_this(this_keyword &&expr);
		static expr_ptr make_unary(unary &&expr);
		static expr_ptr make_variable(variable &&expr);

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

	using expr_ptr = lox::expr::expr_ptr;
}

#endif
