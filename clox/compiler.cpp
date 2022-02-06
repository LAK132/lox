#include "compiler.hpp"
#include "common.hpp"
#include "scanner.hpp"

#include <lak/debug.hpp>
#include <lak/string_literals.hpp>

lox::compile_result<lox::chunk> lox::compile(lak::u8string_view file)
{
	lox::scanner scanner{file};

	lox::parser parser{scanner};

	RES_TRY(parser.next());

	RES_TRY(parser.parse_expression());

	RES_TRY_ASSIGN(lox::token eof_tok =,
	               parser.consume(lox::token_type::EOF_TOK,
	                              u8"Expected end of expression."_view));

	lox::chunk result{lak::move(parser.chunk)};

	result.push_opcode(lox::opcode::OP_RETURN, eof_tok.line);

#ifdef LOX_DEBUG_PRINT_CODE
	result.disassemble(u8"code"_view);
#endif

	return lak::move_ok(result);
}
