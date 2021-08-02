#ifndef LOX_STRING_HACKS_HPP
#define LOX_STRING_HACKS_HPP

#include <string_view>

namespace lox
{
	std::string_view as_astring_view(std::u8string_view str);

	std::u8string_view as_u8string_view(std::string_view str);
}

#endif
