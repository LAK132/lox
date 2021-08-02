#include "string_hacks.hpp"

std::string_view lox::as_astring_view(std::u8string_view str)
{
	return std::string_view(reinterpret_cast<const char *>(str.data()),
	                        str.size());
}

std::u8string_view lox::as_u8string_view(std::string_view str)
{
	return std::u8string_view(reinterpret_cast<const char8_t *>(str.data()),
	                          str.size());
}
