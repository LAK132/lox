#include "chunk.hpp"

#include <lak/string_literals.hpp>
#include <lak/string_ostream.hpp>

#include <iostream>

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	lox::chunk chunk;

	chunk.push_opcode(lox::opcode::OP_CONSTANT, 123);
	chunk.push_code((uint8_t)chunk.push_constant(1.2), 123);
	chunk.push_opcode(lox::opcode::OP_RETURN, 123);

	chunk.disassemble(u8"test chunk"_view);
}
