/**
 ** Copyright (c) 2002 Sun Microsystems, Inc.
 **
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/

/**
 **  gcspy_color_db.h
 **
 **  Maps colors to/from names
 **/

#ifndef _GCSPY_COLOR_DB_H_

#define _GCSPY_COLOR_DB_H_

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} gcspy_color_t;

#define gcspy_color_get_red(_color)     ( (_color)->red )
#define gcspy_color_get_green(_color)   ( (_color)->green )
#define gcspy_color_get_blue(_color)    ( (_color)->blue )

#define gcspy_color_set_red(_color, _red)       \
  do {                                          \
    (_color)->red = (_red);                     \
  } while (0)

#define gcspy_color_set_green(_color, _green)   \
  do {                                          \
    (_color)->green = (_green);                 \
  } while (0)

#define gcspy_color_set_blue(_color, _blue)     \
  do {                                          \
    (_color)->blue = (_blue);                   \
  } while (0)

#define gcspy_color_are_equal(_color1, _color2) \
  ( ((_color1)->red   == (_color2) -> red)   && \
    ((_color1)->green == (_color2) -> green) && \
    ((_color1)->blue  == (_color2) -> blue) )

#define gcspy_color_copy(_dst, _src)            \
  do {                                          \
    (_dst)->red   = (_src)->red;                \
    (_dst)->green = (_src)->green;              \
    (_dst)->blue  = (_src)->blue;               \
  } while (0)

gcspy_color_t *
gcspy_colorDBGetColor (int index);

gcspy_color_t *
gcspy_colorDBGetColorWithName (const char *name);

const char *
gcspy_colorDBGetName (int index);

const char *
gcspy_colorDBGetNameWithColor (gcspy_color_t *color);

int
gcspy_colorDBGetIndex (gcspy_color_t *color);

int
gcspy_colorDBGetLength (void);

void
gcspy_colorDBDump (void);

#endif //_GCSPY_COLOR_DB_H_
