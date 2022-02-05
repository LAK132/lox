#ifndef LOX_TYPE_HPP
#define LOX_TYPE_HPP

#include "callable.hpp"
#include "expr.hpp"
#include "string_map.hpp"
#include "token.hpp"

#include <lak/optional.hpp>
#include <lak/result.hpp>
#include <lak/string.hpp>
#include <lak/string_literals.hpp>

#include <memory>

namespace lox
{
	struct object;
	struct instance;

	struct type
	{
	private:
		struct impl;
		using impl_ptr = lak::shared_ref<impl>;

		impl_ptr _impl;

	public:
		type() = delete;

		type(lak::u8string_view name,
		     lox::string_map<char8_t, lox::object> methods);

		type(lak::u8string_view name,
		     lox::string_map<char8_t, lox::object> methods,
		     const lox::type &superclass);

		type(const type &) = default;
		type &operator=(const type &) = default;

		lak::u8string &name();
		const lak::u8string &name() const;

		lak::optional<type> &superclass();
		const lak::optional<type> &superclass() const;

		lak::result<const lox::callable &> find_method(
		  lak::u8string_view method_name) const;
		lak::result<const lox::callable &> find_method(
		  const lox::token &method_name) const;

		lak::result<lox::callable> find_bound_method(
		  lak::u8string_view method_name, const lox::instance &instance) const;
		lak::result<lox::callable> find_bound_method(
		  const lox::token &method_name, const lox::instance &instance) const;

		lox::callable &constructor();
		const lox::callable &constructor() const;

		lak::u8string to_string() const;

		bool operator==(const type &rhs) const;

		bool operator!=(const type &rhs) const;
	};

	struct instance
	{
	private:
		struct impl;
		using impl_ptr = lak::shared_ref<impl>;

		impl_ptr _impl;

	public:
		instance() = delete;

		instance(const lox::type &type,
		         lox::string_map<char8_t, lox::object> fields);

		instance(const instance &) = default;
		instance &operator=(const instance &) = default;

		const lox::object &emplace(const lox::token &name, lox::object value);

		lak::result<lox::object> find(const lox::token &name) const;

		lak::u8string to_string() const;

		bool operator==(const instance &rhs) const;

		bool operator!=(const instance &rhs) const;
	};
}

#endif
