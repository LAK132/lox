#ifndef LOX_INTERPRETER_HPP
#define LOX_INTERPRETER_HPP

#include "environment.hpp"
#include "expr.hpp"
#include "stmt.hpp"
#include "token.hpp"

#include <lak/result.hpp>

#include <filesystem>
#include <source_location>
#include <unordered_map>

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

		std::vector<std::vector<char8_t>> sources;

		void report(
		  size_t line,
		  lak::u8string_view where,
		  lak::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		void error(
		  const lox::token &token,
		  lak::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		void error(
		  size_t line,
		  lak::u8string_view message,
		  const std::source_location srcloc = std::source_location::current());

		lak::result<lox::object> evaluate(const lox::expr &expr);

		lak::result<lak::monostate> execute(const lox::stmt &stmt);

		lak::result<lox::object> execute_block(
		  lak::span<const lox::stmt_ptr> stmts, const lox::environment_ptr &env);

		void resolve(const lox::expr::variable &expr, size_t distance);
		void resolve(const lox::expr::assign &expr, size_t distance);
		void resolve(const lox::expr::super_keyword &expr, size_t distance);
		void resolve(const lox::expr::this_keyword &expr, size_t distance);

		lak::result<size_t> find(const lox::expr::variable &expr);
		lak::result<size_t> find(const lox::expr::assign &expr);
		lak::result<size_t> find(const lox::expr::super_keyword &expr);
		lak::result<size_t> find(const lox::expr::this_keyword &expr);

		lak::result<std::vector<lox::stmt_ptr>> parse(lak::u8string_view file);
		lak::result<std::vector<lox::stmt_ptr>> parse_file(
		  const std::filesystem::path &file);

		lak::u8string interpret(const lox::expr &expr);

		lak::result<lak::u8string> interpret(const lox::stmt &stmt);

		lak::result<lak::u8string> interpret(lak::span<const lox::stmt_ptr> stmts);

		interpreter &init_globals();

		lak::result<> run(lak::u8string_view file,
		                  lak::u8string *out_str = nullptr);

		lak::result<> run_file(const std::filesystem::path &file_path);

		lak::result<> run_prompt();
	};
}

#endif
