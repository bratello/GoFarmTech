//  Descriptable.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef DESCRIPTABLE_H
#define DESCRIPTABLE_H
#include	<Arduino.h>
#include	<Arduino_JSON.h>
#include	<map>
#include	<list>
#include 	"Convertors.h"


class Description {
public:
	typedef	
		std::function<void(const Description& parent, const Description& child)> 
							description_trapper_t;
	enum Type {
		boolean,
		string,
		number,
		array,
		object,
		mixed
	};

	enum Access {
		read,
		write,
		all
	};
protected:
	typedef
		std::map<String, String>
							properties_map_t;
	typedef
		std::list<String>
							description_list_t;
protected:
	static description_trapper_t	_trapper;
protected:
	properties_map_t			_properties;
	description_list_t			_children;
	description_list_t			_attributes;
public:
	static void setDescriptionTrapper(const description_trapper_t& trapper = [](const Description& parent, const Description& child){return;});
public:
	Description();
	Description(const Description& v);
	void   setType(const Type& v);
	void   setAccess(const Access& v);
	void   setName(const String& v);
	void   setVersion(const int& v);
	void   addChild(const Description& v);
	void   addAttribute(const Description& attr);
	String toJSON() const;
	void   clear();
	bool   isEmpty();

	void   setDefaultValue(const JSONVar& val);
	void   setValue(const JSONVar& val);

public:
	String getName() const;
	String getValue() const;
};

class Descriptable {
public:
	virtual ~Descriptable();
	virtual Description getDescription() = 0;
};


#define DESCRIPTABLE(tp, acc, defVal) virtual Description getDescription() { \
Description meta; \
meta.setName(getModuleName()); \
meta.setType(Description::Type::tp); \
meta.setAccess(Description::Access::acc); \
meta.setDefaultValue(defVal); \
meta.setValue(getValue()); \
Description enabledMeta;\
enabledMeta.setName("enabled");\
enabledMeta.setType(Description::Type::boolean);\
enabledMeta.setAccess(Description::Access::all);\
enabledMeta.setDefaultValue(true);\
enabledMeta.setValue(isEnabled());\
meta.addAttribute(enabledMeta);\
return meta; \
}

#endif