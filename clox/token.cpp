#include "token.hpp"

#include <lak/string_literals.hpp>
#include <lak/string_ostream.hpp>

lak::u8string_view lox::to_string(lox::token_type type)
{
	switch (type)
	{
#define LOX_GEN_TOKEN_TYPE(TOKEN)                                             \
	case token_type::TOKEN: return u8"" #TOKEN ""_view;
		LOX_TOKEN_TYPE_FOREACH(LOX_GEN_TOKEN_TYPE)
#undef LOX_GEN_TOKEN_TYPE
		default: FATAL("Invalid token type");
	}
}
