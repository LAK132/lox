#include "object.hpp"

#include "string_hacks.hpp"

#include <string>

bool lox::object::is_truthy() const
{
	return visit(lox::overloaded{
	  [&](std::monostate) -> bool { return false; },
	  [&](const bool &b) -> bool { return b; },
	  [&](auto &&) -> bool { return true; },
	});
}

const std::u8string *lox::object::get_string() const
{
	return std::holds_alternative<std::u8string>(value)
	         ? &std::get<std::u8string>(value)
	         : nullptr;
}

const double *lox::object::get_number() const
{
	return std::holds_alternative<double>(value) ? &std::get<double>(value)
	                                             : nullptr;
}

const bool *lox::object::get_bool() const
{
	return std::holds_alternative<bool>(value) ? &std::get<bool>(value)
	                                           : nullptr;
}

bool lox::object::operator==(const lox::object &rhs) const
{
	if (value.index() != rhs.value.index()) return false;
	return visit(lox::overloaded{
	  [&](std::monostate) -> bool { return true; },
	  [&](const std::u8string &str) -> bool
	  { return str == std::get<std::u8string>(rhs.value); },
	  [&](const double &number) -> bool
	  { return number == std::get<double>(rhs.value); },
	  [&](const bool &b) -> bool { return b == std::get<bool>(rhs.value); },
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
	});
}
