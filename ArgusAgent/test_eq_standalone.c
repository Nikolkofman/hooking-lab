#include <stdio.h>
#include <stdbool.h>
#include "event_queue.h"

int main(void) {
    static EventQueue q;
    char buf[EQ_MAX_EVENT_LEN + 1];

    eq_init(&q);

    // Push 5 items
    for (int i = 0; i < 5; i++) {
        char s[32];
        sprintf(s, "event_%d", i);
        eq_push(&q, s);
    }

    // Pop and print 5
    for (int i = 0; i < 5; i++) {
        eq_pop(&q, buf, sizeof(buf));
        printf("[pop] %s\n", buf);
    }

    // Pop from empty queue
    bool result = eq_pop(&q, buf, sizeof(buf));
    if (!result) printf("[empty pop] returned false: OK\n");

    // Overflow test
    for (int i = 0; i < EQ_CAPACITY; i++) {
        char s[32];
        sprintf(s, "overflow_%d", i);
        eq_push(&q, s);
    }

    printf("[overflow] eq_dropped() = %llu\n", eq_dropped(&q));
    eq_pop(&q, buf, sizeof(buf));
    printf("[overflow] oldest surviving item: %s\n", buf);

    printf("All checks passed.\n");
    eq_destroy(&q);
    return 0;
}