#include "environment.hpp"

lox::environment *find_ancestor(lox::environment *env, size_t distance)
{
	while (env && distance-- > 0) env = env->enclosing.get();
	return env;
}

const lox::object &lox::environment::emplace(std::u8string_view k,
                                             lox::object v)
{
	return values.insert_or_assign(std::u8string(k), std::move(v)).first->second;
}

const lox::object &lox::environment::emplace(const lox::token &k,
                                             lox::object v)
{
	return emplace(k.lexeme, std::move(v));
}

const lox::object *lox::environment::find(std::u8string_view k)
{
	if (auto it = values.find(k); it != values.end())
		return &it->second;
	else if (enclosing)
		return enclosing->find(k);
	else
		return nullptr;
}

const lox::object *lox::environment::find(std::u8string_view k,
                                          size_t distance)
{
	lox::environment *env = find_ancestor(this, distance);
	return env ? env->find(k) : nullptr;
}

const lox::object *lox::environment::find(const lox::token &k)
{
	return find(k.lexeme);
}

const lox::object *lox::environment::find(const lox::token &k, size_t distance)
{
	return find(k.lexeme, distance);
}

const lox::object *lox::environment::replace(const lox::token &k,
                                             lox::object v)
{
	if (auto it = values.find(k.lexeme); it != values.end())
	{
		it->second = std::move(v);
		return &it->second;
	}
	else if (enclosing)
		return enclosing->replace(k, std::move(v));
	else
		return nullptr;
}

const lox::object *lox::environment::replace(const lox::token &k,
                                             lox::object v,
                                             size_t distance)
{
	lox::environment *env = find_ancestor(this, distance);
	return env ? env->replace(k, std::move(v)) : nullptr;
}

lox::environment_ptr lox::environment::make(lox::environment_ptr enclosing)
{
	return std::make_shared<lox::environment>(
	  lox::environment{.enclosing = enclosing});
}
