#ifndef LOX_VALUE_HPP
#define LOX_VALUE_HPP

#include <lak/string.hpp>

#include <vector>

namespace lox
{
	using value = double;

	using value_array = std::vector<lox::value>;

	lak::u8string to_string(const lox::value &v);
}

#endif
