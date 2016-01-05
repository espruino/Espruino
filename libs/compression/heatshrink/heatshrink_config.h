#ifndef HEATSHRINK_CONFIG_H
#define HEATSHRINK_CONFIG_H

/* Should functionality assuming dynamic allocation be used? */
#define HEATSHRINK_DYNAMIC_ALLOC 0

/* Required parameters for static configuration */
#define HEATSHRINK_STATIC_INPUT_BUFFER_SIZE 32
#define HEATSHRINK_STATIC_WINDOW_BITS 8
#define HEATSHRINK_STATIC_LOOKAHEAD_BITS 6

/* Turn on logging for debugging. */
#define HEATSHRINK_DEBUGGING_LOGS 0

/* Use indexing for faster compression. (This requires additional space.) */
#define HEATSHRINK_USE_INDEX 0

#endif
