#include "scanner.hpp"

#include <lak/char_utils.hpp>

bool lox::is_latin_letter(char8_t c)
{
	return (c >= u8'a' && c <= u8'z') || (c >= u8'A' && c <= u8'Z') ||
	       c == u8'_';
}

bool lox::is_ident_char(char8_t c)
{
	return lak::is_alphanumeric(c) || lox::is_latin_letter(c);
}

lox::scanner::scanner(lak::u8string_view src) : source(src) {}

bool lox::scanner::empty() const
{
	return current >= source.size();
}

char8_t lox::scanner::next()
{
	return source[current++];
}

char8_t lox::scanner::peek() const
{
	return empty() ? u8'\0' : source[current];
}

char8_t lox::scanner::peek_next() const
{
	return current + 1 >= source.size() ? u8'\0' : source[current + 1];
}

bool lox::scanner::match(char8_t expected)
{
	if (empty() || peek() != expected) return false;
	++current;
	return true;
}

lak::ok_t<lox::token> lox::scanner::build_token(lox::token_type type,
                                                lox::value literal)
{
	return {{
	  .type    = type,
	  .lexeme  = source.substr(start, current - start),
	  .literal = lak::move(literal),
	  .line    = line,
	}};
}

lox::scan_result<lox::token> lox::scanner::scan_string()
{
	while (peek() != '"' && !empty())
	{
		if (peek() == '\n') ++line;
		next();
	}

	if (empty())
	{
		return lak::err_t{lox::scan_error(line, u8"Unterminated string."_str)};
	}

	// the closing "
	next();

	// trim the surrounding quotes
	auto value = source.substr(start + 1, (current - 1) - (start + 1));
	return build_token(lox::token_type::STRING,
	                   lox::value{/*value.to_string()*/});
}

lox::scan_result<lox::token> lox::scanner::scan_number()
{
	while (lak::is_alphanumeric(peek())) next();

	if (peek() == '.' && lak::is_alphanumeric(peek_next()))
	{
		// consume the .
		next();
		while (lak::is_alphanumeric(peek())) next();
	}

	auto u8str{
	  lak::u8string_view{source.data() + start, source.data() + current}};

	auto str{lak::as_astring(u8str)};

	double number;

	auto result = std::from_chars(str.begin(), str.end(), number);

	if (result.ec == std::errc())
		// no error
		return build_token(lox::token_type::NUMBER, lox::value{number});
	else
		return lak::err_t{lox::scan_error(
		  line, u8"Invalid number. '"_str + lak::u8string(u8str) + u8"'"_str)};
}

lox::scan_result<lox::token> lox::scanner::scan_identifier()
{
	while (lox::is_ident_char(peek())) next();
	if (auto iter =
	      lox::keywords.find(source.substr(start, current - start).to_string());
	    iter != lox::keywords.end())
		return build_token(iter->second);
	else
		return build_token(lox::token_type::IDENTIFIER);
}

lox::scan_result<lox::token> lox::scanner::scan_token()
{
	while (!empty())
	{
		start = current;
		auto c{next()};
		switch (c)
		{
			case u8'(': return build_token(lox::token_type::LEFT_PAREN);
			case u8')': return build_token(lox::token_type::RIGHT_PAREN);
			case u8'{': return build_token(lox::token_type::LEFT_BRACE);
			case u8'}': return build_token(lox::token_type::RIGHT_BRACE);
			case u8',': return build_token(lox::token_type::COMMA);
			case u8'.': return build_token(lox::token_type::DOT);
			case u8'-': return build_token(lox::token_type::MINUS);
			case u8'+': return build_token(lox::token_type::PLUS);
			case u8';': return build_token(lox::token_type::SEMICOLON);
			case u8'*': return build_token(lox::token_type::STAR);

			case u8'!':
				return build_token(match(u8'=') ? lox::token_type::BANG_EQUAL
				                                : lox::token_type::BANG);

			case u8'=':
				return build_token(match(u8'=') ? lox::token_type::EQUAL_EQUAL
				                                : lox::token_type::EQUAL);

			case u8'<':
				return build_token(match(u8'=') ? lox::token_type::LESS_EQUAL
				                                : lox::token_type::LESS);

			case u8'>':
				return build_token(match(u8'=') ? lox::token_type::GREATER_EQUAL
				                                : lox::token_type::GREATER);

			case u8'/':
				if (match('/'))
				// a comment goes until the end of the line
				{
					while (peek() != u8'\n' && !empty()) next();
					continue;
				}
				else
					return build_token(lox::token_type::SLASH);

			case u8' ': [[fallthrough]];
			case u8'\r': [[fallthrough]];
			case u8'\t':
				// ignore whitespace
				continue;

			case u8'\n': ++line; continue;

			case u8'"': return scan_string();

			case u8'o':
				if (match(u8'r'))
					return build_token(lox::token_type::OR);
				else
					break;

			default:
				if (lak::is_alphanumeric(c))
					return scan_number();
				else if (lox::is_latin_letter(c))
					return scan_identifier();
				else
					break;
		}

		return lak::err_t{
		  lox::scan_error::at(line, u8"Unexpected character."_str)};
	}

	return lak::ok_t{lox::token{
	  .type    = lox::token_type::EOF_TOK,
	  .lexeme  = u8"",
	  .literal = lox::value{},
	  .line    = line,
	}};
}

lox::scan_result<std::vector<lox::token>> lox::scanner::scan_tokens()
{
	std::vector<lox::token> tokens;

	while (!empty())
	{
		RES_TRY_ASSIGN(lox::token token =, scan_token());
		tokens.push_back(token);
	}

	return lak::ok_t<std::vector<lox::token>>{tokens};
}
