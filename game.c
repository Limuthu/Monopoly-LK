#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SQUARE_JAIL_POS 10
#define SQUARE_GOTO_JAIL_POS 30

void init_board(GameState *game);
void init_players(GameState *game);
void roll_for_order(GameState *game);
void sort_players(GameState *game);
int has_tie(GameState *game);
int roll_dice(void);
void roll_two_dice(int *d1, int *d2);
void move_player(GameState *game, int player_index, int dice_roll);
int find_player_index(GameState *game, int player_id);

void should_buy(GameState *game, int player_id, int square_index);
void should_buy_railway(GameState *game, int player_id, int square_index);
void should_buy_utility(GameState *game, int player_id, int square_index);
void pay_property_rent(GameState *game, int player_index, int square_index);
void pay_railway_rent(GameState *game, int player_index, int square_index);
void pay_utility_rent(GameState *game, int player_index, int square_index, int dice_roll);
void player_build_decision(GameState *game, int player_id);
void player_maintenance_decision(GameState *game, int player_id);

double calculate_net_worth(GameState *game, int player_id);
int attempt_raise_funds(GameState *game, int player_id, double amount_needed);
void declare_bankruptcy(GameState *game, int player_id);
void player_loan_decision(GameState *game, int player_id);
void apply_interest_all(GameState *game);
void check_loan_defaults(GameState *game);
void handle_insurance_landing(GameState *game, int player_index);
void update_insurance_durations(GameState *game);
void handle_tax_landing(GameState *game, int player_id);

void apply_property_depreciation(GameState *game);
void apply_building_depreciation(GameState *game);
void handle_inflation(GameState *game);
void update_property_market(GameState *game);
const char* get_group_name(PropertyGroup group);
void trigger_disaster(GameState *game);
void attempt_disaster_repairs(GameState *game, int player_id);

void draw_national_card(GameState *game, int player_index);
void trigger_economic_event(GameState *game);
const char* get_economic_event_name(EconomicEventType type);
void trigger_regional_development(GameState *game);
const char* get_regional_card_name(RegionalCardType type);
void trigger_government_regulation(GameState *game);
const char* get_government_regulation_name(GovernmentRegulationType type);



void handle_landing(GameState *game, int player_position, int player_id, int dice_roll) {
  int visitor_index = find_player_index(game, player_id);

  switch (game->board[player_position].type) {

  case SQUARE_START:
    printf("Landed exactly on GO - LKR 2000 already collected\n");
    break;

  case SQUARE_PROPERTY:
    printf("Landed on %s\n", game->board[player_position].name);

    if (game->board[player_position].owner_id == -1) {
      should_buy(game, player_id, player_position);
    } else if (game->board[player_position].owner_id == player_id) {
      printf("%s owns this property\n", game->players[visitor_index].name);
    } else {
      pay_property_rent(game, visitor_index, player_position);
    }
    break;

  case SQUARE_RAILWAY:
    printf("Landed on %s\n", game->board[player_position].name);

    if (game->board[player_position].owner_id == -1) {
      should_buy_railway(game, player_id, player_position);
    } else if (game->board[player_position].owner_id == player_id) {
      printf("%s owns this railway\n", game->players[visitor_index].name);
    } else {
      pay_railway_rent(game, visitor_index, player_position);
    }
    break;

  case SQUARE_UTILITY:
    printf("Landed on %s\n", game->board[player_position].name);

    if (game->board[player_position].owner_id == -1) {
      should_buy_utility(game, player_id, player_position);
    } else if (game->board[player_position].owner_id == player_id) {
      printf("%s owns this utility\n", game->players[visitor_index].name);
    } else {
      pay_utility_rent(game, visitor_index, player_position, dice_roll);
    }
    break;

  case SQUARE_TAX:
    handle_tax_landing(game, player_id);
    break;
  case SQUARE_INSURANCE:
    handle_insurance_landing(game, visitor_index);
    break;
  case SQUARE_EVENT:
    draw_national_card(game, visitor_index);
    break;
  case SQUARE_JAIL:
    printf("Landed on Jail\n");
    break;
  case SQUARE_SPECIAL:
    if (player_position == SQUARE_GOTO_JAIL_POS) {
      printf("Landed on Go To Jail!\n");
      printf("%s is sent directly to Jail without collecting Go money.\n", game->players[visitor_index].name);
      game->players[visitor_index].in_jail = 1;
      game->players[visitor_index].jail_turns = 0;
      game->players[visitor_index].position = SQUARE_JAIL_POS;
    } else if (player_position == SQUARE_JAIL_POS) {
      printf("Landed on Jail (Just Visiting)\n");
    } else {
      printf("Landed on %s\n", game->board[player_position].name);
    }
    break;
  case SQUARE_BANK:
    printf("Landed on Bank of Ceylon\n");
    player_loan_decision(game, player_id);
    break;
  }
}

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

static int have_all_players_passed_go(GameState *game) {
  for (int i = 0; i < game->num_players; i++) {
    if (!game->players[i].is_bankrupt && !game->players[i].in_jail && !game->players[i].passed_go_this_round) {
      return 0;
    }
  }
  return 1;
}

static int count_solvent_players(GameState *game) {
  int count = 0;
  for (int k = 0; k < game->num_players; k++) {
    if (!game->players[k].is_bankrupt) count++;
  }
  return count;
}

void init_national_deck(GameState *game);

void run_game_simulation(void) {
  GameState game = {0};
  game.current_turn = 0;

  //--------------------Game INITIALIZATION--------------------

  init_board(&game);
  init_players(&game);
  init_national_deck(&game);

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
  const int MAX_ROUNDS = 500;
  while (j < MAX_ROUNDS) {
    if (count_solvent_players(&game) <= 1) {
      printf("\n[GAME END] Only one player remains solvent!\n");
      break;
    }

    printf("---Round %d---\n", j + 1);

    // Turn rotation loop: Continues until ALL solvent players pass GO
    while (!have_all_players_passed_go(&game)) {
      if (count_solvent_players(&game) <= 1) {
        break;
      }

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

        if (count_solvent_players(&game) <= 1) {
          break;
        }
      }
    }

    if (count_solvent_players(&game) <= 1) {
      printf("\n[GAME END] Only one player remains solvent!\n");
      break;
    }

    // Step 4: END-OF-ROUND PROCESSING - compound interest on all active loans
    apply_interest_all(&game);

    // Step 5: Check if any loans have expired - trigger foreclosure
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

    // Update National Event Cards
    for (int p = 0; p < game.num_players; p++) {
      if (game.players[p].active_national_card != NATIONAL_CARD_NONE) {
        if (game.players[p].national_card_rounds_left > 0) {
          game.players[p].national_card_rounds_left--;
          if (game.players[p].national_card_rounds_left == 0) {
            printf("NATIONAL EVENT EXPIRED %s's event card has concluded.\n", game.players[p].name);
            game.players[p].active_national_card = NATIONAL_CARD_NONE;
            game.players[p].revalued_group = GROUP_NONE;
            game.players[p].construction_suspended = 0;
          }
        }
      }
    }

    // Update Closed Properties (Political Rally)
    for (int idx = 0; idx < TOTAL_SQUARES; idx++) {
      if (game.board[idx].type == SQUARE_PROPERTY && game.board[idx].data.property.closed_rounds_left > 0) {
        game.board[idx].data.property.closed_rounds_left--;
        if (game.board[idx].data.property.closed_rounds_left == 0) {
            printf("PROPERTY REOPENED %s is open for business again!\n", game.board[idx].name);
        }
      } else if (game.board[idx].type == SQUARE_RAILWAY && game.board[idx].data.railway.closed_rounds_left > 0) {
        game.board[idx].data.railway.closed_rounds_left--;
        if (game.board[idx].data.railway.closed_rounds_left == 0) {
            printf("PROPERTY REOPENED %s is open for business again!\n", game.board[idx].name);
        }
      } else if (game.board[idx].type == SQUARE_UTILITY && game.board[idx].data.utility.closed_rounds_left > 0) {
        game.board[idx].data.utility.closed_rounds_left--;
        if (game.board[idx].data.utility.closed_rounds_left == 0) {
            printf("PROPERTY REOPENED %s is open for business again!\n", game.board[idx].name);
        }
      }
    }

    // Anti-Speculation Act timers
    for (int idx = 0; idx < TOTAL_SQUARES; idx++) {
      if (game.board[idx].type == SQUARE_PROPERTY && game.board[idx].data.property.forced_development_rounds_left > 0) {
        game.board[idx].data.property.forced_development_rounds_left--;
        if (game.board[idx].data.property.forced_development_rounds_left == 0) {
          if (game.board[idx].data.property.num_houses == 0 && !game.board[idx].data.property.has_hotel) {
            int owner_id = game.board[idx].data.property.owner_id;
            if (owner_id != -1) {
              int p_idx = find_player_index(&game, owner_id);
              if (p_idx != -1) {
                printf("ANTI-SPECULATION PENALTY %s failed to develop %s in time! Fined LKR 5000.\n", 
                       game.players[p_idx].name, game.board[idx].name);
                game.players[p_idx].money -= 5000;
              }
            }
          }
        }
      }
    }

    // Print active market conditions (Rule 36)
    if (game.market_rounds_left > 0) {
      game.market_rounds_left--;
      printf("\nMARKET STATUS Boom: %s Group | Decline: %s Group | Rounds Left: %d\n",
             get_group_name(game.market_boom_group),
             get_group_name(game.market_decline_group),
             game.market_rounds_left);
      
      if (game.market_rounds_left == 0) {
        game.market_boom_group = GROUP_NONE;
        game.market_decline_group = GROUP_NONE;
        printf("MARKET UPDATE The market has stabilized. Conditions return to normal.\n\n");
      }
    }
    
    // Step 12: Economic Events (every 15 rounds)
    if ((j + 1) % 15 == 0) {
      trigger_economic_event(&game);
    }
    
    // Step 13: Update Economic Event Status
    if (game.economic_event_rounds_left > 0) {
      game.economic_event_rounds_left--;
      printf("ECONOMIC STATUS Active Event: %s | Rounds Left: %d\n",
             get_economic_event_name(game.active_economic_event),
             game.economic_event_rounds_left);
             
      if (game.economic_event_rounds_left == 0) {
        game.active_economic_event = EVENT_NONE;
        printf("ECONOMIC UPDATE The national economic event has concluded.\n\n");
      }
    }
    
    // Step 14: Regional Development (every 15 rounds)
    if ((j + 1) % 15 == 0) {
      trigger_regional_development(&game);
    }
    
    // Step 15: Update Regional Development Status
    if (game.regional_card_rounds_left > 0) {
      game.regional_card_rounds_left--;
      printf("REGIONAL STATUS Active Card: %s | Rounds Left: %d\n",
             get_regional_card_name(game.active_regional_card),
             game.regional_card_rounds_left);
             
      if (game.regional_card_rounds_left == 0) {
        game.active_regional_card = CARD_NONE;
        printf("REGIONAL UPDATE The regional development period has concluded.\n\n");
      }
    }

    // Government Regulations (every 20 rounds)
    if ((j + 1) % 20 == 0) {
      trigger_government_regulation(&game);
    }
    
    // Update Government Regulation Status
    if (game.regulation_rounds_left > 0) {
      game.regulation_rounds_left--;
      printf("REGULATION STATUS Active: %s | Rounds Left: %d\n",
             get_government_regulation_name(game.active_regulation),
             game.regulation_rounds_left);
             
      if (game.regulation_rounds_left == 0) {
        game.active_regulation = REGULATION_NONE;
        printf("REGULATION UPDATE The government regulation has concluded.\n\n");
      }
    }
             
    // END-OF-ROUND PLAYER SUMMARY
    printf("\n==================== ROUND %d SUMMARY ====================\n", j + 1);
    for (int p_i = 0; p_i < game.num_players; p_i++) {
      Player *p = &game.players[p_i];
      if (p->is_bankrupt) {
        printf("Player: %s BANKRUPT\n", p->name);
        printf("----------------------------------------------------------\n");
        continue;
      }

      int prop_count = 0;
      int hotel_count = 0;
      for (int sq = 0; sq < TOTAL_SQUARES; sq++) {
        if (game.board[sq].owner_id == p->id) {
          prop_count++;
          if (game.board[sq].type == SQUARE_PROPERTY && game.board[sq].data.property.has_hotel) {
            hotel_count++;
          }
        }
      }

      double net_worth = calculate_net_worth(&game, p->id);

      printf("Player:             %s\n", p->name);
      printf("Cash:             LKR %.0lf\n", p->money);
      printf("Net Worth: LKR %.0lf\n", net_worth);
      printf("Properties: %d\n", prop_count);
      printf("Hotels:           %d\n", hotel_count);
      printf("Outstanding Loan: LKR %.0lf\n", p->has_loan ? p->loan_amount : 0.0);
      printf("----------------------------------------------------------\n");
    }
    printf("==========================================================\n\n");

    for (int p_i = 0; p_i < game.num_players; p_i++) {
      game.players[p_i].passed_go_this_round = 0;
    }

    game.current_turn = game.current_turn + 1;
    j++;
  }

  printf("\n========== GAME OVER ==========\n");
  int winner_index = -1;
  double max_nw = -99999999;
  
  for (int k = 0; k < game.num_players; k++) {
    if (game.players[k].is_bankrupt) continue;
    double nw = calculate_net_worth(&game, game.players[k].id);
    printf("%s's Final Net Worth: LKR %.0lf\n", game.players[k].name, nw);
    if (nw > max_nw) {
      max_nw = nw;
      winner_index = k;
    }
  }
  
  if (winner_index != -1) {
    printf("\n*** THE WINNER IS %s WITH A NET WORTH OF LKR %.0lf! ***\n", game.players[winner_index].name, max_nw);
  } else {
    printf("\n*** EVERYONE IS BANKRUPT! NO WINNER! ***\n");
  }
}