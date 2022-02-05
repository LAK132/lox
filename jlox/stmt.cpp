#include "stmt.hpp"

lox::stmt_ptr lox::stmt::make_block(block &&stmt)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(stmt)}).unwrap();
}

lox::stmt_ptr lox::stmt::make_type(type &&stmt)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(stmt)}).unwrap();
}

lox::stmt_ptr lox::stmt::make_expr(expr &&stmt)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(stmt)}).unwrap();
}

lox::stmt_ptr lox::stmt::make_branch(branch &&stmt)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(stmt)}).unwrap();
}

lox::stmt_ptr lox::stmt::make_print(print &&stmt)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(stmt)}).unwrap();
}

lox::stmt_ptr lox::stmt::make_var(var &&stmt)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(stmt)}).unwrap();
}

lox::stmt_ptr lox::stmt::make_loop(loop &&stmt)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(stmt)}).unwrap();
}

lox::stmt::function_ptr lox::stmt::make_function_ptr(function &&stmt)
{
	return lox::stmt::function_ptr::make(lak::move(stmt)).unwrap();
}

lox::stmt_ptr lox::stmt::make_function_from_ptr(lox::stmt::function_ptr ptr)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(ptr)}).unwrap();
}

lox::stmt_ptr lox::stmt::make_function(function &&stmt)
{
	return lox::stmt_ptr::make(
	         lox::stmt{.value = make_function_ptr(lak::move(stmt))})
	  .unwrap();
}

lox::stmt_ptr lox::stmt::make_ret(ret &&stmt)
{
	return lox::stmt_ptr::make(lox::stmt{.value = lak::move(stmt)}).unwrap();
}
