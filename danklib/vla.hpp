#pragma once

#include <stdlib.h>
#include <functional>
#include <stdio.h>
#include <string.h>
#include "fatal.hpp"

const static int vla_default_size = 10;

template<typename T>
struct vla {

    T *items = NULL;
    int backing_size = 0;
    int length = 0;
    int iter_pos = 0;

    // lazy allocation
    vla() {}

    // 0 initializes
    vla(int backing_size): 
            backing_size{backing_size}, 
            length{backing_size}, 
            items{(T *)malloc(sizeof(T) * backing_size)}
        {}

    vla(T *items, int backing_size, int length):
            backing_size{backing_size},
            length{length},
            items{items}
        {}

    // to move just assign
    vla<T> deep_copy() const {
        T * new_items = (T *)malloc(sizeof(T) * backing_size);
        memcpy(new_items, items, sizeof(T) * length);

        return vla(new_items, backing_size, length);
    }

    // ref vs ptr?
    // will ref do a copy if used as an expression
    // use as lvalue without dereferencing I suppose
    T& operator[](int idx) const {
        if (idx < 0 || idx > length) {
            printf("bad index %d, length %d\n", idx, length);
            fatal("index out of bounds");
        } 
        return items[idx];
    }

    void destroy() {
        if (items) {
            free(items);
            items = NULL;
        }
        length = 0;
        backing_size = 0;
    }

    void push(const T item) {
        if (length == 0) {
            items = (T *)malloc(sizeof(T) * vla_default_size);
            backing_size = vla_default_size;
        } else if (length == backing_size - 1) {
            backing_size *= 2;
            items = (T *)realloc(items, backing_size * sizeof(T));
        }

        items[length++] = item;
    }

    void push(vla<T> items) {
        for (int i = 0; i < items.length; i++) {
            push(items.items[i]);
        }
    }

    T pop_back() {
        if (length == 0) {
            printf("tried to pop empty vla\n");
            exit(1);
        }
        length--;
        return items[length];
    }
    
    bool contains(T thing) {
        for (int i = 0; i < length; i++) {
            if (!memcmp(&items[i], &thing, sizeof(T))) {
                return true;
            }
        }
        return false;
    }

    bool any(std::function<bool(T)> f) {
        for (int i = 0; i < length; i++) {
            if (f(items[i])) return true;
        }
        return false;
    }

    bool all(std::function<bool(T)> f) {
        for (int i = 0; i < length; i++) {
            if (!f(items[i])) return false;
        }
        return true;
    }

    int max(std::function<int (T)> f) {
        int max_idx = 0;
        int curr_max = f(items[0]);
        for (int i = 1; i < length; i++) {
            const auto curr_val = f(items[i]);
            if (curr_val > curr_max) {
                max_idx = i;
                curr_max = curr_val;
            }
        }
        return curr_max;
    }

    int acc(std::function<int(T)> f) {
        int count = 0;
        for (int i = 0; i < length; i++) {
            count += f(items[i]);
        }
        return count;
    }

    T *begin() {
        return &items[0];
    }

    T *end() {
        return &items[length];
    }


    // doesn't preserve order
    void remove_idx(int idx) {
        items[idx] = items[length-1];
        /* bug probably if it was the last thing or something
        for (int i = idx; i < length; i++) {
            items[i] = items[i+1];
        }
        */
        length--;
    }

    void remove_item(T item) {
        for (int i = 0; i < length; i++) {
            if (items[i] == item) {
                remove_idx(i);  
            }
        }
    }

    void sort(std::function<bool(T,T)> less) {
        for (int i = 0; i < length; i++) {
            for (int j = 0; j < length; j++) {
                if (less(items[i], items[j])) {
                    const auto tmp = items[i];
                    items[i] = items[j];
                    items[j] = tmp;
                }
            }
        }
    }

    template<typename MAP_TARGET>
    vla<MAP_TARGET> map(std::function<MAP_TARGET(const T)> f) {
        auto dest_vla = vla<MAP_TARGET>();
        for (T *elem_ptr = begin(); elem_ptr < end(); elem_ptr++) {
            dest_vla.push(f(*elem_ptr));
        }
        return dest_vla;
    }
};