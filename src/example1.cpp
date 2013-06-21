#include "decodeJSON.h"
#include <iostream>

using namespace JSON;
using namespace std;

int main() {
  char *jsonstring = (char *)"{ \"a\": 1, \"b\": \"hello\", \"c\": [5,6,7] }";
  Object *v=(Object *)decodeJSON(jsonstring);

  cout << "a=" << ((Number *)v->value["a"])->value << "\n";
  cout << "b=" << ((String *)v->value["b"])->value << "\n";
  cout << "c=";
  
  Array *c=(Array *)v->value["c"];
  int i;
  for (i=0; i<c->value.size(); i++)
    cout << ((Number *)c->value[i])->value << " ";
  cout << "\n";

  return 0;
}
