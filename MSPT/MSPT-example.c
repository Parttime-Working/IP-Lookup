#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SETBIT(X,Y)     X|=(1U<<(Y))
#define UNSETBIT(X,Y)   X&=(~(1U<<(Y)))

void CountClock();

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
	unsigned int ip;
	unsigned int len;
	unsigned int port;
	struct mslist *left, *right;
	int height;
	unsigned int bitmap;
	unsigned int Eset_port[32];
};
typedef struct mslist msnode;
typedef msnode *MSPT;
MSPT MSPT_root;
int num_node = 0;

struct ENTRY {
	unsigned int ip;
	char len;
	unsigned int port;
};
struct ENTRY *table;
int num_entry = 0;

unsigned int *query;
int num_query = 0;

unsigned int len_mask[33];
MSPT create_MSPT_node()
{
	int i;
	MSPT temp;
	temp = (MSPT)malloc(sizeof(msnode));
	temp->ip = 0;
	temp->len = 0;
	temp->port = 256;
	temp->right = NULL;
	temp->left = NULL;
	temp->height = 0;
	temp->bitmap = 0;
	num_node++;
	for (i = 0; i < 32; i++) temp->Eset_port[i] = 0;
	return temp;
}

void set_query(char *file_name) {
	FILE *fp = fopen(file_name, "r");;
	char str[100];
	unsigned int n[4], len, ip, nexthop;
	int i;

	while (fgets(str, 50, fp) != NULL) {
		num_query++;
	}
	rewind(fp);

	query = (unsigned int *)malloc(num_query * sizeof(unsigned int));
	clock1 = (unsigned long long int *)malloc(num_query * sizeof(unsigned long long int));
	num_query = 0;
	while (fgets(str, 100, fp) != NULL) {
		sscanf(str, "%u.%u.%u.%u/%u%*s", &n[0], &n[1], &n[2], &n[3], &len);

		ip = 0;
		for (i = 0; i < 4; i++) {
			ip = ip << 8;
			ip = ip + n[i];
		}
		ip = ip >> (32 - len);
		ip = ip << (32 - len);

		query[num_query] = ip;
		clock1[num_query++] = 10000000;
	}
	fclose(fp);
}
void set_table(char* fname)
{
	FILE *fp = fopen(fname, "r");
	char str[100];
	unsigned int n[4], len, ip, nexthop;
	int i;
	while (fgets(str, 100, fp) != NULL) num_entry++;
	rewind(fp);
	table = (struct ENTRY *)malloc(sizeof(struct ENTRY) * num_entry);

	num_entry = 0;
	while (fgets(str, 100, fp) != NULL) {
		sscanf(str, "%u.%u.%u.%u/%u%*s", &n[0], &n[1], &n[2], &n[3], &len);

		ip = 0;
		for (i = 0; i < 4; i++) {
			ip = ip << 8;
			ip = ip + n[i];
		}
		ip = ip >> (32 - len);
		ip = ip << (32 - len);
		nexthop = n[2];

		table[num_entry].ip = ip;
		table[num_entry].port = nexthop;
		table[num_entry++].len = len;
	}
	fclose(fp);
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

// balance factor
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
int cover(unsigned int ip1, unsigned int len1, unsigned int ip2, unsigned int len2)
{
	if (len1 <= len2) {
		if ((ip1 & len_mask[len1]) ^ (ip2 & len_mask[len1])) return 0;
		else return 1;
	}
	return 0;
}

void Eset_check(unsigned int ip, unsigned int *bitmap, unsigned int root_ip, unsigned int root_len, unsigned int *root_bitmap)
{
	unsigned int tmp_map = *bitmap;
	unsigned int Eset_len;

	while (tmp_map != 0) {
		Eset_len = BSR(tmp_map); // return a bit position from 31 to 0, which represents 31 to 0 of length
		if (cover(ip, Eset_len, root_ip, root_len)) {
			*bitmap = *bitmap & len_mask[31 - Eset_len];
			break;
		}
		UNSETBIT(tmp_map, Eset_len);
	}

	*root_bitmap = *root_bitmap | tmp_map;

	return;
}

MSPT MSPT_insert(MSPT root, unsigned int ip, unsigned int len, unsigned int port)
{
	if (root == NULL) {
		root = create_MSPT_node();
		root->ip = ip;
		root->len = len;
		root->port = port;
	}
	else {
		if ((ip == root->ip) && (len == root->len)) return root;
		if (cover(root->ip, root->len, ip, len)) {
			root->bitmap = root->bitmap | (1 << root->len);
			root->Eset_port[root->len-1] = root->port;

			root->ip = ip;
			root->len = len;
			root->port = port;
			return root;
		}
		if ( cover(ip, len, root->ip, root->len)) {
			root->Eset_port[len-1] = port;
			root->bitmap = root->bitmap | (1 << len);
			return root;
		}
		if (ip > root->ip) {
			root->right = MSPT_insert(root->right, ip, len, port);
			
			if (BF(root) == -2) {
				if (ip > root->right->ip) {
					// check Eset cover relation before rotation
					// root->right will be the new root.
					if ( root->bitmap != 0) Eset_check(root->ip, &root->bitmap, root->right->ip, root->right->len, &root->right->bitmap);
					root = RR(root);
				}
				else {
					// check Eset cover relation before rotation
					// root->right->left will be the new root.
					if ( root->bitmap != 0) Eset_check(root->ip, &root->bitmap, root->right->left->ip, root->right->left->len, &root->right->left->bitmap);
					if ( root->right->bitmap != 0) Eset_check(root->right->ip, &root->right->bitmap, root->right->left->ip, root->right->left->len, &root->right->left->bitmap);
					root = RL(root);
				}
			}
		}
		else if (ip < root->ip) {
			root->left = MSPT_insert(root->left, ip, len, port);

			if (BF(root) == 2) {
				if (ip < root->left->ip) {
					// check Eset cover relation before rotation
					// root->left will be the new root.
					if ( root->bitmap != 0) Eset_check(root->ip, &root->bitmap, root->left->ip, root->left->len, &root->left->bitmap);
					root = LL(root);
				}
				else {
					// check Eset cover relation before rotation
					// root->left->right will be the new root.
					if ( root->bitmap != 0) Eset_check(root->ip, &root->bitmap, root->left->right->ip, root->left->right->len, &root->left->right->bitmap);
					if ( root->left->bitmap != 0) Eset_check(root->left->ip, &root->left->bitmap, root->left->right->ip, root->left->right->len, &root->left->right->bitmap);
					root = LR(root);
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

	for (i = 0; i < num_entry; ++i)
		MSPT_root = MSPT_insert(MSPT_root, table[i].ip, table[i].len, table[i].port);
	// printf("Avg. insert: %llu\n",total/(unsigned int)(num_entry*0.1));
	// printf("Avg. insert: %llu\n", total / (num_entry - _5percent_index));
	// CountClock();
}

int Eset_match(unsigned int d, unsigned int eset_ip, unsigned int bitmap)
{
	unsigned int eset_value;
	unsigned int Eset_len;
	int i;

	while (bitmap != 0) {
		Eset_len = BSR(bitmap);
		if (cover(eset_ip, Eset_len, d, 32)) return 1;
		UNSETBIT(bitmap, Eset_len);
	}
	return 0;
}

/*
	Top-down search MSPT node, if any node cover dst, return.
	otherwise check Eset of each node from the leaf to root(bottom up).
*/
int search_MSPT(unsigned int d, MSPT root)
{
	MSPT x = root;

	unsigned int node_ip[(root->height) + 1];
	unsigned int bitmap[(root->height) + 1];

	int s = -1;
	// printf("search ip:%u\n", d);
	while (x != NULL) {
		// printf("%u %u\n", x->ip, x->u_bound);
		if ( cover(x->ip, x->len, d, 32 )) {
			return 1;
		}
		else {
			if (x->bitmap != 0) {
				s = s + 1;
				node_ip[s] = x->ip;
				bitmap[s] = x->bitmap;
			}
			if (d < x->ip) x = x->left;
			else x = x->right;
		}
	}

	// printf("s=%d\n", s);

	for (int i = s; i >= 0; --i)
		if (Eset_match(d, node_ip[i], bitmap[i])) return 1;

	// printf("%d-th prefix fail ip=%u\n",de,d);
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
		printf("%d : %d\n", (i + 1) * 100, NumCntClock[i]);
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////////
int arr[100] = {0};
void getheight(MSPT root)
{
	if (root == NULL) return;
	arr[root->height]++;
	getheight(root->left);
	getheight(root->right);
}

void shuffle(unsigned int *array, int n) {
	srand((unsigned)time(NULL));
	unsigned int temp;// = (unsigned int *)malloc(sizeof(unsigned int));

	for (int i = 0; i < n - 1; i++) {
		size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
		temp = array[j];
		array[j] = array[i];
		array[i] = temp;
	}
}

/*
void searchMSPT(MSPT root,unsigned int ip)
{
	while(root!=NULL) {
		printf("%u len:%d bitmap:%u\n", root->ip,root->len,root->bitmap);
		if(root->ip==ip) {
			printf("find\n");
			return;
		}
		else if(root->ip <ip)root=root->right;
		else root=root->left;
	}
}
*/
int total_eset = 0;
void count_eset_num(MSPT ptr)
{
	if (ptr == NULL) return;
	int i, pos;
	count_eset_num(ptr->left);
	count_eset_num(ptr->right);
	unsigned tmp_map;


	if (ptr->bitmap != 0) {
		tmp_map = ptr->bitmap;

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
	for (i = 1; i < 33; i++)
		len_mask[i] = ~((1 << (32 - i)) - 1);
}

int main(int argc, char **argv)
{
	int i, j;
	int count = 0;


	set_table(argv[1]);

	set_query(argv[2]);
	printf("num_entry:%d\n", num_entry);
	precompute_mask();
	create_MSPT();
	printf("num node: %d\n", num_node);
	for (i = 0; i < num_query; i++) clock1[i] = 10000000;

	if (argc == 4) {
		set_table(argv[3]);
		total = 0;
		for (i = 0; i < num_entry; ++i) {
			begin = rdtsc();
			MSPT_root = MSPT_insert(MSPT_root, table[i].ip, table[i].len, table[i].port);
			end = rdtsc();
			if (clock1[i] > (end - begin))
				clock1[i] = (end - begin);
			total += clock1[i];
		}
		printf("Avg. Update: %llu\n", total / num_entry);
		printf("num node: %d\n", num_node);
	}


	printf("num_query: %d\n", num_query);
	shuffle(query, num_query);
	for (i = 0; i < num_query; i++) clock1[i] = 10000000;
	for (j = 0; j < 100; j++) {
		for (i = 0; i < num_query; i++) {
			begin = rdtsc();
			count += search_MSPT(query[i], MSPT_root);
			end = rdtsc();
			if (clock1[i] > (end - begin))
				clock1[i] = (end - begin);
		}
	}
	total = 0;
	for (j = 0; j < num_query; j++)
		total += clock1[j];

	printf("success: %d\n", count);
	printf("Avg. Search: %llu\n", total / num_query);
	CountClock();
	printf("memory %f KB\n", ((double)num_node * 18 / 1024));

	count_eset_num(MSPT_root);
	printf("total_eset:%d\n", total_eset);

	return 0;
}
