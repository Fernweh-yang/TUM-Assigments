#include "utility.h"
 
bool solveSuduko(int grid[N][N], int row, int col)
{
    if (row == N - 1 && col == N) {
        return true;
    }
 
    if (col == N) {
        row++;
        col = 0;
    }

    if (grid[row][col] > 0) {
        return solveSuduko(grid, row, col + 1);
    }
 
    for (int num = 1; num <= N; num++)
    {
        if (isSafe(grid, row, col, num))
        {
            grid[row][col] = num;
           
            if (solveSuduko(grid, row, col + 1)) {
                return true;
            }
        }
       
        grid[row][col] = 0;
    }

    return false;
}

int main()
{   
    int board[N][N];
    
    // This starts the timer.
    std::string line = readInput();;

    int counter = 0;
    char ch;
    std::istringstream iss(line);
    while (iss.get(ch))
    {
        board[counter / N][counter % N] = parseChar(ch);
        counter += 1;
    }
    solveSuduko(board, 0, 0);
    print(board);

    // This stops the timer.
    std::cout << std::endl << "DONE" << std::endl;

    return 0;
}