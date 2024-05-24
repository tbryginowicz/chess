#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SIZE 8
#define SQUARE_SIZE 50
#define WINDOW_WIDTH (SIZE * SQUARE_SIZE)
#define WINDOW_HEIGHT (SIZE * SQUARE_SIZE + 50)
bool whiteInCheck = false;
bool blackInCheck = false;

char board[SIZE][SIZE] = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
};

char currentPlayer = 'W'; // 'W' for White, 'B' for Black

void printBoard() {
    printf("  a b c d e f g h\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%d ", SIZE - i);
        for (int j = 0; j < SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("%d\n", SIZE - i);
    }
    printf("  a b c d e f g h\n");
}

bool isMoveInBounds(int srcX, int srcY, int destX, int destY) {
    return srcX >= 0 && srcX < SIZE && srcY >= 0 && srcY < SIZE &&
           destX >= 0 && destX < SIZE && destY >= 0 && destY < SIZE;
}

bool isOwnPiece(char piece) {
    return (currentPlayer == 'W' && isupper(piece)) || (currentPlayer == 'B' && islower(piece));
}

bool isOpponentPiece(char piece) {
    return (currentPlayer == 'W' && islower(piece)) || (currentPlayer == 'B' && isupper(piece));
}

bool isPathClear(int srcX, int srcY, int destX, int destY) {
    int xDir = (destX > srcX) ? 1 : (destX < srcX) ? -1 : 0;
    int yDir = (destY > srcY) ? 1 : (destY < srcY) ? -1 : 0;
    int x = srcX + xDir, y = srcY + yDir;

    while (x != destX || y != destY) {
        if (board[x][y] != ' ') return false;
        x += xDir;
        y += yDir;
    }
    return true;
}
bool isKingInCheck(char king);

bool willMovePutKingInCheck(int srcX, int srcY, int destX, int destY, char king) {
    char originalPiece = board[destX][destY];
    board[destX][destY] = board[srcX][srcY];
    board[srcX][srcY] = ' ';

    bool kingInCheck = isKingInCheck(king);
    //printf("%i", kingInCheck);
    // Przywróć pierwotną sytuację planszy
    board[srcX][srcY] = board[destX][destY];
    board[destX][destY] = originalPiece;

    return kingInCheck;
}

void forceMovePiece(int srcX, int srcY, int destX, int destY);
bool isWhiteInCheck();
bool isBlackInCheck();

bool isValidMove(int srcX, int srcY, int destX, int destY) {
    if (!isMoveInBounds(srcX, srcY, destX, destY)) return false;
    char piece = board[srcX][srcY];
    char dest = board[destX][destY];
    if (dest == 'k' || dest == 'K') return false;
    if (piece == ' ') return false;
    if (!isOwnPiece(piece)) return false;
    if (isOwnPiece(dest)) return false;
    printf("XD");
    //isWhiteInCheck();
    //isBlackInCheck();
    if(currentPlayer == 'W' && whiteInCheck){
        int srcXcp = srcX;
        int srcYcp = srcY;
        int destXcp = destX;
        int destYcp = destY;
        forceMovePiece(srcXcp,srcYcp,destXcp,destYcp);
        bool inCheck = isWhiteInCheck();
        if (inCheck){
            srcX = srcXcp;
            srcY = srcYcp;
            destX = destXcp;
            destY = destYcp;
            isWhiteInCheck();
            forceMovePiece(destX,destY,srcX,srcY);
            return false;
        }
        srcX = srcXcp;
        srcY = srcYcp;
        destX = destXcp;
        destY = destYcp;
        forceMovePiece(destX,destY,srcX,srcY);
        printf("\nPOOBNO TEN RUHC NIE SPOWODOWAL SZACHU1!\n");
        isWhiteInCheck();
    }
    else if(currentPlayer == 'B' && blackInCheck){
        int srcXcp = srcX;
        int srcYcp = srcY;
        int destXcp = destX;
        int destYcp = destY;
        forceMovePiece(srcX,srcY,destX,destY);
        printf("%d %d %d %d\n",destX,destY,srcX,srcY);
        bool inCheck = isBlackInCheck();
        if (inCheck){
            srcX = srcXcp;
            srcY = srcYcp;
            destX = destXcp;
            destY = destYcp;
            isBlackInCheck();
            forceMovePiece(destX,destY,srcX,srcY);
            return false;
        }
        srcX = srcXcp;
        srcY = srcYcp;
        destX = destXcp;
        destY = destYcp;
        forceMovePiece(destXcp,destYcp,srcXcp,srcYcp);
        printf("\nPOOBNO TEN RUHC NIE SPOWODOWAL SZACHU2!\n");
        isBlackInCheck();
    }
    int xDiff = abs(destX - srcX);
    int yDiff = abs(destY - srcY);

    switch (tolower(piece)) {
        case 'p': {
            int dir = (piece == 'P') ? -1 : 1;
            if ((destX == srcX + dir && destY == srcY && dest == ' ') ||
                (destX == srcX + dir && abs(destY - srcY) == 1 && isOpponentPiece(dest)) ||
                (piece == 'P' && srcX == 6 && destX == 4 && destY == srcY && board[5][srcY] == ' ' && dest == ' ') ||
                (piece == 'p' && srcX == 1 && destX == 3 && destY == srcY && board[2][srcY] == ' ' && dest == ' ')) {
                return true;
            }
            break;
        }
        case 'r':
            return (srcX == destX || srcY == destY) && isPathClear(srcX, srcY, destX, destY);
        case 'n':
            return (xDiff == 2 && yDiff == 1) || (xDiff == 1 && yDiff == 2);
        case 'b':
            return (xDiff == yDiff) && isPathClear(srcX, srcY, destX, destY);
        case 'q':
            return ((srcX == destX || srcY == destY) || (xDiff == yDiff)) && isPathClear(srcX, srcY, destX, destY);
        case 'k':
            return xDiff <= 1 && yDiff <= 1;
    }
    return false;
}

bool movePiece(int srcX, int srcY, int destX, int destY) {
    char king = (currentPlayer == 'W') ? 'k' : 'K';
        bool hey = isKingInCheck(king);
        if(hey){
            printf("HELLO XD");
        } 
    if (isValidMove(srcX, srcY, destX, destY)) {
        board[destX][destY] = board[srcX][srcY];
        board[srcX][srcY] = ' ';
        
        return true;
    } else {
        printf("Invalid move\n");
        return false;
    }
}
void printKings();
void forceMovePiece(int srcX, int srcY, int destX, int destY) {
        printf("FORCE MOVED USED FROM %d %d TO %d %d \n", srcX, srcY, destX, destY);
        board[destX][destY] = board[srcX][srcY];
        board[srcX][srcY] = 'X';
}

void switchPlayer() {
    currentPlayer = (currentPlayer == 'W') ? 'B' : 'W';
}

bool isKingInCheck(char king) {
    int kingX = -1, kingY = -1;
    // Find the king's position
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == king) {
                kingX = i;
                kingY = j;
                
                break;
                
            }
        }
        if (kingX != -1) break;
    }
    if (kingX == -1 || kingY == -1) {
        printf("King not found on the board!\n");
        return false;
    }

    // Check if any opponent piece can move to the king's position
    bool isCheck = false;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (isOpponentPiece(board[i][j])) {
                //printf("%c %d %d \n",board[i][j], kingX, kingY);
                bool validMove = isValidMove(i, j, kingX, kingY);
                if(board[i][j] == 'q'){
                    //printf("%d %d, %d %d, %c \n", i, j, kingX, kingY, board[i][j]);
                    //printf("HAHA %d\n",isValidMove(i, j, kingX, kingY));
                }
                if (validMove) {
                    printf("Opponent piece at (%d, %d) can move to the king's position (%d, %d).\n", i, j, kingX, kingY);
                    isCheck = true;
                }
            }
        }
    }
    return isCheck;
}

bool isCheckmate() {
    char king = (currentPlayer == 'W') ? 'K' : 'k';
    if (!isKingInCheck(king)) return false;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (isOwnPiece(board[i][j])) {
                for (int x = 0; x < SIZE; x++) {
                    for (int y = 0; y < SIZE; y++) {
                        char originalPiece = board[x][y];
                        if (isValidMove(i, j, x, y)) {
                            board[x][y] = board[i][j];
                            board[i][j] = ' ';
                            if (!isKingInCheck(king)) {
                                board[i][j] = board[x][y];
                                board[x][y] = originalPiece;
                                return false;
                            }
                            board[i][j] = board[x][y];
                            board[x][y] = originalPiece;
                        }
                    }
                }
            }
        }
    }
    return true;
}


void drawBoard(SDL_Renderer *renderer) {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Load fonts
    TTF_Font *font = TTF_OpenFont("CASEFONT.ttf", 24);
    TTF_Font *textFont = TTF_OpenFont("arial.ttf", 18);
    if (!font || !textFont) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    // Set text color for current player's turn
    SDL_Color textColor = {255, 255, 255};

    // Draw the current player's turn text
    char playerTurnText[20];
    sprintf(playerTurnText, "CURRENT PLAYER: %c", currentPlayer);
    SDL_Surface *playerTurnSurface = TTF_RenderText_Solid(textFont, playerTurnText, textColor);
    SDL_Texture *playerTurnTexture = SDL_CreateTextureFromSurface(renderer, playerTurnSurface);
    SDL_Rect playerTurnRect = {10, 10, 200, 30};
    SDL_RenderCopy(renderer, playerTurnTexture, NULL, &playerTurnRect);

    // Clean up player turn text surface and texture
    SDL_DestroyTexture(playerTurnTexture);
    SDL_FreeSurface(playerTurnSurface);

    // Draw the chessboard and pieces
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            // Calculate the position of the square
            SDL_Rect rect = {j * SQUARE_SIZE, i * SQUARE_SIZE + 50, SQUARE_SIZE, SQUARE_SIZE};

            // Set the color of the square (green and gray)
            if ((i + j) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 20, 107, 45, 255); // Green
            } else {
                SDL_SetRenderDrawColor(renderer, 151, 163, 124, 255); // blue
            }
            SDL_RenderFillRect(renderer, &rect);

            // Draw the piece on the square
            if (board[i][j] != ' ') {
                // Determine the color of the piece
                SDL_Color pieceColor = isupper(board[i][j]) ? (SDL_Color){255, 255, 255} : (SDL_Color){0, 0, 0};
                
                // Convert the piece to lowercase for rendering
                char piece = tolower(board[i][j]);

                // Render the piece
                SDL_Surface *pieceSurface = TTF_RenderText_Solid(font, (char[2]){piece, '\0'}, pieceColor);
                SDL_Texture *pieceTexture = SDL_CreateTextureFromSurface(renderer, pieceSurface);

                // Position the piece in the center of the square
                SDL_Rect pieceRect = {j * SQUARE_SIZE + 10, i * SQUARE_SIZE + 60, SQUARE_SIZE - 20, SQUARE_SIZE - 20};
                SDL_RenderCopy(renderer, pieceTexture, NULL, &pieceRect);

                // Clean up piece surface and texture
                SDL_DestroyTexture(pieceTexture);
                SDL_FreeSurface(pieceSurface);
            }
        }
    }

    // Present the renderer contents to the screen
    SDL_RenderPresent(renderer);

    // Close fonts
    TTF_CloseFont(font);
    TTF_CloseFont(textFont);
}



int kings[][2]={{-1, -1},{-1, -1}};
void findKings(){
    for (int i = 0; i<= SIZE; i++){
        for (int j = 0; j<=SIZE; j++){
            if(board[i][j] == 'K'){
                kings[0][0] = i;
                kings[0][1] = j;
            }
            if(board[i][j] == 'k'){
                kings[1][0] = i;
                kings[1][1] = j;
            }
        }
    }
}
void printKings(){
    printf("White King: [%d, %d]\nBlack King: [%d, %d]\n\n",kings[0][0], kings[0][1], kings[1][0],kings[1][1]);
}
bool isWhiteInCheck(){
    findKings();
    int white[] = {kings[0][0], kings[0][1]};
    for(int i = 0; i <= SIZE; i++){
        for (int j = 0; j <= SIZE; j++){
            if (isValidMove(i,j,white[0],white[1])){
                whiteInCheck = true;
                printf("white king: %i\n",whiteInCheck);
                return true;
            }else{
                whiteInCheck = false;
            }
        }
    }
    printf("white king: %i\n",whiteInCheck);
    return whiteInCheck;
}
bool isBlackInCheck(){
    findKings();
    int black[] = {kings[1][0], kings[1][1]};
    for(int i = 0; i <= SIZE; i++){
        for (int j = 0; j <= SIZE; j++){
            if (isValidMove(i,j,black[0],black[1])){
                blackInCheck = true;
                printf("black king: %i\n",blackInCheck);
                return true;
            }else{
                blackInCheck = false;
            }
        }
    }
    printf("black king: %i\n",blackInCheck);
    return (bool)blackInCheck;
}
/*
void isEitherKingInCheck(){
    findKings();
    int white[] = {kings[0][0], kings[0][1]};
    int black[] = {kings[1][0], kings[1][1]};
    for(int i = 0; i <= SIZE; i++){
        for (int j = 0; j <= SIZE; j++){
            if (isValidMove(i,j,white[0],white[1])){
                printf("BIALY KROL SZACHOWANY %d %d\n",i,j);
                whiteInCheck = true;
                return;
            }else{
                whiteInCheck = false;
            }
            if(isValidMove(i,j,black[0], black[1])){
                printf("CZARNY KROL SZACHOWANY %d %d\n",i,j);
                blackInCheck = true;
                return;
            }else{
                blackInCheck = false;
            }
        }
    }
    printf("%i %i\n",whiteInCheck, blackInCheck);
}
*/
int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Chess", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool quit = false;
    SDL_Event event;
    int srcX = -1, srcY = -1, destX = -1, destY = -1;
    bool awaitingMove = false; // Flag to indicate whether the game is awaiting a move

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && awaitingMove) {
                int x = event.button.x / SQUARE_SIZE;
                int y = (event.button.y - 50) / SQUARE_SIZE; // Adjust for the offset due to the text area
                isWhiteInCheck();
                isBlackInCheck();
                if (srcX == -1 && srcY == -1) {
                    srcX = y;
                    srcY = x;
                } else {
                    destX = y;
                    destY = x;
                    bool moveSuccessful = movePiece(srcX, srcY, destX, destY);
                    if (!moveSuccessful) {
                        printf("Invalid move. Try again.\n");
                        findKings();
                        printKings();
                    } else {
                        switchPlayer();
                        findKings();
                        printKings();
                        isWhiteInCheck();
                        isBlackInCheck();
                        awaitingMove = false; // Set the flag to false to indicate that move has been made
                        
                    }
                    srcX = -1;
                    srcY = -1;
                    destX = -1;
                    destY = -1;
                }
            }
        }

        if (!awaitingMove) {
            drawBoard(renderer);
            if (isCheckmate()) {
                printf("Checkmate! %s wins!\n", (currentPlayer == 'W') ? "Black" : "White");
                break;
            }
            awaitingMove = true; // Set the flag to true to indicate that game is awaiting a move again
            
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();

    return 0;
}
