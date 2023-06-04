#ifndef NODE_H_
#define NODE_H_

#include <cstdlib>
#include <cmath>
#include "Judge.h"

#define EMPTY                   0
#define PLAYER                  1
#define AI                      2

#define PLAYER_WIN_PROFIT      -1
#define AI_WIN_PROFIT           1
#define TIE_PROFIT              0
#define UNTERMINAL_STATE       -2

const double INF =              2021012957;

const double c = 0.700;         // 比例系数

class Node{
    int visitedNum = 0;
    Node *parent = nullptr;     // 父节点
    Node **children;            // 子节点
    int expandableNum = 0;      // 可扩展的状态数目
    int *expandableNodes;       // 可扩展的节点编号
    int current;                // 当前执棋方
    double profit = 0.0;        // 当前节点收益

    int **board;                // 当前棋盘状态
    int *top;                   // 棋盘顶部状态
    int row, column;            // 行数、列数
    int noX, noY;               // 不可落子位置
    int x, y;                   // 上一落子位置

    bool Expandable() {
        return expandableNum > 0;
    }

    bool IsTerminal() {
        /* 根结点 */
        if (parent == nullptr) {
            return false;
        }
        /* 先判断棋盘是否已满，使得计算更快 */
        return (isTie(column, top) || // 棋盘已满
            (current == AI && userWin(x, y, row, column, board)) || // 玩家胜利
            (current == PLAYER && machineWin(x, y, row, column, board)) // AI胜利
        );
    }

    /*
        随机选择一个落子位置（列）
    */
    int RandomSelect(int *_top, int *weight) {
        int *_expandable = new int[column];
        int _expandableNum = 0;
        for (int i = 0; i < column; ++i) {
            if (_top[i] > 0) {
                _expandable[_expandableNum++] = i;
            }
        }
        int sum = 0;
        for (int i = 0; i < _expandableNum; ++i) {
            sum += weight[_expandable[i]];
        }
        int choice = rand() % sum;
        int _sum = 0;
        for (int i = 0; i < _expandableNum; ++i) {
            _sum += weight[_expandable[i]];
            if (_sum >= choice) {
                int res = _expandable[i];
                delete[] _expandable;
                return res;
            }
        }
        return -1;
    }

    double CalcProfit(int **_board, int *_top, int _x, int _y, int _current) {
        if (_current == AI && userWin(_x, _y, row, column, _board)) {
            return (double)PLAYER_WIN_PROFIT;
        }
        else if (_current == PLAYER && machineWin(_x, _y, row, column, _board)) {
            return (double)AI_WIN_PROFIT;
        }
        else if (isTie(column, _top)) {
            return (double)TIE_PROFIT;
        }
        else return (double)UNTERMINAL_STATE;
    }

public:
    const int get_x() { return x; }
    const int get_y() { return y; }

    Node(int **board, int *top, int row, int column, int noX, int noY, int _x = -1, int _y = -1, int current = AI, Node *parent = nullptr): 
        board(board), top(top), row(row), column(column), noX(noX), noY(noY), x(_x), y(_y), current(current), parent(parent) {
        children = new Node*[column];
        expandableNodes = new int[column];
        for (int i = 0; i < column; ++i) {
            children[i] = nullptr;
            if (top[i] > 0) {
                // 第i列可以扩展
                expandableNodes[expandableNum++] = i;
            }
        }
    }

    int **get_board() {
        // 返回当前节点的棋盘
        int **_board = new int *[row];
        for (int i = 0; i < row; ++i) {
            _board[i] = new int[column];
            for (int j = 0; j < column; ++j) {
                _board[i][j] = board[i][j];
            }
        }
        return _board;
    }

    int *get_top() {
        // 返回当前节点的top[]数组
        int *_top = new int[column];
        for (int i = 0; i < column; ++i) {
            _top[i] = top[i];
        }
        return _top;
    }

    Node *Expand() {  
        int choice = rand() % expandableNum; // 选择一个可以扩展的节点
        int **_board = get_board();
        int *_top = get_top();
        /* 获取落子位置 */
        int move_Y = expandableNodes[choice];
        int move_X = --_top[move_Y];
        _board[move_X][move_Y] = current;
        if (move_X - 1 == noX && move_Y == noY)
            --_top[move_Y];
        std::swap(expandableNodes[choice], expandableNodes[--expandableNum]);
        /* 交换执棋权 */
        int _current = (current == AI) ? PLAYER : AI;
        /* 创造新孩子节点 */
        children[move_Y] = new Node(_board, _top, row, column, noX, noY, move_X, move_Y, _current, this);
        return children[move_Y];
    }

    Node *BestChild() {
        double maxConf = -INF;
        Node *result = nullptr;
        for (int i = 0; i < column; ++i) {
            if (children[i] == nullptr)
                continue;
            /* 根据节点极大极小类型，计算实际收益值，信心值 */
            double realProfit = (current == AI) ? children[i]->profit : -children[i]->profit;
            double Conf = (realProfit / (double)children[i]->visitedNum) + c * sqrt(2 * log((double)visitedNum) / (double)children[i]->visitedNum);
            if (Conf > maxConf) {
                maxConf = Conf;
                result = children[i];
            }
        }
        return result;
    }

    Node *ContestBestChild() {
        double maxConf = -INF;
        Node *result = nullptr;
        for (int i = 0; i < column; ++i) {
            if (children[i] == nullptr)
                continue;
            double Conf = children[i]->profit / (double)children[i]->visitedNum;
            if (Conf > maxConf) {
                maxConf = Conf;
                result = children[i];
            }
        }
        return result;
    }

    void Backup(double delta) {
        Node *node = this;
        while (node) {
            node->visitedNum += 1;
            node->profit += delta;
            node = node->parent;
        }
    }

    Node *TreePolicy() {
        Node *v = this;
        while (!v->IsTerminal()) {
            if (v->Expandable()) {
                return v->Expand();
            }
            else {
                v = v->BestChild();
            }
        }
        return v;
    }

    double DefaultPolicy() {
        int **_board = get_board();
        int *_top = get_top();
        int _current = current;
        
        /* Version 3: 尽量下中间 */
        int *weight = new int[column];
        int mid = column / 2;
        for (int i = 0; i <= mid - 1; ++i) {
            weight[i] = i + 1;
        }
        for (int i = mid; i < column; ++i ) {
            weight[i] = column - i;
        }
        
        double _profit = CalcProfit(_board, _top, x, y, _current);
        while (_profit == (double)UNTERMINAL_STATE) {
            int _i = 0;
            /* 随机选择下一个状态 */
            /* TODO: Make it faster? */
            _i = RandomSelect(_top, weight);
//            while (_top[_i] <= 0)
//                _i = rand() % column;
            int move_X = --_top[_i], move_Y = _i;
            _board[move_X][move_Y] = _current;
            if (move_X - 1 == noX && move_Y == noY) {
                --_top[_i];
            }
            _current = (_current == AI) ? PLAYER : AI;
            _profit = CalcProfit(_board, _top, move_X, move_Y, _current);
        }
        for (int i = 0; i < row; ++i) {
            delete[] _board[i];
        }
        delete[] _board;
        delete[] _top;
        delete[] weight;
        return _profit;
    }

    ~Node() {
        for (int i = 0; i < column; ++i) {
            if (children[i] != nullptr) {
                delete children[i];
            }
        }
        delete[] children;
        delete[] expandableNodes;
        for (int i = 0; i < row; ++i) {
            delete[] board[i];
        }
        delete[] board;
        delete[] top;
    }
};


#endif
