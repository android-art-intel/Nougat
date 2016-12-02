/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_d_utils.h
 **
 **  Utilities shared by GC drivers
 **/

#ifndef _GCSPY_D_UTILS_UTILS_H_

#define _GCSPY_D_UTILS_UTILS_H_

typedef struct {
  /* start of the heap area */
  char *start;
  /* end of the heap area */
  char *end;

  /* first index corresponding to the heap area */
  int firstIndex;
  /* size of blocks in bytes */
  int blockSize;
  /* number of blocks in the heap area */
  int blockNum;

  /* start of the memory area containing the stats */
  char *stats;
  /* size of one stats structure in bytes */
  int statsStructSize;
} gcspy_d_utils_area_t;

#define gcspy_d_utils_get_area_size(_area_)     \
  ( (_area_)->end - (_area_)->start )

#define gcspy_d_utils_is_boundary(_area_, _addr_)                       \
  ( (((size_t) (_area_)->start + (size_t) (_addr_)) % (_area_)->blockSize) == 0 )

#define gcspy_d_utils_get_index(_area_, _addr_)                         \
  ( (_area_)->firstIndex +                                              \
    ((size_t) (_addr_) - (size_t) (_area_)->start) / (_area_)->blockSize )

#define gcspy_d_utils_get_addr(_area_, _index_)                         \
  ((_area_)->start + ((_index_) - (_area_)->firstIndex) * (_area_)->blockSize)

#define gcspy_d_utils_get_stats_struct(_area_, _index_, _offset_)     \
  ( (_area_)->stats + (_offset_) +                                    \
    ((_index_) - (_area_)->firstIndex) * (_area_)->statsStructSize )

#define gcspy_d_utils_next_stats_struct(_area_, _ptr_)  \
  ( (char *) (_ptr_) + (_area_)->statsStructSize )

#define gcspy_d_utils_addr_in_range(_area_, _addr_)               \
  ( ((_addr_) >= (_area_)->start) && ((_addr_) < (_area_)->end) )

#define gcspy_d_utils_index_in_range(_area_, _index_)           \
  ( ((_index_) >= (_area_)->firstIndex) &&                      \
    ((_index_) < ((_area_)->firstIndex + (_area_)->blockNum)) )

/* 1 per 128 bytes */
#define gcspy_d_utils_roots_per_block(_block_size)  \
  ( (_block_size) / 128 )

/* assume that the min obj size is 16 bytes */
#define gcspy_d_utils_objects_per_block(_block_size)  \
  ( (_block_size) / 16 )

/* assume that the min free chunk size is 32 bytes (it should really
   be 16 bytes, but normally we do not have sequential 16-byte free
   chunks, as they would be coalesced; hence 32 bytes refers to a
   16-byte free chunk, followed by a 16-byte object */
#define gcspy_d_utils_chunks_per_block(_block_size) \
  ( (_block_size) / 32 )

/* assume that all of the space of each block is taken up by
   4-byte reference fields */
#define gcspy_d_utils_refs_per_block(_block_size) \
  ( (_block_size) / 4 )

int
gcspy_dUtilsTileNum (char *start,
                     char *end,
                     int blockSize);

void
gcspy_dUtilsRangeString (gcspy_d_utils_area_t *area,
                         int index,
                         char *buffer);

void
gcspy_dUtilsSetPercVal (gcspy_d_utils_area_t *area,
                        char *start,
                        char *end,
                        int offset,
                        int val);

void
gcspy_dUtilsSetPerc (gcspy_d_utils_area_t *area,
                     char *start,
                     char *end,
                     int offset);

void
gcspy_dUtilsAddOne (gcspy_d_utils_area_t *area,
                    char *start,
                    char *end,
                    int offset);

void
gcspy_dUtilsSet (gcspy_d_utils_area_t *area,
                 char *start,
                 char *end,
                 int offset,
                 int val);

void
gcspy_dUtilsAddSingle (gcspy_d_utils_area_t *area,
                       char *start,
                       int offset);


void
gcspy_dUtilsUpdateEnumDesc (gcspy_d_utils_area_t *area,
                            char *start,
                            char *end,
                            int offset,
                            int newVal);


void
gcspy_dUtilsInit (gcspy_d_utils_area_t *area,
                  char *start,
                  char *end,
                  int firstIndex,
                  int blockSize,
                  int blockNum,
                  char *stats,
                  int statsStructSize);

#endif //_GCSPY_D_UTILS_UTILS_H_
