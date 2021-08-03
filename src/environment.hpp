#ifndef LOX_ENVIRONMENT_HPP
#define LOX_ENVIRONMENT_HPP

#include "object.hpp"
#include "token.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace lox
{
	struct environment
	{
		using environment_ptr = std::shared_ptr<environment>;

		environment_ptr enclosing;
		std::unordered_map<std::u8string, lox::object> values;

		using iterator = typename decltype(values)::iterator;

		iterator emplace(const lox::token &k, lox::object v);

		iterator find(const lox::token &k);

		iterator replace(const lox::token &k, lox::object v);

		inline iterator begin() { return values.begin(); }

		inline iterator end() { return values.end(); }

		static environment_ptr make(environment_ptr enclosing = {});
	};

	using environment_ptr = lox::environment::environment_ptr;
}

#endif
