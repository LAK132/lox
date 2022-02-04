#include "object.hpp"

#include "callable.hpp"
#include "type.hpp"

#include <lak/string.hpp>
#include <lak/visit.hpp>

struct lox::object::impl
{
	lox::object::value_type value;
};

lox::object::object() : _impl(lox::object::impl_ptr::make().unwrap()) {}

lox::object::object(lak::monostate value)
: _impl(lox::object::impl_ptr::make(lox::object::impl{
                                      .value = value,
                                    })
          .unwrap())
{
}

lox::object::object(lak::u8string value)
: _impl(lox::object::impl_ptr::make(lox::object::impl{
                                      .value = value,
                                    })
          .unwrap())
{
}

lox::object::object(double value)
: _impl(lox::object::impl_ptr::make(lox::object::impl{
                                      .value = value,
                                    })
          .unwrap())
{
}

lox::object::object(bool value)
: _impl(lox::object::impl_ptr::make(lox::object::impl{
                                      .value = value,
                                    })
          .unwrap())
{
}

lox::object::object(const lox::callable &value)
: _impl(lox::object::impl_ptr::make(lox::object::impl{
                                      .value = value,
                                    })
          .unwrap())
{
}

lox::object::object(const lox::type &value)
: _impl(lox::object::impl_ptr::make(lox::object::impl{
                                      .value = value,
                                    })
          .unwrap())
{
}

lox::object::object(const lox::instance &value)
: _impl(lox::object::impl_ptr::make(lox::object::impl{
                                      .value = value,
                                    })
          .unwrap())
{
}

bool lox::object::is_truthy() const
{
	return visit(lak::overloaded{
	  [&](lak::monostate) -> bool { return false; },
	  [&](const bool &b) -> bool { return b; },
	  [&](auto &&) -> bool { return true; },
	});
}

lox::object::value_type &lox::object::value()
{
	return _impl->value;
}

const lox::object::value_type &lox::object::value() const
{
	return _impl->value;
}

const lak::u8string *lox::object::get_string() const
{
	return _impl->value.template get<lak::u8string>();
}

const double *lox::object::get_number() const
{
	return _impl->value.template get<double>();
}

const bool *lox::object::get_bool() const
{
	return _impl->value.template get<bool>();
}

const lox::callable *lox::object::get_callable() const
{
	if_ref (const lox::type & t, _impl->value.template get<lox::type>())
		return &t.constructor();
	else
		return _impl->value.template get<lox::callable>();
}

const lox::type *lox::object::get_type() const
{
	return _impl->value.template get<lox::type>();
}

lox::instance *lox::object::get_instance()
{
	return _impl->value.template get<lox::instance>();
}

const lox::instance *lox::object::get_instance() const
{
	return _impl->value.template get<lox::instance>();
}

bool lox::object::operator==(const lox::object &rhs) const
{
	if (_impl->value.index() != rhs._impl->value.index()) return false;

	return visit(lak::overloaded{
	  [&](lak::monostate) -> bool { return true; },
	  [&](const lak::u8string &str) -> bool
	  { return str == *rhs._impl->value.template get<lak::u8string>(); },
	  [&](const double &number) -> bool
	  { return number == *rhs._impl->value.template get<double>(); },
	  [&](const bool &b) -> bool
	  { return b == *rhs._impl->value.template get<bool>(); },
	  [&](const lox::callable &c) -> bool
	  { return c == *rhs._impl->value.template get<lox::callable>(); },
	  [&](const lox::type &t) -> bool
	  { return t == *rhs._impl->value.template get<lox::type>(); },
	  [&](const lox::instance &i) -> bool
	  { return i == *rhs._impl->value.template get<lox::instance>(); },
	});
}

bool lox::object::operator!=(const lox::object &rhs) const
{
	return !operator==(rhs);
}

lak::u8string lox::object::to_string() const
{
	return visit(lak::overloaded{
	  [&](lak::monostate) -> lak::u8string { return u8"nil"; },
	  [&](const lak::u8string &str) -> lak::u8string
	  { return u8"\"" + str + u8"\""; },
	  [&](const double &number) -> lak::u8string
	  { return lak::as_u8string(std::to_string(number)).to_string(); },
	  [&](const bool &b) -> lak::u8string { return b ? u8"true" : u8"false"; },
	  [&](const lox::callable &c) -> lak::u8string { return c.to_string(); },
	  [&](const lox::type &t) -> lak::u8string { return t.to_string(); },
	  [&](const lox::instance &i) -> lak::u8string { return i.to_string(); },
	});
}
