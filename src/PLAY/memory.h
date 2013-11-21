#ifndef MEMORY_H
#define MEMORY_H

#define askmem malloc
#define freemem free
#define paskmem askmem
#define pfreemem freemem

extern void *laskmem();
extern void *askmem();
extern void *begmem();

#endif
