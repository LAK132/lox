#ifndef LOX_KLASS_HPP
#define LOX_KLASS_HPP

#include "callable.hpp"
#include "object.hpp"
#include "string_map.hpp"
#include "token.hpp"

#include <memory>
#include <optional>
#include <string>

namespace lox
{
	struct instance;

	struct klass : public std::enable_shared_from_this<klass>
	{
	private:
		klass() = default;

	public:
		klass(klass &&) = default;
		klass &operator=(klass &&) = default;

		std::u8string name;

		lox::string_map<char8_t, lox::object> methods;

		lox::callable constructor;

		static std::shared_ptr<lox::klass> make(
		  std::u8string_view name, lox::string_map<char8_t, lox::object> methods);

		std::shared_ptr<lox::callable> find_method(
		  std::u8string_view method_name) const;
		std::shared_ptr<lox::callable> find_method(
		  const lox::token &method_name) const;

		std::shared_ptr<lox::callable> find_bound_method(
		  std::u8string_view method_name,
		  std::shared_ptr<lox::instance> instance) const;
		std::shared_ptr<lox::callable> find_bound_method(
		  const lox::token &method_name,
		  std::shared_ptr<lox::instance> instance) const;

		std::u8string to_string() const;
	};

	using klass_ptr = std::shared_ptr<lox::klass>;

	struct instance : public std::enable_shared_from_this<instance>
	{
	private:
		instance() = default;

	public:
		instance(instance &&) = default;
		instance &operator=(instance &&) = default;

		lox::klass_ptr klass;
		lox::string_map<char8_t, lox::object> fields;

		static std::shared_ptr<lox::instance> make(
		  lox::klass_ptr klass, lox::string_map<char8_t, lox::object> fields);

		const lox::object &emplace(const lox::token &name, lox::object value);

		std::optional<lox::object> find(const lox::token &name);

		std::u8string to_string() const;

		bool operator==(const instance &rhs) const;

		bool operator!=(const instance &rhs) const;
	};

	using instance_ptr = std::shared_ptr<lox::instance>;
}

#endif
