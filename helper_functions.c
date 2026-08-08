#include "types.h"
#include <stdio.h>

// Property buying mindsets
int count_owned_in_group(GameState *game, int player_id, int square_index) {
  PropertyGroup group = game->board[square_index].data.property.group;

  int count = 0;
  for (int i = 0; i < 40; i++) {
    if (game->board[i].type == SQUARE_PROPERTY &&
        game->board[i].data.property.group == group &&
        game->board[i].owner_id == player_id) {
      count++;
    }
  }
  return count;
}

int group_size(GameState *game, int square_index) {
  PropertyGroup group = game->board[square_index].data.property.group;
  int count = 0;
  for (int i = 0; i < 40; i++) {
    if (game->board[i].type == SQUARE_PROPERTY &&
        game->board[i].data.property.group == group) {
      count++;
    }
  }
  return count;
}