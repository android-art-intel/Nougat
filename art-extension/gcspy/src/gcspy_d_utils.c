/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_d_utils.c
 **
 **  Utilities shared by GC drivers
 **/

#include <stdio.h>
#include <stdint.h>

#include "gcspy_utils.h"
#include "gcspy_d_utils.h"

int
gcspy_dUtilsTileNum (char *start,
                     char *end,
                     int tileSize) {
  int tiles;
  int diff = (int)((size_t) end - (size_t) start);
  tiles = diff / tileSize;
  if ( (diff % tileSize) != 0 )
    ++tiles;
  return tiles;
}

#if 0
int
gcspy_dUtilsIsBoundary (gcspy_d_utils_area_t *area,
                        char *addr) {
  if ( (((size_t) area->start + (size_t) addr) % area->blockSize) == 0 )
    return 1;
  else
    return 0;
}
#endif //0

void
gcspy_dUtilsRangeString (gcspy_d_utils_area_t *area,
                         int index,
                         char *buffer) {
  char *start, *end;
  index -= area->firstIndex;
  start = area->start + (index * area->blockSize);
  end = area->start + ((index+1) * area->blockSize);
  if (end > area->end)
    end = area->end;
  sprintf(buffer, "   [%p-%p)", start, end);
}


void
gcspy_dUtilsSetPercVal (gcspy_d_utils_area_t *area,
                        char *start,
                        char *end,
                        int offset,
                        int val) {
  if (start < area->start || end > area->end || start > end) return;

  int currSize, totalSize = 0;
  int startIndex = gcspy_d_utils_get_index(area, start);
  int endIndex = gcspy_d_utils_get_index(area, end - 1);
  int *addr = (int *) gcspy_d_utils_get_stats_struct(area, startIndex, offset);
  for (int i = startIndex; i <= endIndex; ++i) {
    if (startIndex == endIndex) {
      currSize = end - start;
    } else if (i == startIndex) {
      currSize = (size_t) gcspy_d_utils_get_addr(area, i + 1) - (size_t) start;
    } else if (i == endIndex) {
      currSize = (size_t) end - (size_t) gcspy_d_utils_get_addr(area, i);
    } else {
      currSize = area->blockSize;
    }

    int v = val < currSize ? val : currSize;
    *addr += v;
    totalSize += v;

    addr = (int *) gcspy_d_utils_next_stats_struct(area, addr);
  }
}

void
gcspy_dUtilsSetPerc (gcspy_d_utils_area_t *area,
                     char *start,
                     char *end,
                     int offset) {
  int i;
  int currSize, totalSize = 0;
  int startIndex = gcspy_d_utils_get_index(area, start);
  int endIndex = gcspy_d_utils_get_index(area, end - 1);
  int *addr = (int *) gcspy_d_utils_get_stats_struct(area, startIndex, offset);
  for (i = startIndex; i <= endIndex; ++i) {
    // if ((i == startIndex) && (i == endIndex))
    if (startIndex == endIndex) {
      currSize = end - start;
    } else if (i == startIndex) {
      currSize = (size_t) gcspy_d_utils_get_addr(area, i + 1) - (size_t) start;
    } else if (i == endIndex) {
      currSize = (size_t) end - (size_t) gcspy_d_utils_get_addr(area, i);
    } else {
      currSize = area->blockSize;
    }

    *addr += currSize;
    totalSize += currSize;

    addr = (int *) gcspy_d_utils_next_stats_struct(area, addr);
  }

  if (totalSize != (end - start)) {
    char buffer[512];
    int off = 0;
    off = sprintf(buffer, "PANIC! totalSize is wrong!\n");
    off += sprintf(buffer+off, "space[%p,%p]   object[%p,%p]\n",
                   area->start, area->end,
                   start, end);
    sprintf(buffer+off, "totalSize = %d, should be %d\n",
            totalSize, (int) (end - start));
    gcspy_raise_error(buffer);
  }
}

void
gcspy_dUtilsAddOne (gcspy_d_utils_area_t *area,
                    char *start,
                    char *end,
                    int offset) {
  int i;
  int startIndex = gcspy_d_utils_get_index(area, start);
  int endIndex = gcspy_d_utils_get_index(area, end - 1);
  int *addr = (int *) gcspy_d_utils_get_stats_struct(area, startIndex, offset);
  for (i = startIndex; i <= endIndex; ++i) {
    ++(*addr);

    addr = (int *) gcspy_d_utils_next_stats_struct(area, addr);
  }
}

void
gcspy_dUtilsSet (gcspy_d_utils_area_t *area,
                 char *start,
                 char *end,
                 int offset,
                 int val) {
  int i;
  int startIndex = gcspy_d_utils_get_index(area, start);
  int endIndex = gcspy_d_utils_get_index(area, end - 1);
  int *addr = (int *) gcspy_d_utils_get_stats_struct(area, startIndex, offset);
  for (i = startIndex; i <= endIndex; ++i) {
    *addr = val;

    addr = (int *) gcspy_d_utils_next_stats_struct(area, addr);
  }
}

void
gcspy_dUtilsAddSingle (gcspy_d_utils_area_t *area,
                       char *start,
                       int offset) {
  int index = gcspy_d_utils_get_index(area, start);
  int *addr = (int *) gcspy_d_utils_get_stats_struct(area, index, offset);
  ++(*addr);
}

void
gcspy_dUtilsUpdateEnumDesc (gcspy_d_utils_area_t *area,
                            char *start,
                            char *end,
                            int offset,
                            int val) {
  int i;
  int startIndex = gcspy_d_utils_get_index(area, start);
  int endIndex = gcspy_d_utils_get_index(area, end - 1);
  int *addr = (int *) gcspy_d_utils_get_stats_struct(area, startIndex, offset);
  for (i = startIndex; i <= endIndex; ++i) {
    if (val < *addr)
      *addr = val;

    addr = (int *) gcspy_d_utils_next_stats_struct(area, addr);
  }
}

void
gcspy_dUtilsInit (gcspy_d_utils_area_t *area,
                  char *start,
                  char *end,
                  int firstIndex,
                  int blockSize,
                  int blockNum,
                  char *stats,
                  int statsStructSize) {
  area->start = start;
  area->end = end;
  area->firstIndex = firstIndex;
  area->blockSize = blockSize;
  area->blockNum = blockNum;
  area->stats = stats;
  area->statsStructSize = statsStructSize;
}
