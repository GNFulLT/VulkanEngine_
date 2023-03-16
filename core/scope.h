#ifndef SCOPE_H
#define SCOPE_H


#include <functional>


class Scope
{
public:
	Scope(std::function<void()> onScopeBegins, std::function<void()> onScopeDestroys) : m_scopeDestroys(onScopeDestroys)
	{
		onScopeBegins();
	}

	explicit operator bool() const
	{
		return true;
	}

	Scope(const Scope&) = delete;
	Scope& operator=(const Scope&) = delete;

	~Scope()
	{
		m_scopeDestroys();
	}

private:
	std::function<void()> m_scopeDestroys;

};

#endif // SCOPE_H