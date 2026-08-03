#include "types.h"
#include <stdio.h>

// Forward declarations
void init_board(GameState *game);
void init_players(GameState *game);
void roll_for_order(GameState *game);
void sort_players(GameState *game);
int has_tie(GameState *game);

int main(void) {
  GameState game;

  init_board(&game);
  init_players(&game);

  printf("MONOPOLY-LK Simulation\n\n");

  for (int i = 0; i < game.num_players; i++) {
    printf("Player %d : %s\n", i + 1, game.players[i].name);
  }

  printf("\nEach player begins with LKR %.0lf\n\n", game.players[0].money);

  roll_for_order(&game);

  for (int i = 0; i < game.num_players; i++) {
    printf("%s rolls %d\n", game.players[i].name, game.players[i].roll_result);
  }

  game.current_turn = game.current_turn + 1;

  sort_players(&game);

  printf("\nTurn Order\n");
  for (int i = 0; i < game.num_players; i++) {
    printf("Player %d : %s\n", i + 1, game.players[i].name);
  }

  return 0;
}