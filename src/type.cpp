#include "type.hpp"

#include "object.hpp"

/* --- type --- */

struct lox::type::impl
{
	lak::u8string name;
	lak::optional<lox::type> superclass;
	lox::string_map<char8_t, lox::object> methods;
	lak::optional<lox::callable> constructor;
};

lox::type::type(lak::u8string_view name,
                lox::string_map<char8_t, lox::object> methods)
: _impl(lox::type::impl_ptr::make(lox::type::impl{
                                    .name        = name.to_string(),
                                    .superclass  = lak::nullopt,
                                    .methods     = methods,
                                    .constructor = {},
                                  })
          .unwrap())
{
	_impl->constructor = lox::callable(*this);
}

lox::type::type(lak::u8string_view name,
                lox::string_map<char8_t, lox::object> methods,
                const lox::type &superclass)
: _impl(lox::type::impl_ptr::make(lox::type::impl{
                                    .name        = name.to_string(),
                                    .superclass  = superclass,
                                    .methods     = methods,
                                    .constructor = {},
                                  })
          .unwrap())
{
	_impl->constructor = lox::callable(*this);
}

lak::u8string &lox::type::name()
{
	return _impl->name;
}

const lak::u8string &lox::type::name() const
{
	return _impl->name;
}

lak::optional<lox::type> &lox::type::superclass()
{
	return _impl->superclass;
}

const lak::optional<lox::type> &lox::type::superclass() const
{
	return _impl->superclass;
}

lak::result<const lox::callable &> lox::type::find_method(
  lak::u8string_view method_name) const
{
	return lox::find(_impl->methods, method_name)
	  .and_then([](const std::pair<lak::u8string, lox::object> &pair)
	            { return lak::result_from_pointer(pair.second.get_callable()); })
	  .or_else(
	    [&](auto &&) -> lak::result<const lox::callable &>
	    {
		    if_ref (const auto &super, superclass())
			    return super.find_method(method_name);
		    else
			    return lak::err_t{};
	    });
}

lak::result<const lox::callable &> lox::type::find_method(
  const lox::token &method_name) const
{
	return find_method(method_name.lexeme);
}

lak::result<lox::callable> lox::type::find_bound_method(
  lak::u8string_view method_name, const lox::instance &instance) const
{
	return find_method(method_name)
	  .map(
	    [&](const lox::callable &callable) {
		    return callable.with_binds({{u8"this", lox::object{instance}}});
	    });
}

lak::result<lox::callable> lox::type::find_bound_method(
  const lox::token &method_name, const lox::instance &instance) const
{
	return find_bound_method(method_name.lexeme, instance);
}

lox::callable &lox::type::constructor()
{
	return *_impl->constructor;
}

const lox::callable &lox::type::constructor() const
{
	return *_impl->constructor;
}

lak::u8string lox::type::to_string() const
{
	return name();
}

bool lox::type::operator==(const lox::type &rhs) const
{
	return _impl.get() == rhs._impl.get();
}

bool lox::type::operator!=(const lox::type &rhs) const
{
	return !operator==(rhs);
}

/* --- instance --- */

struct lox::instance::impl
{
	lox::type type;
	lox::string_map<char8_t, lox::object> fields;
};

lox::instance::instance(const lox::type &type,
                        lox::string_map<char8_t, lox::object> fields)
: _impl(lox::instance::impl_ptr::make(lox::instance::impl{
                                        .type   = type,
                                        .fields = lak::move(fields),
                                      })
          .unwrap())
{
}

const lox::object &lox::instance::emplace(const lox::token &name,
                                          lox::object value)
{
	return _impl->fields
	  .insert_or_assign(name.lexeme.to_string(), lak::move(value))
	  .first->second;
}

lak::result<lox::object> lox::instance::find(const lox::token &name) const
{
	return lox::find(_impl->fields, name.lexeme)
	  .map([](const auto &pair) { return pair.second; })
	  .or_else(
	    [&](const auto &)
	    {
		    return _impl->type.find_bound_method(name.lexeme, *this)
		      .map([](const lox::callable &callable) -> lox::object
		           { return {callable}; });
	    });
}

lak::u8string lox::instance::to_string() const
{
	return _impl->type.name() + u8" instance";
}

bool lox::instance::operator==(const lox::instance &rhs) const
{
	return _impl->type == rhs._impl->type;
}

bool lox::instance::operator!=(const lox::instance &rhs) const
{
	return !operator==(rhs);
}
