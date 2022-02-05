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

lox::scanner::scanner(lox::interpreter &interp, lak::u8string_view src)
: interpreter(interp), source(src)
{
}

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

void lox::scanner::add_token(lox::token_type type, lox::object literal)
{
	tokens.push_back(lox::token{
	  .type    = type,
	  .lexeme  = source.substr(start, current - start),
	  .literal = lak::move(literal),
	  .line    = line,
	});
}

void lox::scanner::scan_string()
{
	while (peek() != '"' && !empty())
	{
		if (peek() == '\n') ++line;
		next();
	}

	if (empty())
	{
		interpreter.error(line, u8"Unterminated string.");
		return;
	}

	// the closing "
	next();

	// trim the surrounding quotes
	auto value = source.substr(start + 1, (current - 1) - (start + 1));
	add_token(lox::token_type::STRING, lox::object{value.to_string()});
}

void lox::scanner::scan_number()
{
	while (lak::is_alphanumeric(peek())) next();

	if (peek() == '.' && lak::is_alphanumeric(peek_next()))
	{
		// consume the .
		next();
		while (lak::is_alphanumeric(peek())) next();
	}

	double number;

	auto result =
	  std::from_chars(reinterpret_cast<const char *>(source.data()) + start,
	                  reinterpret_cast<const char *>(source.data()) + current,
	                  number);

	if (result.ec == std::errc())
		// no error
		add_token(lox::token_type::NUMBER, lox::object{number});
	else
		interpreter.error(line, u8"Invalid number.");
}

void lox::scanner::scan_identifier()
{
	while (lox::is_ident_char(peek())) next();
	if (auto iter =
	      lox::keywords.find(source.substr(start, current - start).to_string());
	    iter != lox::keywords.end())
		add_token(iter->second);
	else
		add_token(lox::token_type::IDENTIFIER);
}

void lox::scanner::scan_token()
{
	auto c = next();
	switch (c)
	{
		case u8'(': add_token(lox::token_type::LEFT_PAREN); break;
		case u8')': add_token(lox::token_type::RIGHT_PAREN); break;
		case u8'{': add_token(lox::token_type::LEFT_BRACE); break;
		case u8'}': add_token(lox::token_type::RIGHT_BRACE); break;
		case u8',': add_token(lox::token_type::COMMA); break;
		case u8'.': add_token(lox::token_type::DOT); break;
		case u8'-': add_token(lox::token_type::MINUS); break;
		case u8'+': add_token(lox::token_type::PLUS); break;
		case u8';': add_token(lox::token_type::SEMICOLON); break;
		case u8'*': add_token(lox::token_type::STAR); break;

		case u8'!':
			add_token(match(u8'=') ? lox::token_type::BANG_EQUAL
			                       : lox::token_type::BANG);
			break;

		case u8'=':
			add_token(match(u8'=') ? lox::token_type::EQUAL_EQUAL
			                       : lox::token_type::EQUAL);
			break;

		case u8'<':
			add_token(match(u8'=') ? lox::token_type::LESS_EQUAL
			                       : lox::token_type::LESS);
			break;

		case u8'>':
			add_token(match(u8'=') ? lox::token_type::GREATER_EQUAL
			                       : lox::token_type::GREATER);
			break;

		case u8'/':
			if (match('/'))
				// a comment goes until the end of the line
				while (peek() != u8'\n' && !empty()) next();
			else
				add_token(lox::token_type::SLASH);
			break;

		case u8' ': [[fallthrough]];
		case u8'\r': [[fallthrough]];
		case u8'\t':
			// ignore whitespace
			break;

		case u8'\n': ++line; break;

		case u8'"': scan_string(); break;

		case u8'o':
			if (match(u8'r')) add_token(lox::token_type::OR);
			break;

		default:
			if (lak::is_alphanumeric(c))
				scan_number();
			else if (lox::is_latin_letter(c))
				scan_identifier();
			else
				interpreter.error(line, u8"Unexpected character.");
			break;
	}
}

std::vector<lox::token> lox::scanner::scan_tokens()
{
	while (!empty())
	{
		start = current;
		scan_token();
	}

	tokens.push_back(lox::token{
	  .type    = lox::token_type::EOF_TOK,
	  .lexeme  = u8"",
	  .literal = lox::object{},
	  .line    = line,
	});

	return tokens;
}
