#ifndef LOX_VIRTUAL_MACHINE_HPP
#define LOX_VIRTUAL_MACHINE_HPP

#include "chunk.hpp"
#include "common.hpp"
#include "compiler.hpp"
#include "value.hpp"

#include <lak/array.hpp>
#include <lak/binary_reader.hpp>
#include <lak/memory.hpp>
#include <lak/result.hpp>

namespace lox
{
#define LOX_INTERPRET_ERROR_FOREACH(MACRO, ...)                               \
	EXPAND(MACRO(INTERPRET_COMPILE_ERROR, __VA_ARGS__))                         \
	EXPAND(MACRO(INTERPRET_RUNTIME_ERROR, __VA_ARGS__))

	enum struct interpret_error
	{
#define LOX_INTERPRET_ERROR_ENUM(ERR, ...) ERR,
		LOX_INTERPRET_ERROR_FOREACH(LOX_INTERPRET_ERROR_ENUM)
#undef LOX_INTERPRET_ERROR_ENUM
	};

	lak::u8string_view to_string(lox::interpret_error err);

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

		lox::interpret_result<> interpret(lox::chunk *c);

		lox::interpret_result<> interpret(lak::u8string_view file);

		lox::interpret_result<> run();

		lox::interpret_result<> run_file(const std::filesystem::path &file_path);

		lox::interpret_result<> run_prompt();
	};
}

#endif
