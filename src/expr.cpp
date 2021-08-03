#include "expr.hpp"

lox::expr::expr_ptr lox::expr::make_assign(assign &&expr)
{
	return std::make_unique<lox::expr>(lox::expr{.value = std::move(expr)});
}

lox::expr::expr_ptr lox::expr::make_binary(binary &&expr)
{
	return std::make_unique<lox::expr>(lox::expr{.value = std::move(expr)});
}

lox::expr::expr_ptr lox::expr::make_grouping(grouping &&expr)
{
	return std::make_unique<lox::expr>(lox::expr{.value = std::move(expr)});
}

lox::expr::expr_ptr lox::expr::make_literal(literal &&expr)
{
	return std::make_unique<lox::expr>(lox::expr{.value = std::move(expr)});
}

lox::expr::expr_ptr lox::expr::make_unary(unary &&expr)
{
	return std::make_unique<lox::expr>(lox::expr{.value = std::move(expr)});
}

lox::expr::expr_ptr lox::expr::make_variable(variable &&expr)
{
	return std::make_unique<lox::expr>(lox::expr{.value = std::move(expr)});
}
