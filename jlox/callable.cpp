#include "callable.hpp"

#include "evaluator.hpp"
#include "type.hpp"

#include <lak/macro_utils.hpp>
#include <lak/variant.hpp>
#include <lak/visit.hpp>

struct lox::callable::impl
{
	struct native
	{
		lak::result<lox::object> (*function)(lox::interpreter &,
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

	lak::variant<native, interpreted, constructor> value;
};

lox::callable::callable(native_function_ptr_t function, size_t arity)
: _impl(lox::callable::impl_ptr::make(lox::callable::impl{
                                        .value =
                                          lox::callable::impl::native{
                                            .function = function,
                                            .arity    = arity,
                                          },
                                      })
          .unwrap())
{
}

lox::callable::callable(lox::stmt::function_ptr function,
                        lox::environment_ptr closure,
                        bool is_init)
: _impl(lox::callable::impl_ptr::make(lox::callable::impl{
                                        .value =
                                          lox::callable::impl::interpreted{
                                            .function = function,
                                            .closure  = closure,
                                            .is_init  = is_init,
                                          },
                                      })
          .unwrap())
{
}

lox::callable::callable(const lox::type &type)
: _impl(lox::callable::impl_ptr::make(lox::callable::impl{
                                        .value =
                                          lox::callable::impl::constructor{
                                            .type = type,
                                          },
                                      })
          .unwrap())
{
}

[[nodiscard]] lox::callable lox::callable::with_binds(
  std::initializer_list<lak::pair<lak::u8string_view, lox::object>> binds)
  const
{
	return with_binds(lak::span(binds));
}

[[nodiscard]] lox::callable lox::callable::with_binds(
  lak::span<const lak::pair<lak::u8string_view, lox::object>> binds) const
{
	if_ref (const auto &interpreted,
	        _impl->value.template get<lox::callable::impl::interpreted>())
	{
		lox::environment_ptr env = lox::environment::make(interpreted.closure);

		for (const auto &[key, value] : binds) env->emplace(key, value);

		return lox::callable(interpreted.function, env, interpreted.is_init);
	}
	else
		return *this;
}

size_t lox::callable::arity() const
{
	return lak::visit(
	  lak::overloaded{
	    [](const lox::callable::impl::native &c) -> size_t { return c.arity; },
	    [](const lox::callable::impl::interpreted &c) -> size_t
	    { return c.function->parameters.size(); },
	    [](const lox::callable::impl::constructor &c) -> size_t
	    {
		    return c.type.find_method(u8"init").map_or(
		      [](const lox::callable &init) { return init.arity(); }, size_t(0));
	    },
	  },
	  _impl->value);
}

lak::u8string lox::callable::to_string() const
{
	return lak::visit(
	  lak::overloaded{
	    [](const lox::callable::impl::native &) -> lak::u8string
	    { return u8"<native function>"; },
	    [](const lox::callable::impl::interpreted &c) -> lak::u8string
	    { return u8"<fn " + c.function->name.lexeme.to_string() + u8">"; },
	    [](const lox::callable::impl::constructor &c) -> lak::u8string
	    { return u8"<ctor " + c.type.name() + u8">"; },
	  },
	  _impl->value);
}

bool lox::callable::operator==(const callable &rhs) const
{
	if (_impl->value.index() != rhs._impl->value.index()) return false;

	return lak::visit(
	  lak::overloaded{
	    [&](const lox::callable::impl::native &c) -> bool
	    {
		    return c.function ==
		           rhs._impl->value.template get<lox::callable::impl::native>()
		             ->function;
	    },
	    [&](const lox::callable::impl::interpreted &c) -> bool
	    {
		    auto *interpreted =
		      rhs._impl->value.template get<lox::callable::impl::interpreted>();
		    return c.function.get() == interpreted->function.get() &&
		           c.closure == interpreted->closure &&
		           c.is_init == interpreted->is_init;
	    },
	    [&](const lox::callable::impl::constructor &c) -> bool
	    {
		    return c.type == rhs._impl->value
		                       .template get<lox::callable::impl::constructor>()
		                       ->type;
	    },
	  },
	  _impl->value);
}

lak::result<lox::object> lox::callable::operator()(
  lox::interpreter &interpreter, std::vector<lox::object> &&arguments) const
{
	return lak::visit(
	  lak::overloaded{
	    [&](const lox::callable::impl::native &c) -> lak::result<lox::object>
	    { return c.function(interpreter, lak::move(arguments)); },
	    [&](
	      const lox::callable::impl::interpreted &c) -> lak::result<lox::object>
	    {
		    lox::environment_ptr env = lox::environment::make(c.closure);

		    for (size_t i = 0; i < c.function->parameters.size(); ++i)
			    env->emplace(c.function->parameters[i].lexeme, arguments[i]);

		    RES_TRY_ASSIGN(
		      lox::object result =,
		      interpreter.execute_block(
		        lak::span<const lox::stmt_ptr>(c.function->body), env));

		    if (c.is_init)
			    return lak::ok_t<lox::object>{*c.closure->find(u8"this"_view)};
		    else
			    return lak::ok_t{result};
	    },
	    [&](
	      const lox::callable::impl::constructor &c) -> lak::result<lox::object>
	    {
		    lox::instance instance = lox::instance(c.type, {});

		    return c.type.find_bound_method(u8"init"_view, instance)
		      .visit(lak::overloaded{
		        [&](const lox::callable &init) -> lak::result<lox::object>
		        { return init(interpreter, lak::move(arguments)); },
		        [&](const lak::monostate &) -> lak::result<lox::object>
		        { return lak::ok_t<lox::object>{instance}; },
		      });
	    },
	  },
	  _impl->value);
}
