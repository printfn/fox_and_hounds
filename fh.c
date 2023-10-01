#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#if INT_MAX < 2147483647
typedef long int32;
#else
typedef int int32;
#endif

static void assert_eq(int32 a, int32 b) {
	if (a != b) {
		fprintf(stderr, "assertion failed: %ld != %ld\n", (long)a, (long)b);
		exit(1);
	}
}

static int32 choose(int32 a, int32 b) {
	int32 result = a, x = a-1;
	if (a < b) {
		return 0;
	}
	if (a == b || b == 0) {
		return 1;
	}
	for (; x > a - b; --x) {
		result *= x;
	}
	for (x = 2; x <= b; ++x) {
		result /= x;
	}
	return result;
}

typedef struct PiecePositions {
	int32 hounds[4];
	int32 fox;
} PiecePositions;

/* https://en.wikipedia.org/wiki/Combinatorial_number_system */

/* (32 choose 4) * 28 */
#define POSITION_END 1006880

static int32 idx_to_pos(int32 idx) {
	return idx * 2 + idx / 4 % 2;
}

static int32 decompose_next(int32 *k, int *i) {
	int32 c, next_c = 0, n = *k - 1, result;
	do {
		c = next_c;
		++n;
		next_c = choose(n, *k);
	} while (next_c <= *i);
	result = n - 1;
	--*k;
	*i -= c;
	return result;
}

static PiecePositions decompose(int32 i) {
	PiecePositions result;
	int32 k = 4, fox_idx;
	fox_idx = i % 28;
	i /= 28;
	result.hounds[3] = idx_to_pos(decompose_next(&k, &i));
	result.hounds[2] = idx_to_pos(decompose_next(&k, &i));
	result.hounds[1] = idx_to_pos(decompose_next(&k, &i));
	result.hounds[0] = idx_to_pos(decompose_next(&k, &i));
	for (k = 0; k < 32; ++k) {
		i = idx_to_pos(k);
		if (i == result.hounds[0] || i == result.hounds[1] || i == result.hounds[2] || i == result.hounds[3]) {
			continue;
		}
		if (fox_idx-- == 0) {
			result.fox = i;
			break;
		}
	}
	return result;
}

static int32 recompose(PiecePositions pos) {
	int32 result = 0, fox_idx = 0, i = 1, j;
	while (i < 4) {
		j = i;
		while (j > 0 && pos.hounds[j-1] > pos.hounds[j]) {
			result = pos.hounds[j-1];
			pos.hounds[j-1] = pos.hounds[j];
			pos.hounds[j] = result;
			--j;
		}
		++i;
	}
	i = 0;
	result = choose(pos.hounds[0] / 2, 1);
	result += choose(pos.hounds[1] / 2, 2);
	result += choose(pos.hounds[2] / 2, 3);
	result += choose(pos.hounds[3] / 2, 4);
	for (; i < 32; ++i) {
		if (i == pos.hounds[0] / 2 || i == pos.hounds[1] / 2 || i == pos.hounds[2] / 2 || i == pos.hounds[3] / 2) {
			continue;
		}
		if (pos.fox / 2 == i) {
			break;
		}
		++fox_idx;
	}
	result = result * 28 + fox_idx;
	return result;
}

static void print_board(int32 i) {
	PiecePositions p;
	int32 idx = 56;
	p = decompose(i);
	assert_eq(i, recompose(p));
	printf("+---+---+---+---+---+---+---+---+\n");
	for (;;) {
		if (p.fox == idx) {
			printf("| f ");
		} else if (p.hounds[0] == idx || p.hounds[1] == idx || p.hounds[2] == idx || p.hounds[3] == idx) {
			printf("| h ");
		} else {
			printf("|   ");
		}
		if (idx % 8 == 7) {
			printf("|\n+---+---+---+---+---+---+---+---+\n");
			if (idx == 7) {
				printf("\n\n");
				break;
			}
			idx -= 15;
		} else {
			++idx;
		}
	}
}

static int current_player_fox(const PiecePositions *p) {
	return (p->fox + p->hounds[0] + p->hounds[1] + p->hounds[2] + p->hounds[3]) % 2;
}

static int occupied_by_hound(const PiecePositions *p, int32 idx) {
	return p->hounds[0] == idx || p->hounds[1] == idx || p->hounds[2] == idx || p->hounds[3] == idx;
}

static int32 valid_moves(PiecePositions *p, PiecePositions moves[]) {
	int32 count = 0, i;
	if (current_player_fox(p)) {
		if (p->fox < 56 && p->fox % 8 < 7 && !occupied_by_hound(p, p->fox + 9)) {
			moves[count] = *p;
			moves[count++].fox += 9;
		}
		if (p->fox < 56 && p->fox % 8 > 0 && !occupied_by_hound(p, p->fox + 7)) {
			moves[count] = *p;
			moves[count++].fox += 7;
		}
		if (p->fox > 7 && p->fox % 8 < 7 && !occupied_by_hound(p, p->fox - 7)) {
			moves[count] = *p;
			moves[count++].fox -= 7;
		}
		if (p->fox > 7 && p->fox % 8 > 0 && !occupied_by_hound(p, p->fox - 9)) {
			moves[count] = *p;
			moves[count++].fox -= 9;
		}
	} else {
		for (i = 0; i < 4; ++i) {
			if (p->hounds[i] > 7 && p->hounds[i] % 8 < 7 && p->hounds[i] - 7 != p->fox && !occupied_by_hound(p, p->hounds[i] - 7)) {
				moves[count] = *p;
				moves[count++].hounds[i] -= 7;
			}
			if (p->hounds[i] > 7 && p->hounds[i] % 8 > 0 && p->hounds[i] - 9 != p->fox && !occupied_by_hound(p, p->hounds[i] - 9)) {
				moves[count] = *p;
				moves[count++].hounds[i] -= 9;
			}
		}
	}
	return count;
}

#define FOX_WON 1
#define HOUNDS_WON 2

static int game_over(PiecePositions *p) {
	if (p->fox > 56) {
		return FOX_WON;
	}
	if (current_player_fox(p)) {
		PiecePositions moves[4];
		if (valid_moves(p, moves) == 0) {
			return HOUNDS_WON;
		}
	}
	return 0;
}

/* eval: 0: unknown, positive: hounds win at +infinity, negative: fox wins at -infinity */
#define INFINITY 50
typedef struct Node {
	int32 eval;
} Node;

static int32 sign(int32 val) {
	if (val == 0) {
		return 0;
	}
	if (val > 0) {
		return 1;
	}
	return -1;
}

static Node *create_reverse_tree(void) {
	Node val;
	Node *tree;
	int32 i = 0, remaining = 0, step = 1, moves_length, j, next;
	PiecePositions p;
	PiecePositions moves[8];
	tree = calloc(POSITION_END, sizeof val);
	if (!tree) {
		return NULL;
	}
	for (; i < POSITION_END; ++i) {
		tree[i].eval = 0;
		p = decompose(i);
		if (game_over(&p) == HOUNDS_WON) {
			tree[i].eval = INFINITY;
		} else if (game_over(&p) == FOX_WON) {
			tree[i].eval = -INFINITY;
		} else {
			++remaining;
		}
	}
	while (remaining && step < 1000) {
		printf("step %ld, %ld/%ld remaining\n", (long)step, (long)remaining, (long)POSITION_END);
		remaining = 0;
		++step;
		for (i = 0; i < POSITION_END; ++i) {
			/* skip if fully determined */
			if (tree[i].eval)
				continue;
			p = decompose(i);
			moves_length = valid_moves(&p, moves);
			next = current_player_fox(&p) ? INFINITY : -INFINITY;
			for (j = 0; j < moves_length; ++j) {
				val = tree[recompose(moves[j])];
				if (current_player_fox(&p)) {
					if (val.eval < next) {
						next = val.eval - sign(val.eval);
					}
				} else {
					if (val.eval > next) {
						next = val.eval - sign(val.eval);
					}
				}
			}
			tree[i].eval = next;
			if (!tree[i].eval) {
				++remaining;
				if (step == 1000) {
					print_board(i);
				}
			}
		}
	}
	for (i = -INFINITY; i <= INFINITY; ++i) {
		j = 0;
		for (step = 0; step < POSITION_END; ++step) {
			if (tree[step].eval == i) {
				++j;
				if (i == -5) {
					print_board(step);
				}
			}
		}
		printf("eval %2ld has %7ld positions out of %ld\n", (long)i, (long)j, (long)POSITION_END);
	}
	return tree;
}

static void test(void) {
	assert_eq(choose(20, 5), 15504);
}

int main(void) {
	Node *tree;
	test();
	tree = create_reverse_tree();
	if (!tree) {
		return EXIT_FAILURE;
	}
	free(tree);
	return EXIT_SUCCESS;
}
