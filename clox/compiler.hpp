#ifndef LOX_COMPILER_HPP
#define LOX_COMPILER_HPP

#include "scanner.hpp"

#include <lak/file.hpp>
#include <lak/result.hpp>
#include <lak/string.hpp>
#include <lak/string_view.hpp>
#include <lak/variant.hpp>

namespace lox
{
	using compile_error = lak::variant<lox::scan_error>;

	template<typename T = lak::monostate>
	using compile_result = lak::result<T, lox::compile_error>;

	lox::compile_result<> compile(lak::u8string_view file);
}

#endif
