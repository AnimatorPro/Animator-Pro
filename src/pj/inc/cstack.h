#ifndef CSTACK_H
#define CSTACK_H

/**** macros for C stack manipulation ******/

/* size of a stack object */
#define sob_size(sz) ((sz+sizeof(int)-1)&~(sizeof(int)-1)) 

/* sizeof type as stack object */
#define ssizeof(type) sob_size(sizeof(type)) 

#endif /* CSTACK_H */
