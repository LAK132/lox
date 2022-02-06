#ifndef LOX_VIRTUAL_MACHINE_HPP
#define LOX_VIRTUAL_MACHINE_HPP

#include "chunk.hpp"
#include "common.hpp"
#include "compiler.hpp"
#include "error.hpp"
#include "value.hpp"

#include <lak/array.hpp>
#include <lak/binary_reader.hpp>
#include <lak/memory.hpp>
#include <lak/result.hpp>

namespace lox
{
	struct runtime_error_tag
	{
	};
	using runtime_error = lox::positional_error<lox::runtime_error_tag>;

	using interpret_error = lox::result_set<lox::scan_error,
	                                        lox::parse_error,
	                                        lox::compile_error,
	                                        lox::runtime_error>;

	template<typename T = lak::monostate>
	using interpret_result = lak::result<T, lox::interpret_error>;

	struct virtual_machine
	{
		lox::chunk *chunk{nullptr};

		lak::binary_reader ip;

		lak::array<lox::value, LOX_STACK_MAX> stack;
		size_t stack_top{0U};

		lak::result<> stack_push(lox::value v);
		lak::result<lox::value> stack_pop();
		lak::result<const lox::value &> stack_peek(size_t depth) const;

		lox::interpret_result<> interpret(lox::chunk *c);

		lox::interpret_result<> interpret(lak::u8string_view file);

		lox::interpret_result<> run();

		using run_file_error  = lox::result_set<lak::errno_error,
                                           lox::scan_error,
                                           lox::parse_error,
                                           lox::compile_error,
                                           lox::runtime_error>;
		using run_file_result = lak::result<lak::monostate, run_file_error>;
		run_file_result run_file(const std::filesystem::path &file_path);

		lox::interpret_result<> run_prompt();
	};
}

#endif
