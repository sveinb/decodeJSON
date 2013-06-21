#include "JSONschema.h"
#include <iostream>

using namespace JSONSchema;
using namespace std;

class MyClass {
public:
  // === JSON hooks start here === 

  static string memberName[];
  static Type *memberType[];
  void *member(int n);
  bool unfreeze(JSON::ErrFunc *err, void *errData) {return true;}
  void freeze() {}

  // === JSON hooks end here === 

  double a;
  string b;
  vector<double> c;
};

string MyClass::memberName[]={
  "a",
  "b",
  "c"
};

Type *MyClass::memberType[]={
  Number,
  String,
  NumberArray
};

void *MyClass::member(int n) {
  switch(n) {
  case 0: return &a; break;
  case 1: return &b; break;
  case 2: return &c; break;
  }
}

static Type *myClassType=new Object<MyClass>;

int main() {
  char *jsonstring = (char *)"{ \"a\": 1, \"b\": \"hello\", \"c\": [5,6,7] }";
  MyClass myObject;

  if (!decodeJSON(jsonstring, myClassType, &myObject))
    exit(1);

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
