#include <stdio.h>
#include "types.h"

// Forward declarations
void init_board(GameState *game);

void print_square(GameState *game, int index) {

    Square sq = game->board[index];
    printf("Index: %d | Name: %s\n", sq.index, sq.name);
    
    switch (sq.type) {
        case SQUARE_PROPERTY:
            printf("Type: PROPERTY\n");
            printf("Group: %d\n", sq.data.property.group);
            printf("Price: LKR %.0f | Mortgage: LKR %.0f\n", 
                   sq.data.property.price, sq.data.property.mortgage_value);
            printf("House Cost: LKR %.0f | Hotel Cost: LKR %.0f\n", 
                   sq.data.property.house_cost, sq.data.property.hotel_cost);
            printf("Owner ID: %d | Houses: %d | Has Hotel: %d\n", 
                   sq.data.property.owner_id, sq.data.property.num_houses, sq.data.property.has_hotel);
            break;
            
        case SQUARE_RAILWAY:
            printf("Type: RAILWAY\n");
            printf("Price: LKR %.0f | Owner ID: %d\n", 
                   sq.data.railway.price, sq.data.railway.owner_id);
            break;
            
        case SQUARE_UTILITY:
            printf("Type: UTILITY\n");
            printf("Price: LKR %.0f | Owner ID: %d\n", 
                   sq.data.utility.price, sq.data.utility.owner_id);
            break;
            
        case SQUARE_START:
            printf("Type: START\n");
            break;
            
        case SQUARE_EVENT:
            printf("Type: EVENT\n");
            break;
            
        case SQUARE_TAX:
            printf("Type: TAX\n");
            break;
            
        case SQUARE_SPECIAL:
            printf("Type: SPECIAL\n");
            break;
            
        case SQUARE_INSURANCE:
            printf("Type: INSURANCE\n");
            break;
            
        case SQUARE_BANK:
            printf("Type: BANK\n");
            break;
            
        default:
            printf("Type: UNKNOWN\n");
            break;
    }
}

int main(void) {
    GameState game;

    init_board(&game);
    
    // Print details of all squares
    for (int i = 0; i < TOTAL_SQUARES; i++) {
        print_square(&game, i);
        printf("\n");
    }
    
    return 0;
}