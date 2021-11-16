#include "interpreter.hpp"

#include "callable.hpp"
#include "evaluator.hpp"
#include "lox.hpp"
#include "overloaded.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "scanner.hpp"
#include "string_hacks.hpp"

#include <cassert>
#include <chrono>
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
	std::optional<lox::object> result = expr.visit(lox::evaluator(*this));
	return result ? result->to_string() : u8"";
}

std::optional<std::u8string> lox::interpreter::interpret(const lox::stmt &stmt)
{
	return stmt.visit(lox::evaluator(*this));
}

std::optional<std::u8string> lox::interpreter::interpret(
  std::span<const lox::stmt_ptr> stmts)
{
	std::u8string str;

	for (const auto &stmt : stmts)
	{
		if (!stmt) return std::nullopt;

		std::optional<std::u8string> result = interpret(*stmt);
		if (had_error || !result) return std::nullopt;
		str += *result;
	}

	return str;
}

std::optional<lox::object> (*lox_clock)(lox::interpreter &) =
  [](lox::interpreter &) -> std::optional<lox::object> {
	return lox::object{std::chrono::duration_cast<std::chrono::milliseconds>(
	                     std::chrono::system_clock::now().time_since_epoch())
	                     .count() /
	                   1000.0};
};

std::optional<lox::object> (*lox_to_string)(lox::interpreter &,
                                            lox::object &&) =
  [](lox::interpreter &, lox::object &&obj) -> std::optional<lox::object> {
	return lox::object{obj.to_string()};
};

void lox::interpreter::init_globals()
{
	global_environment = lox::environment::make();

	global_environment->emplace(
	  u8"clock",
	  lox::object{
	    std::make_shared<lox::callable>(LOX_CALLABLE_MAKE_NATIVE(lox_clock)),
	  });

	global_environment->emplace(
	  u8"to_string",
	  lox::object{
	    std::make_shared<lox::callable>(LOX_CALLABLE_MAKE_NATIVE(lox_to_string)),
	  });
}

int lox::interpreter::run(std::u8string_view file, std::u8string *out_str)
{
	assert(global_environment);

	lox::scanner scanner{*this, file};
	auto tokens = scanner.scan_tokens();
	if (had_error) return EXIT_FAILURE;

	lox::parser parser{*this, std::move(tokens)};

	std::vector<lox::stmt_ptr> stmts = parser.parse();
	if (had_error) return EXIT_FAILURE;

	std::optional<std::u8string> result = interpret(stmts);
	if (had_error || !result) return EXIT_FAILURE;

	if (out_str) *out_str += *result;

	return EXIT_SUCCESS;
}

int lox::interpreter::run_file(const std::filesystem::path &file_path)
{
	init_globals();
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
	init_globals();
	for (bool running = true; running;)
	{
		std::cout << "> ";
		std::string string;
		std::getline(std::cin, string);
		if (!std::cin.good()) break;
		if (string.empty()) break;
		string += "\n";
		std::u8string out;
		run(lox::as_u8string_view(string), &out);
		std::cout << lox::as_astring_view(out);
		had_error = false;
	}
	std::cout << "\n";
	return 0;
}
