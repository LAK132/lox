#include "virtual_machine.hpp"
#include "common.hpp"

lak::u8string_view lox::to_string(lox::interpret_error err)
{
	switch (err)
	{
#define LOX_INTERPRET_ERROR_TO_STRING(OP, ...)                                \
	case lox::interpret_error::OP: return u8"" #OP ""_view;
		LOX_INTERPRET_ERROR_FOREACH(LOX_INTERPRET_ERROR_TO_STRING)
#undef LOX_INTERPRET_ERROR_TO_STRING
		default: FATAL("Invalid error ", static_cast<unsigned>(err));
	}
}

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

lox::interpret_result<> lox::virtual_machine::interpret(lox::chunk *c)
{
	chunk = c;
	ip    = lak::binary_reader{lak::span<const byte_t>{lak::span{chunk->code}}};
	return run();
}

lox::interpret_result<> lox::virtual_machine::interpret(
  lak::u8string_view file)
{
	return lox::compile(file).map_err(
	  [](const lox::compile_error &err) -> lox::interpret_error
	  {
		  lak::visit(err,
		             lak::overloaded{
		               [](const lox::scan_error &err)
		               {
			               using lak::operator<<;
			               // :TODO:
			               std::cerr << err.message << "\n";
		               },
		               [](auto &&) {},
		             });
		  return lox::interpret_error::INTERPRET_COMPILE_ERROR;
	  });
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

#define LOX_BINARY_OP(op)                                                     \
	do                                                                          \
	{                                                                           \
		lox::value b{stack_pop().unwrap()};                                       \
		lox::value a{stack_pop().unwrap()};                                       \
		stack_push(a op b).unwrap();                                              \
	} while (false)

			case lox::opcode::OP_ADD: LOX_BINARY_OP(+); break;
			case lox::opcode::OP_SUBTRACT: LOX_BINARY_OP(-); break;
			case lox::opcode::OP_MULTIPLY: LOX_BINARY_OP(*); break;
			case lox::opcode::OP_DIVIDE: LOX_BINARY_OP(/); break;

#undef LOX_BINARY_OP

			case lox::opcode::OP_NEGATE:
			{
				stack_push(-stack_pop().unwrap()).unwrap();
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

lox::interpret_result<> lox::virtual_machine::run_file(
  const std::filesystem::path &file_path)
{
	return lak::read_file(file_path)
	  .map_err(
	    [&](const lak::errno_error &err) -> lox::interpret_error
	    {
		    std::cerr << "Failed to read file '" << file_path << "': " << err
		              << "\n";
		    return lox::interpret_error::INTERPRET_COMPILE_ERROR;
	    })
	  .and_then(
	    [&](const lak::array<byte_t> &arr) -> lox::interpret_result<>
	    {
		    return interpret(
		      lak::u8string_view(lak::span<const char8_t>(lak::span(arr))));
	    });
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

		// lak::u8string out;
		interpret(lak::as_u8string(string)).discard();
		// std::cout << out;
	}

	std::cout << "\n";
	return lak::ok_t{};
}
