#include <stdio.h>
#include "types.h"

extern void init_board(void);
extern Square board[TOTAL_SQUARES];

void print_square(int index) {

    Square sq = board[index];
    printf("Index: %d | Name: %s\n", sq.index, sq.name);
    
    switch (sq.type) {
        case SQUARE_PROPERTY:
            printf("Type: PROPERTY\n");
            printf("Group: %d\n", sq.data.property.group);
            printf("Price: LKR %d | Mortgage: LKR %d\n", 
                   sq.data.property.price, sq.data.property.mortgage_value);
            printf("House Cost: LKR %d | Hotel Cost: LKR %d\n", 
                   sq.data.property.house_cost, sq.data.property.hotel_cost);
            printf("Owner ID: %d | Houses: %d | Has Hotel: %d\n", 
                   sq.data.property.owner_id, sq.data.property.num_houses, sq.data.property.has_hotel);
            break;
            
        case SQUARE_RAILWAY:
            printf("Type: RAILWAY\n");
            printf("Price: LKR %d | Owner ID: %d\n", 
                   sq.data.railway.price, sq.data.railway.owner_id);
            break;
            
        case SQUARE_UTILITY:
            printf("Type: UTILITY\n");
            printf("Price: LKR %d | Owner ID: %d\n", 
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
    init_board();
    
    // Print details of all squares
    for (int i = 0; i < TOTAL_SQUARES; i++) {
        print_square(i);
        printf("\n");
    }
    
    return 0;
}