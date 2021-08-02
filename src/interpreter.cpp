#include "interpreter.hpp"

#include "lox.hpp"
#include "overloaded.hpp"
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

void lox::interpreter::error(size_t line, std::u8string_view message)
{
	report(line, u8"", message);
}

int lox::interpreter::run(std::u8string_view file)
{
	lox::scanner scanner{*this, file};
	auto tokens = scanner.scan_tokens();
	if (had_error) return EXIT_FAILURE;

	for (const auto &token : tokens) std::cout << token << "\n";

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
