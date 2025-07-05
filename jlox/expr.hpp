#ifndef LOX_EXPR_HPP
#define LOX_EXPR_HPP

#include "object.hpp"
#include "token.hpp"

#include <lak/memory.hpp>
#include <lak/variant.hpp>
#include <lak/visit.hpp>

#include <vector>

namespace lox
{
	struct expr;

	using expr_ptr = lak::unique_ref<lox::expr>;

	struct expr
	{
		struct assign
		{
			lox::token name;
			lox::expr_ptr value;
		};

		struct binary
		{
			lox::expr_ptr left;
			lox::token op;
			lox::expr_ptr right;
		};

		struct call
		{
			lox::expr_ptr callee;
			lox::token paren;
			std::vector<lox::expr_ptr> arguments;
		};

		struct get
		{
			lox::expr_ptr object;
			lox::token name;
		};

		struct grouping
		{
			lox::expr_ptr expression;
		};

		struct literal
		{
			lox::object value;
		};

		struct logical
		{
			lox::expr_ptr left;
			lox::token op;
			lox::expr_ptr right;
		};

		struct set
		{
			lox::expr_ptr object;
			lox::token name;
			lox::expr_ptr value;
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
			lox::expr_ptr right;
		};

		struct variable
		{
			lox::token name;
		};

		using value_type = lak::variant<assign,
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
		                                variable>;

		value_type value;

		static lox::expr_ptr make_assign(assign &&expr);
		static lox::expr_ptr make_binary(binary &&expr);
		static lox::expr_ptr make_call(call &&expr);
		static lox::expr_ptr make_get(get &&expr);
		static lox::expr_ptr make_grouping(grouping &&expr);
		static lox::expr_ptr make_literal(literal &&expr);
		static lox::expr_ptr make_logical(logical &&expr);
		static lox::expr_ptr make_set(set &&expr);
		static lox::expr_ptr make_super(super_keyword &&expr);
		static lox::expr_ptr make_this(this_keyword &&expr);
		static lox::expr_ptr make_unary(unary &&expr);
		static lox::expr_ptr make_variable(variable &&expr);

		template<typename F>
		inline auto visit(F &&f)
		{
			return lak::visit(f, value);
		}

		template<typename F>
		inline auto visit(F &&f) const
		{
			return lak::visit(f, value);
		}
	};
}

#endif
