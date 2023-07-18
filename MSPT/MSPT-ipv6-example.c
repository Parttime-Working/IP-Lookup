#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SETBIT(X,Y)     X|=(1U<<(Y))
#define UNSETBIT(X,Y)   X&=(~(1U<<(Y)))

void CountClock();
unsigned long long int *clock2;

int BSR (int n) {
	asm __volatile__ (
	    "  movl %0,%%eax      \n\t"       // eax=%0
	    "  bsr  %%eax,%0          " /*Bit Scan Reverse*/ // eax最高位bit位置被assign到%0
	    : "=r" (n) : "0" (n) : "%eax");  // : %0跟n共用register, 直接指定%0被assign n, clobber說明eax register會被修改
	return n;
}

////////////////////////////////////////////////////////////////////////////////////
static __inline__ unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (unsigned long long)lo) | ( ((unsigned long long)hi) << 32 );
}
unsigned long long int begin, end, total = 0;
unsigned long long int *clock1;
////////////////////////////////////////////////////////////////////////////////////
// MSPT node
struct mslist
{
	unsigned long long int ip[2];
	unsigned int len;
	unsigned int port;
	struct mslist *left, *right;
	int height;
	unsigned int bitmap[4];
	// unsigned int Eset_port[128];
};
typedef struct mslist msnode;
typedef msnode *MSPT;
MSPT MSPT_root;
int num_node = 0;

struct ENTRY {
	unsigned long long int ip[2];
	unsigned char len;
	unsigned int port;
};
struct ENTRY *table;
int num_entry = 0;
struct q_entry
{
	unsigned long long int ip[2];
};
struct q_entry *query;
int num_query = 0;

unsigned long long int len_mask[65];
unsigned int __32_len_mask[33];
MSPT create_MSPT_node()
{
	int i;
	MSPT temp;
	temp = (MSPT)malloc(sizeof(msnode));
	temp->ip[0] = 0;
	temp->ip[1] = 0;
	temp->len = 0;
	temp->right = NULL;
	temp->left = NULL;
	temp->port = 256;
	temp->height = 0;
	for (i = 0; i < 4; i++) temp->bitmap[i] = 0;

	// for (i = 0; i < 128; i++) temp->Eset_port[i] = 0;
	num_node++;
	return temp;
}
void read_table(char *str, unsigned long long int *upper_ip, unsigned long long int *lower_ip, int *len, unsigned int *nexthop) {
	char tok[] = "/";
	char tok2[] = ":";
	int count = 0;
	char buf[100], *str1;
	unsigned long long int n[8] = {0};
	char *cur = str;
	int check = 0;
	int skip_point = 0;
	int c = 0;
	while ( *cur != '\0' ) {
		if ( *cur == ':' ) {
			cur++;
			check = 1;
			c++;
		}
		if ( *cur == ':' && check) {
			skip_point = c;
		}
		else {
			check = 0;
			cur++;
		}
	}

	char *substr = strtok(str, tok);
	sprintf(buf, "%s\0", strtok(NULL, tok));
	*len = atoi(buf);
	substr = strtok(str, tok2);


	do {
		sprintf(buf, "%s\0", substr);
		n[count] = strtol(buf, NULL, 16);
		count++;
		substr = strtok(NULL, tok2);
	} while (substr != NULL);


	unsigned long long int group[8] = {0};
	for ( int i = 0; i < skip_point; i++ ) {
		group[i] = n[i];
	}
	for ( int i = skip_point;  i + 8 - count < 8; i++ ) {
		group[i + 8 - count] = n[i];
		n[i] = 0;
	}

	for ( int i = 0; i < 8; i++ ) {
		if ( i < 4 ) {
			*upper_ip <<= 16;
			*upper_ip += group[i];
		}
		else {
			*lower_ip <<= 16;
			*lower_ip += group[i];
		}

	}
	*nexthop = 1;

}

void set_query(char *file_name) {
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp = fopen(file_name, "r");
	while (fgets(string, 50, fp) != NULL) {
		//read_table(string,&ip,&len,&nexthop);
		num_query++;
	}
	rewind(fp);
	query = (struct q_entry *)malloc(num_query * sizeof(struct q_entry));
	clock1 = (unsigned long long int *)malloc(num_query * sizeof(unsigned long long int));
	num_query = 0;
	while (fgets(string, 50, fp) != NULL) {
		read_table(string, &upper_ip, &lower_ip, &len, &nexthop);

		query[num_query].ip[0] = upper_ip;
		query[num_query].ip[1] = lower_ip;
		clock1[num_query++] = 10000000;
	}
}


void set_table(char *file_name) {
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp = fopen(file_name, "r");
	while (fgets(string, 50, fp) != NULL) {
		//read_table(string,&ip,&len,&nexthop);
		num_entry++;
	}
	rewind(fp);
	table = (struct ENTRY *)malloc(num_entry * sizeof(struct ENTRY));

	num_entry = 0;
	while (fgets(string, 50, fp) != NULL) {
		read_table(string, &upper_ip, &lower_ip, &len, &nexthop);
		int shift = 0;

		table[num_entry].ip[0] = upper_ip;
		table[num_entry].ip[1] = lower_ip;

		table[num_entry].port = 1;
		table[num_entry++].len = len;
	}
}

int max(int a, int b) {
	if (a > b) return a;
	else return b;
}

int height(MSPT root)
{
	int lh, rh;
	if (root == NULL)
		return 0;
	else {
		if (root->left == NULL) lh = 0;
		else lh = 1 + root->left->height;
		if (root->right == NULL) rh = 0;
		else rh = 1 + root->right->height;
		if (lh > rh) return lh;
		else return rh;
	}

}
int BF(MSPT root)
{
	int lh, rh;
	if (root == NULL)
		return 0;

	if (root->left == NULL)
		lh = 0;
	else
		lh = 1 + root->left->height;

	if (root->right == NULL)
		rh = 0;
	else
		rh = 1 + root->right->height;

	return (lh - rh);
}
MSPT rotateright(MSPT x)
{
	MSPT y;
	y = x->left;
	x->left = y->right;
	y->right = x;
	x->height = height(x);
	y->height = height(y);
	return y;
}

MSPT rotateleft(MSPT x)
{
	MSPT y;
	y = x->right;
	x->right = y->left;
	y->left = x;
	x->height = height(x);
	y->height = height(y);

	return y;
}

MSPT RR(MSPT root)
{
	root = rotateleft(root);
	return root;
}

MSPT LL(MSPT root)
{
	root = rotateright(root);
	return root;
}

MSPT LR(MSPT root)
{
	root->left = rotateleft(root->left);
	root = rotateright(root);

	return root;
}

MSPT RL(MSPT root)
{
	root->right = rotateright(root->right);
	root = rotateleft(root);
	return root;
}
// range-1 cover range-2
int cover(unsigned long long int ip11, unsigned long long int ip12, unsigned int len1, unsigned long long int ip21, unsigned long long int ip22, unsigned int len2)
{
	if (len1 <= len2) {
		if (len1 <= 64) {
			if ((ip11 & len_mask[len1]) ^ (ip21 & len_mask[len1])) return 0;
			else return 1;
		}
		else {
			if (ip11 == ip21) {
				if ((ip12 & len_mask[len1 - 64]) ^ (ip22 & len_mask[len1 - 64])) return 0;
				else return 1;
			}
		}
	}
	return 0;
}
void Eset_check(MSPT node, MSPT new_root)
{
	int i, j, found = 0;
	unsigned int mask;
	unsigned int Eset_len, index;
	unsigned int tmp_map;
	int part;

	if (node->bitmap[3] != 0) part = 3;
	else if (node->bitmap[2] != 0) part = 2;
	else if (node->bitmap[1] != 0) part = 1;
	else part = 0;

	for (i = part; i >= 0; i--) {
		tmp_map = node->bitmap[i];
		while (tmp_map != 0) {
			index = BSR(tmp_map);
			Eset_len = 32 * i + index ; // prefix length
			// check cover
			if (cover(node->ip[0], node->ip[1], Eset_len, new_root->ip[0], new_root->ip[1], new_root->len)) {
				node->bitmap[i] = node->bitmap[i] & __32_len_mask[31 - index];
				new_root->bitmap[i] = new_root->bitmap[i] | tmp_map;
				found = 1;
				break;
			}
			UNSETBIT(tmp_map, index);
		}
		if (found) break;
	}

	for (j = i - 1; j >= 0; j--) {
		new_root->bitmap[j] = node->bitmap[j];
		node->bitmap[j] = 0;
	}
	return;
}

MSPT MSPT_insert(MSPT root, unsigned long long int ip0, unsigned long long int ip1, unsigned int len, unsigned int port, int debug)
{
	if (root == NULL) {
		root = create_MSPT_node();
		root->ip[0] = ip0;
		root->ip[1] = ip1;
		root->len = len;
		root->port = port;
	}
	else {
		if ((ip0 == root->ip[0]) && (ip1 == root->ip[1]) && (len == root->len)) return root;
		if (cover(root->ip[0], root->ip[1], root->len, ip0, ip1, len)) {
			if (root->len < 32)
				root->bitmap[0] = root->bitmap[0] | (1 << root->len);
			else if (root->len < 64)
				root->bitmap[1] = root->bitmap[1] | (1 << (root->len - 32));
			else if (root->len < 96)
				root->bitmap[2] = root->bitmap[2] | (1 << (root->len - 64));
			else
				root->bitmap[3] = root->bitmap[3] | (1 << (root->len - 96));

			// root->Eset_port[root->len - 1] = root->port;
			root->ip[0] = ip0;
			root->ip[1] = ip1;
			root->len = len;
			root->port = port;
			return root;
		}
		if (cover(ip0, ip1, len, root->ip[0], root->ip[1], root->len)) {
			if (len < 32)
				root->bitmap[0] = root->bitmap[0] | (1 << len);
			else if (len < 64)
				root->bitmap[1] = root->bitmap[1] | (1 << (len - 32));
			else if (len < 96)
				root->bitmap[2] = root->bitmap[2] | (1 << (len - 64));
			else
				root->bitmap[3] = root->bitmap[3] | (1 << (len - 96));
			// root->Eset_port[len - 1] = port;
			return root;
		}


		if (ip0 > root->ip[0]) {
			root->right = MSPT_insert(root->right, ip0, ip1, len, port, debug);
		}
		else if (ip0 < root->ip[0]) {
			root->left = MSPT_insert(root->left, ip0, ip1, len, port, debug);
		}
		else if (ip0 == root->ip[0]) {
			if (ip1 > root->ip[1]) {
				root->right = MSPT_insert(root->right, ip0, ip1, len, port, debug);
			}
			else {
				root->left = MSPT_insert(root->left, ip0, ip1, len, port, debug);
			}
		}

		if (BF(root) == -2) {
			if (ip0 > root->right->ip[0]) {
				if ( root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0)
					Eset_check(root, root->right);
				root = RR(root);
			}
			else if (ip0 < root->right->ip[0]) {
				if ( root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0)
					Eset_check(root, root->right->left);
				if ( root->right->bitmap[0] != 0 | root->right->bitmap[1] != 0 | root->right->bitmap[2] != 0 | root->right->bitmap[3] != 0)
					Eset_check(root->right, root->right->left);
				root = RL(root);
			}
			else if (ip0 == root->right->ip[0]) {
				if (ip1 > root->right->ip[1]) {
					if ( root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0)
						Eset_check(root, root->right);
					root = RR(root);
				}
				else {
					if ( root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0)
						Eset_check(root, root->right->left);
					if ( root->right->bitmap[0] != 0 | root->right->bitmap[1] != 0 | root->right->bitmap[2] != 0 | root->right->bitmap[3] != 0)
						Eset_check(root->right, root->right->left);
					root = RL(root);
				}
			}
		}
		else if (BF(root) == 2) {
			if (ip0 < root->left->ip[0]) {
				if ( root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0)
					Eset_check(root, root->left);
				root = LL(root);
			}
			else if (ip0 > root->left->ip[0]) {
				if ( root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0)
					Eset_check(root, root->left->right);
				if ( root->left->bitmap[0] != 0 | root->left->bitmap[1] != 0 | root->left->bitmap[2] != 0 | root->left->bitmap[3] != 0)
					Eset_check(root->left, root->left->right);
				root = LR(root);
			}
			else if (ip0 == root->left->ip[0]) {
				if (ip1 > root->left->ip[1]) {
					if ( root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0)
						Eset_check(root, root->left->right);
					if ( root->left->bitmap[0] != 0 | root->left->bitmap[1] != 0 | root->left->bitmap[2] != 0 | root->left->bitmap[3] != 0)
						Eset_check(root->left, root->left->right);
					root = LR(root);
				}
				else {
					if ( root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0)
						Eset_check(root, root->left);
					root = LL(root);
				}
			}
		}
	}

	root->height = height(root);
	return root;
}

void create_MSPT()
{
	int i;

	total = 0;
	for (i = 0; i < num_entry; i++)
		MSPT_root = MSPT_insert(MSPT_root , table[i].ip[0], table[i].ip[1], table[i].len, table[i].port, i);

	printf("before update: tree height: %d(root count as 0)\n", MSPT_root->height);
	// CountClock();
}

int Eset_match(unsigned long long int ip0, unsigned long long int ip1, unsigned long long int eset_ip0, unsigned long long int eset_ip1, unsigned int bitmap, int part)
{
	unsigned int Eset_len;
	int index;

	while (bitmap != 0) {
		index = BSR(bitmap);
		Eset_len = 32 * part + index;
		if (cover(eset_ip0, eset_ip1, Eset_len, ip0, ip1, 128)) return 1;
		UNSETBIT(bitmap, index);
	}

	return 0;
}

int eset_searched = 0;
int memory_access = 0;
int search_MSPT(unsigned long long int ip0, unsigned long long int ip1, MSPT root)
{
	unsigned int long long node_ip[(root->height) + 1][2];
	unsigned int bitmap[(root->height) + 1][4];
	int i, j;
	int s = -1;

	while (root != NULL) {
		memory_access++;
		if (cover(root->ip[0], root->ip[1], root->len, ip0, ip1, 128)) return 1;
		if (root->bitmap[0] != 0 | root->bitmap[1] != 0 | root->bitmap[2] != 0 | root->bitmap[3] != 0) {
			s = s + 1;
			node_ip[s][0] = root->ip[0];
			node_ip[s][1] = root->ip[1];
			bitmap[s][0] = root->bitmap[0];
			bitmap[s][1] = root->bitmap[1];
			bitmap[s][2] = root->bitmap[2];
			bitmap[s][3] = root->bitmap[3];
		}
		if (ip0 < root->ip[0]) root = root->left;
		else if (ip0 > root->ip[0]) root = root->right;
		else {
			if (ip1 < root->ip[1]) root = root->left;
			else root = root->right;
		}
	}
	for (i = s; i >= 0; i--) {
		for (j = 3; j >= 0; j--) {
			if (bitmap[i][j] != 0) {
				if (Eset_match(ip0, ip1, node_ip[i][0], node_ip[i][1], bitmap[i][j], j)) {
					eset_searched = eset_searched + (s - i);
					return 1;
				}
			}
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
void CountClock()
{
	unsigned int i;
	unsigned int* NumCntClock = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for (i = 0; i < 50; i++) NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for (i = 0; i < num_query; i++)
	{
		if (clock1[i] > MaxClock) MaxClock = clock1[i];
		if (clock1[i] < MinClock) MinClock = clock1[i];
		if (clock1[i] / 100 < 50) NumCntClock[clock1[i] / 100]++;
		else NumCntClock[49]++;
	}
	printf("(MaxClock, MinClock) = (%5llu, %5llu)\n", MaxClock, MinClock);

	for (i = 0; i < 50; i++)
	{
		printf("%d\n", NumCntClock[i]);
	}
	return;
}


void shuffle(struct q_entry *array, int n) {
	int i;
	srand((unsigned)time(NULL));
	struct q_entry *temp = (struct q_entry *)malloc(sizeof(struct q_entry));

	for (i = 0; i < n - 1; i++) {
		size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
		temp->ip[0] = array[j].ip[0];
		temp->ip[1] = array[j].ip[1];

		array[j].ip[0] = array[i].ip[0];
		array[j].ip[1] = array[i].ip[1];

		array[i].ip[0] = temp->ip[0];
		array[i].ip[1] = temp->ip[1];
	}
}

int count_num_eset(MSPT node, int level)
{
	if (node == NULL || level > 19) return 0;
	int has_eset = 0;
	if (node->bitmap[0] != 0 | node->bitmap[1] != 0 | node->bitmap[2] != 0 | node->bitmap[3] != 0) has_eset = 1;
	return count_num_eset(node->left, level + 1) + count_num_eset(node->right, level + 1) + has_eset;

}
int total_eset = 0;
void count_eset_num(MSPT ptr)
{
	if (ptr == NULL) return;
	int i, pos;
	count_eset_num(ptr->left);
	count_eset_num(ptr->right);
	unsigned tmp_map;

	for (i = 0; i < 4; i++) {
		if (ptr->bitmap[i] == 0) continue;
		tmp_map = ptr->bitmap[i];

		while (tmp_map != 0) {
			pos = BSR(tmp_map);
			total_eset++;
			UNSETBIT(tmp_map, pos);
		}
	}
}

void precompute_mask()
{
	int i;
	len_mask[0] = 0;
	for (i = 1; i < 65; i++) {
		len_mask[i] = ~((1ULL << (64 - i)) - 1);
	}

	__32_len_mask[0] = 0;
	for (i = 1; i < 33; i++)
		__32_len_mask[i] = ~((1 << (32 - i)) - 1);
}
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

	int i, j;
	int count = 0;
	set_table(argv[1]);
	set_query(argv[2]);
	precompute_mask();
	printf("\n----------build----------\n");
	create_MSPT();
	printf("num_entry:%d\n", num_entry);
	printf("num_node: %d\n", num_node);
	count_eset_num(MSPT_root);
	printf("total eset prefix:%d\n", total_eset);
	total_eset = 0;
	printf("-------------------------\n");
	if (argc == 4) {
		set_table(argv[3]);
		total = 0;
		for (i = 0; i < num_entry; ++i) {
			begin = rdtsc();
			MSPT_root = MSPT_insert(MSPT_root, table[i].ip[0], table[i].ip[1], table[i].len, table[i].port, i);
			end = rdtsc();
			if (clock1[i] > (end - begin))
				clock1[i] = (end - begin);
			total += clock1[i];
		}
		printf("\n----------Update----------\n");
		printf("Avg. Update: %llu\n", total / num_entry);
		printf("num update:%d\n", num_entry);
		printf("num_node: %d\n", num_node);
		printf("tree height:%d\n", MSPT_root->height);
		printf("-------------------------\n");
		// CountClock();
	}


	// int tmp;
	printf("num_query:%d\n", num_query);
	shuffle(query, num_query);
	for (j = 0; j < 100; j++) {
		for (i = 0; i < num_query; i++) {
			// tmp = count;
			begin = rdtsc();
			count += search_MSPT(query[i].ip[0], query[i].ip[1], MSPT_root);
			end = rdtsc();
			if (clock1[i] > (end - begin))
				clock1[i] = (end - begin);
			// if (tmp == count) printf("fail %d\n", i);
		}
	}
	total = 0;

	for (j = 0; j < num_query; j++)
		total += clock1[j];
	CountClock();
	printf("success:%d\n", count);
	printf("Avg. Search: %llu\n", total / num_query);
	// CountClock();
	// printf("# search eset: %d\n", eset_searched);
	// printf("total eset_node:%d\n", count_num_eset(MSPT_root, 0)); // level<=19

	printf("memory %f KB\n", ((double)num_node * 34 / 1024));
	printf("Avg. memory_access: %f\n", (double) memory_access / (double) num_query);
	count_eset_num(MSPT_root);
	printf("total eset prefix:%d\n", total_eset);
	return 0;

}
