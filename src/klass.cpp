#include "klass.hpp"

lox::klass_ptr lox::klass::make(std::u8string_view name,
                                std::shared_ptr<const lox::klass> superclass,
                                lox::string_map<char8_t, lox::object> methods)
{
	lox::klass_ptr result = std::make_shared<lox::klass>(lox::klass{});
	result->name          = name;
	result->superclass    = superclass;
	result->methods       = std::move(methods);
	result->constructor   = lox::callable::make_constructor({.klass = result});
	return result;
}

std::shared_ptr<lox::callable> lox::klass::find_method(
  std::u8string_view method_name) const
{
	if (auto method = methods.find(method_name);
	    method != methods.end() &&
	    std::holds_alternative<std::shared_ptr<lox::callable>>(
	      method->second.value))
		return std::get<std::shared_ptr<lox::callable>>(method->second.value);
	else if (superclass)
		return superclass->find_method(method_name);
	else
		return {};
}

std::shared_ptr<lox::callable> lox::klass::find_method(
  const lox::token &method_name) const
{
	return find_method(method_name.lexeme);
}

std::shared_ptr<lox::callable> lox::klass::find_bound_method(
  std::u8string_view method_name,
  std::shared_ptr<lox::instance> instance) const
{
	if (auto method = methods.find(method_name);
	    method != methods.end() &&
	    std::holds_alternative<std::shared_ptr<lox::callable>>(
	      method->second.value))
	{
		std::shared_ptr<lox::callable> callable =
		  std::get<std::shared_ptr<lox::callable>>(method->second.value);

		if (std::holds_alternative<lox::callable::interpreted>(callable->value))
		{
			const lox::callable::interpreted &interpreted =
			  std::get<lox::callable::interpreted>(callable->value);

			lox::environment_ptr env = lox::environment::make(interpreted.closure);

			env->emplace(u8"this", lox::object{instance});

			return std::make_shared<lox::callable>(lox::callable::make_interpreted({
			  .func    = interpreted.func,
			  .closure = env,
			  .is_init = interpreted.is_init,
			}));
		}
		else
			return callable;
	}
	else if (superclass)
		return superclass->find_bound_method(method_name, instance);
	else
		return {};
}

std::shared_ptr<lox::callable> lox::klass::find_bound_method(
  const lox::token &method_name, std::shared_ptr<lox::instance> instance) const
{
	return find_bound_method(method_name.lexeme, std::move(instance));
}

std::u8string lox::klass::to_string() const
{
	return name;
}

lox::instance_ptr lox::instance::make(
  lox::klass_ptr klass, lox::string_map<char8_t, lox::object> fields)
{
	lox::instance_ptr result = std::make_shared<lox::instance>(lox::instance{});
	result->klass            = klass;
	result->fields           = std::move(fields);
	return result;
}

const lox::object &lox::instance::emplace(const lox::token &name,
                                          lox::object value)
{
	return fields.insert_or_assign(std::u8string(name.lexeme), std::move(value))
	  .first->second;
}

std::optional<lox::object> lox::instance::find(const lox::token &name)
{
	if (auto field = fields.find(name.lexeme); field != fields.end())
		return std::make_optional<lox::object>(field->second);
	else if (std::shared_ptr<lox::callable> method =
	           klass->find_bound_method(name.lexeme, shared_from_this());
	         method != nullptr)
		return std::make_optional<lox::object>(lox::object{method});
	else
		return std::nullopt;
}

std::u8string lox::instance::to_string() const
{
	return klass->name + u8" instance";
}

bool lox::instance::operator==(const lox::instance &rhs) const
{
	return klass == rhs.klass;
}

bool lox::instance::operator!=(const lox::instance &rhs) const
{
	return !operator==(rhs);
}
