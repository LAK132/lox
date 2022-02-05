#include "evaluator.hpp"

#include "callable.hpp"
#include "type.hpp"

#include <lak/defer.hpp>
#include <lak/string_ostream.hpp>

lak::err_t<> lox::evaluator::error(const lox::token &token,
                                   lak::u8string_view message,
                                   const std::source_location srcloc)
{
	interpreter.error(token, message, srcloc);
	return lak::err_t{};
}

lak::result<lak::u8string> lox::evaluator::execute_block(
  lak::span<const lox::stmt_ptr> statements, const lox::environment_ptr &env)
{
	lox::environment_ptr previous = std::exchange(environment, env);
	DEFER(environment = previous);

	for (const auto &s : statements)
	{
		RES_TRY(s->visit(*this));
		if (block_ret_value) break;
	}

	return lak::ok_t<lak::u8string>{};
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::assign &expr)
{
	return expr.value->visit(*this).and_then(
	  [&](const lox::object &value) -> lak::result<lox::object>
	  {
		  return interpreter.find(expr).visit(lak::overloaded{
		    [&](size_t distance) -> lak::result<lox::object>
		    {
			    return lak::copy_result_from_pointer(
			             environment->replace(expr.name, value, distance))
			      .or_else(
			        [&](auto &&) -> lak::result<lox::object>
			        {
				        return error(expr.name,
				                     u8"Undefined local variable '"_str +
				                       expr.name.lexeme.to_string() + u8"'.");
			        });
		    },
		    [&](lak::monostate) -> lak::result<lox::object>
		    {
			    return lak::copy_result_from_pointer(
			             interpreter.global_environment->replace(expr.name, value))
			      .or_else(
			        [&](auto &&) -> lak::result<lox::object>
			        {
				        return error(expr.name,
				                     u8"Undefined global variable '"_str +
				                       expr.name.lexeme.to_string() + u8"'.");
			        });
		    },
		  });
	  });
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::binary &expr)
{
	RES_TRY_ASSIGN(lox::object left =, expr.left->visit(*this));

	RES_TRY_ASSIGN(lox::object right =, expr.right->visit(*this));

	switch (expr.op.type)
	{
		using enum lox::token_type;

		case EQUAL_EQUAL: return lak::ok_t{lox::object{left == right}};

		case BANG_EQUAL: return lak::ok_t{lox::object{left != right}};

		case GREATER:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lak::ok_t{lox::object{*left_num > *right_num}};
		}

		case GREATER_EQUAL:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lak::ok_t{lox::object{*left_num >= *right_num}};
		}

		case LESS:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lak::ok_t{lox::object{*left_num < *right_num}};
		}

		case LESS_EQUAL:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lak::ok_t{lox::object{*left_num <= *right_num}};
		}

		case PLUS:
		{
			{
				auto left_num  = left.get_number();
				auto right_num = right.get_number();
				if (left_num && right_num)
					return lak::ok_t{lox::object{*left_num + *right_num}};
			}
			{
				auto left_string  = left.get_string();
				auto right_string = right.get_string();
				if (left_string && right_string)
					return lak::ok_t{lox::object{*left_string + *right_string}};
			}
			return error(expr.op, u8"Operands must be two numbers or two strings.");
		}

		case MINUS:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lak::ok_t{lox::object{*left_num - *right_num}};
		}

		case SLASH:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lak::ok_t{lox::object{*left_num / *right_num}};
		}

		case STAR:
		{
			auto left_num  = left.get_number();
			auto right_num = right.get_number();
			if (!left_num || !right_num)
				return error(expr.op, u8"Operands must be numbers.");
			return lak::ok_t{lox::object{*left_num * *right_num}};
		}
	}

	return lak::ok_t<lox::object>{};
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::call &expr)
{
	RES_TRY_ASSIGN(lox::object callee =, expr.callee->visit(*this));

	const lox::callable *maybe_callable = callee.get_callable();
	if (!maybe_callable)
		return error(expr.paren, u8"Can only call functions and classes.");

	const lox::callable &callable = *maybe_callable;

	std::vector<lox::object> arguments;
	arguments.reserve(expr.arguments.size());
	for (const auto &argument : expr.arguments)
	{
		RES_TRY_ASSIGN(lox::object obj =, argument->visit(*this));
		arguments.push_back(lak::move(obj));
	}

	if (arguments.size() != callable.arity())
		return error(expr.paren,
		             lak::as_u8string("Expected " +
		                              std::to_string(callable.arity()) +
		                              " arguments but got " +
		                              std::to_string(arguments.size()) + "."));

	return callable(interpreter, lak::move(arguments));
}

lak::result<lox::object> lox::evaluator::operator()(const lox::expr::get &expr)
{
	RES_TRY_ASSIGN(lox::object object =, expr.object->visit(*this));

	if_ref (const lox::instance & instance, object.get_instance())
		return instance.find(expr.name).or_else(
		  [&](auto &&) -> lak::result<lox::object>
		  {
			  return error(expr.name,
			               u8"Undefined property '" + expr.name.lexeme.to_string() +
			                 u8"'.");
		  });
	else
		return error(expr.name, u8"Only instances have properties.");
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::grouping &expr)
{
	return expr.expression->visit(*this);
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::literal &expr)
{
	return lak::ok_t{expr.value};
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::logical &expr)
{
	RES_TRY_ASSIGN(lox::object left =, expr.left->visit(*this));

	if (expr.op.type == lox::token_type::OR)
	{
		if (left.is_truthy()) return lak::ok_t{lak::move(left)};
	}
	else
	{
		if (!left.is_truthy()) return lak::ok_t{lak::move(left)};
	}

	return expr.right->visit(*this);
}

lak::result<lox::object> lox::evaluator::operator()(const lox::expr::set &expr)
{
	RES_TRY_ASSIGN(lox::object object =, expr.object->visit(*this));

	lox::instance *maybe_instance = object.get_instance();
	if (!maybe_instance)
		return error(expr.name, u8"Only instances have fields.");

	RES_TRY_ASSIGN(lox::object value =, expr.value->visit(*this));

	const lox::object &result =
	  maybe_instance->emplace(expr.name, lak::move(value));

	return lak::ok_t{result};
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::super_keyword &expr)
{
	auto invalid_super = [&](auto &&...) -> lak::result<lox::object>
	{ return error(expr.keyword, u8"Invalid 'super'."); };

	RES_TRY_ASSIGN(size_t distance =,
	               interpreter.find(expr).if_err(invalid_super));

	const lox::object *maybe_super_object =
	  environment->find(u8"super", distance);
	if (!maybe_super_object) return invalid_super();

	const lox::type *maybe_super = maybe_super_object->get_type();
	if (!maybe_super) return invalid_super();

	const lox::object *maybe_this = environment->find(u8"this", distance - 1U);
	if (!maybe_this) return error(expr.keyword, u8"Invalid 'this'.");

	const lox::instance *maybe_instance = maybe_this->get_instance();
	if (!maybe_instance)
		return error(expr.keyword, u8"Invalid 'this', expected an instance.");

	RES_TRY_ASSIGN(
	  lox::callable method =,
	  maybe_super->find_bound_method(expr.method.lexeme, *maybe_instance)
	    .if_err(
	      [&](auto &&)
	      {
		      error(expr.method,
		            u8"Undefined property '" + expr.method.lexeme.to_string() +
		              u8"'.");
	      }));

	return lak::ok_t<lox::object>{method};
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::this_keyword &expr)
{
	return find_variable(expr.keyword, expr);
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::unary &expr)
{
	RES_TRY_ASSIGN(lox::object right =, expr.right->visit(*this));

	switch (expr.op.type)
	{
		using enum lox::token_type;

		case BANG: return lak::ok_t<lox::object>{!right.is_truthy()};

		case MINUS:
		{
			if_ref (const auto &num, right.get_number())
				return lak::ok_t<lox::object>{-num};
			else
				return error(expr.op, u8"Operand must be a number.");
		}

		default: return lak::ok_t<lox::object>{};
	}
}

lak::result<lox::object> lox::evaluator::operator()(
  const lox::expr::variable &expr)
{
	return find_variable(expr.name, expr);
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::block &stmt)
{
	return execute_block(lak::span(stmt.statements),
	                     lox::environment::make(environment));
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::type &stmt)
{
	const lox::type *superclass = nullptr;
	if_ref (const auto &supervar, stmt.superclass)
	{
		RES_TRY_ASSIGN(lox::object super =, (*this)(supervar));

		const lox::type *maybe_type = super.get_type();
		if (!maybe_type)
			return error(supervar.name, u8"Superclass must be a class.");

		superclass = maybe_type;
	}

	environment->emplace(stmt.name, lox::object{});

	if (superclass)
	{
		environment = lox::environment::make(environment);
		environment->emplace(u8"super", lox::object{*superclass});
	}

	lox::string_map<char8_t, lox::object> methods;
	for (const lox::stmt::function_ptr &method : stmt.methods)
	{
		methods[method->name.lexeme] = lox::object{
		  lox::callable(method, environment, method->name.lexeme == u8"init")};
	}

	lox::type type =
	  superclass ? lox::type(stmt.name.lexeme, lak::move(methods), *superclass)
	             : lox::type(stmt.name.lexeme, lak::move(methods));

	if (superclass) environment = environment->enclosing;

	environment->replace(stmt.name, lox::object{type});

	return lak::ok_t<lak::u8string>{};
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::expr &stmt)
{
	return stmt.expression->visit(*this).map(
	  [](const lox::object &obj) { return obj.to_string() + u8"\n"; });
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::branch &stmt)
{
	return stmt.condition->visit(*this).and_then(
	  [&](const lox::object &condition) -> lak::result<lak::u8string>
	  {
		  if (condition.is_truthy())
			  return stmt.then_branch->visit(*this);
		  else if_ref (const auto &else_branch, stmt.else_branch)
			  return else_branch->visit(*this);
		  else
			  return lak::ok_t<lak::u8string>{};
	  });
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::print &stmt)
{
	RES_TRY_ASSIGN(lox::object value =, stmt.expression->visit(*this));

	using lak::operator<<;
	std::cout << value.to_string() << "\n";

	return lak::ok_t<lak::u8string>{};
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::var &stmt)
{
	if_ref (const auto &init, stmt.init)
		return init->visit(*this).map(
		  [&](lox::object &&value) -> lak::u8string
		  {
			  environment->emplace(stmt.name, lak::move(value));
			  return {};
		  });

	environment->emplace(stmt.name, lox::object{});

	return lak::ok_t<lak::u8string>{};
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::loop &stmt)
{
	RES_TRY_ASSIGN(lox::object condition =, stmt.condition->visit(*this));

	lak::u8string result;

	while (condition.is_truthy())
	{
		RES_TRY_ASSIGN(result +=, stmt.body->visit(*this));

		if (block_ret_value) return lak::ok_t{result};

		RES_TRY_ASSIGN(condition =, stmt.condition->visit(*this));
	}

	return lak::ok_t{result};
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::function_ptr &stmt)
{
	environment->emplace(stmt->name.lexeme,
	                     lox::object{lox::callable(stmt, environment, false)});
	return lak::ok_t<lak::u8string>{};
}

lak::result<lak::u8string> lox::evaluator::operator()(
  const lox::stmt::ret &stmt)
{
	if_ref (const auto &value, stmt.value)
	{
		RES_TRY_ASSIGN(block_ret_value =, value->visit(*this));
	}
	else
		block_ret_value = lox::object{};

	return lak::ok_t<lak::u8string>{};
}
