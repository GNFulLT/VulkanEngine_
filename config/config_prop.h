#ifndef CONFIG_PROP_H
#define CONFIG_PROP_H


#include <concepts>
#include "../core/typedefs.h"
#include <functional>
#include <boost/signals2.hpp>


#include "../core/object/object.h"

template <typename T>
concept CanbeConfigPropConcept = not std::convertible_to<T,Object>;

template<CanbeConfigPropConcept T>
class ConfigProp : public Object
{
	OBJECT_DEF(ConfigProp, Object)
public:
	typedef boost::signals2::signal<void(const T&)> ConfigChangedSignal;
	ConfigProp(const T& initalValue)
	{
		m_prop = initalValue;
	}

	ConfigProp(const T& initalValue,const typename ConfigProp<T>::ConfigChangedSignal::slot_type& ownerCallback,boost::signals2::connection* con)
	{
		m_prop = initalValue;

		// ConfigProp firstly wil call owner
		(*con) =  m_listeners.connect(ownerCallback);

	}
	ConfigProp(const ConfigProp&) = delete;
	ConfigProp& operator=(const ConfigProp&) = delete;

	inline boost::signals2::connection subscribe_changed_event(const typename ConfigProp<T>::ConfigChangedSignal::slot_type& fnc) const
	{
		return m_listeners.connect(fnc);
	}
	
	_INLINE_ const T* get_prop() const 
	{
		return &m_prop;
	}

	_INLINE_ void set_prop(const T& prop)
	{
		m_prop = prop;
		m_listeners(prop);
	}

	typedef T value_type;
private:
	T m_prop;
	mutable typename ConfigProp<T>::ConfigChangedSignal m_listeners;
};

template<typename T,typename U = typename T::value_type>
concept ConfigurationProp = std::convertible_to<T, ConfigProp<U>>;

#endif // CONFIG_PROP_H