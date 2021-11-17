#include "callable.hpp"

#include "evaluator.hpp"
#include "klass.hpp"
#include "overloaded.hpp"

lox::callable lox::callable::make_native(lox::callable::native &&c)
{
	return lox::callable{.value = std::move(c)};
}

lox::callable lox::callable::make_interpreted(lox::callable::interpreted &&c)
{
	return lox::callable{.value = std::move(c)};
}

lox::callable lox::callable::make_constructor(lox::callable::constructor &&c)
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
	    [](const lox::callable::constructor &c) -> size_t
	    {
		    if (std::shared_ptr<lox::callable> init =
		          c.klass->find_method(u8"init");
		        init)
			    return init->arity();
		    else
			    return 0;
	    },
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
	    [](const lox::callable::constructor &c) -> std::u8string
	    { return u8"<ctor " + std::u8string(c.klass->name) + u8">"; },
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
		             std::get<lox::callable::interpreted>(rhs.value).closure &&
		           c.is_init ==
		             std::get<lox::callable::interpreted>(rhs.value).is_init;
	    },
	    [&](const lox::callable::constructor &c) -> bool {
		    return c.klass ==
		           std::get<lox::callable::constructor>(rhs.value).klass;
	    },
	  },
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

		    std::optional<lox::object> result = interpreter.execute_block(
		      std::span<const lox::stmt_ptr>(c.func->body), env);

		    if (c.is_init)
			    return std::make_optional<lox::object>(*c.closure->find(u8"this"));
		    else
			    return result;
	    },
	    [&](const lox::callable::constructor &c) -> std::optional<lox::object>
	    {
		    std::shared_ptr<lox::instance> instance =
		      lox::instance::make(c.klass, {});

		    if (std::shared_ptr<lox::callable> init =
		          c.klass->find_bound_method(u8"init", instance);
		        init)
			    return init->operator()(interpreter, std::move(arguments));
		    else
			    return std::make_optional<lox::object>(instance);
	    },
	  },
	  value);
}
