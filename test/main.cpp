#include "routines.hpp"

#include <stdio.h>

#define UNUSED(x) ((void) x)


int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    {
        C a1 = create_closure(3, "a1");
        printf("got a1\n\n");

        C a2(create_closure(5, "a2"));
        printf("got a2\n\n");

        C1 a3 = create_closure2(7, "a3");
        printf("got a3\n\n");

        C a4(a1);
        printf("got a4\n\n");

        C1 a5(a2);
        printf("got a5\n\n");

        C1 a6;
        a6 = a5;
        printf("got a6\n\n");

        C1 a7(create_closure2(7, "a7"));
        a7 = static_cast<C1&&>(a6);
        printf("got a7, killed a6\n\n");

        printf("%d\n", a1(3)); // 3 + 3
        printf("%d\n", a2(4)); // 5 + 4
        printf("%d\n", a3(5)); // 7 + 5

        printf("%d\n", a4(3)); // 3 + 3
        printf("%d\n", a5(4)); // 5 + 4

        printf("%d\n", a7(7)); // 5 + 7

        printf("\n\n");
    }

    {
        printf("\n");
        Value vb1 {3, "b1"};
        Value vb2 {3, "b2"};
        Value vb3 {3, "b3"};
        D b1 = create_delegate(vb1);
        D b2 = create_delegate(vb2);
        D b3 = create_delegate(vb3);

        printf("%d\n", b1(3));
        printf("%d\n", b2(4));
        printf("%d\n", b3(5));

    }
    {
        printf("\n");
        E d = create_delegate_freefunction();
        int val = 41;
        printf("%d %d\n",d(val), val);
    }

    return 0;
}
