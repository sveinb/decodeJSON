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
#include <iostream>

using namespace JSONSchema;
using namespace std;

class MyClass {
public:
  // === JSON hooks start here === 

  static Type *elementType[];

  void *element(int n) {
    switch(n) {
    case 0: return &a; break;
    case 1: return &b; break;
    case 2: return &c; break;
    }
  }

  bool unfreeze(JSON::ErrFunc *err, void *errData) {
    return true;
  }
  
  void freeze() {}

  // === JSON hooks end here === 

  double a;
  string b;
  vector<double> c;
};

Type *MyClass::elementType[]={
  Number,
  String,
  NumberArray
};

static Type *myClassType=new List<MyClass>;

int main() {
  char *jsonstring = (char *)"[1, \"hello\", [5,6,7] ]";
  MyClass myObject;

  decodeJSON(jsonstring, myClassType, &myObject);

  cout << "a=" << myObject.a << "\n";
  cout << "b=" << myObject.b << "\n";
  cout << "c=";

  int i;
  for (i=0; i<myObject.c.size(); i++)
    cout << myObject.c[i] << " ";
  cout << "\n";

  cout << encodeJSON(myClassType, &myObject) <<"\n";

  return 0;
}
