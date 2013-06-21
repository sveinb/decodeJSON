//
//  JSONschema.cpp
//  offcourse
//
//  Created by Svein Berge on 28.03.13.
//
//

#include "JSONschema.h"
#include "decodeJSON.h"
#include <iostream>

using namespace std;

namespace JSONSchema {
  NumberClass *Number;
  StringClass *String;
  BoolClass *Bool;
  Array<std::string> *StringArray;
  Array<double> *NumberArray;
  Array<bool> *BoolArray;
  Map<std::string> *StringMap;
  Map<double> *NumberMap;
  Map<bool> *BoolMap;
  

  static int nifty_counter;
  JSONSchema_Initializer::JSONSchema_Initializer() {
    if (nifty_counter++==0) {
      Number=new NumberClass;
      String=new StringClass;
      Bool=new BoolClass;
      StringArray=new Array<std::string>(String);
      NumberArray=new Array<double>(Number);
      BoolArray=new Array<bool>(Bool);
      StringMap=new Map<std::string>(String);
      NumberMap=new Map<double>(Number);
      BoolMap=new Map<bool>(Bool);
    }
  }
  
  template<> bool Array<bool>::fill(JSON::Value *v, void *ret,
				    JSON::ErrFunc *err, void *errData) {
    JSON::Array *a=(JSON::Array *)v;
    std::vector<bool> *o=(std::vector<bool> *)ret;
    o->resize(a->value.size());
    int i;
    for (i=0; i<a->value.size(); i++) {
      if (a->value[i]->getType()!=JSON::Value::boolean) {
        schemaerror(a->value[i], err, errData);
        return false;
      }
      (*o)[i]=((JSON::Boolean *)a->value[i])->value;
    }
    return true;
  }

  template<> std::string Array<bool>::encode(void *obj) {
    std::string ret("[");
    int i;
    std::vector<bool> *o=(std::vector<bool> *)obj;
    for (i=0; i<o->size(); i++) {
      bool tmp=((*o)[i]);
      ret+=encodeJSON(Bool, &tmp);
      if (i<o->size()-1)
        ret+=",";
    }
    ret+=std::string("]");
    return ret;
  }

  static void defaultError(void *dummy, string msg) {
    cerr << msg << "\n";
  }

  void schemaerror(JSON::Value *v,
		   JSON::ErrFunc *err, void *errData) {
    char buf[80];
    sprintf(buf, "JSONSchema: Type error at line no %d", v->lineno);
    err(errData, buf);
  }
  
  bool decodeJSON(std::string str, Type *t, void *ret,
		  JSON::ErrFunc *err, void *errData) {
    JSON::Value *v=JSON::decodeJSON(str);
    if (!v)
      return false;
    bool ok=convertJSON(v, t, ret, err, errData);
    delete v;
    return ok;
  }
  
  bool convertJSON(JSON::Value *v, Type *t, void *ret,
		   JSON::ErrFunc *err, void *errData) {
    if (v->getType()!=t->getType()) {
      schemaerror(v, err, errData);
      return false;
    }
    return t->fill(v, ret, err, errData);
  }

  std::string encodeJSON(Type *t, void *obj) {
    return t->encode(obj);
  }

  
}
