#ifndef LOX_SCANNER_HPP
#define LOX_SCANNER_HPP

#include "token.hpp"
#include "value.hpp"

#include <lak/result.hpp>
#include <lak/string_view.hpp>

#include <unordered_map>
#include <vector>

namespace lox
{
	bool is_latin_letter(char8_t c);

	bool is_ident_char(char8_t c);

	const std::unordered_map<lak::u8string, lox::token_type> keywords = {
	  {u8"and"_str, lox::token_type::AND},
	  {u8"class"_str, lox::token_type::CLASS},
	  {u8"else"_str, lox::token_type::ELSE},
	  {u8"false"_str, lox::token_type::FALSE},
	  {u8"for"_str, lox::token_type::FOR},
	  {u8"fun"_str, lox::token_type::FUN},
	  {u8"if"_str, lox::token_type::IF},
	  {u8"nil"_str, lox::token_type::NIL},
	  {u8"or"_str, lox::token_type::OR},
	  {u8"print"_str, lox::token_type::PRINT},
	  {u8"return"_str, lox::token_type::RETURN},
	  {u8"super"_str, lox::token_type::SUPER},
	  {u8"this"_str, lox::token_type::THIS},
	  {u8"true"_str, lox::token_type::TRUE},
	  {u8"var"_str, lox::token_type::VAR},
	  {u8"while"_str, lox::token_type::WHILE},
	};

	struct positional_error
	{
		size_t line;
		lak::u8string message;
	};

	template<typename T = lak::monostate>
	using scan_result = lak::result<T, lox::positional_error>;

	struct scanner
	{
		lak::u8string_view source;
		size_t start   = 0;
		size_t current = 0;
		size_t line    = 1;

		scanner(lak::u8string_view src);

		bool empty() const;

		char8_t next();

		char8_t peek() const;

		char8_t peek_next() const;

		bool match(char8_t expected);

		lak::ok_t<lox::token> build_token(lox::token_type type,
		                                  lox::value literal = lox::value{});

		lox::scan_result<lox::token> scan_string();

		lox::scan_result<lox::token> scan_number();

		lox::scan_result<lox::token> scan_identifier();

		lox::scan_result<lox::token> scan_token();

		lox::scan_result<std::vector<lox::token>> scan_tokens();
	};

}

#endif
