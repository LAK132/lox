#ifndef LOX_COMPILER_HPP
#define LOX_COMPILER_HPP

#include "chunk.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "scanner.hpp"

#include <lak/file.hpp>
#include <lak/result.hpp>
#include <lak/string.hpp>
#include <lak/string_view.hpp>
#include <lak/variant.hpp>

namespace lox
{
	struct compile_error_tag;

	using compile_error = lox::positional_error<lox::compile_error_tag>;

	template<typename T = lak::monostate>
	using compile_result = lak::result<
	  T,
	  lox::result_set<lox::scan_error, lox::parse_error, lox::compile_error>>;

	lox::compile_result<lox::chunk> compile(lak::u8string_view file);
}

#endif
