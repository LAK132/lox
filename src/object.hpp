#ifndef LOX_OBJECT_HPP
#define LOX_OBJECT_HPP

#include <lak/memory.hpp>
#include <lak/string_ostream.hpp>
#include <lak/string_view.hpp>
#include <lak/variant.hpp>

#include <iostream>

namespace lox
{
	struct callable;
	struct type;
	struct instance;

	struct object
	{
		using value_type = lak::variant<lak::monostate,
		                                lak::u8string,
		                                double,
		                                bool,
		                                lox::callable,
		                                lox::type,
		                                lox::instance>;

	private:
		struct impl;
		using impl_ptr = lak::shared_ref<impl>;

		impl_ptr _impl;

	public:
		object();
		object(const object &) = default;
		object &operator=(const object &) = default;

		object(lak::monostate value);
		object(lak::u8string value);
		object(double value);
		object(bool value);
		object(const lox::callable &value);
		object(const lox::type &value);
		object(const lox::instance &value);

		lak::u8string to_string() const;

		bool is_truthy() const;

		value_type &value();
		const value_type &value() const;

		const lak::u8string *get_string() const;

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
			return lak::visit(value(), f);
		}

		template<typename F>
		auto visit(F &&f) const
		{
			return lak::visit(value(), f);
		}

		friend inline std::ostream &operator<<(std::ostream &strm,
		                                       const object &obj)
		{
			using lak::operator<<;
			return strm << obj.to_string();
		}
	};
}

#endif
