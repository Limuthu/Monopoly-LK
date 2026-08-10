#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Forward declarations
void init_board(GameState *game);
void init_players(GameState *game);
void roll_for_order(GameState *game);
void sort_players(GameState *game);
int has_tie(GameState *game);
int roll_dice();
void move_player(GameState *game, int player_index, int dice_roll);
void handle_landing(GameState *game, int player_position, int player_id, int dice_roll);

int main(void) {
  GameState game;
  game.current_turn = 0;
  srand((unsigned int)time(NULL));

  //--------------------Game INITIALIZATION--------------------

  init_board(&game);
  init_players(&game);

  printf("MONOPOLY-LK Simulation\n\n");

  for (int i = 0; i < game.num_players; i++) {
    printf("Player %d : %s\n", i + 1, game.players[i].name);
  }

  printf("\nEach player begins with LKR %.0lf\n\n", game.players[0].money);

  //--------------------ROLLING FOR TURN ORDER--------------------

  roll_for_order(&game);

  for (int i = 0; i < game.num_players; i++) {
    printf("%s rolls %d\n", game.players[i].name, game.players[i].roll_result);
  }

  sort_players(&game);

  printf("\nTurn Order\n");
  for (int i = 0; i < game.num_players; i++) {
    printf("Player %d : %s\n", i + 1, game.players[i].name);
  }

  //--------------------PLAYER MOVEMENTS--------------------
  int j = 0;
  while (j < 20) {
    printf("---Round %d---\n", j + 1);
    for (int i = 0; i < game.num_players; i++) {
      int dice_roll = roll_dice();
      printf("%s rolled : %d\n", game.players[i].name, dice_roll);
      move_player(&game, i, dice_roll);

      //--------------------PLAYER LANDING--------------------

      handle_landing(&game, game.players[i].position, game.players[i].id, dice_roll);
      printf("\n");
    }
    game.current_turn = game.current_turn + 1;
    j++;
  }

  return 0;
}