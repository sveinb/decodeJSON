//
//  JSONschema.h
//  offcourse
//
//  Created by Svein Berge on 28.03.13.
//
//

#ifndef __offcourse__JSONschema__
#define __offcourse__JSONschema__

#include <vector>
#include <string>
#include <iostream>

#include "decodeJSON.h"

namespace JSONSchema {
  void schemaerror(JSON::Value *v,
		   JSON::ErrFunc *err, void *errData);
  class Type;

  /**
     Converts a JSON string into a C++ object with JSON hooks.
     
     @t The type of the object, given as a Type object.
     @obj A pointer to the object

     @return true if the conversion was successful, false otherwise.

     For complete documentation, see http://github.com/sveinb/decodeJSON/wiki

   */

  bool decodeJSON(std::string str, Type *t, void *ret,
		  JSON::ErrFunc *err=NULL, void *errData=NULL);

  /**
     Converts a C++ object with JSON hooks into a string.
     
     @t The type of the object, given as a Type object.
     @obj A pointer to the object

     @return A JSON string.

     For complete documentation, see http://github.com/sveinb/decodeJSON/wiki
  */

  std::string encodeJSON(Type *t, void *obj);

  bool convertJSON(JSON::Value *v, Type *t, void *ret,
		   JSON::ErrFunc *err, void *errData);

  /**
     Describes the type of the C++ object that a JSON string
     should populate.

     For complete documentation, see http://github.com/sveinb/decodeJSON/wiki
   */

  class Type {
  public:
    virtual JSON::Value::type getType()=0;
//    virtual void *create()=0;
    virtual bool fill(JSON::Value *v, void *ret,
		      JSON::ErrFunc *err, void *errData)=0;
    virtual std::string encode(void *obj)=0;
  };

  template<class T> class Array : public Type {
  public:
    Array(Type *at):
    t(at) {};
    
    JSON::Value::type getType() {
      return JSON::Value::array;
    }
    
    Type *elementType() {
      return t;
    }
    
    bool fill(JSON::Value *v, void *ret,
	      JSON::ErrFunc *err, void *errData) {
      JSON::Array *a=(JSON::Array *)v;
      std::vector<T> *o=(std::vector<T> *)ret;
      o->resize(a->value.size());
      int i;
      for (i=0; i<a->value.size(); i++) {
        if (!convertJSON(a->value[i], elementType(), &((*o)[i]),
			 err, errData))
          return false;
      }
      return true;
    }
    
    std::string encode(void *obj) {
      std::string ret("[");
      int i;
      std::vector<T> *o=(std::vector<T> *)obj;
      for (i=0; i<o->size(); i++) {
        ret+=encodeJSON(elementType(), &((*o)[i]));
        if (i<o->size()-1)
          ret+=",";
      }
      ret+=std::string("]");
      return ret;
    }
    
    Type *t;
  };
  
  
  template<> bool Array<bool>::fill(JSON::Value *v, void *ret,
				    JSON::ErrFunc *err, void *errData);
  template<> std::string Array<bool>::encode(void *obj);

  template<class T> class Map : public Type {
  public:
    Map(Type *at):
    t(at) {};
    
    JSON::Value::type getType() {
      return JSON::Value::object;
    }
    
    Type *elementType() {
      return t;
    }
    
    bool fill(JSON::Value *v, void *ret,
	      JSON::ErrFunc *err, void *errData) {
      JSON::Object *a=(JSON::Object *)v;
      std::map<std::string, T> *o=(std::map<std::string, T> *)ret;
      std::map<std::string, JSON::Value *>::iterator I;
      for (I=a->value.begin(); I!=a->value.end(); ++I) {
        std::pair<std::string, T> e;
        if (!convertJSON(I->second, elementType(), &e.second,
			 err, errData))
          return false;
        e.first=I->first;
        o->insert(e);
      }
      return true;
    }
    
    std::string encode(void *obj) {
      std::string ret("{");
      std::map<std::string, T> *o=(std::map<std::string, T> *)obj;
      typename std::map<std::string, T>::iterator I;
      for (I=o->begin(); I!=o->end();) {
        ret+=std::string("\"")+I->first+std::string("\":");
        ret+=encodeJSON(elementType(), &I->second);
        ++I;
        if (I!=o->end())
          ret+=",";
      }
      ret+=std::string("}");
      return ret;
    }

    Type *t;
  };
  

  class NumberClass : public Type {
  public:
    JSON::Value::type getType() {
      return JSON::Value::number;
    }
    bool fill(JSON::Value *v, void *ret,
	      JSON::ErrFunc *err, void *errData) {
      *(double *)ret=((JSON::Number *)v)->value;
      return true;
    }
    std::string encode(void *obj) {
      char buf[80];
      sprintf(buf, "%g", *(double *)obj);
      return std::string(buf);
    }
  };

  class StringClass : public Type {
  public:
    JSON::Value::type getType() {
      return JSON::Value::string;
    }
    bool fill(JSON::Value *v, void *ret,
	      JSON::ErrFunc *err, void *errData) {
      *(std::string *)ret=((JSON::String *)v)->value;
      return true;
    }
    std::string encode(void *obj) {
      return std::string("\"")+*(std::string *)obj+std::string("\"");
    }
  };

  class BoolClass : public Type {
  public:
    JSON::Value::type getType() {
      return JSON::Value::boolean;
    }
    bool fill(JSON::Value *v, void *ret,
	      JSON::ErrFunc *err, void *errData) {
      *(bool *)ret=((JSON::Boolean *)v)->value;
      return true;
    }
    std::string encode(void *obj) {
      return *(bool *)obj?"true":"false";
    }
  };
  
  template<class T> class Object : public Type {
  public:
    JSON::Value::type getType() {
      return JSON::Value::object;
    }
    
    int nMembers() {
      return sizeof(T::memberType)/sizeof(T::memberType[0]);
    }
    
    std::string memberName(int n) {
      return T::memberName[n];
    }
    
    Type *memberType(int n) {
      return T::memberType[n];
    }
    
    std::string encode(void *obj) {
      ((T *)obj)->freeze();
      std::string ret("{");
      int i;
      for (i=0; i<nMembers(); i++) {
        ret+=std::string("\"")+memberName(i)+std::string("\":");
        ret+=encodeJSON(memberType(i), ((T *)obj)->member(i));
        if (i<nMembers()-1)
          ret+=",";
      }
      ret+=std::string("}");
      return ret;
    }
    
    bool fill(JSON::Value *v, void *ret,
	      JSON::ErrFunc *err, void *errData) {
      JSON::Object *o=(JSON::Object *)v;
      int i;
      for (i=0; i<nMembers(); i++) {
        std::map<std::string, JSON::Value *>::iterator F;
        F=o->value.find(memberName(i));
        if (F!=o->value.end()) {
          JSON::Value *v=F->second;
          void *p=((T *)ret)->member(i);
          if (!convertJSON(v, memberType(i), p, err, errData)) {
            return false;
          }
        }
      }
      if (!((T *)ret)->unfreeze(err, errData)) {
        schemaerror(v, err, errData);
        return false;
      }
      return true;
    }
    
  };
  
  template<class T> class List : public Type {
  public:
    JSON::Value::type getType() {
      return JSON::Value::array;
    }
    
    int nElements() {
      return sizeof(T::elementType)/sizeof(T::elementType[0]);
    }
    
    Type *elementType(int n) {
      return T::elementType[n];
    }
    
    std::string encode(void *obj) {
      ((T *)obj)->freeze();
      std::string ret("[");
      int i;
      for (i=0; i<nElements(); i++) {
        ret+=encodeJSON(elementType(i), ((T *)obj)->element(i));
        if (i<nElements()-1)
          ret+=",";
      }
      ret+=std::string("]");
      return ret;
    }
    
    bool fill(JSON::Value *v, void *ret,
	      JSON::ErrFunc *err, void *errData) {
      JSON::Array *o=(JSON::Array *)v;
      int i;
      for (i=0; i<o->value.size() &&
	     i<nElements(); i++) {
	JSON::Value *v=o->value[i];
	void *p=((T *)ret)->element(i);
	if (!convertJSON(v, elementType(i), p,
			 err, errData)) {
	  return false;
	}
      }
      if (!((T *)ret)->unfreeze(err, errData)) {
        schemaerror(v, err, errData);
        return false;
      }
      return true;
    }
    
  };
  
  extern NumberClass *Number;
  extern StringClass *String;
  extern BoolClass *Bool;
  extern Array<std::string> *StringArray;
  extern Array<double> *NumberArray;
  extern Array<bool> *BoolArray;
#define ObjectArray(class) new Array<class>(new Object<class>)
#define ListArray(class) new Array<class>(new List<class>)
  extern Map<std::string> *StringMap;
  extern Map<double> *NumberMap;
  extern Map<bool> *BoolMap;
#define ObjectMap(class) new Map<class>(new Object<class>)
#define ListMap(class) new Map<class>(new List<class>)
  
  
  static class JSONSchema_Initializer {
  public:
    JSONSchema_Initializer();
  } JSONSchema_initializer;
  
}


#endif /* defined(__offcourse__JSONschema__) */
