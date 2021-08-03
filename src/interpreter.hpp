#ifndef LOX_INTERPRETER_HPP
#define LOX_INTERPRETER_HPP

#include "token.hpp"

#include <filesystem>
#include <string_view>

namespace lox
{
	struct interpreter
	{
		bool had_error = false;

		void report(size_t line,
		            std::u8string_view where,
		            std::u8string_view message);

		void error(const lox::token &token, std::u8string_view message);

		void error(size_t line, std::u8string_view message);

		int run(std::u8string_view file);

		int run_file(const std::filesystem::path &file_path);

		int run_prompt();
	};
}

#endif
