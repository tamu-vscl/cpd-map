
#ifndef MAPCONSTS_H_
#define MAPCONSTS_H_

#define DEBUG_MAP 0

enum MapOrientation {
  NORTH_UP,
  TRACK_UP
};

const int ZOOM_MAX = 19;  // Level 19 is the closest in one can zoom
const int ZOOM_MIN = 0;   // Level 0 shows the whole planet

// The default map size, and also the minimum size
const int DEFAULT_MAP_WIDTH  = 300;
const int DEFAULT_MAP_HEIGHT = 300;
const int MAP_PADDING = 11;  // margins=11px

#endif

