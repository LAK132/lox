#include "callable.hpp"

#include "evaluator.hpp"
#include "overloaded.hpp"
#include "type.hpp"

struct lox::callable::impl
{
	struct native
	{
		std::optional<lox::object> (*function)(lox::interpreter &,
		                                       std::vector<lox::object> &&);
		size_t arity;
	};

	struct interpreted
	{
		lox::stmt::function_ptr function;
		lox::environment_ptr closure;
		bool is_init;
	};

	struct constructor
	{
		lox::type type;
	};

	std::variant<native, interpreted, constructor> value;
};

lox::callable::callable(native_function_ptr_t function, size_t arity)
{
	_impl = std::make_shared<lox::callable::impl>(lox::callable::impl{
	  .value =
	    lox::callable::impl::native{
	      .function = function,
	      .arity    = arity,
	    },
	});
}

lox::callable::callable(lox::stmt::function_ptr function,
                        lox::environment_ptr closure,
                        bool is_init)
{
	_impl = std::make_shared<lox::callable::impl>(lox::callable::impl{
	  .value =
	    lox::callable::impl::interpreted{
	      .function = function,
	      .closure  = closure,
	      .is_init  = is_init,
	    },
	});
}

lox::callable::callable(const lox::type &type)
{
	_impl = std::make_shared<lox::callable::impl>(lox::callable::impl{
	  .value =
	    lox::callable::impl::constructor{
	      .type = type,
	    },
	});
}

[[nodiscard]] lox::callable lox::callable::with_binds(
  std::initializer_list<std::pair<std::u8string_view, lox::object>> binds)
  const
{
	return with_binds(std::span(binds));
}

[[nodiscard]] lox::callable lox::callable::with_binds(
  std::span<const std::pair<std::u8string_view, lox::object>> binds) const
{
	if (std::holds_alternative<lox::callable::impl::interpreted>(_impl->value))
	{
		const auto &interpreted =
		  std::get<lox::callable::impl::interpreted>(_impl->value);

		lox::environment_ptr env = lox::environment::make(interpreted.closure);

		for (const auto &[key, value] : binds) env->emplace(key, value);

		return lox::callable(interpreted.function, env, interpreted.is_init);
	}
	else
		return *this;
}

size_t lox::callable::arity() const
{
	return std::visit(
	  lox::overloaded{
	    [](const lox::callable::impl::native &c) -> size_t { return c.arity; },
	    [](const lox::callable::impl::interpreted &c) -> size_t
	    { return c.function->parameters.size(); },
	    [](const lox::callable::impl::constructor &c) -> size_t
	    {
		    if (const lox::callable *init = c.type.find_method(u8"init"); init)
			    return init->arity();
		    else
			    return 0;
	    },
	  },
	  _impl->value);
}

std::u8string lox::callable::to_string() const
{
	return std::visit(
	  lox::overloaded{
	    [](const lox::callable::impl::native &) -> std::u8string
	    { return u8"<native function>"; },
	    [](const lox::callable::impl::interpreted &c) -> std::u8string
	    { return u8"<fn " + std::u8string(c.function->name.lexeme) + u8">"; },
	    [](const lox::callable::impl::constructor &c) -> std::u8string
	    { return u8"<ctor " + std::u8string(c.type.name()) + u8">"; },
	  },
	  _impl->value);
}

bool lox::callable::operator==(const callable &rhs) const
{
	if (_impl->value.index() != rhs._impl->value.index()) return false;

	return std::visit(
	  lox::overloaded{
	    [&](const lox::callable::impl::native &c) -> bool
	    {
		    return c.function ==
		           std::get<lox::callable::impl::native>(rhs._impl->value)
		             .function;
	    },
	    [&](const lox::callable::impl::interpreted &c) -> bool
	    {
		    return c.function ==
		             std::get<lox::callable::impl::interpreted>(rhs._impl->value)
		               .function &&
		           c.closure ==
		             std::get<lox::callable::impl::interpreted>(rhs._impl->value)
		               .closure &&
		           c.is_init ==
		             std::get<lox::callable::impl::interpreted>(rhs._impl->value)
		               .is_init;
	    },
	    [&](const lox::callable::impl::constructor &c) -> bool
	    {
		    return c.type ==
		           std::get<lox::callable::impl::constructor>(rhs._impl->value)
		             .type;
	    },
	  },
	  _impl->value);
}

std::optional<lox::object> lox::callable::operator()(
  lox::interpreter &interpreter, std::vector<lox::object> &&arguments) const
{
	return std::visit(
	  lox::overloaded{
	    [&](const lox::callable::impl::native &c) -> std::optional<lox::object>
	    { return c.function(interpreter, std::move(arguments)); },
	    [&](const lox::callable::impl::interpreted &c)
	      -> std::optional<lox::object>
	    {
		    lox::environment_ptr env = lox::environment::make(c.closure);

		    for (size_t i = 0; i < c.function->parameters.size(); ++i)
			    env->emplace(c.function->parameters[i].lexeme, arguments[i]);

		    std::optional<lox::object> result = interpreter.execute_block(
		      std::span<const lox::stmt_ptr>(c.function->body), env);

		    if (c.is_init)
			    return std::make_optional<lox::object>(*c.closure->find(u8"this"));
		    else
			    return result;
	    },
	    [&](const lox::callable::impl::constructor &c)
	      -> std::optional<lox::object>
	    {
		    lox::instance instance = lox::instance(c.type, {});

		    if (std::optional<lox::callable> init =
		          c.type.find_bound_method(u8"init", instance);
		        init)
			    return (*init)(interpreter, std::move(arguments));
		    else
			    return std::make_optional<lox::object>(instance);
	    },
	  },
	  _impl->value);
}
