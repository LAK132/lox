#include "resolver.hpp"

#include <assert.h>

std::nullopt_t lox::resolver::error(const lox::token &token,
                                    std::u8string_view message)
{
	interpreter.error(token, message);
	return std::nullopt;
}

std::optional<std::monostate> lox::resolver::resolve(
  std::span<const lox::stmt_ptr> statements)
{
	for (const auto &s : statements)
		if (!s->visit(*this)) return std::nullopt;

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::resolve_function(
  const lox::stmt::function_ptr &func, lox::function_type type)
{
	lox::function_type enclosing_function_type = current_function;
	current_function                           = type;

	scopes.emplace_back();

	for (const lox::token &param : func->parameters)
	{
		if (!declare(param)) return std::nullopt;
		define(param);
	}

	if (!resolve(func->body)) return std::nullopt;

	scopes.pop_back();

	current_function = enclosing_function_type;

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::declare(const lox::token &name)
{
	if (!scopes.empty())
	{
		auto &scope = scopes.back();

		if (scope.find(name.lexeme) != scope.end())
			return error(name, u8"Already a variable with this name in this scope.");

		scope.emplace(name.lexeme, false);
	}

	return std::make_optional<std::monostate>();
}

void lox::resolver::define(const lox::token &name)
{
	if (scopes.empty()) return;

	auto &scope = scopes.back();
	if (auto iter = scope.find(name.lexeme); iter != scope.end())
		iter->second = true;
	else
		scope.emplace(name.lexeme, true);
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::expr::assign &expr)
{
	if (!expr.value->visit(*this)) return std::nullopt;

	resolve_local(expr, expr.name);

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::expr::binary &expr)
{
	if (!expr.left->visit(*this)) return std::nullopt;

	if (!expr.right->visit(*this)) return std::nullopt;

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::expr::call &expr)
{
	if (!expr.callee->visit(*this)) return std::nullopt;

	for (auto &arg : expr.arguments)
		if (!arg->visit(*this)) return std::nullopt;

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::expr::grouping &expr)
{
	return expr.expression->visit(*this);
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::expr::literal &)
{
	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::expr::logical &expr)
{
	if (!expr.left->visit(*this)) return std::nullopt;

	if (!expr.right->visit(*this)) return std::nullopt;

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::expr::unary &expr)
{
	return expr.right->visit(*this);
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::expr::variable &expr)
{
	if (!scopes.empty())
		if (auto iter = scopes.back().find(expr.name.lexeme);
		    iter != scopes.back().end() && iter->second == false)
			return error(expr.name,
			             u8"Can't read local variable in its own initialiser.");

	resolve_local(expr, expr.name);

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::stmt::block &stmt)
{
	scopes.emplace_back();

	if (!resolve(stmt.statements)) return std::nullopt;

	scopes.pop_back();

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::stmt::expr &stmt)
{
	return stmt.expression->visit(*this);
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::stmt::branch &stmt)
{
	if (!stmt.condition->visit(*this)) return std::nullopt;

	if (!stmt.then_branch->visit(*this)) return std::nullopt;

	if (stmt.else_branch && !stmt.else_branch->visit(*this)) return std::nullopt;

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::stmt::print &stmt)
{
	return stmt.expression->visit(*this);
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::stmt::var &stmt)
{
	if (!declare(stmt.name)) return std::nullopt;

	if (stmt.init)
		if (!stmt.init->visit(*this)) return std::nullopt;

	define(stmt.name);

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::stmt::loop &stmt)
{
	if (!stmt.condition->visit(*this)) return std::nullopt;

	if (!stmt.body->visit(*this)) return std::nullopt;

	return std::make_optional<std::monostate>();
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::stmt::function_ptr &stmt)
{
	if (!declare(stmt->name)) return std::nullopt;

	define(stmt->name);

	return resolve_function(stmt, lox::function_type::FUNCTION);
}

std::optional<std::monostate> lox::resolver::operator()(
  const lox::stmt::ret &stmt)
{
	if (current_function == lox::function_type::NONE)
		return error(stmt.keyword, u8"Can't return from top-level code.");

	if (stmt.value)
		return stmt.value->visit(*this);
	else
		return std::make_optional<std::monostate>();
}
