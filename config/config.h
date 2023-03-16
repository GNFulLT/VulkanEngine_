#ifndef CONFIG_H
#define CONFIG_H

#include <unordered_map>
#include <typeindex>
#include <any>
#include <cassert>


#include "../core/object/object.h"
#include "config_property.h"

template<typename T>
struct ConfigPropRegistery
{
	const char* id;
	std::shared_ptr<ConfigProperty<T>> conf;

	ConfigPropRegistery(const char* id, std::shared_ptr<ConfigProperty<T>> confProp) : id(id),conf(confProp){}

	typedef T value_type;
};

template<typename T, typename U = typename T::value_type>
concept ConfigPropRegistery_Concept = std::convertible_to<T, ConfigPropRegistery<U>>;

class Config : public Object
{
	OBJECT_DEF(Config, Object)
public:
	virtual ~Config() = default;
	Config(const Config&) = delete;
	Config& operator=(const Config&) = delete;

	template<typename... Registries>
	requires (ConfigPropRegistery_Concept<Registries> && ...)
	Config(Object* owner,const Registries&... registries) : m_owner(owner)
	{
		([&]
			{
				String id = String(registries.id);
				add_config_prop(id, static_cast<std::shared_ptr<ConfigProperty<typename Registries::value_type>>>(registries.conf));
			} (), ...);
	}

	_INLINE_ Object* get_owner()
	{			
		return m_owner;
	}

	template<typename T>
	_INLINE_ ConfigProperty<T>* get_config_prop(const String& id) const
	{
		assert(m_propMap.find(typeid(T)) != m_propMap.end());
		assert(m_propMap.find(typeid(T))->second.find(id) != m_propMap.find(typeid(T))->second.end());
		
		return std::any_cast<std::shared_ptr<ConfigProperty<T>>>(m_propMap.find(typeid(T))->second.find(id)->second).get();
	}

	template<typename T>
	_INLINE_ ConfigProperty<T>* try_get_config_prop(const String& id) const
	{
		if (m_propMap.find(typeid(T)) == m_propMap.end())
		{
			return nullptr;
		}
		if(m_propMap.find(typeid(T))->second.find(id) == m_propMap.find(typeid(T))->second.end())
		{
			return nullptr;
		}
		return std::any_cast<std::shared_ptr<ConfigProperty<T>>>(m_propMap.find(typeid(T))->second.find(id)->second).get();
	}

	template<typename T>
	_INLINE_ boost::signals2::connection listen_config_prop(const String& id,std::function<void(const T&)> onPropChanged)
	{
		assert(m_propMap.find(typeid(T)) != m_propMap.end());
		assert(m_propMap.find(typeid(T))->second.find(id) != m_propMap.find(typeid(T))->second.end());

		std::any_cast<std::shared_ptr<ConfigProperty<T>>>(m_propMap.find(typeid(T))->second.find(id)->second)->subscribe_changed_event(onPropChanged);
	}

	template<typename T>
	_INLINE_ void set_config_prop(const String& id,const T& newValue)
	{
		assert(m_propMap.find(typeid(T)) != m_propMap.end());
		assert(m_propMap.find(typeid(T))->second.find(id) != m_propMap.find(typeid(T))->second.end());
		std::any_cast<std::shared_ptr<ConfigProperty<T>>>(m_propMap.find(typeid(T))->second.find(id)->second)->set_prop(newValue);
	}

	virtual bool is_serializable();
	virtual bool serialize(const String& key, const String& path);
	virtual bool serialize(const String& path);
	
protected:

	template<typename T>
	_INLINE_ void add_config_prop(String& id, std::shared_ptr<ConfigProperty<T>> prop)
	{
		
		// Already a unordered map created for this type
		if (std::unordered_map<std::type_index, std::unordered_map<std::string, std::any>>::iterator idMap = m_propMap.find(typeid(T));idMap != m_propMap.end())
		{
			idMap->second.emplace(id,prop);
		}
		else
		{
			// No created map First create map
			m_propMap.emplace(typeid(T), std::unordered_map<std::string, std::any>());
			m_propMap[typeid(T)].emplace(id,prop);
		}
		
	}

protected:

	std::unordered_map<std::type_index, std::unordered_map<std::string,std::any>> m_propMap;

	Object* m_owner;
private:
};


template<typename T>
concept Configuration = std::convertible_to<T,Config>;


#endif // CONFIG_H