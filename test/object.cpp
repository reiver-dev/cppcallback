#include "object.hpp"

#include <stdio.h>

static char* _print_obj(char *buf, const Object& obj)
{
    sprintf(buf, "(name=%s, epoch=%d, alive=%i)", obj.name, obj.epoch, obj.alive);
    return buf;
}

static void _print(const char *action, const Object& from, const Object& to)
{
    char b1[512];
    char b2[512];
    printf("%s => %s -> %s\n", action, _print_obj(b1, from), _print_obj(b2, to));
}

static void _print(const char *action, const Object& obj)
{
    char b1[512];
    printf("%s => %s\n", action, _print_obj(b1, obj));
}


Object::Object(const char *name)
    : name(name), epoch(0), alive(ALIVE)
{
    _print("Object", *this);
}

Object::Object(const Object& other)
    : name(other.name), epoch(other.epoch + 1), alive(other.alive)
{
    _print("Copy", other, *this);
}

Object& Object::operator=(const Object& other)
{
    if (this == &other) {
        return *this;
    }

    name = other.name;
    epoch = other.epoch + 1;
    alive = other.alive;

    _print("CopyOp", other, *this);

    return *this;
}

Object::Object(Object&& other)
    : name(other.name), epoch(other.epoch + 1), alive(true)
{
    _print("Move", other, *this);
    other.alive = MOVED;
}

Object& Object::operator=(Object&& other)
{
    if (this == &other) {
        return *this;
    }

    name = other.name;
    epoch = other.epoch + 1;
    alive = other.alive;

    _print("MoveOp", other, *this);
    other.alive = MOVED;

    return *this;
}

Object::~Object()
{
    _print("Destroy", *this);
    alive = DESTROYED;
}

void Object::doit()
{
    _print("DO", *this);
}
