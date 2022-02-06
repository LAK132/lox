#ifndef LOX_VALUE_HPP
#define LOX_VALUE_HPP

#include <lak/result.hpp>
#include <lak/string.hpp>
#include <lak/variant.hpp>

#include <iostream>
#include <vector>

namespace lox
{
	struct value
	{
		using value_type = lak::variant<lak::monostate, bool, double>;
		value_type _value;

		value();

		value(lak::monostate);

		value(bool b);

		value(double d);

		bool is_nil() const;

		bool is_bool() const;

		bool is_number() const;

		bool is_truthy() const;

		lak::result<lak::monostate &> as_nil();
		lak::result<const lak::monostate &> as_nil() const;

		lak::result<bool &> as_bool();
		lak::result<const bool &> as_bool() const;

		lak::result<double &> as_number();
		lak::result<const double &> as_number() const;

		template<typename F>
		auto visit(F &&f)
		{
			return lak::visit(_value, lak::forward<F>(f));
		}
		template<typename F>
		auto visit(F &&f) const
		{
			return lak::visit(_value, lak::forward<F>(f));
		}

		bool operator==(const value &other) const;
	};

	std::ostream &operator<<(std::ostream &strm, const lox::value &val);

	using value_array = std::vector<lox::value>;

	lak::u8string to_string(const lox::value &v);
}

#endif
