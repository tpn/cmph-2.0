#ifndef __CMPH_STRUCTS_H__
#define __CMPH_STRUCTS_H__

#include "cmph.h"

/** Hash generation algorithm data
  */
struct __config_t
{
        CMPH_ALGO algo;
        cmph_io_adapter_t *key_source;
        cmph_uint32 verbosity;
        double c;
        void *data; // algorithm dependent data

        //
        // XXX: Begin memory map hack.
        //

        cmph_uint32 num_keys;
        cmph_uint32 keylen;
        void *base_address;

        //
        // XXX: End memory map hack.
        //
};

/** Hash querying algorithm data
  */
struct __cmph_t
{
        CMPH_ALGO algo;
        cmph_uint32 size;
        cmph_io_adapter_t *key_source;
        void *data; // algorithm dependent data

        //
        // XXX: Begin memory map hack.
        //

        cmph_uint32 num_keys;
        cmph_uint32 keylen;
        void *base_address;

        //
        // XXX: End memory map hack.
        //
};

cmph_config_t *__config_new(cmph_io_adapter_t *key_source);
void __config_destroy(cmph_config_t*);
void __cmph_dump(cmph_t *mphf, FILE *);
cmph_t *__cmph_load(FILE *f);


#endif
