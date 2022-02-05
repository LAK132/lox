#include "resolver.hpp"

#include <assert.h>

lak::err_t<> lox::resolver::error(const lox::token &token,
                                  lak::u8string_view message,
                                  const std::source_location srcloc)
{
	interpreter.error(token, message, srcloc);
	return lak::err_t{};
}

lak::result<> lox::resolver::resolve(lak::span<const lox::stmt_ptr> statements)
{
	for (const auto &s : statements) RES_TRY(s->visit(*this));

	return lak::ok_t{};
}

lak::result<> lox::resolver::resolve_function(
  const lox::stmt::function_ptr &func, lox::function_type type)
{
	lox::function_type enclosing_function_type = current_function;
	current_function                           = type;

	scopes.emplace_back();

	for (const lox::token &param : func->parameters)
	{
		RES_TRY(declare(param));
		define(param);
	}

	RES_TRY(resolve(func->body));

	scopes.pop_back();

	current_function = enclosing_function_type;

	return lak::ok_t{};
}

lak::result<> lox::resolver::declare(const lox::token &name)
{
	if (!scopes.empty())
	{
		auto &scope = scopes.back();

		if (scope.find(name.lexeme) != scope.end())
			return error(name, u8"Already a variable with this name in this scope.");

		scope.emplace(name.lexeme, false);
	}

	return lak::ok_t{};
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

lak::result<> lox::resolver::operator()(const lox::expr::assign &expr)
{
	RES_TRY(expr.value->visit(*this));

	resolve_local(expr, expr.name);

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::expr::binary &expr)
{
	RES_TRY(expr.left->visit(*this));

	RES_TRY(expr.right->visit(*this));

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::expr::call &expr)
{
	RES_TRY(expr.callee->visit(*this));

	for (auto &arg : expr.arguments) RES_TRY(arg->visit(*this));

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::expr::get &expr)
{
	return expr.object->visit(*this);
}

lak::result<> lox::resolver::operator()(const lox::expr::grouping &expr)
{
	return expr.expression->visit(*this);
}

lak::result<> lox::resolver::operator()(const lox::expr::literal &)
{
	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::expr::logical &expr)
{
	RES_TRY(expr.left->visit(*this));

	RES_TRY(expr.right->visit(*this));

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::expr::set &expr)
{
	RES_TRY(expr.object->visit(*this));

	RES_TRY(expr.value->visit(*this));

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::expr::super_keyword &expr)
{
	if (current_class == lox::class_type::NONE)
		return error(expr.keyword, u8"Can't use 'super' outside of a class.");

	if (current_class != lox::class_type::SUBCLASS)
		return error(expr.keyword,
		             u8"Can't use 'super' in a class with no superclass.");

	resolve_local(expr, expr.keyword);

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::expr::this_keyword &expr)
{
	if (current_class == lox::class_type::NONE)
		return error(expr.keyword, u8"Can't use 'this' outside of a class.");

	resolve_local(expr, expr.keyword);

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::expr::unary &expr)
{
	return expr.right->visit(*this);
}

lak::result<> lox::resolver::operator()(const lox::expr::variable &expr)
{
	if (!scopes.empty())
		if (auto iter = scopes.back().find(expr.name.lexeme);
		    iter != scopes.back().end() && iter->second == false)
			return error(expr.name,
			             u8"Can't read local variable in its own initialiser.");

	resolve_local(expr, expr.name);

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::stmt::block &stmt)
{
	scopes.emplace_back();

	RES_TRY(resolve(stmt.statements));

	scopes.pop_back();

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::stmt::type &stmt)
{
	lox::class_type enclosing_class_type = current_class;
	current_class                        = lox::class_type::CLASS;

	RES_TRY(declare(stmt.name));

	define(stmt.name);

	if_ref (const auto &superclass, stmt.superclass)
	{
		if (stmt.name.lexeme == superclass.name.lexeme)
			return error(superclass.name, u8"A class can't inherit from itself.");

		current_class = lox::class_type::SUBCLASS;

		RES_TRY((*this)(superclass));

		scopes.emplace_back();

		scopes.back().insert_or_assign(u8"super", true);
	}

	scopes.emplace_back();

	scopes.back().insert_or_assign(u8"this", true);

	for (const lox::stmt::function_ptr &method : stmt.methods)
		RES_TRY(resolve_function(method,
		                         method->name.lexeme == u8"init"
		                           ? lox::function_type::INIT
		                           : lox::function_type::METHOD));

	scopes.pop_back();

	if (stmt.superclass) scopes.pop_back();

	current_class = enclosing_class_type;

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::stmt::expr &stmt)
{
	return stmt.expression->visit(*this);
}

lak::result<> lox::resolver::operator()(const lox::stmt::branch &stmt)
{
	RES_TRY(stmt.condition->visit(*this));

	RES_TRY(stmt.then_branch->visit(*this));

	if_ref (const auto &else_branch, stmt.else_branch)
		RES_TRY(else_branch->visit(*this));

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::stmt::print &stmt)
{
	return stmt.expression->visit(*this);
}

lak::result<> lox::resolver::operator()(const lox::stmt::var &stmt)
{
	RES_TRY(declare(stmt.name));

	if_ref (const auto &init, stmt.init) RES_TRY(init->visit(*this));

	define(stmt.name);

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::stmt::loop &stmt)
{
	RES_TRY(stmt.condition->visit(*this));

	RES_TRY(stmt.body->visit(*this));

	return lak::ok_t{};
}

lak::result<> lox::resolver::operator()(const lox::stmt::function_ptr &stmt)
{
	RES_TRY(declare(stmt->name));

	define(stmt->name);

	return resolve_function(stmt, lox::function_type::FUNCTION);
}

lak::result<> lox::resolver::operator()(const lox::stmt::ret &stmt)
{
	if (current_function == lox::function_type::NONE)
		return error(stmt.keyword, u8"Can't return from top-level code.");

	if_ref (const auto &value, stmt.value)
	{
		if (current_function == lox::function_type::INIT)
			return error(stmt.keyword,
			             u8"Can't return a value from an initialiser.");
		return value->visit(*this);
	}
	else
		return lak::ok_t{};
}
