#include "stmt.hpp"

lox::stmt_ptr lox::stmt::make_block(block &&stmt)
{
	return std::make_unique<lox::stmt>(lox::stmt{.value = std::move(stmt)});
}

lox::stmt_ptr lox::stmt::make_expr(expr &&stmt)
{
	return std::make_unique<lox::stmt>(lox::stmt{.value = std::move(stmt)});
}

lox::stmt_ptr lox::stmt::make_print(print &&stmt)
{
	return std::make_unique<lox::stmt>(lox::stmt{.value = std::move(stmt)});
}

lox::stmt_ptr lox::stmt::make_var(var &&stmt)
{
	return std::make_unique<lox::stmt>(lox::stmt{.value = std::move(stmt)});
}