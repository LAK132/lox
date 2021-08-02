#ifndef LOX_OBJECT_HPP
#define LOX_OBJECT_HPP

#include "overloaded.hpp"
#include "string_hacks.hpp"

#include <iostream>
#include <string_view>
#include <variant>

namespace lox
{
	struct object
	{
		std::variant<std::monostate, std::u8string_view, double> value;

		friend inline std::ostream &operator<<(std::ostream &strm,
		                                       const object &obj)
		{
			return std::visit(
			  lox::overloaded{
			    [&](std::monostate) -> std::ostream & { return strm; },
			    [&](std::u8string_view str) -> std::ostream & {
				    return strm << "`" << lox::as_astring_view(str) << "`";
			    },
			    [&](const double &number) -> std::ostream & {
				    return strm << number;
			    },
			  },
			  obj.value);
		}
	};

}

#endif
