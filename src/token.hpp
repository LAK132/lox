#ifndef LOX_TOKEN_HPP
#define LOX_TOKEN_HPP

#include "object.hpp"

#include <lak/string_ostream.hpp>
#include <lak/string_view.hpp>

#include <cstdint>
#include <iostream>

namespace lox
{
#define TOKEN_TYPE(MACRO)                                                     \
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
	MACRO(EOF_TOK)

	enum struct token_type
	{
#define _GEN_TOKEN_TYPE(TOKEN) TOKEN,
		TOKEN_TYPE(_GEN_TOKEN_TYPE)
#undef _GEN_TOKEN_TYPE
	};

	lak::u8string_view token_type_name(token_type type);

	std::ostream &operator<<(std::ostream &strm, const lox::token_type &type);

	struct token
	{
		lox::token_type type;
		lak::u8string_view lexeme;
		lox::object literal;
		size_t line = 1;

		friend inline std::ostream &operator<<(std::ostream &strm,
		                                       const lox::token &token)
		{
			strm << token.type << " " << token.lexeme << " " << token.literal;
		}
	};
}

#endif
