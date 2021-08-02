#ifndef LOX_SCANNER_HPP
#define LOX_SCANNER_HPP

#include "interpreter.hpp"
#include "object.hpp"
#include "token.hpp"

#include <string_view>
#include <unordered_map>
#include <vector>

namespace lox
{
	bool is_digit(char8_t c);

	bool is_alpha(char8_t c);

	bool is_alphanumeric(char8_t c);

	const std::unordered_map<std::u8string, lox::token_type> keywords = {
	  {u8"and", lox::token_type::AND},
	  {u8"class", lox::token_type::CLASS},
	  {u8"else", lox::token_type::ELSE},
	  {u8"false", lox::token_type::FALSE},
	  {u8"for", lox::token_type::FOR},
	  {u8"fun", lox::token_type::FUN},
	  {u8"if", lox::token_type::IF},
	  {u8"nil", lox::token_type::NIL},
	  {u8"or", lox::token_type::OR},
	  {u8"print", lox::token_type::PRINT},
	  {u8"return", lox::token_type::RETURN},
	  {u8"super", lox::token_type::SUPER},
	  {u8"this", lox::token_type::THIS},
	  {u8"true", lox::token_type::TRUE},
	  {u8"var", lox::token_type::VAR},
	  {u8"while", lox::token_type::WHILE},
	};

	struct scanner
	{
		lox::interpreter &interpreter;
		std::u8string_view source;
		std::vector<lox::token> tokens;
		size_t start   = 0;
		size_t current = 0;
		size_t line    = 1;

		scanner(lox::interpreter &interp, std::u8string_view src);

		bool empty() const;

		char8_t next();

		char8_t peek() const;

		char8_t peek_next() const;

		bool match(char8_t expected);

		void add_token(lox::token_type type, lox::object literal = lox::object{});

		void scan_string();

		void scan_number();

		void scan_identifier();

		void scan_token();

		std::vector<lox::token> scan_tokens();
	};

}

#endif
