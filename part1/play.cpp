#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cassert>

#include <map>
#include <set>
#include <utility>
#include <string>


using namespace std;

#define MAX 65536

#define T 0
#define I 1
#define S 2
#define Z 3
#define O 4
#define L 5
#define J 6

#define ROW 1
#define LINE 0


const int kMaxLine = 12;
const int kMaxRow = 30;
const int kRowPerUint64 = 5;
const int kTotalUint = (kMaxRow + kRowPerUint64 - 1) / kRowPerUint64;

const int kMaxTilePerBlock = 4;
const int kMaxBlockTypeCnt = 7;

typedef long long int64;
typedef unsigned long long uint64;

int64 score;
char IN[3];
char G[kMaxRow][kMaxLine];

static const int block_rotate_type[7] =
{
    4, 2, 2, 2, 1, 4, 4
};

static const int block_rows_range[7][4][2] =
{
    { { -1, 1 },{ -1, 2 },{ 0, 2 },{ -1, 2 } },
    { { -1, 3 },{ 0, 1 },{ -1, 3 },{ 0, 1 } },
    { { 0, 2 },{ -1, 2 },{ 0, 2 },{ -1, 2 } },
    { { 0, 2 },{ -1, 2 },{ 0, 2 },{ -1, 2 } },
    { { 0, 2 },{ 0, 2 },{ 0, 2 },{ 0, 2 } },
    { { 0, 3 },{ -1, 1 },{ -2, 1 },{ 0, 2 } },
    { { 0, 3 },{ 0, 2 },{ -2, 1 },{ -1, 1 } }
};

static const int block_points[7][4][4][2] =
{

    {
        // T 型 
        {{0, 0}, {-1, 0}, {1, 0}, {0, -1}},
        {{0, 0}, {-1, 0}, {0, 1}, {0, -1}},
        {{0, 0}, {-1, 0}, {0, 1}, {1, 0}},
        {{0, 0}, {0, -1}, {0, 1}, {1, 0}}
    },
    
    {
        // I 型
        {{0, 0}, {0, -1}, {0, -2}, {0, 1}},
        {{0, 0}, {-1, 0}, {1, 0}, {2, 0}},
        {{0, 0}, {0, -1}, {0, 1}, {0, 2}},
        {{0, 0}, {-1, 0}, {1, 0}, {2, 0}}
    },
    {
        // S 型
        {{0, 0}, {-1, 0}, {0, -1}, {1, -1}},
        {{0, 0}, {0, -1}, {1, 0}, {1, 1}},
        {{0, 0}, {-1, 0}, {0, 1}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, -1}}
    },
    {
        // Z 型 
        {{0, 0}, {-1, -1}, {0, -1}, {1, 0}},
        {{0, 0}, {0, 1}, {1, 0}, {1, -1}},
        {{0, 0}, {-1, 1}, {0, 1}, {1, 0}},
        {{0, 0}, {-1, -1}, {0, 1}, {-1, 0}}
    },
    {
        // O 型 
        {{0, 0}, {0, -1}, {1, 0}, {1, -1}},
        {{0, 0}, {1, 1}, {0, 1}, {1, 0}},
        {{0, 0}, {1, 1}, {0, 1}, {1, 0}},
        {{0, 0}, {1, 1}, {0, 1}, {1, 0}}
    },
    {
        // L 型 
        {{0, 0}, {0, -1}, {0, -2}, {1, 0}},
        {{0, 0}, {0, 1}, {1, 0}, {2, 0}},
        {{0, 0}, {-1, 0}, {0, 1}, {0, 2}}, 
        {{0, 0}, {-1, 0}, {-2, 0}, {0, -1}}
    },
    {
        // J 型 
        {{0, 0}, {-1, 0}, {0, -1}, {0, -2}},
        {{0, 0}, {0, -1}, {1, 0}, {2, 0}},
        {{0, 0}, {0, 1}, {0, 2}, {1, 0}}, 
        {{0, 0}, {-1, 0}, {-2, 0}, {0, 1}}
    }
    
};

int BlockNo(const char c) {
    if ('T' == c) return 0;
    if ('I' == c) return 1;
    if ('S' == c) return 2;
    if ('Z' == c) return 3;
    if ('O' == c) return 4;
    if ('L' == c) return 5;
    if ('J' == c) return 6;
    return -1;
}
int Gain(int row_cnt) {
    if (row_cnt == 1) return 10;
    if (row_cnt == 2) return 30;
    if (row_cnt == 3) return 60;
    if (row_cnt == 4) return 100;
    return 0;
}

struct State {

    State(char g[kMaxRow][kMaxLine]) :data{ 0 }, line_bits_cnt{ 0 } {
        for (int i = 0; i < kMaxRow; ++i) {
            int idx = i / kRowPerUint64;
            int seq = i % kRowPerUint64;
            uint64 r = 0;
            for (int j = 0; j < kMaxLine; ++j) {
                if ('#' == g[i][j]) {
                    r |= 0x01LL << j;
                }
            }
            r <<= seq * kMaxLine;
            data[idx] |= r;
        }
    }

    void setRow(const int row, const uint64 new_row) {
        int idx = row / kRowPerUint64;
        int seq = row % kRowPerUint64;
        data[idx] &= ~(0xfffLL << (kMaxLine * seq));
        data[idx] |= (new_row << (kMaxLine * seq));
    }

    uint64 getRow(const int row) {
        int idx = row / kRowPerUint64;
        int seq = row % kRowPerUint64;
        return (data[idx] >> (kMaxLine * seq)) & 0xfffLL;
    }

    int refresh(const int start_row, const int end_row) {
        int cnt = 0;
        for (int i = start_row; i < end_row; ++i) {
            if (getRow(i) == 0xfffLL) {
                cnt++;
                setRow(i, 0x0LL);
            }
        }
        int idx = start_row;
        for (int i = start_row; i < end_row; ++i) {
            auto tmp = getRow(i);
            if (tmp != 0x0LL) {
                setRow(idx++, tmp);
            }
        }
        for (int i = 0; i < kMaxLine; ++i) {
            line_bits_cnt[i] -= cnt;
        }
        return Gain(cnt);
    }

    int scoreGained(const int block_no, const int rotate, const int final_row) {
        return refresh(final_row + block_rows_range[block_no][rotate][0], final_row + block_rows_range[block_no][rotate][1]);
    }

    void randomMovement(const int block_no, const int rotate, const int row, const int line, int &res_row, int &res_line) {
        auto range = this->horizonRangeForMoving(block_no, rotate, row, line);
        // TODO: movement decide by line_bits_cnt
        int movement = (rand() % 2) ? range.first : range.second - 1;
        if (rand() % 2) {
            movement = (rand() % (range.second - range.first)) + range.first;   
        }
        res_line = line + movement;
        res_row = row;
    }

    bool needContinueMovement(const int block_no, const int rotate, const int row, const int line) {
        if (rand() % 2) {
            return rand() % 2;
        }
        auto range = this->horizonRangeForMoving(block_no, rotate, row, line);
        return !(range.first == 0 || range.second == 1);
    }

    // caution: call it after blockDown return true
    pair<int, int> horizonRangeForMoving(const int block_no, const int rotate, const int row, const int line) {
        pair<int, int> ans{ 0, 1 };
        fillBlock(block_no, rotate, row, line, false);
        for (int i = -1; ; --i) {
            if (!blockIsFit(block_no, rotate, row, line + i)) {
                ans.first = i + 1;
                break;
            }
        }
        for (int i = 1; ; ++i) {
            if (!blockIsFit(block_no, rotate, row, line + i)) {
                ans.second = i;
                break;
            }
        }
        fillBlock(block_no, rotate, row, line, true);
        return ans;
    }

    bool blockDown(const int block_no, const int rotate, const int row, const int line, int &res_row, int &res_line) {
        fillBlock(block_no, rotate, row, line, false);
        int top_row = topBit(line);
        for (int i = top_row - 1; i >= 0; --i) {
            if (blockIsFit(block_no, rotate, i, line)) {
                fillBlock(block_no, rotate, i, line, true);
                res_row = i;
                res_line = line;
                return true;
            }
        }
        return false;
    }

    // cautuion: call after blockIsFit return true
    void fillBlock(const int block_no, const int rotate, const int row, const int line, bool is_one) {
        for (int i = 0; i < kMaxTilePerBlock; ++i) {
            int r = row + block_points[block_no][rotate][i][ROW];
            int l = line + block_points[block_no][rotate][i][LINE];
            setBit(r, l, is_one);
        }
    }

    bool blockIsFit(const int block_no, const int rotate, const int row, const int line) {
        for (int i = 0; i < kMaxTilePerBlock; ++i) {
            int r = row + block_points[block_no][rotate][i][ROW];
            int l = line + block_points[block_no][rotate][i][LINE];
            if (r >= kMaxRow /*|| r < 0*/ || l >= kMaxLine || l < 0 || getBit(r, l)) {
                return false;
            }
        }
        return true;
    }

    int topBit(const int line) {
        for (int i = 0; i < kMaxRow; ++i) {
            if (getBit(i, line)) {
                return i;
            }
        }
        return kMaxRow;
    }

    void setBit(int row, int line, bool is_one) {
        assert(row < kMaxRow);
        assert(line >= 0 && line < kMaxLine);
        if (row < 0) {
            return;
        }
        int idx = row / kRowPerUint64;
        int seq = row % kRowPerUint64;
        auto mask = 0x01LL << (seq * kMaxLine + line);
        if (is_one) {
            if (!(data[idx] & mask)) {
                line_bits_cnt[line]++;
                data[idx] |= mask;
            }
        }
        else {
            if (data[idx] & mask) {
                line_bits_cnt[line]--;
                data[idx] &= ~mask;
            }
        }
    }

    bool getBit(int row, int line) {
        assert(row < kMaxRow);
        assert(line >= 0 && line < kMaxLine);
        if (row < 0) {
            return false;
        }
        int idx = row / kRowPerUint64;
        int seq = row % kRowPerUint64;
        return data[idx] & 0x01LL << (seq * kMaxLine + line);
    }

    bool operator==(const State &that) const {
        return 0 == memcmp(this->data, that.data, kTotalUint * sizeof(uint64));
    }

    bool operator<(const State &that) const {
        return memcmp(this->data, that.data, kTotalUint * sizeof(uint64)) < 0;
    }

public:
    uint64 data[kTotalUint] = { 0 };
    int line_bits_cnt[kMaxLine] = { 0 };
};


const int kMaxDepth = 3;
const int kMaxFanOut = 2;
const int kInitRow = 0, kInitLine = 5;


int MCTS(State &state, int cur_score, int cur_block, int cur_rotate, int next_block, int depth) {
    if (depth >= kMaxDepth) {
        return cur_score;
    }
    // random expansion
    double average = 0;
    int cnt = 0;
    for (int i = 0; i < kMaxFanOut; ++i) {
        State new_state = state;
        int row = kInitRow, line = kInitLine;
        // place current block
        bool success = true;
        for (; new_state.needContinueMovement(cur_block, cur_rotate, row, line); ) {
            new_state.randomMovement(cur_block, cur_rotate, row, line, row, line);
            if (!new_state.blockDown(cur_block, cur_rotate, row, line, row, line)) {
                success = false;
                break;
            }
        }
        if (!success) {
            continue;
        }
        auto socre_gained = new_state.scoreGained(cur_block, cur_rotate, row);
        // recursive
        for (int b = 0; b < kMaxBlockTypeCnt; ++b) {
            for (int r = 0; r < block_rotate_type[next_block]; ++r) {
                if (b == S && r == 0) continue;
                average += MCTS(new_state, cur_score + socre_gained, next_block, r, b, depth + 1) / static_cast<double>(++cnt);
            }
        }
    }
    return average;
}

int main()
{
    srand(time(NULL));
    while (true) {
        if (EOF == scanf("%lld", &score)) {
            break;
        }
        scanf("%s", IN);
        for (int i = 0; i < kMaxRow; ++i) {
            scanf("%s", G[i]);
        }
        int init_block = BlockNo(IN[0]), init_next_block = BlockNo(IN[1]);
        State init_state{ G };
        double max_aver = 0;
        string cmd;
        for (int r = 0; r < block_rotate_type[init_block]; ++r) {
            for (int i = 0; i < kMaxFanOut; ++i) {
                double average = 0;
                int cnt = 0;
                string cur_cmd = 0 == r ? "" : "C" + to_string(r);
                State new_state = init_state;
                int row = kInitRow, line = kInitLine;
                // place current block
                bool success = true;
                for (; new_state.needContinueMovement(init_block, r, row, line); ) {
                    int origin_row = row, origin_line = line;
                    new_state.randomMovement(init_block, r, row, line, row, line);
                    int movement = line - origin_line;
                    // fprintf(stderr, "move to line: %d\n", line);
                    if (movement != 0) {
                        if (!cur_cmd.empty()) {
                            cur_cmd += ",";
                        }
                        cur_cmd += movement < 0 ? ("L" + to_string(-movement)) : ("R" + to_string(movement));
                    }
                    origin_row = row, origin_line = line;
                    if (!new_state.blockDown(init_block, r, row, line, row, line)) {
                        // fprintf(stderr, "blockDown fail, row: %d line: %d\n", row, line);

                        success = false;
                        break;
                    }
                    movement = row - origin_row;
                    if (movement != 0) {
                        if (!cur_cmd.empty()) {
                            cur_cmd += ",";
                        }
                        cur_cmd += "D" + to_string(movement);
                    }
                }
                if (!success) {
                    continue;
                }
                auto socre_gained = new_state.scoreGained(init_block, r, row);
                // fprintf(stderr, "CMD: %s\n", cur_cmd.c_str());
                // recursive
                for (int b = 0; b < kMaxBlockTypeCnt; ++b) {
                    for (int next_r = 0; next_r < block_rotate_type[init_next_block]; ++next_r) {
                        average += MCTS(new_state, 0 + socre_gained, init_next_block, next_r, b, 1) / static_cast<double>(++cnt);
                    }
                }
                if (average >= max_aver) {
                    max_aver = average;
                    cmd = cur_cmd;
                }
            }
        }

        printf("%s\n", cmd.c_str());
        fflush(stdout);
    }
    return 0;
}
