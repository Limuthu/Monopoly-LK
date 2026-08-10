#include "types.h"
#include <stdio.h>


// Owned property count in landed group

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

// Owned railway count in landed group

int count_owned_railways(GameState *game, int owner_id) {
  int count = 0;
  for (int i = 0; i < 40; i++) {
    if (game->board[i].type == SQUARE_RAILWAY &&
        game->board[i].owner_id == owner_id) {
      count++;
    }
  }
  return count;
}

// Owned utility count in landed group

int count_owned_utilities(GameState *game, int owner_id) {
  int count = 0;
  for (int i = 0; i < 40; i++) {
    if (game->board[i].type == SQUARE_UTILITY &&
        game->board[i].owner_id == owner_id) {
      count++;
    }
  }
  return count;
}

// Property count in each group

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

int find_player_index(GameState *game, int player_id) {
  int player_index = -1;
  for (int i = 0; i < game->num_players; i++) {
    if (game->players[i].id == player_id) {
      player_index = i;
      break;
    }
  }
  return player_index;
}


