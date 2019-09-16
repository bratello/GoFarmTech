//  Descriptable.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"Descriptable.h"

Description::description_trapper_t Description::_trapper = [](const Description& parent, const Description& child){};
void Description::setDescriptionTrapper(const description_trapper_t& trapper) {
	_trapper = trapper;
}

Description::Description() {
	clear();
}

Description::Description(const Description& v) {
	_properties = v._properties;
	_attributes = v._attributes;
	_children = v._children;
}

void Description::setType(const Type& v) {
	_properties["tp"] = JSONVar::stringify(JSONVar(v));
}

void Description::setAccess(const Access& v) {
	_properties["ac"] = JSONVar::stringify(JSONVar(v));
}

void Description::setName(const String& v) {
	_properties["nm"] = JSONVar::stringify(JSONVar(v));
}

String Description::getName() const {
	auto it = _properties.find("nm");
	if(it != _properties.end()) {
		auto name = it->second;
		name.replace("\"", "");
		return name;
	}
	return String();
}

String Description::getValue() const {
	auto it = _properties.find("v");
	if(it != _properties.end()) {
		return it->second;
	}
	return String();
}

void Description::setVersion(const int& v) {
	_properties["ver"] = JSONVar::stringify(JSONVar(v));
}

void Description::addChild(const Description& v) {
	::yield();
	_trapper(*this, v);
	_children.push_back(v.toJSON());
}

void Description::addAttribute(const Description& attr) {
	::yield();
	_trapper(*this, attr);
	_attributes.push_back(attr.toJSON());
}

void  Description::setDefaultValue(const JSONVar& val) {
	_properties["dv"] = JSONVar::stringify(val);
}

void  Description::setValue(const JSONVar& val) {
	_properties["v"] = JSONVar::stringify(val);
}

String 	Description::toJSON() const {
	::yield();
	String ret = "{";
	auto print_properties = [this, &ret] () {
		int i = 0;
		for(const auto& prop : _properties) {
			::yield();
			if(i++ != 0) {
				ret += ",";
			}
			ret += "\"";
			ret += prop.first;
			ret += "\":";
			ret += prop.second;
		}
	};

	auto print_array = [this, &ret](const description_list_t& items, const char* name) {
		if(!items.size()) {
			return;
		}
		int i = 0;
		ret += String(",\"") + name + "\":[";
		for(const auto& item : items) {
			::yield();
			if(i++ != 0) {
				ret += ",";
			}
			ret += item;
		}
		ret += "]";
	};
	print_properties();
	print_array(_attributes, "ats");
	print_array(_children, "its");
	ret += "}";
	return  ret;
}

void	Description::clear() {
	_properties.clear();
	_attributes.clear();
	_children.clear();
}

bool 	Description::isEmpty() {
	return _properties.size() == 0;
}

Descriptable::~Descriptable() {}