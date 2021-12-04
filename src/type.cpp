#include "type.hpp"

#include "object.hpp"

/* --- type --- */

struct lox::type::impl
{
	std::u8string name;
	std::optional<lox::type> superclass;
	lox::string_map<char8_t, lox::object> methods;
	std::optional<lox::callable> constructor;
};

lox::type::type(std::u8string_view name,
                lox::string_map<char8_t, lox::object> methods)
{
	_impl = std::make_shared<lox::type::impl>(lox::type::impl{
	  .name        = std::u8string(name),
	  .superclass  = std::nullopt,
	  .methods     = methods,
	  .constructor = {},
	});

	_impl->constructor = lox::callable(*this);
}

lox::type::type(std::u8string_view name,
                lox::string_map<char8_t, lox::object> methods,
                const lox::type &superclass)
{
	_impl = std::make_shared<lox::type::impl>(lox::type::impl{
	  .name        = std::u8string(name),
	  .superclass  = superclass,
	  .methods     = methods,
	  .constructor = {},
	});

	_impl->constructor = lox::callable(*this);
}

std::u8string &lox::type::name()
{
	return _impl->name;
}

const std::u8string &lox::type::name() const
{
	return _impl->name;
}

std::optional<lox::type> &lox::type::superclass()
{
	return _impl->superclass;
}

const std::optional<lox::type> &lox::type::superclass() const
{
	return _impl->superclass;
}

const lox::callable *lox::type::find_method(
  std::u8string_view method_name) const
{
	auto get_callable = [&]() -> const lox::callable *
	{
		auto method = _impl->methods.find(method_name);
		return method != _impl->methods.end() ? method->second.get_callable()
		                                      : nullptr;
	};

	if (auto callable = get_callable(); callable)
		return callable;
	else if (superclass())
		return superclass()->find_method(method_name);
	else
		return nullptr;
}

const lox::callable *lox::type::find_method(
  const lox::token &method_name) const
{
	return find_method(method_name.lexeme);
}

std::optional<lox::callable> lox::type::find_bound_method(
  std::u8string_view method_name, const lox::instance &instance) const
{
	if (auto callable = find_method(method_name); callable)
		return callable->with_binds({{u8"this", lox::object{instance}}});
	else
		return {};
}

std::optional<lox::callable> lox::type::find_bound_method(
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

std::u8string lox::type::to_string() const
{
	return name();
}

bool lox::type::operator==(const lox::type &rhs) const
{
	return _impl == rhs._impl;
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
{
	_impl = std::make_shared<lox::instance::impl>(lox::instance::impl{
	  .type   = type,
	  .fields = std::move(fields),
	});
}

const lox::object &lox::instance::emplace(const lox::token &name,
                                          lox::object value)
{
	return _impl->fields
	  .insert_or_assign(std::u8string(name.lexeme), std::move(value))
	  .first->second;
}

std::optional<lox::object> lox::instance::find(const lox::token &name) const
{
	if (auto field = _impl->fields.find(name.lexeme);
	    field != _impl->fields.end())
		return std::make_optional<lox::object>(field->second);
	else if (std::optional<lox::callable> method =
	           _impl->type.find_bound_method(name.lexeme, *this);
	         method)
		return std::make_optional<lox::object>(lox::object{*method});
	else
		return std::nullopt;
}

std::u8string lox::instance::to_string() const
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
