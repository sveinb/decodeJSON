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
