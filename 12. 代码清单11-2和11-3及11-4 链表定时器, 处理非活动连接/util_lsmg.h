//
// Created by lsmg on 2/2/20.
//

#ifndef UNTITLED_UTIL_LSMG_H
#define UNTITLED_UTIL_LSMG_H

#define exit_if(r, ...) \
{   \
    if (r)  \
    {   \
        printf(__VA_ARGS__);    \
        printf("\nerrno no: %d, error msg is %s", errno, strerror(errno));    \
        exit(1);    \
    }   \
}   \

#endif //UNTITLED_UTIL_LSMG_H
