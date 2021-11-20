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
	struct klass;
	struct instance;

	struct object
	{
		using callable_ref = std::shared_ptr<lox::callable>;

		using value_type = std::variant<std::monostate,
		                                std::u8string,
		                                double,
		                                bool,
		                                callable_ref,
		                                std::shared_ptr<lox::klass>,
		                                std::shared_ptr<lox::instance>>;

		value_type value;

		// move these to the .cpp so we have access to the complete callable type
		object();
		object(const object &);
		object(object &&);
		~object();
		object &operator=(const object &);
		object &operator=(object &&);

		object(const value_type &v);
		object(value_type &&v);
		object &operator=(const value_type &v);
		object &operator=(value_type &&v);

		std::u8string to_string() const;

		bool is_truthy() const;

		const std::u8string *get_string() const;

		const double *get_number() const;

		const bool *get_bool() const;

		const lox::callable *get_callable() const;

		// may be empty
		std::shared_ptr<lox::klass> get_klass() const;

		// may be empty
		std::shared_ptr<lox::instance> get_instance() const;

		bool operator==(const lox::object &rhs) const;

		bool operator!=(const lox::object &rhs) const;

		template<typename F>
		auto visit(F &&f)
		{
			return std::visit(f, value);
		}

		template<typename F>
		auto visit(F &&f) const
		{
			return std::visit(f, value);
		}

		friend inline std::ostream &operator<<(std::ostream &strm,
		                                       const object &obj)
		{
			return strm << lox::as_astring_view(obj.to_string());
		}
	};
}

#endif
