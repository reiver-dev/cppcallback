#ifndef OBJECT_HPP_
#define OBJECT_HPP_

class Object
{
public:

    enum state {
        DESTROYED = -1,
        MOVED = 0,
        ALIVE = 1
    };

    Object(const char *name);

    Object(const Object& other);
    Object& operator=(const Object& other);

    Object(Object&& other);
    Object& operator=(Object&& other);

    void doit();

    ~Object();

    const char *name;
    unsigned epoch;
    int alive;

};

#endif
