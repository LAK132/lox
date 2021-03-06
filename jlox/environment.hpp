#ifndef LOX_ENVIRONMENT_HPP
#define LOX_ENVIRONMENT_HPP

#include "object.hpp"
#include "string_map.hpp"
#include "token.hpp"

#include <lak/memory.hpp>
#include <lak/string.hpp>
#include <lak/string_view.hpp>

namespace lox
{
	struct environment
	{
		using environment_ptr = lak::shared_ptr<environment>;

		environment_ptr enclosing;
		lox::string_map<char8_t, lox::object> values;

		const lox::object &emplace(lak::u8string_view k, lox::object v);

		const lox::object &emplace(const lox::token &k, lox::object v);

		const lox::object *find(lak::u8string_view k);

		const lox::object *find(lak::u8string_view k, size_t distance);

		const lox::object *find(const lox::token &k);

		const lox::object *find(const lox::token &k, size_t distance);

		const lox::object *replace(const lox::token &k, lox::object v);

		const lox::object *replace(const lox::token &k,
		                           lox::object v,
		                           size_t distance);

		static environment_ptr make(environment_ptr enclosing = {});
	};

	using environment_ptr = lox::environment::environment_ptr;
}

#endif
