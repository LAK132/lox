#include "object.hpp"

#include "callable.hpp"
#include "string_hacks.hpp"
#include "type.hpp"

#include <string>

struct lox::object::impl
{
	lox::object::value_type value;
};

lox::object::object()
{
	_impl = std::make_shared<lox::object::impl>();
}

lox::object::object(std::monostate value)
{
	_impl = std::make_shared<lox::object::impl>(lox::object::impl{
	  .value = value,
	});
}

lox::object::object(std::u8string value)
{
	_impl = std::make_shared<lox::object::impl>(lox::object::impl{
	  .value = value,
	});
}

lox::object::object(double value)
{
	_impl = std::make_shared<lox::object::impl>(lox::object::impl{
	  .value = value,
	});
}

lox::object::object(bool value)
{
	_impl = std::make_shared<lox::object::impl>(lox::object::impl{
	  .value = value,
	});
}

lox::object::object(const lox::callable &value)
{
	_impl = std::make_shared<lox::object::impl>(lox::object::impl{
	  .value = value,
	});
}

lox::object::object(const lox::type &value)
{
	_impl = std::make_shared<lox::object::impl>(lox::object::impl{
	  .value = value,
	});
}

lox::object::object(const lox::instance &value)
{
	_impl = std::make_shared<lox::object::impl>(lox::object::impl{
	  .value = value,
	});
}

bool lox::object::is_truthy() const
{
	return visit(lox::overloaded{
	  [&](std::monostate) -> bool { return false; },
	  [&](const bool &b) -> bool { return b; },
	  [&](auto &&) -> bool { return true; },
	});
}

lox::object::value_type &lox::object::value()
{
	return _impl->value;
}

const lox::object::value_type &lox::object::value() const
{
	return _impl->value;
}

const std::u8string *lox::object::get_string() const
{
	return std::holds_alternative<std::u8string>(_impl->value)
	         ? &std::get<std::u8string>(_impl->value)
	         : nullptr;
}

const double *lox::object::get_number() const
{
	return std::holds_alternative<double>(_impl->value)
	         ? &std::get<double>(_impl->value)
	         : nullptr;
}

const bool *lox::object::get_bool() const
{
	return std::holds_alternative<bool>(_impl->value)
	         ? &std::get<bool>(_impl->value)
	         : nullptr;
}

const lox::callable *lox::object::get_callable() const
{
	return std::holds_alternative<lox::callable>(_impl->value)
	         ? &std::get<lox::callable>(_impl->value)
	         : (std::holds_alternative<lox::type>(_impl->value)
	              ? &std::get<lox::type>(_impl->value).constructor()
	              : nullptr);
}

const lox::type *lox::object::get_type() const
{
	return std::holds_alternative<lox::type>(_impl->value)
	         ? &std::get<lox::type>(_impl->value)
	         : nullptr;
}

lox::instance *lox::object::get_instance()
{
	return std::holds_alternative<lox::instance>(_impl->value)
	         ? &std::get<lox::instance>(_impl->value)
	         : nullptr;
}

const lox::instance *lox::object::get_instance() const
{
	return std::holds_alternative<lox::instance>(_impl->value)
	         ? &std::get<lox::instance>(_impl->value)
	         : nullptr;
}

bool lox::object::operator==(const lox::object &rhs) const
{
	if (_impl->value.index() != rhs._impl->value.index()) return false;

	return visit(lox::overloaded{
	  [&](std::monostate) -> bool { return true; },
	  [&](const std::u8string &str) -> bool
	  { return str == std::get<std::u8string>(rhs._impl->value); },
	  [&](const double &number) -> bool
	  { return number == std::get<double>(rhs._impl->value); },
	  [&](const bool &b) -> bool
	  { return b == std::get<bool>(rhs._impl->value); },
	  [&](const lox::callable &c) -> bool
	  { return c == std::get<lox::callable>(rhs._impl->value); },
	  [&](const lox::type &t) -> bool
	  { return t == std::get<lox::type>(rhs._impl->value); },
	  [&](const lox::instance &i) -> bool
	  { return i == std::get<lox::instance>(rhs._impl->value); },
	});
}

bool lox::object::operator!=(const lox::object &rhs) const
{
	return !operator==(rhs);
}

std::u8string lox::object::to_string() const
{
	return visit(lox::overloaded{
	  [&](std::monostate) -> std::u8string { return u8"nil"; },
	  [&](const std::u8string &str) -> std::u8string
	  { return u8"\"" + str + u8"\""; },
	  [&](const double &number) -> std::u8string
	  { return std::u8string(lox::as_u8string_view(std::to_string(number))); },
	  [&](const bool &b) -> std::u8string { return b ? u8"true" : u8"false"; },
	  [&](const lox::callable &c) -> std::u8string { return c.to_string(); },
	  [&](const lox::type &t) -> std::u8string { return t.to_string(); },
	  [&](const lox::instance &i) -> std::u8string { return i.to_string(); },
	});
}
