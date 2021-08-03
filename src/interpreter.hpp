#ifndef LOX_INTERPRETER_HPP
#define LOX_INTERPRETER_HPP

#include "environment.hpp"
#include "expr.hpp"
#include "stmt.hpp"
#include "token.hpp"

#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace lox
{
	struct interpreter
	{
		bool had_error = false;

		lox::environment_ptr global_environment;

		void report(size_t line,
		            std::u8string_view where,
		            std::u8string_view message);

		void error(const lox::token &token, std::u8string_view message);

		void error(size_t line, std::u8string_view message);

		std::u8string interpret(const lox::expr &expr);

		std::optional<std::u8string> interpret(const lox::stmt &stmt);

		std::optional<std::u8string> interpret(
		  std::span<const lox::stmt_ptr> stmts);

		int run(std::u8string_view file, std::u8string *out_str = nullptr);

		int run_file(const std::filesystem::path &file_path);

		int run_prompt();
	};
}

#endif
