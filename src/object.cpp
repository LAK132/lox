#include "object.hpp"

#include "string_hacks.hpp"

#include <string>

std::u8string lox::object::to_string() const
{
	return visit(lox::overloaded{
	  [&](std::monostate) -> std::u8string { return u8"nil"; },
	  [&](std::u8string_view str) -> std::u8string
	  { return std::u8string(u8"`") + std::u8string(str) + u8"`"; },
	  [&](const double &number) -> std::u8string
	  { return std::u8string(lox::as_u8string_view(std::to_string(number))); },
	  [&](const bool &b) -> std::u8string { return b ? u8"true" : u8"false"; },
	});
}
