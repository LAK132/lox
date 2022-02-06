#include "virtual_machine.hpp"
#include "common.hpp"

lak::result<> lox::virtual_machine::stack_push(lox::value v)
{
	if (stack_top == stack.size()) return lak::err_t{};
	stack[stack_top++] = v;
	return lak::ok_t{};
}

lak::result<lox::value> lox::virtual_machine::stack_pop()
{
	if (stack_top == 0) return lak::err_t{};
	return lak::ok_t<lox::value>{stack[--stack_top]};
}

lak::result<const lox::value &> lox::virtual_machine::stack_peek(
  size_t depth) const
{
	if (depth >= stack_top) return lak::err_t{};
	return lak::ok_t<const lox::value &>{stack[stack_top - (depth + 1)]};
}

lox::interpret_result<> lox::virtual_machine::interpret(lox::chunk *c)
{
	chunk = c;
	ip    = lak::binary_reader{lak::span<const byte_t>{lak::span{chunk->code}}};
	return run();
}

lox::interpret_result<> lox::virtual_machine::interpret(
  lak::u8string_view file)
{
	RES_TRY_ASSIGN(lox::chunk chunk =, lox::compile(file));

	return interpret(&chunk);
}

lox::interpret_result<> lox::virtual_machine::run()
{
	ASSERT_NOT_EQUAL(chunk, nullptr);

	for (;;)
	{
#ifdef LOX_DEBUG_TRACE_EXECUTION
		std::cout << "          ";
		for (const lox::value &v : lak::span(stack).first(stack_top))
			std::cout << "[ " << v << " ]";
		std::cout << "\n";
		chunk->disassemble_instruction(ip.position());
#endif

		lox::opcode instruction;
		switch (instruction = static_cast<lox::opcode>(ip.read_u8().unwrap()))
		{
			case lox::opcode::OP_CONSTANT:
			{
				const lox::value &constant = chunk->constants[ip.read_u8().unwrap()];
				stack_push(constant).unwrap();
			}
			break;

			case lox::opcode::OP_NIL: stack_push(lox::value{}).unwrap(); break;

			case lox::opcode::OP_TRUE: stack_push(lox::value{true}).unwrap(); break;

			case lox::opcode::OP_FALSE:
				stack_push(lox::value{false}).unwrap();
				break;

			case lox::opcode::OP_EQUAL:
			{
				const auto a{stack_pop().unwrap()};
				const auto b{stack_pop().unwrap()};
				stack_push(a == b).unwrap();
			}
			break;

#define LOX_BINARY_OP(op)                                                     \
	do                                                                          \
	{                                                                           \
		if (!stack_peek(0).unwrap().is_number() ||                                \
		    !stack_peek(1).unwrap().is_number())                                  \
			return lak::err_t{lox::runtime_error::at(                               \
			  chunk->lines[ip.position() - 1], u8"Operands must be numbers."_str)}; \
		const double b{stack_pop().unsafe_unwrap().as_number().unsafe_unwrap()};  \
		const double a{stack_pop().unsafe_unwrap().as_number().unsafe_unwrap()};  \
		stack_push(a op b).unwrap();                                              \
	} while (false)

			case lox::opcode::OP_GREATER: LOX_BINARY_OP(>); break;
			case lox::opcode::OP_LESS: LOX_BINARY_OP(<); break;
			case lox::opcode::OP_ADD: LOX_BINARY_OP(+); break;
			case lox::opcode::OP_SUBTRACT: LOX_BINARY_OP(-); break;
			case lox::opcode::OP_MULTIPLY: LOX_BINARY_OP(*); break;
			case lox::opcode::OP_DIVIDE: LOX_BINARY_OP(/); break;
#undef LOX_BINARY_OP

			case lox::opcode::OP_NOT:
				stack_push(!stack_pop().unwrap().is_truthy());
				break;

			case lox::opcode::OP_NEGATE:
			{
				if (!stack_peek(0).unwrap().is_number())
				{
					return lak::err_t{
					  lox::runtime_error::at(chunk->lines[ip.position() - 1],
					                         u8"Operand must be a number."_str)};
				}
				stack_push(-stack_pop().unsafe_unwrap().as_number().unsafe_unwrap())
				  .unwrap();
			}
			break;

			case lox::opcode::OP_RETURN:
			{
				std::cout << stack_pop().unwrap() << "\n";
				return lak::ok_t{};
			}
		}
	}
}

lox::virtual_machine::run_file_result lox::virtual_machine::run_file(
  const std::filesystem::path &file_path)
{
	RES_TRY_ASSIGN(const lak::array<byte_t> &arr =, lak::read_file(file_path));
	RES_TRY(
	  interpret(lak::u8string_view(lak::span<const char8_t>(lak::span(arr)))));
	return lak::ok_t{};
}

lox::interpret_result<> lox::virtual_machine::run_prompt()
{
	for (bool running = true; running;)
	{
		using lak::operator<<;

		std::cout << "> ";

		lak::astring string;
		std::getline(std::cin, string);
		if (!std::cin.good()) break;
		if (string.empty()) break;

		string += "\n";

		if_let_err (const lox::interpret_error & err,
		            interpret(lak::as_u8string(string)))
		{
			err.visit(
			  []<typename TAG>(const lox::positional_error<TAG> &err)
			  {
				  using lak::operator<<;
				  std::cerr << lox::to_string(err) << "\n";
			  });
		}
	}

	std::cout << "\n";
	return lak::ok_t{};
}
