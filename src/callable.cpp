#include "callable.hpp"

#include "evaluator.hpp"
#include "overloaded.hpp"

lox::callable lox::callable::make_native(lox::callable::native &&c)
{
	return lox::callable{.value = std::move(c)};
}

lox::callable lox::callable::make_interpreted(lox::callable::interpreted &&c)
{
	return lox::callable{.value = std::move(c)};
}

size_t lox::callable::arity() const
{
	return std::visit(
	  lox::overloaded{
	    [](const lox::callable::native &c) -> size_t { return c.arity; },
	    [](const lox::callable::interpreted &c) -> size_t
	    { return c.func->parameters.size(); },
	  },
	  value);
}

std::u8string lox::callable::to_string() const
{
	return std::visit(
	  lox::overloaded{
	    [](const lox::callable::native &) -> std::u8string
	    { return u8"<native function>"; },
	    [](const lox::callable::interpreted &c) -> std::u8string
	    { return u8"<fn " + std::u8string(c.func->name.lexeme) + u8">"; },
	  },
	  value);
}

bool lox::callable::operator==(const callable &rhs) const
{
	if (value.index() != rhs.value.index()) return false;

	return std::visit(
	  lox::overloaded{
	    [&](const lox::callable::native &c) -> bool {
		    return c.function ==
		           std::get<lox::callable::native>(rhs.value).function;
	    },
	    [&](const lox::callable::interpreted &c) -> bool
	    {
		    return c.func ==
		             std::get<lox::callable::interpreted>(rhs.value).func &&
		           c.closure ==
		             std::get<lox::callable::interpreted>(rhs.value).closure;
	    }},
	  value);
}

std::optional<lox::object> lox::callable::operator()(
  lox::interpreter &interpreter, std::vector<lox::object> &&arguments) const
{
	return std::visit(
	  lox::overloaded{
	    [&](const lox::callable::native &c) -> std::optional<lox::object>
	    { return c.function(interpreter, std::move(arguments)); },
	    [&](const lox::callable::interpreted &c) -> std::optional<lox::object>
	    {
		    lox::environment_ptr env = lox::environment::make(c.closure);

		    for (size_t i = 0; i < c.func->parameters.size(); ++i)
			    env->emplace(c.func->parameters[i].lexeme, arguments[i]);

		    if (lox::evaluator eval = lox::evaluator{interpreter};
		        eval.execute_block(std::span<const lox::stmt_ptr>(c.func->body),
		                           env))
			    return eval.block_ret_value ? eval.block_ret_value
			                                : std::make_optional<lox::object>();
		    else
			    return std::nullopt;
	    },
	  },
	  value);
}
