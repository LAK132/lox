#include "printer.hpp"

#include <cassert>
#include <string>

/* --- prefix_ast_printer_t --- */

std::u8string lox::prefix_ast_printer_t::parenthesize(
  std::u8string_view name, std::initializer_list<lox::expr *> exprs) const
{
	std::u8string result;
	result += u8"(";
	result += name;
	for (const auto &expr : exprs)
	{
		assert(expr != nullptr);
		result += u8" ";
		result += expr->visit(*this);
	}
	result += u8")";
	return result;
}

std::u8string lox::prefix_ast_printer_t::operator()(
  const lox::expr::binary &expr) const
{
	return parenthesize(expr.op.lexeme, {expr.left.get(), expr.right.get()});
}

std::u8string lox::prefix_ast_printer_t::operator()(
  const lox::expr::grouping &expr) const
{
	return parenthesize(u8"group", {expr.expression.get()});
}

std::u8string lox::prefix_ast_printer_t::operator()(
  const lox::expr::literal &expr) const
{
	return expr.value.to_string();
}

std::u8string lox::prefix_ast_printer_t::operator()(
  const lox::expr::unary &expr) const
{
	return parenthesize(expr.op.lexeme, {expr.right.get()});
}

/* --- postfix_ast_printer_t --- */

std::u8string lox::postfix_ast_printer_t::common(
  std::u8string_view name, std::initializer_list<lox::expr *> exprs) const
{
	std::u8string result;
	for (const auto &expr : exprs)
	{
		assert(expr != nullptr);
		result += expr->visit(*this);
		result += u8" ";
	}
	result += name;
	return result;
}

std::u8string lox::postfix_ast_printer_t::operator()(
  const lox::expr::binary &expr) const
{
	return common(expr.op.lexeme, {expr.left.get(), expr.right.get()});
}

std::u8string lox::postfix_ast_printer_t::operator()(
  const lox::expr::grouping &expr) const
{
	return common(u8"group", {expr.expression.get()});
}

std::u8string lox::postfix_ast_printer_t::operator()(
  const lox::expr::literal &expr) const
{
	return expr.value.to_string();
}

std::u8string lox::postfix_ast_printer_t::operator()(
  const lox::expr::unary &expr) const
{
	return common(expr.op.lexeme, {expr.right.get()});
}