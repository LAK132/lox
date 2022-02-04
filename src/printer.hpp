#ifndef LOX_PRINTER_HPP
#define LOX_PRINTER_HPP

#include "expr.hpp"
#include "stmt.hpp"

#include <lak/string.hpp>

namespace lox
{
	struct dot_digraph_ast_printer_t
	{
		static lak::u8string digraph_wrapper(lak::u8string &&str);

		lak::u8string operator()(const lox::stmt::block &stmt) const;
		lak::u8string operator()(const lox::stmt::type &stmt) const;
		lak::u8string operator()(const lox::stmt::expr &stmt) const;
		lak::u8string operator()(const lox::stmt::branch &stmt) const;
		lak::u8string operator()(const lox::stmt::print &stmt) const;
		lak::u8string operator()(const lox::stmt::var &stmt) const;
		lak::u8string operator()(const lox::stmt::loop &stmt) const;
		lak::u8string operator()(const lox::stmt::function_ptr &stmt) const;
		lak::u8string operator()(const lox::stmt::ret &stmt) const;

		lak::u8string operator()(const lox::expr::assign &expr) const;
		lak::u8string operator()(const lox::expr::binary &expr) const;
		lak::u8string operator()(const lox::expr::call &expr) const;
		lak::u8string operator()(const lox::expr::get &expr) const;
		lak::u8string operator()(const lox::expr::grouping &expr) const;
		lak::u8string operator()(const lox::expr::literal &expr) const;
		lak::u8string operator()(const lox::expr::logical &expr) const;
		lak::u8string operator()(const lox::expr::set &expr) const;
		lak::u8string operator()(const lox::expr::super_keyword &expr) const;
		lak::u8string operator()(const lox::expr::this_keyword &expr) const;
		lak::u8string operator()(const lox::expr::unary &expr) const;
		lak::u8string operator()(const lox::expr::variable &expr) const;
	};

	static constexpr dot_digraph_ast_printer_t dot_digraph_ast_printer{};

	struct dot_subgraph_ast_printer_t
	{
		lak::u8string operator()(const lox::stmt::block &stmt) const;
		lak::u8string operator()(const lox::stmt::type &stmt) const;
		lak::u8string operator()(const lox::stmt::expr &stmt) const;
		lak::u8string operator()(const lox::stmt::branch &stmt) const;
		lak::u8string operator()(const lox::stmt::print &stmt) const;
		lak::u8string operator()(const lox::stmt::var &stmt) const;
		lak::u8string operator()(const lox::stmt::loop &stmt) const;
		lak::u8string operator()(const lox::stmt::function_ptr &stmt) const;
		lak::u8string operator()(const lox::stmt::ret &stmt) const;

		lak::u8string operator()(const lox::expr::assign &expr) const;
		lak::u8string operator()(const lox::expr::binary &expr) const;
		lak::u8string operator()(const lox::expr::call &expr) const;
		lak::u8string operator()(const lox::expr::get &expr) const;
		lak::u8string operator()(const lox::expr::grouping &expr) const;
		lak::u8string operator()(const lox::expr::literal &expr) const;
		lak::u8string operator()(const lox::expr::logical &expr) const;
		lak::u8string operator()(const lox::expr::set &expr) const;
		lak::u8string operator()(const lox::expr::super_keyword &expr) const;
		lak::u8string operator()(const lox::expr::this_keyword &expr) const;
		lak::u8string operator()(const lox::expr::unary &expr) const;
		lak::u8string operator()(const lox::expr::variable &expr) const;
	};

	static constexpr dot_subgraph_ast_printer_t dot_subgraph_ast_printer{};
}

#endif
