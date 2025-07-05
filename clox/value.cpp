#include "value.hpp"

#include <lak/streamify.hpp>

lox::value::value()
: _value(lak::in_place_index<value_type::index_of<lak::monostate>>,
         lak::monostate{})
{
}

lox::value::value(lak::monostate)
: _value(lak::in_place_index<value_type::index_of<lak::monostate>>,
         lak::monostate{})
{
}

lox::value::value(bool b)
: _value(lak::in_place_index<value_type::index_of<bool>>, b)
{
}

lox::value::value(double d)
: _value(lak::in_place_index<value_type::index_of<double>>, d)
{
}

bool lox::value::is_nil() const
{
	return _value.index() == value_type::index_of<lak::monostate>;
}

bool lox::value::is_bool() const
{
	return _value.index() == value_type::index_of<bool>;
}

bool lox::value::is_number() const
{
	return _value.index() == value_type::index_of<double>;
}

bool lox::value::is_truthy() const
{
	return visit(lak::overloaded{
	  [](lak::monostate) -> bool { return false; },
	  [](bool b) -> bool { return b; },
	  [](double) -> bool { return true; },
	});
}

lak::result<lak::monostate &> lox::value::as_nil()
{
	return lak::result_from_pointer(_value.template get<lak::monostate>());
}

lak::result<const lak::monostate &> lox::value::as_nil() const
{
	return lak::result_from_pointer(_value.template get<lak::monostate>());
}

lak::result<bool &> lox::value::as_bool()
{
	return lak::result_from_pointer(_value.template get<bool>());
}

lak::result<const bool &> lox::value::as_bool() const
{
	return lak::result_from_pointer(_value.template get<bool>());
}

lak::result<double &> lox::value::as_number()
{
	return lak::result_from_pointer(_value.template get<double>());
}

lak::result<const double &> lox::value::as_number() const
{
	return lak::result_from_pointer(_value.template get<double>());
}

bool lox::value::operator==(const lox::value &other) const
{
	if (_value.index() != other._value.index()) return false;
	return visit(lak::overloaded{
	  [&](lak::monostate) -> bool { return true; },
	  [&](const bool &b) -> bool
	  { return b == *other._value.template get<bool>(); },
	  [&](const double &d) -> bool
	  { return d == *other._value.template get<double>(); },
	});
}

std::ostream &lox::operator<<(std::ostream &strm, const lox::value &val)
{
	lak::visit(
	  lak::overloaded{
	    [&](lak::monostate) { strm << "nil"; },
	    [&](const bool &b) { strm << (b ? "true" : "false"); },
	    [&](const double &d) { strm << d; },
	  },
	  val._value);
	return strm;
}

lak::u8string lox::to_string(const lox::value &v) { return lak::streamify(v); }
