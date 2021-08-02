#include "token.hpp"

std::u8string_view lox::token_type_name(lox::token_type type)
{
	switch (type)
	{
#define _GEN_TOKEN_TYPE(TOKEN)                                                \
	case token_type::TOKEN: return u8"" #TOKEN;
		TOKEN_TYPE(_GEN_TOKEN_TYPE)
#undef _GEN_TOKEN_TYPE
		default: std::abort();
	}
}

std::ostream &lox::operator<<(std::ostream &strm, const lox::token_type &type)
{
	return strm << lox::as_astring_view(lox::token_type_name(type));
}
