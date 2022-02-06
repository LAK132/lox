#ifndef LOX_TOKEN_HPP
#define LOX_TOKEN_HPP

#include "value.hpp"

#include <lak/stdint.hpp>
#include <lak/string_ostream.hpp>
#include <lak/string_view.hpp>

namespace lox
{
#undef TRUE
#undef FALSE
#define LOX_TOKEN_TYPE_FOREACH(MACRO)                                         \
	/* single charcter tokens */                                                \
	MACRO(LEFT_PAREN)                                                           \
	MACRO(RIGHT_PAREN)                                                          \
	MACRO(LEFT_BRACE)                                                           \
	MACRO(RIGHT_BRACE)                                                          \
	MACRO(COMMA)                                                                \
	MACRO(DOT)                                                                  \
	MACRO(MINUS)                                                                \
	MACRO(PLUS)                                                                 \
	MACRO(SEMICOLON)                                                            \
	MACRO(SLASH)                                                                \
	MACRO(STAR)                                                                 \
	/* one or two charcter tokens */                                            \
	MACRO(BANG)                                                                 \
	MACRO(BANG_EQUAL)                                                           \
	MACRO(EQUAL)                                                                \
	MACRO(EQUAL_EQUAL)                                                          \
	MACRO(GREATER)                                                              \
	MACRO(GREATER_EQUAL)                                                        \
	MACRO(LESS)                                                                 \
	MACRO(LESS_EQUAL)                                                           \
	/* literals */                                                              \
	MACRO(IDENTIFIER)                                                           \
	MACRO(STRING)                                                               \
	MACRO(NUMBER)                                                               \
	/* keywords */                                                              \
	MACRO(AND)                                                                  \
	MACRO(CLASS)                                                                \
	MACRO(ELSE)                                                                 \
	MACRO(FALSE)                                                                \
	MACRO(FUN)                                                                  \
	MACRO(FOR)                                                                  \
	MACRO(IF)                                                                   \
	MACRO(NIL)                                                                  \
	MACRO(OR)                                                                   \
	MACRO(PRINT)                                                                \
	MACRO(RETURN)                                                               \
	MACRO(SUPER)                                                                \
	MACRO(THIS)                                                                 \
	MACRO(TRUE)                                                                 \
	MACRO(VAR)                                                                  \
	MACRO(WHILE)                                                                \
	MACRO(ERROR_TOK)                                                            \
	MACRO(EOF_TOK)

	enum struct token_type
	{
#define LOX_GEN_TOKEN_TYPE(TOKEN) TOKEN,
		LOX_TOKEN_TYPE_FOREACH(LOX_GEN_TOKEN_TYPE)
#undef LOX_GEN_TOKEN_TYPE
	};

	lak::u8string_view to_string(token_type type);

	struct token
	{
		lox::token_type type;
		lak::u8string_view lexeme;
		lox::value literal;
		size_t line = 1;

		friend inline std::ostream &operator<<(std::ostream &strm,
		                                       const lox::token &token)
		{
			using lak::operator<<;
			strm << lox::to_string(token.type) << " " << token.lexeme << " "
			     << lox::to_string(token.literal);
		}
	};
}

#endif
