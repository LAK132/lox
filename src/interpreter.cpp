#include "interpreter.hpp"

#include "callable.hpp"
#include "evaluator.hpp"
#include "lox.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "resolver.hpp"
#include "scanner.hpp"

#include <lak/debug.hpp>
#include <lak/file.hpp>
#include <lak/string_ostream.hpp>

#include <chrono>
#include <iostream>

void lox::interpreter::report(size_t line,
                              lak::u8string_view where,
                              lak::u8string_view message,
                              const std::source_location srcloc)
{
#ifndef NDEBUG
	std::cerr << srcloc.file_name() << ":" << srcloc.line() << ":"
	          << srcloc.column() << ": ";
#else
	(void)srcloc;
#endif
	using lak::operator<<;
	std::cerr << "[line " << line << "] Error" << where << ": " << message
	          << "\n";
	had_error = true;
}

void lox::interpreter::error(const lox::token &token,
                             lak::u8string_view message,
                             const std::source_location srcloc)
{
	if (token.type == lox::token_type::EOF_TOK)
		report(token.line, u8" at end", message, srcloc);
	else
		report(token.line,
		       u8" at '" + token.lexeme.to_string() + u8"'",
		       message,
		       srcloc);
}

void lox::interpreter::error(size_t line,
                             lak::u8string_view message,
                             const std::source_location srcloc)
{
	report(line, u8"", message, srcloc);
}

lak::result<lox::object> lox::interpreter::evaluate(const lox::expr &expr)
{
	return expr.visit(lox::evaluator(*this));
}

lak::result<lak::monostate> lox::interpreter::execute(const lox::stmt &stmt)
{
	return stmt.visit(lox::evaluator(*this))
	  .map([](auto &&) -> lak::monostate { return {}; });
}

lak::result<lox::object> lox::interpreter::execute_block(
  lak::span<const lox::stmt_ptr> stmts, const lox::environment_ptr &env)
{
	lox::evaluator eval{*this};
	return eval.execute_block(stmts, env)
	  .map(
	    [&](auto &&) -> lox::object {
		    return eval.block_ret_value ? *eval.block_ret_value : lox::object{};
	    });
}

void lox::interpreter::resolve(const lox::expr::variable &expr,
                               size_t distance)
{
	local_declares[&expr] = distance;
}

void lox::interpreter::resolve(const lox::expr::assign &expr, size_t distance)
{
	local_assigns[&expr] = distance;
}

void lox::interpreter::resolve(const lox::expr::super_keyword &expr,
                               size_t distance)
{
	local_super[&expr] = distance;
}

void lox::interpreter::resolve(const lox::expr::this_keyword &expr,
                               size_t distance)
{
	local_this[&expr] = distance;
}

lak::result<size_t> lox::interpreter::find(const lox::expr::variable &expr)
{
	auto distance = local_declares.find(&expr);
	if (distance != local_declares.end())
		return lak::ok_t<size_t>{distance->second};
	else
		return lak::err_t{};
}

lak::result<size_t> lox::interpreter::find(const lox::expr::assign &expr)
{
	auto distance = local_assigns.find(&expr);
	if (distance != local_assigns.end())
		return lak::ok_t<size_t>{distance->second};
	else
		return lak::err_t{};
}

lak::result<size_t> lox::interpreter::find(
  const lox::expr::super_keyword &expr)
{
	auto distance = local_super.find(&expr);
	if (distance != local_super.end())
		return lak::ok_t<size_t>{distance->second};
	else
		return lak::err_t{};
}

lak::result<size_t> lox::interpreter::find(const lox::expr::this_keyword &expr)
{
	auto distance = local_this.find(&expr);
	if (distance != local_this.end())
		return lak::ok_t<size_t>{distance->second};
	else
		return lak::err_t{};
}

lak::result<std::vector<lox::stmt_ptr>> lox::interpreter::parse(
  lak::u8string_view file)
{
	ASSERT(global_environment);

	lox::scanner scanner{*this, file};
	auto tokens{scanner.scan_tokens()};
	if (had_error) return lak::err_t{};

	lox::parser parser{*this, lak::move(tokens)};
	RES_TRY_ASSIGN(std::vector<lox::stmt_ptr> stmts =, parser.parse());
	if (had_error) return lak::err_t{};

	return lak::ok_t<std::vector<lox::stmt_ptr>>{lak::move(stmts)};
}

lak::result<std::vector<lox::stmt_ptr>> lox::interpreter::parse_file(
  const std::filesystem::path &file_path)
{
	ASSERT(global_environment);

	return lak::read_file(file_path)
	  .map_err(
	    [&](const lak::errno_error &err) -> lak::monostate
	    {
		    std::cerr << "Failed to read file '" << file_path << "': " << err
		              << "\n";
		    return {};
	    })
	  .and_then(
	    [&](const lak::array<byte_t> &arr)
	      -> lak::result<std::vector<lox::stmt_ptr>>
	    {
		    auto span{lak::span<const char8_t>(lak::span(arr))};
		    sources.push_back(std::vector<char8_t>(span.begin(), span.end()));
		    return parse(lak::u8string_view(sources.back()));
	    });
}

lak::u8string lox::interpreter::interpret(const lox::expr &expr)
{
	return evaluate(expr).map_or(
	  [](const lox::object &obj) { return obj.to_string(); }, u8""_str);
}

lak::result<lak::u8string> lox::interpreter::interpret(const lox::stmt &stmt)
{
	return stmt.visit(lox::evaluator(*this));
}

lak::result<lak::u8string> lox::interpreter::interpret(
  lak::span<const lox::stmt_ptr> stmts)
{
	lak::u8string str;

	for (const auto &stmt : stmts)
	{
		RES_TRY_ASSIGN(lak::u8string result =, interpret(*stmt));
		if (had_error) return lak::err_t{};

		str += result;
	}

	return lak::ok_t{lak::move(str)};
}

lak::result<lox::object> (*lox_clock)(lox::interpreter &) =
  [](lox::interpreter &) -> lak::result<lox::object>
{
	return lak::ok_t<lox::object>{
	  std::chrono::duration_cast<std::chrono::milliseconds>(
	    std::chrono::system_clock::now().time_since_epoch())
	    .count() /
	  1000.0};
};

lak::result<lox::object> (*lox_to_string)(lox::interpreter &, lox::object &&) =
  [](lox::interpreter &, lox::object &&obj) -> lak::result<lox::object>
{ return lak::ok_t<lox::object>{obj.to_string()}; };

lox::interpreter &lox::interpreter::init_globals()
{
	global_environment = lox::environment::make();

	global_environment->emplace(
	  u8"clock",
	  lox::object{
	    lox::callable(LOX_CALLABLE_MAKE_NATIVE(lox_clock)),
	  });

	global_environment->emplace(
	  u8"to_string",
	  lox::object{
	    lox::callable(LOX_CALLABLE_MAKE_NATIVE(lox_to_string)),
	  });

	return *this;
}

lak::result<> lox::interpreter::run(lak::u8string_view file,
                                    lak::u8string *out_str)
{
	ASSERT(global_environment);

	lox::scanner scanner{*this, file};
	auto tokens{scanner.scan_tokens()};
	if (had_error) return lak::err_t{};

	lox::parser parser{*this, lak::move(tokens)};
	RES_TRY_ASSIGN(std::vector<lox::stmt_ptr> stmts =, parser.parse());
	if (had_error) return lak::err_t{};

	lox::resolver resolver{*this};
	RES_TRY(resolver.resolve(stmts));
	if (had_error) return lak::err_t{};

	RES_TRY_ASSIGN(lak::u8string result =, interpret(stmts));
	if (had_error) return lak::err_t{};

	if (out_str) *out_str += result;

	return lak::ok_t{};
}

lak::result<> lox::interpreter::run_file(
  const std::filesystem::path &file_path)
{
	ASSERT(global_environment);

	return lak::read_file(file_path)
	  .map_err(
	    [&](const lak::errno_error &err) -> lak::monostate
	    {
		    std::cerr << "Failed to read file '" << file_path << "': " << err
		              << "\n";
		    return {};
	    })
	  .and_then(
	    [&](const lak::array<byte_t> &arr) -> lak::result<>
	    {
		    auto span{lak::span<const char8_t>(lak::span(arr))};
		    sources.push_back(std::vector<char8_t>(span.begin(), span.end()));
		    return run(lak::u8string_view(sources.back()));
	    });
}

lak::result<> lox::interpreter::run_prompt()
{
	ASSERT(global_environment);

	for (bool running = true; running;)
	{
		using lak::operator<<;

		std::cout << "> ";

		lak::astring string;
		std::getline(std::cin, string);
		if (!std::cin.good()) break;
		if (string.empty()) break;

		string += "\n";

		lak::u8string out;
		run(lak::as_u8string(string), &out).discard();

		std::cout << out;

		had_error = false;
	}

	std::cout << "\n";
	return lak::ok_t{};
}
