#include "environment.hpp"

lox::environment::iterator lox::environment::emplace(const lox::token &k,
                                                     lox::object v)
{
	return values.insert_or_assign(std::u8string(k.lexeme), std::move(v)).first;
}

lox::environment::iterator lox::environment::find(const lox::token &k)
{
	auto it = values.find(std::u8string(k.lexeme));
	if (it == values.end() && enclosing) it = enclosing->find(k);
	return it;
}

lox::environment::iterator lox::environment::replace(const lox::token &k,
                                                     lox::object v)
{
	auto it = find(k);
	if (it != values.end()) it->second = std::move(v);
	return it;
}

lox::environment_ptr lox::environment::make(lox::environment_ptr enclosing)
{
	return std::make_shared<lox::environment>(
	  lox::environment{.enclosing = enclosing});
}
