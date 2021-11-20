#ifndef LOX_INTERPRETER_HPP
#define LOX_INTERPRETER_HPP

#include "environment.hpp"
#include "expr.hpp"
#include "stmt.hpp"
#include "token.hpp"

#include <filesystem>
#include <source_location>
#include <span>
#include <string>
#include <string_view>

namespace lox
{
	struct interpreter
	{
		bool had_error = false;

		lox::environment_ptr global_environment;
		std::unordered_map<const lox::expr::variable *, size_t> local_declares;
		std::unordered_map<const lox::expr::assign *, size_t> local_assigns;
		std::unordered_map<const lox::expr::super_keyword *, size_t> local_super;
		std::unordered_map<const lox::expr::this_keyword *, size_t> local_this;

		void report(
		  size_t line,
		  std::u8string_view where,
		  std::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		void error(
		  const lox::token &token,
		  std::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		void error(
		  size_t line,
		  std::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		std::optional<lox::object> evaluate(const lox::expr &expr);

		std::optional<std::monostate> execute(const lox::stmt &stmt);

		std::optional<lox::object> execute_block(
		  std::span<const lox::stmt_ptr> stmts, const lox::environment_ptr &env);

		void resolve(const lox::expr::variable &expr, size_t distance);
		void resolve(const lox::expr::assign &expr, size_t distance);
		void resolve(const lox::expr::super_keyword &expr, size_t distance);
		void resolve(const lox::expr::this_keyword &expr, size_t distance);

		std::optional<size_t> find(const lox::expr::variable &expr);
		std::optional<size_t> find(const lox::expr::assign &expr);
		std::optional<size_t> find(const lox::expr::super_keyword &expr);
		std::optional<size_t> find(const lox::expr::this_keyword &expr);

		std::u8string interpret(const lox::expr &expr);

		std::optional<std::u8string> interpret(const lox::stmt &stmt);

		std::optional<std::u8string> interpret(
		  std::span<const lox::stmt_ptr> stmts);

		void init_globals();

		int run(std::u8string_view file, std::u8string *out_str = nullptr);

		int run_file(const std::filesystem::path &file_path);

		int run_prompt();
	};
}

#endif
