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


#ifndef decodeJSON_h
#define decodeJSON_h

#include <string>
#include <ostream>
#include <map>
#include <vector>

namespace JSON {
  
  typedef char jschar;

  /**
     Parent class of the different types of JSON values. Contains
     the line number at which the value is stored, for error reporting.

   */
  
  class Value {
  public:

    /**

       Constructor, called during parsing

     */

    Value (int alineno):
    lineno(alineno) {}

    /**
       
       The six json value types: object, array, string, number, boolean and null.

     */
    
    enum type {
      object,
      array,
      string,
      number,
      boolean,
      null
    };

    /**
      Returns the type of the json value using the type enum.

     */
    
    virtual type getType()=0;

    /**
       Converts the json value to a string. Only for debugging.

    */

    virtual void print(std::ostream &o)=0;

    /**
       Destructor
    */

    virtual ~Value(){}

    int lineno;
  };

  /**
     A JSON object, represented by a std::map<std::string, Value *>

   */
  
  class Object : public Value {
  public:
    /**
       Constructor. Called during parsing.
     */
    
    Object(int lineno):Value(lineno){}

    /**
       The data contained in the object, represented by a std::map<std::string, Value *>.
     */

    std::map<std::string, Value *> value;

    /**
       @return Always returns JSON::Value::object
     */

    type getType() {
      return object;
    }

    /**
       Converts the json value to a string. Only for debugging.

    */

    void print(std::ostream &o);
    ~Object();
  };
  
  /**
     A JSON array, represented by a std::vector<Value *>

   */

  class Array : public Value {
  public:
    /**
       Constructor. Called during parsing.
     */
    Array(int lineno):Value(lineno){}
    /**
       The data contained in the array, represented by a std::vector<Value *>.
     */
    std::vector<Value *> value;
    /**
       @return Always returns JSON::Value::array
     */
    type getType() {
      return array;
    }
    /**
       Converts the json value to a string. Only for debugging.

    */
    void print(std::ostream &o);
    ~Array();
  };
  
  /**
     A JSON number, represented by a double

   */
  class Number : public Value {
  public:
    /**
       Constructor. Called during parsing.
     */

    Number(double an, int lineno):Value(lineno),value(an){};
    /**
       The value of the number, represented by a double
    */
    double value;
    /**
       @return Always returns JSON::Value::number
     */
    type getType() {
      return number;
    }
    /**
       Converts the json value to a string. Only for debugging.

    */
    void print(std::ostream &o);
  };
  
  /**
     A JSON string, represented by a std::string

   */
  class String : public Value {
  public:
    /**
       Constructor. Called during parsing.
     */
    String(std::string s, int lineno):Value(lineno),value(s){};
    /**
       The value of the string, represented by a std::string
    */
    std::string value;
    /**
       @return Always returns JSON::Value::string
     */
    type getType() {
      return string;
    }
    /**
       Converts the json value to a string. Only for debugging.

    */
    void print(std::ostream &o);
  };
  
  /**
     A JSON boolean, represented by a bool

   */
  class Boolean : public Value {
  public:
    /**
       Constructor. Called during parsing.
     */
    Boolean(bool b, int lineno):Value(lineno), value(b){};
    /**
       The value of the boolean, represented by a bool
    */
    bool value;
    /**
       @return Always returns JSON::Value::boolean
     */
    type getType() {
      return boolean;
    }
    /**
       Converts the json value to a string. Only for debugging.

    */
    void print(std::ostream &o);
  };
  
  /**
     A JSON Null value

   */
  class Null : public Value {
  public:
    /**
       Constructor. Called during parsing.
     */
    Null(int lineno):Value(lineno){};
    /**
       @return Always returns JSON::Value::null
     */
    type getType() {
      return null;
    }
    /**
       Converts the json value to a string. Only for debugging.

    */
    void print(std::ostream &o) {o<<"null";}
  };

  typedef void ErrFunc(void *errdata, std::string);
  
  /**
     Convert a string containing JSON data into a JSON::Value.

     @str The string containing JSON data
     @return A JSON value or NULL in case of syntax error

     If an error occurs during parsing, an error message is printed to
     stderr and the function returns NULL.
   */
  Value *decodeJSON(std::string str, ErrFunc *err=NULL, void *errdata=NULL);
  
}

#endif
