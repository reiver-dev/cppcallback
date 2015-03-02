#include <closure.hpp>
#include <delegate.hpp>
#include "object.hpp"

using C = CB::StaticClosure<int (int), 64>;
using C1 = CB::StaticClosure<int (int), 76>;
using D = CB::Delegate<int (int)>;

struct Value {

    int value;
    Object _garbage;

    int apply(int other) const {
        return value + other;
    }

};

C create_closure(int init, const char *text);
C1 create_closure2(int init, const char *text);
D create_delegate(Value &v);
