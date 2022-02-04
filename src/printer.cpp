#include "printer.hpp"

#include <lak/macro_utils.hpp>

/* --- dot_digraph_ast_printer_t --- */

lak::u8string replace_all(lak::u8string str,
                          lak::u8string_view what,
                          lak::u8string_view with)
{
	for (lak::astring::size_type pos = 0U;
	     str.npos != (pos = str.find(what.data(), pos, what.size()));
	     pos += with.size())
	{
		str.replace(pos, what.size(), with.data(), with.size());
	}
	return str;
}

lak::u8string escape(lak::u8string str)
{
	str = replace_all(str, u8"\\", u8"\\\\");
	str = replace_all(str, u8"<", u8"&lt;");
	str = replace_all(str, u8">", u8"&gt;");
	str = replace_all(str, u8"\"", u8"\\\"");
	return str;
}

lak::u8string lox::dot_digraph_ast_printer_t::digraph_wrapper(
  lak::u8string &&str)
{
	return u8"digraph {\n"
	       "\tnode [shape = Mrecord];\n" +
	       str + u8"}\n";
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::block &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::type &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::expr &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::branch &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::print &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::var &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::loop &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::function_ptr &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::stmt::ret &stmt) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(stmt));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::assign &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::binary &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::call &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::get &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::grouping &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::literal &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::set &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::super_keyword &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::this_keyword &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::logical &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::unary &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

lak::u8string lox::dot_digraph_ast_printer_t::operator()(
  const lox::expr::variable &expr) const
{
	return digraph_wrapper(lox::dot_subgraph_ast_printer(expr));
}

/* --- dot_subgraph_ast_printer_t --- */

struct subgraph_entry
{
	lak::u8string to;
	lak::u8string label;
};

lak::u8string subgraph(lak::u8string_view name,
                       lak::u8string_view label,
                       lak::span<const subgraph_entry> entries = {})
{
	lak::u8string result;

	result += u8"\tsubgraph ";
	result += name;
	result += u8" {\n";

	result += u8"\t\t";
	result += name;
	if (!label.empty())
	{
		result += u8" [label = \"";
		result += label;
		result += u8"\"]";
	}
	result += u8";\n";

	for (const auto &entry : entries)
	{
		result += u8"\t\t";
		result += entry.to;
		result += u8" -> ";
		result += name;
		if (!entry.label.empty())
		{
			result += u8" [label = \"";
			result += entry.label;
			result += u8"\"]";
		}
		result += u8";\n";
	}

	result += u8"\t};\n";

	return result;
}

lak::u8string ptr_to_string(const void *ptr)
{
	return lak::as_u8string(std::to_string(reinterpret_cast<uintptr_t>(ptr)))
	  .to_string();
};

auto visit_ptr_string = [](const auto &expr) -> lak::u8string
{ return ptr_to_string(&expr); };

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::block &stmt) const
{
	lak::u8string result;

	std::vector<subgraph_entry> entries;

	size_t i = 0;
	for (const auto &st : stmt.statements)
		entries.push_back({
		  .to{st->visit(visit_ptr_string)},
		  .label{lak::as_u8string(std::to_string(i++))},
		});

	result += subgraph(ptr_to_string(&stmt), u8"block", entries);

	for (const auto &st : stmt.statements) result += st->visit(*this);

	return result;
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::type &stmt) const
{
	lak::u8string result;

	std::vector<subgraph_entry> entries;

	if_ref (const auto &superclass, stmt.superclass)
		entries.push_back({
		  .to{visit_ptr_string(superclass)},
		  .label{u8"superclass"},
		});

	for (const auto &fn : stmt.methods)
		entries.push_back({
		  .to{ptr_to_string(&fn)},
		  .label{u8"method"},
		});

	result += subgraph(ptr_to_string(&stmt),
	                   u8"{type|"_str + stmt.name.lexeme.to_string() + u8"}",
	                   entries);

	if_ref (const auto &superclass, stmt.superclass)
		result += (*this)(superclass);

	for (const auto &fn : stmt.methods) result += (*this)(fn);

	return result;
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::expr &stmt) const
{
	return subgraph(ptr_to_string(&stmt),
	                u8"expr",
	                {subgraph_entry{
	                  .to{stmt.expression->visit(visit_ptr_string)},
	                  .label{},
	                }}) +
	       stmt.expression->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::branch &stmt) const
{
	lak::u8string result;

	std::vector<subgraph_entry> entries;

	entries.push_back({
	  .to{stmt.condition->visit(visit_ptr_string)},
	  .label{u8"if"},
	});

	entries.push_back({
	  .to{stmt.then_branch->visit(visit_ptr_string)},
	  .label{u8"then"},
	});

	if_ref (const auto &else_branch, stmt.else_branch)
		entries.push_back({
		  .to{else_branch->visit(visit_ptr_string)},
		  .label{u8"else"},
		});

	result += subgraph(ptr_to_string(&stmt), u8"branch", entries);

	result += stmt.condition->visit(*this);
	result += stmt.then_branch->visit(*this);
	if_ref (const auto &else_branch, stmt.else_branch)
		result += else_branch->visit(*this);

	return result;
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::print &stmt) const
{
	return subgraph(ptr_to_string(&stmt),
	                u8"print",
	                {subgraph_entry{
	                  .to{stmt.expression->visit(visit_ptr_string)},
	                  .label{u8"expr"},
	                }}) +
	       stmt.expression->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::var &stmt) const
{
	lak::u8string result;

	std::vector<subgraph_entry> entries;

	if_ref (const auto &init, stmt.init)
		entries.push_back({
		  .to{init->visit(visit_ptr_string)},
		  .label{u8"init"},
		});

	result += subgraph(ptr_to_string(&stmt),
	                   u8"{var|"_str + stmt.name.lexeme.to_string() + u8"}",
	                   entries);

	if_ref (const auto &init, stmt.init) result += init->visit(*this);

	return result;
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::loop &stmt) const
{
	return subgraph(ptr_to_string(&stmt),
	                u8"loop",
	                {subgraph_entry{
	                   .to{stmt.condition->visit(visit_ptr_string)},
	                   .label{u8"while"},
	                 },
	                 subgraph_entry{
	                   .to{stmt.body->visit(visit_ptr_string)},
	                   .label{u8"do"},
	                 }}) +
	       stmt.condition->visit(*this) + stmt.body->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::function_ptr &stmt) const
{
	lak::u8string result;

	std::vector<subgraph_entry> entries;

	size_t i = 0;
	for (const auto &st : stmt->body)
		entries.push_back({
		  .to{st->visit(visit_ptr_string)},
		  .label{lak::as_u8string(std::to_string(i++))},
		});

	lak::u8string function_sig;
	function_sig += stmt->name.lexeme;
	function_sig += u8"(";
	for (const auto &param : stmt->parameters)
	{
		function_sig += param.lexeme;
		function_sig += u8", ";
	}
	while (!function_sig.empty() &&
	       (function_sig.back() == u8' ' || function_sig.back() == ','))
		function_sig.pop_back();
	function_sig += u8")";

	result += subgraph(
	  ptr_to_string(&stmt), u8"{function|"_str + function_sig + u8"}", entries);

	for (const auto &st : stmt->body) result += st->visit(*this);

	return result;
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::stmt::ret &stmt) const
{
	lak::u8string result;

	std::vector<subgraph_entry> entries;

	if_ref (const auto &value, stmt.value)
		entries.push_back({
		  .to{value->visit(visit_ptr_string)},
		  .label{},
		});

	result += subgraph(ptr_to_string(&stmt), u8"ret", entries);

	if_ref (const auto &value, stmt.value) result += value->visit(*this);

	return result;
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::assign &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{assign|"_str + escape(expr.name.lexeme.to_string()) +
	                  u8"}",
	                {subgraph_entry{
	                  .to{expr.value->visit(visit_ptr_string)},
	                  .label{},
	                }}) +
	       expr.value->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::binary &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{binary|"_str + escape(expr.op.lexeme) + u8"}",
	                {subgraph_entry{
	                   .to{expr.left->visit(visit_ptr_string)},
	                   .label{u8"L"},
	                 },
	                 subgraph_entry{
	                   .to{expr.right->visit(visit_ptr_string)},
	                   .label{u8"R"},
	                 }}) +
	       expr.left->visit(*this) + expr.right->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::call &expr) const
{
	lak::u8string result;

	std::vector<subgraph_entry> entries;

	entries.push_back({
	  .to{expr.callee->visit(visit_ptr_string)},
	  .label{u8"callee"},
	});

	size_t i = 0;
	for (const auto &arg : expr.arguments)
		entries.push_back({
		  .to{arg->visit(visit_ptr_string)},
		  .label{lak::as_u8string("arg "_str + std::to_string(i++))},
		});

	result += subgraph(ptr_to_string(&expr), u8"call", entries);

	result += expr.callee->visit(*this);
	for (const auto &arg : expr.arguments) result += arg->visit(*this);

	return result;
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::get &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{get|."_str + expr.name.lexeme.to_string() + u8"}",
	                {subgraph_entry{
	                  .to{expr.object->visit(visit_ptr_string)},
	                  .label{u8"object"},
	                }}) +
	       expr.object->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::super_keyword &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{super|"_str + expr.method.lexeme.to_string() + u8"}");
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::this_keyword &expr) const
{
	return subgraph(ptr_to_string(&expr), u8"this");
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::grouping &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"group",
	                {subgraph_entry{
	                  .to{expr.expression->visit(visit_ptr_string)},
	                  .label{},
	                }}) +
	       expr.expression->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::literal &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{literal|"_str + escape(expr.value.to_string()) + u8"}");
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::logical &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{logical|"_str + escape(expr.op.lexeme) + u8"}",
	                {subgraph_entry{
	                   .to{expr.left->visit(visit_ptr_string)},
	                   .label{u8"L"},
	                 },
	                 subgraph_entry{
	                   .to{expr.right->visit(visit_ptr_string)},
	                   .label{u8"R"},
	                 }}) +
	       expr.left->visit(*this) + expr.right->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::set &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{set|."_str + expr.name.lexeme.to_string() + u8"}",
	                {subgraph_entry{
	                   .to{expr.object->visit(visit_ptr_string)},
	                   .label{u8"object"},
	                 },
	                 subgraph_entry{
	                   .to{expr.value->visit(visit_ptr_string)},
	                   .label{u8"value"},
	                 }}) +
	       expr.object->visit(*this) + expr.value->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::unary &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{unary|"_str + escape(expr.op.lexeme) + u8"}",
	                {subgraph_entry{
	                  .to{expr.right->visit(visit_ptr_string)},
	                  .label{u8"R"},
	                }}) +
	       expr.right->visit(*this);
}

lak::u8string lox::dot_subgraph_ast_printer_t::operator()(
  const lox::expr::variable &expr) const
{
	return subgraph(ptr_to_string(&expr),
	                u8"{variable|"_str + escape(expr.name.lexeme) + u8"}");
}
