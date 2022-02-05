#include "value.hpp"

#include <lak/streamify.hpp>

lak::u8string lox::to_string(const lox::value &v)
{
	return lak::streamify(v);
}
