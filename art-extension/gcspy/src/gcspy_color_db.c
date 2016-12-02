/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_color_db.c
 **
 **  Maps colors to/from names
 **/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "gcspy_color_db.h"
#include "gcspy_utils.h"

#define GCSPY_COLOR_DB_DEFAULT_INDEX          0
#define GCSPY_COLOR_DB_DEFAULT_COLOR                  \
  gcspy_colorDBGetColor(GCSPY_COLOR_DB_DEFAULT_INDEX)

#define GCSPY_COLOR_DB_LENGTH             15

static gcspy_color_t gcspyColors[] = {
  {   0,   0,   0 },
  {   0,   0, 255 },
  {   0, 255, 255 },
  {  64,  64,  64 },
  { 128, 128, 128 },
  {   0, 255,   0 },
  { 192, 192, 192 },
  { 255,   0, 255 },
  { 160, 160, 160 },
  {   0,   0, 150 },
  { 230, 230, 230 },
  { 255, 200,   0 },
  { 255, 175, 175 },
  { 255,   0,   0 },
  { 255, 255, 255 },
  { 255, 255,   0 }
};

static const char *gcspyColorNames[] = {
  "Black",
  "Blue",
  "Cyan",
  "Dark Gray",
  "Gray",
  "Green",
  "Light Gray",
  "Magenta",
  "Mid Gray",
  "Navy Blue",
  "Off White",
  "Orange",
  "Pink",
  "Red",
  "White",
  "Yellow"
};

gcspy_color_t *
gcspy_colorDBGetColor (int index) {
  return &gcspyColors[index];
}

gcspy_color_t *
gcspy_colorDBGetColorWithName (const char *name) {
  int i;
  if (name == NULL)
    return GCSPY_COLOR_DB_DEFAULT_COLOR;
  for (i = 0; i < GCSPY_COLOR_DB_LENGTH; ++i) {
    if (!strcasecmp(name, gcspyColorNames[i])) {
      return &gcspyColors[i];
    }
  }
  return GCSPY_COLOR_DB_DEFAULT_COLOR;
}

const char *
gcspy_colorDBGetName (int index) {
  return gcspyColorNames[index];
}

const char *
gcspy_colorDBGetNameWithColor (gcspy_color_t *color) {
  return gcspyColorNames[gcspy_colorDBGetIndex(color)];
}

int
gcspy_colorDBGetIndex (gcspy_color_t *color) {
  int i;
  if (color == NULL)
    return -1;
  for (i = 0; i < GCSPY_COLOR_DB_LENGTH; ++i) {
    if (gcspy_color_are_equal(color, &gcspyColors[i]))
      return i;
  }
  return -1;
}

int
gcspy_colorDBGetLength () {
  return GCSPY_COLOR_DB_LENGTH;
}

void
gcspy_colorDBDump () {
  int i;
  for (i = 0; i < gcspy_colorDBGetLength(); ++i) {
    gcspy_color_t *color = gcspy_colorDBGetColor(i);
    gcspy_print_mesg("%d.%s: %d,%d,%d\n",
                     i, gcspy_colorDBGetName(i),
                     gcspy_color_get_red(color),
                     gcspy_color_get_green(color),
                     gcspy_color_get_blue(color));
  }
}
