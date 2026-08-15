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
void update_property_market(GameState *game);
const char* get_group_name(PropertyGroup group);
void trigger_disaster(GameState *game);

void update_insurance_durations(GameState *game);
int find_player_index(GameState *game, int player_id);
void attempt_disaster_repairs(GameState *game, int player_id);

void trigger_economic_event(GameState *game);
const char* get_economic_event_name(EconomicEventType type);

void trigger_regional_development(GameState *game);
const char* get_regional_card_name(RegionalCardType type);

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
      if (game.players[i].is_bankrupt)
        continue;

      // Pre-turn Maintenance Decision
      player_maintenance_decision(&game, game.players[i].id);

      printf("----- %s's Turn -----\n", game.players[i].name);

      // Step 0: Check if player can pay off any disaster damages
      attempt_disaster_repairs(&game, game.players[i].id);

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

    // Step 8: Dynamic Property Market Cycle (every 10 rounds)
    if ((j + 1) % 10 == 0) {
      update_property_market(&game);
    }
    
    // Step 9: Disasters (every 10 rounds)
    if ((j + 1) % 10 == 0) {
      trigger_disaster(&game);
    }
    
    // Step 10: Update Insurance Policies
    update_insurance_durations(&game);

    // Step 10.5: Update National Event Cards
    for (int p = 0; p < game.num_players; p++) {
      if (game.players[p].active_national_card != NATIONAL_CARD_NONE) {
        if (game.players[p].national_card_rounds_left > 0) {
          game.players[p].national_card_rounds_left--;
          if (game.players[p].national_card_rounds_left == 0) {
            printf("[NATIONAL EVENT EXPIRED] %s's event card has concluded.\n", game.players[p].name);
            game.players[p].active_national_card = NATIONAL_CARD_NONE;
            game.players[p].revalued_group = GROUP_NONE;
            game.players[p].construction_suspended = 0;
          }
        }
      }
    }

    // Step 10.6: Update Closed Properties (Political Rally)
    for (int idx = 0; idx < TOTAL_SQUARES; idx++) {
      if (game.board[idx].type == SQUARE_PROPERTY && game.board[idx].data.property.closed_rounds_left > 0) {
        game.board[idx].data.property.closed_rounds_left--;
        if (game.board[idx].data.property.closed_rounds_left == 0) {
            printf("[PROPERTY REOPENED] %s is open for business again!\n", game.board[idx].name);
        }
      } else if (game.board[idx].type == SQUARE_RAILWAY && game.board[idx].data.railway.closed_rounds_left > 0) {
        game.board[idx].data.railway.closed_rounds_left--;
        if (game.board[idx].data.railway.closed_rounds_left == 0) {
            printf("[PROPERTY REOPENED] %s is open for business again!\n", game.board[idx].name);
        }
      } else if (game.board[idx].type == SQUARE_UTILITY && game.board[idx].data.utility.closed_rounds_left > 0) {
        game.board[idx].data.utility.closed_rounds_left--;
        if (game.board[idx].data.utility.closed_rounds_left == 0) {
            printf("[PROPERTY REOPENED] %s is open for business again!\n", game.board[idx].name);
        }
      }
    }

    // Step 10.7: Anti-Speculation Act timers
    for (int idx = 0; idx < TOTAL_SQUARES; idx++) {
      if (game.board[idx].type == SQUARE_PROPERTY && game.board[idx].data.property.forced_development_rounds_left > 0) {
        game.board[idx].data.property.forced_development_rounds_left--;
        if (game.board[idx].data.property.forced_development_rounds_left == 0) {
          if (game.board[idx].data.property.num_houses == 0 && !game.board[idx].data.property.has_hotel) {
            int owner_id = game.board[idx].data.property.owner_id;
            if (owner_id != -1) {
              int p_idx = find_player_index(&game, owner_id);
              if (p_idx != -1) {
                printf("[ANTI-SPECULATION PENALTY] %s failed to develop %s in time! Fined LKR 5000.\n", 
                       game.players[p_idx].name, game.board[idx].name);
                game.players[p_idx].money -= 5000;
              }
            }
          }
        }
      }
    }


    // Step 11: Print active market conditions (Rule 36)
    if (game.market_rounds_left > 0) {
      game.market_rounds_left--;
      printf("\n[MARKET STATUS] Boom: %s Group | Decline: %s Group | Rounds Left: %d\n",
             get_group_name(game.market_boom_group),
             get_group_name(game.market_decline_group),
             game.market_rounds_left);
      
      if (game.market_rounds_left == 0) {
        game.market_boom_group = GROUP_NONE;
        game.market_decline_group = GROUP_NONE;
        printf("[MARKET UPDATE] The market has stabilized. Conditions return to normal.\n\n");
      }
    }
    
    // Step 12: Economic Events (every 15 rounds)
    if ((j + 1) % 15 == 0) {
      trigger_economic_event(&game);
    }
    
    // Step 13: Update Economic Event Status
    if (game.economic_event_rounds_left > 0) {
      game.economic_event_rounds_left--;
      printf("[ECONOMIC STATUS] Active Event: %s | Rounds Left: %d\n",
             get_economic_event_name(game.active_economic_event),
             game.economic_event_rounds_left);
             
      if (game.economic_event_rounds_left == 0) {
        game.active_economic_event = EVENT_NONE;
        printf("[ECONOMIC UPDATE] The national economic event has concluded.\n\n");
      }
    }
    
    // Step 14: Regional Development (every 15 rounds)
    if ((j + 1) % 15 == 0) {
      trigger_regional_development(&game);
    }
    
    // Step 15: Update Regional Development Status
    if (game.regional_card_rounds_left > 0) {
      game.regional_card_rounds_left--;
      printf("[REGIONAL STATUS] Active Card: %s | Rounds Left: %d\n",
             get_regional_card_name(game.active_regional_card),
             game.regional_card_rounds_left);
             
      if (game.regional_card_rounds_left == 0) {
        game.active_regional_card = CARD_NONE;
        printf("[REGIONAL UPDATE] The regional development period has concluded.\n\n");
      }
    }

    // Step 16: Government Regulations (every 20 rounds)
    if ((j + 1) % 20 == 0) {
      void trigger_government_regulation(GameState *game);
      trigger_government_regulation(&game);
    }
    
    // Step 17: Update Government Regulation Status
    if (game.regulation_rounds_left > 0) {
      game.regulation_rounds_left--;
      const char* get_government_regulation_name(GovernmentRegulationType type);
      printf("[REGULATION STATUS] Active: %s | Rounds Left: %d\n",
             get_government_regulation_name(game.active_regulation),
             game.regulation_rounds_left);
             
      if (game.regulation_rounds_left == 0) {
        game.active_regulation = REGULATION_NONE;
        printf("[REGULATION UPDATE] The government regulation has concluded.\n\n");
      }
    }

    game.current_turn = game.current_turn + 1;
    j++;
  }

  return 0;
}