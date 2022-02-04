#include "expr.hpp"

lox::expr_ptr lox::expr::make_assign(assign &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_binary(binary &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_call(call &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_get(get &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_grouping(grouping &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_literal(literal &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_logical(logical &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_set(set &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_super(super_keyword &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_this(this_keyword &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_unary(unary &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}

lox::expr_ptr lox::expr::make_variable(variable &&expr)
{
	return lox::expr_ptr::make(lox::expr{.value = lak::move(expr)}).unwrap();
}
