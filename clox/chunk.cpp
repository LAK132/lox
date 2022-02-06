#include "chunk.hpp"

#include <lak/debug.hpp>
#include <lak/string_literals.hpp>

#include <iomanip>

lak::u8string_view lox::to_string(lox::opcode op)
{
	switch (op)
	{
#define LOX_OPCODE_TO_STRING(OP, ...)                                         \
	case lox::opcode::OP: return u8"" #OP ""_view;
		LOX_OPCODE_FOREACH(LOX_OPCODE_TO_STRING)
#undef LOX_OPCODE_TO_STRING
		default: FATAL("Invalid opcode ", static_cast<unsigned>(op));
	}
}

void lox::chunk::disassemble(lak::u8string_view name) const
{
	std::cout << "== " << name << " ==\n";

	for (size_t offset = 0; offset < code.size();)
	{
		offset = disassemble_instruction(offset);
	}
}

size_t constant_instruction(const lox::chunk &chunk,
                            lak::u8string_view name,
                            size_t offset)
{
	using lak::operator<<;

	std::cout << name;
	for (size_t i = name.size(); i < 16; ++i) std::cout << " ";
	std::cout << " ";

	std::cout << std::setfill('0') << std::setw(4)
	          << unsigned(chunk.code[offset + 1]) << " ";

	std::cout << "'" << lox::to_string(chunk.constants[chunk.code[offset + 1]])
	          << "'\n";

	return offset + 2U;
}

size_t simple_instruction(lak::u8string_view name, size_t offset)
{
	using lak::operator<<;

	std::cout << name << "\n";

	return offset + 1U;
}

size_t lox::chunk::disassemble_instruction(size_t offset) const
{
	ASSERT_LESS(offset, code.size());
	ASSERT_LESS(offset, lines.size());

	std::cout << std::setfill('0') << std::setw(4) << offset << " ";

	if (offset > 0 && lines[offset] == lines[offset - 1U])
		std::cout << "   | ";
	else
		std::cout << std::setfill('0') << std::setw(4) << lines[offset] << " ";

	const uint8_t instruction = code[offset];

	switch (static_cast<lox::opcode>(instruction))
	{
		case lox::opcode::OP_CONSTANT:
			return constant_instruction(*this, u8"OP_CONSTANT"_view, offset);

		case lox::opcode::OP_NIL:
			return simple_instruction(u8"OP_NIL"_view, offset);

		case lox::opcode::OP_TRUE:
			return simple_instruction(u8"OP_TRUE"_view, offset);

		case lox::opcode::OP_FALSE:
			return simple_instruction(u8"OP_FALSE"_view, offset);

		case lox::opcode::OP_EQUAL:
			return simple_instruction(u8"OP_EQUAL"_view, offset);

		case lox::opcode::OP_GREATER:
			return simple_instruction(u8"OP_GREATER"_view, offset);

		case lox::opcode::OP_LESS:
			return simple_instruction(u8"OP_LESS"_view, offset);

		case lox::opcode::OP_ADD:
			return simple_instruction(u8"OP_ADD"_view, offset);

		case lox::opcode::OP_SUBTRACT:
			return simple_instruction(u8"OP_SUBTRACT"_view, offset);

		case lox::opcode::OP_MULTIPLY:
			return simple_instruction(u8"OP_MULTIPLY"_view, offset);

		case lox::opcode::OP_DIVIDE:
			return simple_instruction(u8"OP_DIVIDE"_view, offset);

		case lox::opcode::OP_NOT:
			return simple_instruction(u8"OP_NOT"_view, offset);

		case lox::opcode::OP_NEGATE:
			return simple_instruction(u8"OP_NEGATE"_view, offset);

		case lox::opcode::OP_RETURN:
			return simple_instruction(u8"OP_RETURN"_view, offset);

		default:
			std::cout << "Unknown opcode " << unsigned(instruction) << "\n";
			return offset + 1U;
	}
}
