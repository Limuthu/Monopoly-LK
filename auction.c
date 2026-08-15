#include "types.h"
#include <stdio.h>

// Forward declarations
int find_player_index(GameState *game, int player_id);
void purchase_property(GameState *game, int player_index, int square_index);
void purchase_railway(GameState *game, int player_index, int square_index);
void purchase_utility(GameState *game, int player_index, int square_index);
double get_dynamic_auction_start(GameState *game, int square_index);

// =============================================================
// HELPER: GET MARKET VALUE
// =============================================================
// Returns the listed price of any ownable square (property, railway, utility)

double get_market_value(GameState *game, int square_index) {
  if (game->board[square_index].type == SQUARE_PROPERTY) {
    return game->board[square_index].data.property.price;
  } else if (game->board[square_index].type == SQUARE_RAILWAY) {
    return game->board[square_index].data.railway.price;
  } else if (game->board[square_index].type == SQUARE_UTILITY) {
    return game->board[square_index].data.utility.price;
  }
  return 0;
}

// =============================================================
// AI BIDDING STRATEGY
// =============================================================
// Returns the player's bid amount, or 0 to withdraw.
// current_bid is the highest bid so far.
// The minimum new bid = current_bid + 250 (Rule-LK 20).

double decide_bid(GameState *game, int player_index, int square_index,
                  double current_bid, double market_value) {

  double new_bid = current_bid + 250;
  int player_id = game->players[player_index].id;
  double cash = game->players[player_index].money;

  // Cannot bid more than cash on hand (Rule-LK 22)
  if (new_bid > cash) return 0;

  // ---------------------------------------------------------
  // Player 1 — Aggressive Investor
  // Bids aggressively up to 120% of market value
  // ---------------------------------------------------------
  if (player_id == 1) {
    double max_bid = market_value * 1.20;
    if (new_bid <= max_bid && new_bid <= cash) {
      return new_bid;
    }
    return 0;
  }

  // ---------------------------------------------------------
  // Player 2 — Conservative Banker
  // Bargain hunter — exits above market value
  // Also maintains 50% cash reserve after bidding
  // ---------------------------------------------------------
  else if (player_id == 2) {
    if (new_bid <= market_value && new_bid <= cash * 0.5) {
      return new_bid;
    }
    return 0;
  }

  // ---------------------------------------------------------
  // Player 3 — Risk Taker
  // All-in — bids until cash is exhausted
  // ---------------------------------------------------------
  else if (player_id == 3) {
    if (new_bid <= cash) {
      return new_bid;
    }
    return 0;
  }

  // ---------------------------------------------------------
  // Player 4 — Opportunistic Trader
  // Calculated — prefers discount buys (max 90% of market value)
  // Maintains LKR 500 reserve
  // ---------------------------------------------------------
  else if (player_id == 4) {
    double max_bid = market_value * 0.90;
    if (new_bid <= max_bid && (cash - new_bid) >= 500) {
      return new_bid;
    }
    return 0;
  }

  return 0;
}

// =============================================================
// CORE AUCTION ENGINE
// =============================================================
// Run a round-robin auction for a single square.
// triggering_player_id is the player who caused the auction
// (e.g. the one who declined to buy).

void run_auction(GameState *game, int square_index, int triggering_player_id) {

  // Loan-locked properties cannot be auctioned (Rule-LK 3)
  if (game->board[square_index].type == SQUARE_PROPERTY &&
      game->board[square_index].data.property.is_loan_locked) {
    return;
  }
  if (game->board[square_index].type == SQUARE_RAILWAY &&
      game->board[square_index].data.railway.is_loan_locked) {
    return;
  }
  if (game->board[square_index].type == SQUARE_UTILITY &&
      game->board[square_index].data.utility.is_loan_locked) {
    return;
  }

  // Must be unowned to auction
  if (game->board[square_index].owner_id != -1) return;

  double market_value = get_market_value(game, square_index);
  if (market_value <= 0) return;

  // Starting bid (adjusted by Dynamic Property Market)
  double current_bid = get_dynamic_auction_start(game, square_index);

  printf("\n========== AUCTION: %s ==========\n", game->board[square_index].name);
  printf("Market Value: LKR %.0lf | Starting Bid: LKR %.0lf\n",
         market_value, current_bid);

  // Initialize bidder tracking
  int still_in[MAX_PLAYERS];
  int active_count = 0;

  for (int i = 0; i < game->num_players; i++) {
    if (game->players[i].is_bankrupt) {
      still_in[i] = 0;
    } else {
      still_in[i] = 1;
      active_count++;
    }
  }

  // Need at least 1 bidder
  if (active_count == 0) {
    printf("No eligible bidders — %s remains with the Bank\n",
           game->board[square_index].name);
    printf("========== AUCTION END ==========\n\n");
    return;
  }

  int winning_player_index = -1;
  int rounds_with_no_new_bid = 0;

  // Round-robin bidding loop
  while (active_count > 0) {
    int any_bid_this_round = 0;

    for (int i = 0; i < game->num_players; i++) {
      if (!still_in[i]) continue;

      // If only 1 bidder remains and they already hold the highest bid, stop
      if (active_count == 1 && winning_player_index == i) {
        goto auction_done;
      }

      double bid = decide_bid(game, i, square_index, current_bid, market_value);

      if (bid <= 0) {
        // Player withdraws permanently
        printf("  %s withdraws from the auction\n", game->players[i].name);
        still_in[i] = 0;
        active_count--;

        // If only 1 bidder left, end immediately
        if (active_count <= 1) {
          // Check if the remaining bidder already has a winning bid
          if (winning_player_index != -1) {
            goto auction_done;
          }
          // If no one has bid yet and only 1 remains, give them a chance
          if (active_count == 1) {
            // Find the remaining bidder
            for (int j = 0; j < game->num_players; j++) {
              if (still_in[j]) {
                double last_bid = decide_bid(game, j, square_index,
                                             current_bid, market_value);
                if (last_bid > 0) {
                  current_bid = last_bid;
                  winning_player_index = j;
                  printf("  %s bids LKR %.0lf\n", game->players[j].name,
                         current_bid);
                } else {
                  printf("  %s withdraws from the auction\n",
                         game->players[j].name);
                  still_in[j] = 0;
                  active_count--;
                }
                goto auction_done;
              }
            }
          }
          goto auction_done;
        }
      } else {
        // Player places a bid
        current_bid = bid;
        winning_player_index = i;
        any_bid_this_round = 1;
        printf("  %s bids LKR %.0lf\n", game->players[i].name, current_bid);
      }
    }

    // Safety: if a full rotation passed with no new bids, stop
    if (!any_bid_this_round) {
      rounds_with_no_new_bid++;
      if (rounds_with_no_new_bid >= 2) break;
    } else {
      rounds_with_no_new_bid = 0;
    }
  }

auction_done:

  // Determine winner and transfer ownership
  if (winning_player_index != -1) {
    // Deduct the winning bid amount
    game->players[winning_player_index].money -= current_bid;

    // Transfer ownership
    game->board[square_index].owner_id =
        game->players[winning_player_index].id;

    if (game->board[square_index].type == SQUARE_PROPERTY) {
      game->board[square_index].data.property.owner_id =
          game->players[winning_player_index].id;
    } else if (game->board[square_index].type == SQUARE_RAILWAY) {
      game->board[square_index].data.railway.owner_id =
          game->players[winning_player_index].id;
    } else if (game->board[square_index].type == SQUARE_UTILITY) {
      game->board[square_index].data.utility.owner_id =
          game->players[winning_player_index].id;
    }

    printf("  >>> %s WINS the auction for %s at LKR %.0lf (saved LKR %.0lf)\n",
           game->players[winning_player_index].name,
           game->board[square_index].name, current_bid,
           market_value - current_bid);
  } else {
    printf("  No bids placed — %s remains with the Bank\n",
           game->board[square_index].name);
  }

  printf("========== AUCTION END ==========\n\n");
}
