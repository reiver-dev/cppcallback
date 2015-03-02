#include "routines.hpp"


C create_closure(int init, const char *text)
{
    Value _v {init, text};
    return C([v = static_cast<Value&&>(_v)](int item){return v.value + item;});
}

C1 create_closure2(int init, const char *text)
{
    Value _v {init, text};
    return C1([v = static_cast<Value&&>(_v)](int item){return v.value + item;});
}

D create_delegate(Value &v)
{
    D d = CB_DELEGATE_INIT(&v, &Value::apply);
    return d;
}

