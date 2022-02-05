#ifndef LOX_VIRTUAL_MACHINE_HPP
#define LOX_VIRTUAL_MACHINE_HPP

#include "chunk.hpp"
#include "common.hpp"
#include "value.hpp"

#include <lak/array.hpp>
#include <lak/binary_reader.hpp>
#include <lak/memory.hpp>
#include <lak/result.hpp>

namespace lox
{
	enum struct interpret_error
	{
		INTERPRET_COMPILE_ERROR,
		INTERPRET_RUNTIME_ERROR,
	};

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

		lox::interpret_result<> run();
	};
}

#endif
