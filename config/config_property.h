#ifndef CONFIG_PROPERTY_H
#define CONFIG_PROPERTY_H

#include "../core/object/property.h"
#include "../core/typedefs.h"
#include "../manager/memory_manager.h"

#include <boost/signals2.hpp>
#include <concepts>
//
//template <typename T>
//concept CanbeConfigPropConcept = not std::convertible_to<T, Object>;

template<typename T>
class ConfigProperty : protected Property
{
public:
	typedef boost::signals2::signal<void(const T&)> ConfigChangedSignal;

	ConfigProperty() : Property()
	{

	}

	ConfigProperty(Object* owner,const T& defaultArg,const char* name, PROPERTY_HINT hint = PROPERTY_HINT_NILL) : Property(owner,MemoryManager::get_singleton()->create_argument(defaultArg), name, PROPERTY_USAGE_CONFIG, hint)
	{
	}

	ConfigProperty(const ConfigProperty&) = delete;
	ConfigProperty& operator=(const ConfigProperty&) = delete;

	_F_INLINE_ T* get() const
	{
		return (((Property*)this))->get<T>();
	}

	_INLINE_ void set(const T& prop)
	{
		change<T>(prop);
		m_listeners(prop);
	}

	_INLINE_ boost::signals2::connection subscribe_changed_event(const typename ConfigProperty<T>::ConfigChangedSignal::slot_type& fnc) const
	{
		return m_listeners.connect(fnc);
	}

private:
	mutable typename ConfigProperty<T>::ConfigChangedSignal m_listeners;

};


#endif // CONFIG_PROPERTY_H