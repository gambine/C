#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "atoms.h"
#define MAX_WIDTH 255
#define MIN_WIDTH 2
#define MAX_HEIGHT 255
#define MIN_height 2
#define MAX_PLAYER 6
#define MIN_PLAYER 2
#define MAX_LINE 255

void help();
void quit();
void start(char *k_str, char *width_str, char *height_str);
void display();
bool isPositive(char *str);
int calculateSpace();
void printCurrentPlayer();
void nextPlayer();
void stat();
void place(char *x, char *y);
void expand(int i, int j);
void replace(int i, int j);
void resetGrid(int i, int j);
void clean();
bool isPlayerDead(int i);

int k;
int width;
int height;
char command[MAX_LINE];
char param1[MAX_LINE];
char param2[MAX_LINE];
char param3[MAX_LINE];
char line[MAX_LINE];
bool isStart = false;
int player = 0;
player_t players[] = {{"Red", 0}, {"Green", 0}, {"Purple", 0}, {"Blue", 0}, {"Yellow", 0}, {"White", 0}};
bool playerPlaced[] = {false, false, false, false, false, false};
grid_t ***board;

int main(int argc, char **argv)
{
    while (1)
    {
        fgets(line, MAX_LINE, stdin);
        sscanf(line, "%s %s %s %s\n", command, param1, param2, param3);

        if (strcmp(command, "HELP") == 0)
        {
            help();
        }
        else if (strcmp(command, "QUIT") == 0)
        {
            quit();
        }
        else if (strcmp(command, "START") == 0)
        {
            if (isStart == true)
            {
                printf("Invalid Command\n");
                exit(0);
            }
            // sscanf(line, "%d,%d,%d\n", &k, &width, &height);
            start(param1, param2, param3);
        }
        else if (strcmp(command, "STAT") == 0)
        {
            stat();
        }
        else if (strcmp(command, "PLACE") == 0)
        {
            place(param1, param2);
        }
        else if (strcmp(command, "DISPLAY") == 0)
        {
            display();
        }
        printf("\n");
    }

    return 0;
}

int gridLimit(int i, int j)
{
    // corner = 2;
    if (
        // top left
        (i == 0 && j == 0) ||
        // top right
        (i == 0 && j == width - 1) ||
        // bottom left
        (i == height - 1 && j == 0) ||
        // bottom right
        (i == height - 1 && j == width - 1))
    {
        return 2;
    }

    // side = 3;
    if (i == 0 || j == 0 || i == height - 1 || j == width - 1)
    {
        return 3;
    }

    // default = 4;
    return 4;
}

void help()
{
    printf("\n");
    printf("HELP displays this help message\n");
    printf("QUIT quits the current game\n");
    printf("\n");
    printf("DISPLAY draws the game board in terminal\n");
    printf("START <number of players> <width> <height> starts the game\n");
    printf("PLACE <x> <y> places an atom in a grid space\n");
    printf("UNDO undoes the last move made\n");
    printf("STAT displays game statistics\n");
    printf("\n");
    printf("SAVE <filename> saves the state of the game\n");
    printf("LOAD <filename> loads a save file\n");
    printf("PLAYFROM <turn> plays from n steps into the game\n");
}

void clean()
{
    // free memory
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            free(board[i][j]);
        }
        free(board[i]);
    }
    free(board);
}

void quit()
{
    clean();
    printf("Bye!\n");
    exit(0);
}

void stat()
{
    for (int i = 0; i < k; i++)
    {
        player_t p = players[i];
        printf("Player %s:\n", p.colour);
        if (isPlayerDead(i))
        {
            printf("Lost\n");
        }
        else
        {
            printf("Grid Count: %d\n", p.grids_owned);
        }

        if (i < k - 1)
        {
            printf("\n");
        }
    }
}

void replace(int i, int j)
{
    if (i < 0 || j < 0 || i > height - 1 || j > height - 1)
    {
        return;
    }

    // place
    if (board[i][j]->owner == NULL)
    {
        // unoccupied
        board[i][j]->owner = &players[player];
        board[i][j]->atom_count++;

        players[player].grids_owned++;
    }
    else
    {

        // if expand will replace other owner's spot
        if (board[i][j]->owner != &players[player])
        {
            players[player].grids_owned++;
            board[i][j]->owner->grids_owned--;
        }

        board[i][j]->owner = &players[player];
        board[i][j]->atom_count++;

        // expand if need to
        expand(i, j);
    }
}

void resetGrid(int i, int j)
{
    if (board[i][j]->owner != NULL)
    {
        board[i][j]->owner->grids_owned--;
    }

    board[i][j]->owner = NULL;
    board[i][j]->atom_count = 0;
}

int checkWinner()
{
    int playerDeadCount = 0;
    int winnerIndex = -1;
    for (int i = 0; i < k; i++)
    {
        if (isPlayerDead(i))
        {
            playerDeadCount++;
        }
        else
        {
            winnerIndex = i;
        }
    }

    if (playerDeadCount == k - 1)
    {
        return winnerIndex;
    }
    else
    {
        return -1;
    }
}

void expand(int i, int j)
{
    if (board[i][j]->atom_count == gridLimit(i, j))
    {
        resetGrid(i, j);
        // should expand
        replace(i - 1, j);
        replace(i, j + 1);
        replace(i + 1, j);
        replace(i, j - 1);
    }
}

void place(char *x, char *y)
{
    // validation is number
    if (!isPositive(x) || !isPositive(y))
    {
        printf("Invalid command arguments");
        return;
    }

    int i = atoi(y), j = atoi(x);
    // current player

    if (board[i][j]->owner == NULL)
    {
        // unoccupied
        board[i][j]->owner = &players[player];
        board[i][j]->atom_count++;

        players[player].grids_owned++;
    }
    else
    {
        if (board[i][j]->owner == &players[player])
        {
            board[i][j]->atom_count++;

            // expand if need to
            expand(i, j);
        }
        else
        {
            // when place on other player's spot
            printf("Cannot Place Atom Here\n");
            return;
        }
    }

    playerPlaced[player] = true;

    int winnerIndex = checkWinner();

    if (winnerIndex == -1)
    {
        nextPlayer();
        printCurrentPlayer();
    }
    else
    {
        printf("%s Wins!\n", players[winnerIndex].colour);
        clean();
        exit(0);
    }
}

bool isPositive(char *str)
{
    // loop each char check is Digit
    for (int i = 0; i < strlen(str); i++)
    {
        char ch = str[i];
        if (!isdigit(ch))
        {
            return false;
        }
    }
    return true;
}

int calculateSpace()
{
    int count = 0;
    for (int i = 0; i < strlen(line); i++)
    {
        if (line[i] == ' ')
        {
            count++;
        }
    }
    return count;
}

void printCurrentPlayer()
{
    printf("%s's Turn\n", players[player].colour);
}

bool isPlayerDead(int i)
{
    if (playerPlaced[i] && players[i].grids_owned == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void nextPlayer()
{
    while (true)
    {
        player = (player + 1) % k;
        if (!isPlayerDead(player))
        {
            break;
        }
    }
}

// void start(int k, int width, int height, char line[MAX_LINE], bool isStart)
void start(char *k_str, char *width_str, char *height_str)
{
    // validation is number
    if (!isPositive(k_str))
    {
        printf("Invalid command arguments");
        return;
    }

    if (!isPositive(width_str))
    {
        printf("Invalid command arguments");
        return;
    }

    if (!isPositive(height_str))
    {
        printf("Invalid command arguments");
        return;
    }

    k = atoi(k_str);
    width = atoi(width_str);
    height = atoi(height_str);

    int count = calculateSpace();

    //Missing Argument
    if (count < 3)
    {
        printf("Missing Argument\n");
        return;
    }
    //Too many Arguments
    if (count > 3)
    {
        printf("Too many Arguments\n");
        return;
    }

    //Cannot Start Game
    if (k > width * height)
    {
        printf("Cannot Start Game\n");
        return;
    }

    // START GAME HERE

    // TODO: free memory
    board = malloc(height * sizeof(grid_t *));
    for (int i = 0; i < height; i++)
    {
        board[i] = malloc(width * sizeof(grid_t));

        for (int j = 0; j < width; j++)
        {
            // Create new Struct, same as Java grid_t g = new grid_t();
            // grid_t g;
            // g.owner = NULL;
            // g.atom_count = 0;
            // board[i][j] = g;

            // malloc can only assign to POINTER variable
            board[i][j] = malloc(sizeof(grid_t));
            board[i][j]->owner = NULL;
            board[i][j]->atom_count = 0;
        }
    }

    printf("Game Ready\n");
    //which player??
    printCurrentPlayer();
    isStart = true;
}

void display()
{
    printf("\n");
    for (int i = 0; i <= 3 * width; i++)
    {
        if (i == 0)
        {
            printf("+");
        }
        else if (i == (3 * width))
        {
            printf("+\n");
        }
        else
        {
            printf("-");
        }
    }
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (board[i][j]->owner == NULL)
            {
                printf("|  ");
            }
            else
            {
                printf("|%c%d", board[i][j]->owner->colour[0], board[i][j]->atom_count);
                // printf("| %d", board[i][j]->atom_count);
            }

            if (j == width - 1)
            {
                printf("|\n");
            }
        }
    }
    for (int i = 0; i <= 3 * width; i++)
    {
        if (i == 0)
        {
            printf("+");
        }
        else if (i == (3 * width))
        {
            printf("+\n");
        }
        else
        {
            printf("-");
        }
    }
}

