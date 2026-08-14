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
void player_build_decision(GameState *game, int player_id);
void apply_interest_all(GameState *game);
void check_loan_defaults(GameState *game);

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

  //--------------------GAME ROUNDS--------------------
  int j = 0;
  while (j < 100) {
    printf("---Round %d---\n", j + 1);
    for (int i = 0; i < game.num_players; i++) {

      // Skip bankrupt players
      if (game.players[i].is_bankrupt) continue;

      // Step 1: Roll dice and move
      int dice_roll = roll_dice();
      printf("%s rolled : %d\n", game.players[i].name, dice_roll);
      move_player(&game, i, dice_roll);

      // Step 2: Handle landing (buy / pay rent / events)
      handle_landing(&game, game.players[i].position, game.players[i].id, dice_roll);

      // Step 3: Building decision (construct houses/hotels if monopoly exists)
      player_build_decision(&game, game.players[i].id);

      printf("\n");
    }

    // Step 4: END-OF-ROUND PROCESSING — compound interest on all active loans
    apply_interest_all(&game);

    // Step 5: Check if any loans have expired — trigger foreclosure
    check_loan_defaults(&game);

    game.current_turn = game.current_turn + 1;
    j++;
  }

  return 0;
}