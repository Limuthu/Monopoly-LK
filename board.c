#include "types.h"
#include <stdio.h>
#include <string.h>

void init_board(BoardSquare board[]) {
    for (int i = 0; i < TOTAL_SQUARES; i++) {
        board[i].id = i;
    }

    strcpy(board[0].name, "GO");
    board[0].type = SQUARE_START;

    strcpy(board[1].name, "Pettah");
    board[1].type = SQUARE_PROPERTY;

    strcpy(board[2].name, "Community Development Fund");
    board[2].type = SQUARE_EVENT;

    strcpy(board[3].name, "Maradana");
    board[3].type = SQUARE_PROPERTY;

    strcpy(board[4].name, "Income Tax");
    board[4].type = SQUARE_TAX;

    strcpy(board[5].name, "Colombo Fort Railway Station");
    board[5].type = SQUARE_RAILWAY;

    strcpy(board[6].name, "Bambalapitiya");
    board[6].type = SQUARE_PROPERTY;

    strcpy(board[7].name, "National Event Card");
    board[7].type = SQUARE_EVENT;

    strcpy(board[8].name, "Wellawatte");
    board[8].type = SQUARE_PROPERTY;

    strcpy(board[9].name, "Mount Lavinia");
    board[9].type = SQUARE_PROPERTY;

    strcpy(board[10].name, "Jail / Just Visiting");
    board[10].type = SQUARE_SPECIAL;

    strcpy(board[11].name, "Nugegoda");
    board[11].type = SQUARE_PROPERTY;

    strcpy(board[12].name, "Ceylon Electricity Board");
    board[12].type = SQUARE_UTILITY;

    strcpy(board[13].name, "Maharagama");
    board[13].type = SQUARE_PROPERTY;

    strcpy(board[14].name, "Kottawa");
    board[14].type = SQUARE_PROPERTY;

    strcpy(board[15].name, "Kandy Railway Station");
    board[15].type = SQUARE_RAILWAY;

    strcpy(board[16].name, "Negombo");
    board[16].type = SQUARE_PROPERTY;

    strcpy(board[17].name, "Sri Lanka Insurance");
    board[17].type = SQUARE_INSURANCE;

    strcpy(board[18].name, "Katunayake");
    board[18].type = SQUARE_PROPERTY;

    strcpy(board[19].name, "Ja-Ela");
    board[19].type = SQUARE_PROPERTY;

    strcpy(board[20].name, "Free Parking");
    board[20].type = SQUARE_SPECIAL;

    strcpy(board[21].name, "Kandy City");
    board[21].type = SQUARE_PROPERTY;

    strcpy(board[22].name, "National Event Card");
    board[22].type = SQUARE_EVENT;

    strcpy(board[23].name, "Peradeniya");
    board[23].type = SQUARE_PROPERTY;

    strcpy(board[24].name, "Katugastota");
    board[24].type = SQUARE_PROPERTY;

    strcpy(board[25].name, "Galle Railway Station");
    board[25].type = SQUARE_RAILWAY;

    strcpy(board[26].name, "Galle Fort");
    board[26].type = SQUARE_PROPERTY;

    strcpy(board[27].name, "Unawatuna");
    board[27].type = SQUARE_PROPERTY;

    strcpy(board[28].name, "National Water Supply and Drainage Board");
    board[28].type = SQUARE_UTILITY;

    strcpy(board[29].name, "Hikkaduwa");
    board[29].type = SQUARE_PROPERTY;

    strcpy(board[30].name, "Go To Jail");
    board[30].type = SQUARE_SPECIAL;

    strcpy(board[31].name, "Jaffna Town");
    board[31].type = SQUARE_PROPERTY;

    strcpy(board[32].name, "Nallur");
    board[32].type = SQUARE_PROPERTY;

    strcpy(board[33].name, "Ceylinco Insurance");
    board[33].type = SQUARE_INSURANCE;

    strcpy(board[34].name, "Trincomalee");
    board[34].type = SQUARE_PROPERTY;

    strcpy(board[35].name, "Jaffna Railway Station");
    board[35].type = SQUARE_RAILWAY;

    strcpy(board[36].name, "National Event Card");
    board[36].type = SQUARE_EVENT;

    strcpy(board[37].name, "Nuwara Eliya");
    board[37].type = SQUARE_PROPERTY;

    strcpy(board[38].name, "Bank of Ceylon");
    board[38].type = SQUARE_BANK;

    strcpy(board[39].name, "Galle Face");
    board[39].type = SQUARE_PROPERTY;
}

int main() {
    BoardSquare board[TOTAL_SQUARES];
    init_board(board);
    return 0;
}