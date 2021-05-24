#include "utility.h"
#include <omp.h>

#define SIZEOF_SUDOKU N*N*sizeof(int)

bool found_solution = false;

int answer[N][N];

void solveSuduko(int grid[N][N], int row, int col)
{   
    if(found_solution){
        return;
    }
    if (row == N - 1 && col == N) {
        found_solution = true;
        std::memcpy(answer,grid,SIZEOF_SUDOKU);
        return;
    }
 
    if (col == N) {
        row++;
        col = 0;
    }

    if (grid[row][col] > 0) {
        solveSuduko(grid, row, col + 1);
        return;
    }
 
    for (int num = 1; num <= N; num++)
    {
        if (isSafe(grid, row, col, num))
        {
            int local_grid[N][N];
            std::memcpy(local_grid,grid,SIZEOF_SUDOKU);
            local_grid[row][col] = num;
            #pragma omp task default(none) firstprivate(local_grid,row,col) final(row>0)
            solveSuduko(local_grid, row, col + 1);
        }
       
        grid[row][col] = 0;
    }

    return;
}

int main()
{   
    int board[N][N];
    
    // This starts the timer.
    std::string line = readInput();;

    int counter = 0;
    char ch;
    // 从string对象line中读取字符.需要包含头文件<sstream>
    std::istringstream iss(line);
    // get().挨个读取字符到ch。
    while (iss.get(ch))
    {   
        // parseChar在utility内，根据读取的字符来算出数独的数字
        board[counter / N][counter % N] = parseChar(ch);
        counter += 1;
    }

    omp_set_num_threads(32);
    #pragma omp parallel
    {   
        #pragma omp single nowait
        solveSuduko(board, 0, 0);
    }
    print(answer);

    // This stops the timer.
    std::cout << std::endl << "DONE" << std::endl;

    return 0;
}