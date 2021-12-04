#ifndef LOX_TYPE_HPP
#define LOX_TYPE_HPP

#include "callable.hpp"
#include "expr.hpp"
#include "string_map.hpp"
#include "token.hpp"

#include <memory>
#include <optional>
#include <string>

namespace lox
{
	struct object;
	struct instance;

	struct type
	{
	private:
		struct impl;

		std::shared_ptr<impl> _impl;

	public:
		type() = delete;

		type(std::u8string_view name,
		     lox::string_map<char8_t, lox::object> methods);

		type(std::u8string_view name,
		     lox::string_map<char8_t, lox::object> methods,
		     const lox::type &superclass);

		type(const type &) = default;
		type &operator=(const type &) = default;

		std::u8string &name();
		const std::u8string &name() const;

		std::optional<type> &superclass();
		const std::optional<type> &superclass() const;

		const lox::callable *find_method(std::u8string_view method_name) const;
		const lox::callable *find_method(const lox::token &method_name) const;

		std::optional<lox::callable> find_bound_method(
		  std::u8string_view method_name, const lox::instance &instance) const;
		std::optional<lox::callable> find_bound_method(
		  const lox::token &method_name, const lox::instance &instance) const;

		lox::callable &constructor();
		const lox::callable &constructor() const;

		std::u8string to_string() const;

		bool operator==(const type &rhs) const;

		bool operator!=(const type &rhs) const;
	};

	struct instance
	{
	private:
		struct impl;

		std::shared_ptr<impl> _impl;

	public:
		instance() = delete;

		instance(const lox::type &type,
		         lox::string_map<char8_t, lox::object> fields);

		instance(const instance &) = default;
		instance &operator=(const instance &) = default;

		const lox::object &emplace(const lox::token &name, lox::object value);

		std::optional<lox::object> find(const lox::token &name) const;

		std::u8string to_string() const;

		bool operator==(const instance &rhs) const;

		bool operator!=(const instance &rhs) const;
	};
}

#endif
