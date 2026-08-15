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
void roll_two_dice(int *d1, int *d2);
void apply_property_depreciation(GameState *game);
void apply_building_depreciation(GameState *game);
void player_maintenance_decision(GameState *game, int player_id);
void handle_inflation(GameState *game);

void handle_jail_turn(GameState *game, int player_index, int *dice_roll_out, int *moved_this_turn) {
  Player *p = &game->players[player_index];
  p->jail_turns++;
  printf("%s is in Jail (Turn %d/3).\n", p->name, p->jail_turns);

  int pay_bail = 0;

  if (p->jail_turns >= 3) {
    printf("%s has served 3 turns and must pay LKR 300 bail.\n", p->name);
    pay_bail = 1;
  } else {
    if (p->id == 1 && p->money > 1000) {
      pay_bail = 1;
    } else if (p->id == 2 && p->money > 3000) {
      pay_bail = 1;
    } else if (p->id == 4 && p->money >= 1500) {
      pay_bail = 1;
    }
  }

  if (pay_bail) {
    p->money -= 300;
    printf("%s pays LKR 300 bail and is released.\n", p->name);
    p->in_jail = 0;
    p->jail_turns = 0;

    *dice_roll_out = roll_dice();
    printf("%s rolled : %d\n", p->name, *dice_roll_out);
    move_player(game, player_index, *dice_roll_out);
    *moved_this_turn = 1;
  } else {
    int d1, d2;
    roll_two_dice(&d1, &d2);
    printf("%s attempts to roll doubles... rolls %d and %d.\n", p->name, d1, d2);
    if (d1 == d2) {
      printf("Doubles! %s is released from Jail.\n", p->name);
      p->in_jail = 0;
      p->jail_turns = 0;
      *dice_roll_out = d1 + d2;
      move_player(game, player_index, *dice_roll_out);
      *moved_this_turn = 1;
    } else {
      printf("Not doubles. %s remains in Jail.\n", p->name);
      *moved_this_turn = 0;
    }
  }
}

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
  while (j < 300) {
    printf("---Round %d---\n", j + 1);
    for (int i = 0; i < game.num_players; i++) {

      // Skip bankrupt players
      if (game.players[i].is_bankrupt) continue;

      // Step 0: Pre-turn Maintenance Decision
      player_maintenance_decision(&game, game.players[i].id);

      int dice_roll = 0;
      int moved_this_turn = 1;

      if (game.players[i].in_jail) {
        handle_jail_turn(&game, i, &dice_roll, &moved_this_turn);
      } else {
        // Step 1: Roll dice and move
        dice_roll = roll_dice();
        printf("%s rolled : %d\n", game.players[i].name, dice_roll);
        move_player(&game, i, dice_roll);
      }

      if (moved_this_turn) {
        // Step 2: Handle landing (buy / pay rent / events)
        handle_landing(&game, game.players[i].position, game.players[i].id, dice_roll);
      }

      // Step 3: Building decision (construct houses/hotels if monopoly exists)
      player_build_decision(&game, game.players[i].id);

      printf("\n");
    }

    // Step 4: END-OF-ROUND PROCESSING — compound interest on all active loans
    apply_interest_all(&game);

    // Step 5: Check if any loans have expired — trigger foreclosure
    check_loan_defaults(&game);

    // Step 6: Depreciation
    apply_property_depreciation(&game);
    apply_building_depreciation(&game);

    // Step 7: Inflation Cycle (every 10 rounds)
    if ((j + 1) % 10 == 0) {
      handle_inflation(&game);
    }

    game.current_turn = game.current_turn + 1;
    j++;
  }

  return 0;
}