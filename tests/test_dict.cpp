#include <stdlib.h>
#include <stdio.h>
#include "dict.hpp"

// so I guess add a bunch of ints and range over them

void dict_repr(int a) {
    printf("%d\n", a);
}

int main(int argc, char** argv) {
    int seed = 5;
    
    {
        printf("normal iter\n");
        auto d = dict<int>();
        for (int i = 0; i < 10; i++) {
            d.set(hash(hash(seed) + i), i);
        }

        d.debug_print(dict_repr);

        for (int i = 0; i < 10; i++) {
            if (!d.get(hash(hash(seed) + i))) {
                printf("%d not in dict!\n", i);
            }
            if (*d.get(hash(hash(seed) + i)) != i) {
                printf("thing is wrong %d != %d\n", i, *d.get(hash(i)));
            }
        }

        d.iter_begin();
        while (auto x = d.iter_next()) {
            printf("%d\n", x->item);
        }
    }

    {
        printf("collision iter\n");
        // seems like that bug was happening on collisions so
        auto d = dict<int>();
        d.set(100, 1);
        d.set(200, 2);
        d.set(300, 3);
        d.set(400, 4);
        d.set(500, 5);
        d.set(600, 6);
        d.debug_print(dict_repr);

        d.iter_begin();
        while (auto x = d.iter_next()) {
            printf("%d\n", x->item);
        }
    }
}