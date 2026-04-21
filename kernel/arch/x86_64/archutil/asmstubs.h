#ifndef ASSEMBLY_STUBS_H
#define ASSEMBLY_STUBS_H

inline void __attribute__((force_inline)) pause(void);

void load_cr3( void* cr3_value );

#endif
