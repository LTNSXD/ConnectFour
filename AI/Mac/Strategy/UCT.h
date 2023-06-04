#ifndef UCT_H_
#define UCT_H_

#include "Node.h"
#include "Judge.h"
#include <ctime>

const double TIME_LIMIT = 1.800 * CLOCKS_PER_SEC;

double STARTTIME;

class UCT {
    Node *root; 
    int row, column;
    int noX, noY;
    double startTime;

public:
    UCT(int **board, const int *top, int row, int column, int noX, int noY): row(row), column(column), noX(noX), noY(noY) {
        
        int **_board = new int*[row];
        for (int i = 0; i < row; ++i) {
            _board[i] = new int[column];
            for (int j = 0; j < column; ++j) {
                _board[i][j] = board[i][j];
            }
        }

        int *_top = new int[column];
        for (int i = 0; i < column; ++i) {
            _top[i] = top[i];
        }
        root = new Node(_board, _top, row, column, noX, noY);
    }

    Node *UCTSearch() {
        int count = 0;
        startTime = STARTTIME;
        while (clock() - startTime <= TIME_LIMIT) {
            ++count;
            Node *v = root->TreePolicy();
            double delta = v->DefaultPolicy();
            v->Backup(delta);
        }
        /* 除去信心中的第二项 */
        // std::cerr << "new_count = " << count << std::endl;
        return root->ContestBestChild();
    }

    ~UCT() {
        delete root;
    }

};

#endif
