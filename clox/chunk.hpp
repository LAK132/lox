#ifndef LOX_CHUNK_HPP
#define LOX_CHUNK_HPP

#include "value.hpp"

#include <lak/stdint.hpp>
#include <lak/string_literals.hpp>
#include <lak/string_view.hpp>

#include <vector>

namespace lox
{
#define LOX_OPCODE_FOREACH(MACRO, ...)                                        \
	EXPAND(MACRO(OP_CONSTANT, __VA_ARGS__))                                     \
	EXPAND(MACRO(OP_NIL, __VA_ARGS__))                                          \
	EXPAND(MACRO(OP_TRUE, __VA_ARGS__))                                         \
	EXPAND(MACRO(OP_FALSE, __VA_ARGS__))                                        \
	EXPAND(MACRO(OP_EQUAL, __VA_ARGS__))                                        \
	EXPAND(MACRO(OP_GREATER, __VA_ARGS__))                                      \
	EXPAND(MACRO(OP_LESS, __VA_ARGS__))                                         \
	EXPAND(MACRO(OP_ADD, __VA_ARGS__))                                          \
	EXPAND(MACRO(OP_SUBTRACT, __VA_ARGS__))                                     \
	EXPAND(MACRO(OP_MULTIPLY, __VA_ARGS__))                                     \
	EXPAND(MACRO(OP_DIVIDE, __VA_ARGS__))                                       \
	EXPAND(MACRO(OP_NOT, __VA_ARGS__))                                          \
	EXPAND(MACRO(OP_NEGATE, __VA_ARGS__))                                       \
	EXPAND(MACRO(OP_RETURN, __VA_ARGS__))

	enum struct opcode : uint8_t
	{
#define LOX_OPCODE_ENUM(OP, ...) OP,
		LOX_OPCODE_FOREACH(LOX_OPCODE_ENUM)
#undef LOX_OPCODE_ENUM
	};

	lak::u8string_view to_string(lox::opcode op);

	struct chunk
	{
		std::vector<uint8_t> code;
		std::vector<size_t> lines;
		lox::value_array constants;

		inline void push_code(uint8_t c, size_t line)
		{
			code.push_back(c);
			lines.push_back(line);
		}

		inline void push_opcode(lox::opcode inst, size_t line)
		{
			push_code(static_cast<uint8_t>(inst), line);
		}

		inline size_t push_constant(const lox::value &val)
		{
			constants.push_back(val);
			return constants.size() - 1U;
		}

		void disassemble(lak::u8string_view name) const;

		size_t disassemble_instruction(size_t offset) const;
	};
}

#endif
