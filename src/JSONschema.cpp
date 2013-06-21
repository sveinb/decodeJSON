/*

Copyright (c) 2013, Svein Berge
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL SVEIN BERGE BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
