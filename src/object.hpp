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
		std::variant<std::monostate, std::u8string, double, bool> value;

		std::u8string to_string() const;

		bool is_truthy() const;

		const std::u8string *get_string() const;

		const double *get_number() const;

		const bool *get_bool() const;

		bool operator==(const lox::object &rhs) const;

		bool operator!=(const lox::object &rhs) const;

		template<typename F>
		auto visit(F &&f)
		{
			return std::visit(f, value);
		}

		template<typename F>
		auto visit(F &&f) const
		{
			return std::visit(f, value);
		}

		friend inline std::ostream &operator<<(std::ostream &strm,
		                                       const object &obj)
		{
			return strm << lox::as_astring_view(obj.to_string());
		}
	};

}

#endif
