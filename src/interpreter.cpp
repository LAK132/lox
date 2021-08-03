#include "interpreter.hpp"

#include "evaluator.hpp"
#include "lox.hpp"
#include "overloaded.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "scanner.hpp"
#include "string_hacks.hpp"

#include <iostream>

void lox::interpreter::report(size_t line,
                              std::u8string_view where,
                              std::u8string_view message)
{
	std::cerr << "[line " << line << "] Error" << lox::as_astring_view(where)
	          << ": " << lox::as_astring_view(message) << "\n";
	had_error = true;
}

void lox::interpreter::error(const lox::token &token,
                             std::u8string_view message)
{
	if (token.type == lox::token_type::EOF_TOK)
		report(token.line, u8" at end", message);
	else
		report(
		  token.line, u8" at '" + std::u8string(token.lexeme) + u8"'", message);
}

void lox::interpreter::error(size_t line, std::u8string_view message)
{
	report(line, u8"", message);
}

std::u8string lox::interpreter::interpret(const lox::expr &expr)
{
	std::optional<lox::object> result = expr.visit(lox::evaluator{*this});
	return result ? result->to_string() : u8"";
}

int lox::interpreter::run(std::u8string_view file)
{
	lox::scanner scanner{*this, file};
	auto tokens = scanner.scan_tokens();
	if (had_error) return EXIT_FAILURE;

	lox::parser parser{*this, std::move(tokens)};

	while (!parser.empty())
	{
		lox::expr_ptr expr = parser.parse();
		if (had_error || !expr) return EXIT_FAILURE;

		std::u8string result = interpret(*expr);
		if (had_error) return EXIT_FAILURE;

		std::cout << lox::as_astring_view(result) << "\n";
	}

	return EXIT_SUCCESS;
}

int lox::interpreter::run_file(const std::filesystem::path &file_path)
{
	return std::visit(
	  overloaded{[&](const std::error_code &err) -> int
	             {
		             std::cerr << "Failed to read file '" << file_path
		                       << "': " << err << "\n";
		             return EXIT_FAILURE;
	             },
	             [&](const std::u8string &str) -> int { return run(str); }},
	  lox::read_file(file_path));
}

int lox::interpreter::run_prompt()
{
	for (bool running = true; running;)
	{
		std::cout << "> ";
		std::string string;
		std::getline(std::cin, string);
		if (!std::cin.good()) break;
		if (string.empty()) break;
		string += "\n";
		run(lox::as_u8string_view(string));
		had_error = false;
	}
	std::cout << "\n";
	return 0;
}
