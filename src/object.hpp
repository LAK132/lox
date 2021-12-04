#ifndef LOX_OBJECT_HPP
#define LOX_OBJECT_HPP

#include "overloaded.hpp"
#include "string_hacks.hpp"

#include <iostream>
#include <memory>
#include <string_view>
#include <variant>

namespace lox
{
	struct callable;
	struct type;
	struct instance;

	struct object
	{
		using value_type = std::variant<std::monostate,
		                                std::u8string,
		                                double,
		                                bool,
		                                lox::callable,
		                                lox::type,
		                                lox::instance>;

	private:
		struct impl;
		std::shared_ptr<impl> _impl;

	public:
		object();
		object(const object &) = default;
		object &operator=(const object &) = default;

		object(std::monostate value);
		object(std::u8string value);
		object(double value);
		object(bool value);
		object(const lox::callable &value);
		object(const lox::type &value);
		object(const lox::instance &value);

		std::u8string to_string() const;

		bool is_truthy() const;

		value_type &value();
		const value_type &value() const;

		const std::u8string *get_string() const;

		const double *get_number() const;

		const bool *get_bool() const;

		const lox::callable *get_callable() const;

		// may be empty
		const lox::type *get_type() const;

		// may be empty
		lox::instance *get_instance();
		const lox::instance *get_instance() const;

		bool operator==(const lox::object &rhs) const;

		bool operator!=(const lox::object &rhs) const;

		template<typename F>
		auto visit(F &&f)
		{
			return std::visit(f, value());
		}

		template<typename F>
		auto visit(F &&f) const
		{
			return std::visit(f, value());
		}

		friend inline std::ostream &operator<<(std::ostream &strm,
		                                       const object &obj)
		{
			return strm << lox::as_astring_view(obj.to_string());
		}
	};
}

#endif
