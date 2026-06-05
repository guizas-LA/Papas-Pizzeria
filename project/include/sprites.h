#ifndef PROJECT_SPRITES_H
#define PROJECT_SPRITES_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t *colors;
  uint16_t width;
  uint16_t height;
} Sprite;

extern Sprite *buttonSprite;
extern Sprite *buttonPressedSprite;
extern Sprite *mouseCursorSprite;
extern Sprite *orderTicketSprite;
extern Sprite *openSignSprite;
extern Sprite *closedSignSprite;

Sprite *createSprite(xpm_map_t sprite);
void destroy_sprite(Sprite *sprite);
int drawSprite(Sprite *sprite, int x, int y);
int drawSpriteScaled(Sprite *sprite, int x, int y, int width, int height);
int drawButtonSprite(int x, int y, int width, int height, bool selected, uint32_t color);
int drawMouseCursorSprite(int x, int y);
int drawOrderTicketSprite(int x, int y, int width, int height);
int drawOpenSignSprite(int x, int y, int width, int height);
int drawClosedSignSprite(int x, int y, int width, int height);
int loadSprites(void);
void unloadSprites(void);

#endif
