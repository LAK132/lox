#include "token.hpp"

#include <lak/string_literals.hpp>
#include <lak/string_ostream.hpp>

lak::u8string_view lox::token_type_name(lox::token_type type)
{
	switch (type)
	{
#define _GEN_TOKEN_TYPE(TOKEN)                                                \
	case token_type::TOKEN: return u8"" #TOKEN ""_view;
		TOKEN_TYPE(_GEN_TOKEN_TYPE)
#undef _GEN_TOKEN_TYPE
		default: std::abort();
	}
}

std::ostream &lox::operator<<(std::ostream &strm, const lox::token_type &type)
{
	return strm << lox::token_type_name(type);
}
