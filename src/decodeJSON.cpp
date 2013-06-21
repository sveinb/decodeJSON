#include <math.h>
#include "decodeJSON.h"
#include <iomanip>
#include <iostream>

using namespace std;

namespace JSON {
  
  void Object::print(ostream &o) {
    o << "{";
    map<std::string, Value *>::iterator I;
    for (I=value.begin(); I!=value.end(); ) {
      String s(I->first, 0);
      s.print(o);
      o << ":";
      I->second->print(o);
      ++I;
      if (I!=value.end())
        o << ",";
    }
    o <<"}";
  }
  
  void Array::print(ostream &o) {
    o << "[";
    int i;
    for (i=0; i<value.size();) {
      value[i]->print(o);
      i++;
      if (i<value.size())
        o << ",";
    }
    o << "]";
  }
  
  void Number::print(ostream &o) {
    o << number;
  }
  
  void String::print(ostream &o) {
    o << '"';
    int i;
    for (i=0; i<value.size(); i++) {
      if (value[i]>=' ' && value[i]<128 && value[i]!='"')
        o << value[i];
      else
        o << "\\x" << hex << setw(2) << (int)value[i];
    }
    o << '"';
  }
  
  void Boolean::print(ostream &o) {
    o << (value ? "true":"false");
  }
  
  Array::~Array() {
    int i;
    for (i=0; i<value.size(); i++) {
      delete value[i];
    }
  }
  
  Object::~Object() {
    map<std::string, Value *>::iterator I;
    for (I=value.begin(); I!=value.end(); ++I) {
      delete I->second;
    }
  }
  
  struct JSON {
    jschar *p;
    jschar *start;
    int line_no;
    ErrFunc *err;
    void *errData;
  };
  
  static void defaultError(void *dummy, string msg) {
    cerr << msg << "\n";
  }

  static void *syntaxerror(struct JSON *s) {
    char buf[80];
    sprintf(buf, "JSON: Syntax error at line no %d", s->line_no);
    s->err(s->errData, buf);
    return NULL;
  }
  
  static Array *parse_array(struct JSON *s);
  static inline Value *parse_value(struct JSON *s, jschar end);
  
  static inline int parse_barename(struct JSON *s) {
    char *start=s->p;
    char f=*start;
    
    if (!(f>='A' && f<='Z') &&
        !(f>='a' && f<='z') &&
        f!='_')
      return -1;
    
    s->p++;
    while ((f=*s->p)) {
      if (!(f>='A' && f<='Z') &&
          !(f>='a' && f<='z') &&
          !(f>='0' && f<='9') &&
          f!='_')
        return (int)(s->p-start);
      s->p++;
    }
    return (int)(s->p-start);
  }
  
  static inline int parse_unescape(struct JSON *s) {
    int val=0;
    int i;
    jschar *p=s->p;
    jschar *start=p;
    
    s->p++; // "
    for (;;) {
      switch (*s->p) {
        case '\\':
          s->p++;
          switch(*(s->p++)) {
            case '\"':
              *(p++)='\"';
              break;
            case '\\':
              *(p++)='\\';
              break;
            case '/':
            case 'b':
              *(p++)='/';
              break;
            case 'f':
              *(p++)='\f';
              break;
            case 'n':
              *(p++)='\n';
              break;
            case 'r':
              *(p++)='\r';
              break;
            case 't':
              *(p++)='\t';
              break;
            case 'u':
              val=0;
              for (i=4; --i;) {
                switch(*s->p) {
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                    val = val<<4 | (*(s->p)-'0');
                    s->p++;
                    break;
                  case 'a':
                  case 'b':
                  case 'c':
                  case 'd':
                  case 'e':
                  case 'f':
                    val = val<<4 | (*(s->p)-'a'+10);
                    s->p++;
                    break;
                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                    val = val<<4 | (*(s->p)-'A'+10);
                    s->p++;
                    break;
                  default:
                    return -1;
                }
              }
              *(p++)=val;
              break;
              
            default:
              *(p++)=s->p[-1];
          }
          break;
          
        case 0:
          return -1;
          
        case '\"':
          s->p++;
          return (int)(p-start);
          
        default:
          *(p++)=*(s->p++);
      }
    }
  }
  
  static inline String *parse_string(struct JSON *s) {
    jschar *start=s->p;
    int len=parse_unescape(s);
    
    if (len==-1)
      return (String *)syntaxerror(s);
    
    return new String(string(start,len),s->line_no);
  }
  
  
  static inline int ignore_line_comment(struct JSON *s) {
    while (s->p[1]) {
      if (s->p[1] == '\n')
        break;
      s->p++;
    }
    return 1;
  }
  
  static inline int ignore_block_comment(struct JSON *s) {
    s->p+=2;
    while (s->p[0]) {
      if (s->p[-1] == '*' && s->p[0] == '/')
        return 1;
      if (s->p[0]=='\n')
        s->line_no++;
      s->p++;
    }
    return 0;
  }
  
  static inline int ignore_comment(struct JSON *s) {
    if (s->p[0]=='#' || (s->p[0]=='/' &&
                         s->p[1]=='/')) return ignore_line_comment(s);
    else if (s->p[0]=='/' &&
             s->p[1]=='*') return ignore_block_comment(s);
    else return 0;
  }
  
  static Object *parse_object(struct JSON *s) {
    Object *object=new Object(s->line_no);
    Value *v;
    jschar *name;
    int namelen;
    
    //  s->vp=object;
    
    s->p++; // {
    
    for (;;) {
      switch(*s->p) {
        case '\n':
          s->line_no++; // fall through
        case ' ':
        case '\t':
        case '\r':
          s->p++;
          break;
        case '#':
        case '/':
          if (!ignore_comment(s))
            return (Object *)syntaxerror(s);
          s->p++;
          break;
          
        case '}':
          s->p++;
          return object;
          
        default:
          if (*s->p=='"') {
            name=s->p;
            namelen=parse_unescape(s);
          } else {
            name=s->p;
            namelen=parse_barename(s);
          }
          
          if (namelen==-1) {
            delete object;
            return (Object *)syntaxerror(s);
          }
          
          // scan for colon
          while (*s->p!=':') {
            switch(*(s->p++)) {
              case '\n':
                s->line_no++; // fall through
              case ' ':
              case '\t':
              case '\r':
                break;
              case '#':
              case '/':
                if (!ignore_comment(s))
                  return (Object *)syntaxerror(s);
                break;
                
              default:
                delete object;
                return (Object *)syntaxerror(s);
            }
          }
          s->p++;
          
          v=parse_value(s, '}');
          if (!v) {
            delete object;
            return NULL;
          }
          object->value[string(name, namelen)]=v;
          
          // Scan for comma
          
          while (*s->p!=',') {
            switch(*(s->p++)) {
              case '}':
                return object;
              case '\n':
                s->line_no++; // fall through
              case ' ':
              case '\t':
              case '\r':
                break;
              case '#':
              case '/':
                if (!ignore_comment(s))
                  return (Object *)syntaxerror(s);
                break;
              default:
                delete object;
                return (Object *)syntaxerror(s);
            }
          }
          s->p++;
          break;
          
          //      default:
          //        delete object;
          //        return (Object *)syntaxerror(s);
      }
    }
    
    // not reached
    delete object;
    return false;
  }
  
  static inline Boolean *parse_true(struct JSON *s) {
    s->p++; //t
    if (*(s->p++)!='r')
      return (Boolean *)syntaxerror(s);
    if (*(s->p++)!='u')
      return (Boolean *)syntaxerror(s);
    if (*(s->p++)!='e')
      return (Boolean *)syntaxerror(s);
    
    return new Boolean(true, s->line_no);
  }
  
  static inline Boolean *parse_false(struct JSON *s) {
    s->p++; //f
    if (*(s->p++)!='a')
      return (Boolean *)syntaxerror(s);
    if (*(s->p++)!='l')
      return (Boolean *)syntaxerror(s);
    if (*(s->p++)!='s')
      return (Boolean *)syntaxerror(s);
    if (*(s->p++)!='e')
      return (Boolean *)syntaxerror(s);
    return new Boolean(false, s->line_no);
  }
  
  static inline Null *parse_null(struct JSON *s) {
    s->p++; //n
    if (*(s->p++)!='u')
      return (Null *)syntaxerror(s);
    if (*(s->p++)!='l')
      return (Null *)syntaxerror(s);
    if (*(s->p++)!='l')
      return (Null *)syntaxerror(s);
    return new Null(s->line_no);
  }
  
  static inline Number *parse_number(struct JSON *s) {
    double n=0.;
    int sgn=-1;
    int expsgn=1;
    //  int exp=0;
    double mul;
    int expn=0;
    
    if (*s->p=='-')
      s->p++;
    else
      sgn=1;
    
    switch(*s->p) {
      case '0':
        s->p++;
        goto intend;
        
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        n=(double) (*s->p-'0');
        s->p++;
        for (;;) {
          switch(*s->p) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              n=n*10+(*s->p-'0');
              s->p++;
              break;
              
            case '.':
            case 'e':
            case 'E':
              goto intend;
              
            default:
              goto end;
          }
        }
      default:
        return (Number *)syntaxerror(s);
    }
    
    
  intend:
    switch(*s->p) {
      case '.':
        s->p++;
        goto fraction;
      case 'e':
      case 'E':
        goto exp;
      default:
        goto end;
    }
    
  fraction:
    mul=0.1;
    for (;;) {
      switch(*s->p) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          n+=mul*(*s->p-'0');
          s->p++;
          mul/=10.;
          break;
        case 'e':
        case 'E':
          goto exp;
        default:
          goto end;
      }
    }
    
  exp:
    s->p++; //e or E
    switch(*s->p) {
      case '+':
        s->p++;
        // fall through
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        break;
      case '-':
        expsgn=-1;
        s->p++;
        break;
      default:
        return (Number *)syntaxerror(s);
    }
    
    for (;;) {
      switch(*s->p) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          expn=expn*10+*(s->p++)-'0';
          break;
        default:
          goto expend;
      }
    }
    
  expend:
    n*=pow(10., expn*expsgn);
    
  end:
    return new Number(sgn==1?n:-n, s->line_no);
  }
  
  
  static inline Value *parse_value(struct JSON *s, jschar end) {
    for (;;) {
      switch(*s->p) {
        case ',':
          if (end!=0) {
            return new Null(s->line_no);
          } else {
            return (Value *)syntaxerror(s);
          }
          
        case ']':
        case '}':
          if (*s->p==end) {
            return new Null(s->line_no);
          } else {
            return (Value *)syntaxerror(s);
          }
          
        case '"':
          return parse_string(s);
          
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '+':
          return parse_number(s);
          
        case 't':
          return parse_true(s);
          
        case 'f':
          return parse_false(s);
          
        case 'n':
          return parse_null(s);
          
        case '[':
          return parse_array(s);
          
        case '{':
          return parse_object(s);
          
        case '\n':
          s->line_no++; // fall through
        case ' ':
        case '\t':
        case '\r':
          s->p++;
          break;
        case '#':
        case '/':
          if (!ignore_comment(s))
            return (Value *)syntaxerror(s);
          s->p++;
          break;
        default:
          return (Value *)syntaxerror(s);
      }
    }
  }
  
  static Array *parse_array(struct JSON *s) {
    Array *array=new Array(s->line_no);
    
    s->p++; // [
    
    for (;;) {
      if ((*s->p)!=' ' &&
          (*s->p)!='\t' &&
          (*s->p)!='\n' &&
          (*s->p)!='/' &&
          (*s->p)!='#' &&
          (*s->p)!='\r') break;
      if ((*s->p)=='\n')
        s->line_no++;
      if (((*s->p)=='/' || (*s->p)=='#') && !ignore_comment(s))
        return (Array *)syntaxerror(s);
      s->p++;
    }
    
    if (*s->p==']') {
      s->p++;
      return array;
    }
    
    for (;;) {
      Value *v=parse_value(s, ']');
      if (!v) {
        delete array;
        return NULL;
      }
      array->value.push_back(v);
      
      // Scan for comma
      
      while (*s->p!=',') {
        switch(*(s->p++)) {
          case ']':
            return array;
          case '\n':
            s->line_no++; // fall through
          case ' ':
          case '\t':
          case '\r':
            break;
          case '/':
          case '#':
            if (!ignore_comment(s))
              return (Array *)syntaxerror(s);
            break;
          default:
            return (Array *)syntaxerror(s);
        }
      }
      s->p++;
    }
    
    // not reached
    return false;
  }
  
  
  Value *decodeJSON(string str, ErrFunc *err, void *errData) {
    struct JSON s;
    
    s.line_no=1;
    if (err)
      s.err=err;
    else
      s.err=defaultError;
    s.errData=errData;

    jschar *c=strdup(str.c_str());
    
    s.p=c;
    s.start=s.p;
    Value *ret=parse_value(&s, 0);
    free(c);
    return ret;
  }
  
}
