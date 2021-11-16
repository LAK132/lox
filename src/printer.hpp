#ifndef LOX_PRINTER_HPP
#define LOX_PRINTER_HPP

#include "expr.hpp"

#include <initializer_list>
#include <string_view>

namespace lox
{
	struct prefix_ast_printer_t
	{
		std::u8string parenthesize(std::u8string_view name,
		                           std::initializer_list<lox::expr *> exprs) const;
		std::u8string operator()(const lox::expr::assign &expr) const;
		std::u8string operator()(const lox::expr::binary &expr) const;
		std::u8string operator()(const lox::expr::call &expr) const;
		std::u8string operator()(const lox::expr::grouping &expr) const;
		std::u8string operator()(const lox::expr::literal &expr) const;
		std::u8string operator()(const lox::expr::logical &expr) const;
		std::u8string operator()(const lox::expr::unary &expr) const;
		std::u8string operator()(const lox::expr::variable &expr) const;
	};

	static constexpr prefix_ast_printer_t prefix_ast_printer{};

	struct postfix_ast_printer_t
	{
		std::u8string common(std::u8string_view name,
		                     std::initializer_list<lox::expr *> exprs) const;
		std::u8string operator()(const lox::expr::assign &expr) const;
		std::u8string operator()(const lox::expr::binary &expr) const;
		std::u8string operator()(const lox::expr::call &expr) const;
		std::u8string operator()(const lox::expr::grouping &expr) const;
		std::u8string operator()(const lox::expr::literal &expr) const;
		std::u8string operator()(const lox::expr::logical &expr) const;
		std::u8string operator()(const lox::expr::unary &expr) const;
		std::u8string operator()(const lox::expr::variable &expr) const;
	};

	static constexpr postfix_ast_printer_t postfix_ast_printer{};
}

#endif
