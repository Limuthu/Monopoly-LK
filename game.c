#include "types.h"
#include <stdio.h>

void handle_landing(GameState *game, int player_position) {
  switch (game->board[player_position].type) {
  case SQUARE_START:
    printf("Passed Go\n");
    break;
  case SQUARE_PROPERTY:
    printf("Landed on %s\n", game->board[player_position].name);
    break;
  case SQUARE_RAILWAY:
    printf("Landed on Railway\n");
    break;
  case SQUARE_UTILITY:
    printf("Landed on Utility\n");
    break;
  case SQUARE_TAX:
    printf("Landed on Tax\n");
    break;
  case SQUARE_INSURANCE:
    printf("Landed on Insurance\n");
    break;
  case SQUARE_EVENT:
    printf("Landed on Event\n");
    break;
  case SQUARE_JAIL:
    printf("Landed on Jail\n");
    break;
  case SQUARE_SPECIAL:
    printf("Landed on Special\n");
    break;
  case SQUARE_BANK:
    printf("Landed on Bank\n");
    break;
  }
}