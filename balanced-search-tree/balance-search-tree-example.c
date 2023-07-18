#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
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
struct list {
	unsigned int ip;
	unsigned int len;
	unsigned int u_bound;
	unsigned int port;
	struct list *left, *right;
	int height;
};
typedef struct list node;
typedef node *BST;
typedef BST *forest;
int num_BFnode = 0;
forest Bforest;
// int max_level;

struct ENTRY {
	unsigned int ip;
	unsigned char len;
	unsigned int port;
};
struct ENTRY *table;
int num_entry;

//////////////// segment table ////////////////
typedef struct element
{
	unsigned int port;
	forest ptr;
	int max_level;
} seg;
typedef seg *segment_table;
segment_table seg_table;

void CountClock()
{
	unsigned int i;
	unsigned int* NumCntClock = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for (i = 0; i < 50; i++) NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for (i = 0; i < num_entry; i++)
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
////////////////////////////////////////////////////////////////////////////////////
int countnode(BST root)
{
	if (root == NULL) return 0;
	return 1 + countnode(root->left) + countnode(root->right);
}

BST create_node()
{
	BST temp;
	temp = (BST)malloc(sizeof(node));
	temp->ip = 0;
	temp->len = 0;
	temp->u_bound = 0;
	temp->right = NULL;
	temp->left = NULL;
	temp->port = 256;
	temp->height = 0;
	num_BFnode = num_BFnode + 1;
	//temp->layer=0;
	return temp;
}
int height(BST root)
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
int BF(BST root)
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
BST rotateright(BST x)
{
	BST y;
	y = x->left;
	x->left = y->right;
	y->right = x;
	x->height = height(x);
	y->height = height(y);
	return y;
}

BST rotateleft(BST x)
{
	BST y;
	y = x->right;
	x->right = y->left;
	y->left = x;
	x->height = height(x);
	y->height = height(y);

	return y;
}

BST RR(BST root)
{
	root = rotateleft(root);
	return root;
}

BST LL(BST root)
{
	root = rotateright(root);
	return root;
}

BST LR(BST root)
{
	root->left = rotateleft(root->left);
	root = rotateright(root);

	return root;
}

BST RL(BST root)
{
	root->right = rotateright(root->right);
	root = rotateleft(root);
	return root;
}

BST AVL_insert(BST root, unsigned int ip, unsigned int len, unsigned int u_bound, unsigned int port)
{

	if (root == NULL) {
		root = create_node();
		root->ip = ip;
		root->len = len;
		root->u_bound = u_bound;
		root->port = port;

	}
	else {

		if (ip > root->ip) {
			root->right = AVL_insert(root->right, ip, len, u_bound, port);
			if (BF(root) == -2)
				if (ip > root->right->ip)
					root = RR(root);
				else
					root = RL(root);

		}
		else if (ip < root->ip) {
			root->left = AVL_insert(root->left, ip, len, u_bound, port);
			if (BF(root) == 2)
				if (ip < root->left->ip)
					root = LL(root);
				else
					root = LR(root);

		}
	}
	root->height = height(root);
	return root;
}

int cover(unsigned int L1, unsigned int U1, unsigned int L2, unsigned int U2)
{
	if ( (L1 <= L2) && (U2 <= U1)) return 1;
	else return 0;
}
void insert(unsigned int ip, unsigned int len, unsigned int nexthop, forest root, int *s)
{
	BST x, q;
	unsigned int p_ip = ip, p_u_bound;
	unsigned int p_len = len, p_port = nexthop;
	int lh, rh;

	if (len == 0) p_u_bound = 0;
	else p_u_bound = ip + ((1 << (32 - len)) - 1);
	for (int i = 0; i <= *s; ++i) {
		x = root[i];

		while (x != NULL) {
			if ( (p_ip == x->ip) && (p_len == x->len)) return;
			if ( cover(x->ip, x->u_bound, p_ip, p_u_bound)) {
				q = create_node();
				num_BFnode = num_BFnode - 1;
				q->ip = x->ip;
				q->len = x->len;
				q->u_bound = x->u_bound;
				q->port = x->port;

				x->ip = p_ip;
				x->len = p_len;
				x->u_bound = p_u_bound;
				x->port = p_port;

				p_ip = q->ip;
				p_len = q->len;
				p_u_bound = q->u_bound;
				p_port = q->port;
				free(q);
				q = NULL;
				break;
			}
			if ( cover( p_ip, p_u_bound, x->ip, x->u_bound)) break;
			if ( p_ip > x->ip) {
				if (x->right == NULL) {

					root[i] = AVL_insert(root[i], p_ip, p_len, p_u_bound, p_port);
					return;
				}
				else {

					x = x->right;
				}
			}
			else {
				if (x->left == NULL) {

					root[i] = AVL_insert(root[i], p_ip, p_len, p_u_bound, p_port);
					return;
				}
				else {

					x = x->left;
				}
			}

		}
	}
	*s = (*s) + 1;

	root[*s] = create_node();
	root[*s]->ip = p_ip;
	root[*s]->height = 0;
	root[*s]->len = p_len;
	root[*s]->u_bound = p_u_bound;
	root[*s]->port = p_port;

	return;
}


void build_BF16()
{
	int i, j;
	int index_r1, index_r2;
	int build_node = 0;
	int _10percent_index = 997952;
	for (i = 0; i < num_entry; i++) clock1[i] = 10000000;

	for (i = 0; i < num_entry; ++i) {
		begin = rdtsc();
		if (table[i].len < 16) {

			index_r1 = table[i].ip >> 16;
			index_r2 = index_r1 + (1 << (16 - table[i].len) - 1) ;
			while ( index_r1 <= index_r2) seg_table[index_r1++].port = table[i].port;

		}
		else if (table[i].len == 16) seg_table[(table[i].ip >> 16)].port = table[i].port;
		else {

			if (seg_table[(table[i].ip >> 16)].ptr == NULL) {
				seg_table[(table[i].ip >> 16)].ptr = (forest) malloc(20 * sizeof(BST));
				for (j = 0; j < 20; ++j) seg_table[(table[i].ip >> 16)].ptr[j] = NULL;
			}

			insert(table[i].ip, table[i].len, table[i].port, seg_table[(table[i].ip >> 16)].ptr, &seg_table[(table[i].ip >> 16)].max_level);
		}

		end = rdtsc();
		if (clock1[i] > (end - begin)) clock1[i] = (end - begin);
	}

	total = 0;

	for (j = _10percent_index; j < num_entry; j++)
		total += clock1[j];
	printf("Avg. insert: %llu\n", total / (num_entry - _10percent_index));
	// CountClock();
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

	clock1 = (unsigned long long int *)malloc(num_entry * sizeof(unsigned long long int));

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
		table[num_entry].len = len;
		clock1[num_entry++] = 10000000;

	}

	fclose(fp);
}

int search(unsigned int ip, forest root, int s)
{
	BST x;
	for (int i = 0; i <= s; ++i)
	{
		x = root[i];
		while (x != NULL) {
			if (cover(x->ip, x->u_bound, ip, ip)) return 1;
			else {
				if (ip < x->ip) x = x->left;
				else x = x->right;
			}
		}
	}


	if (seg_table[(ip >> 16)].port != 256) return 1;


	return 0;
}


//////////////////////delete//////////////////////////////////////////////////////////////
BST search_cover(unsigned int ip, unsigned int u_bound, BST root)
{

	if (root == NULL) return NULL;
	if (cover(root->ip, root->u_bound, ip, u_bound)) return root;
	else {
		if (ip > root->ip) return search_cover(ip, u_bound, root->right);
		else if (ip < root->ip) return search_cover(ip, u_bound, root->left);

	}
}
BST smallest_prefix(BST root)
{
	while (root->left != NULL) {
		root = root->left;
	}
	return root;
}
BST largest_prefix(BST root)
{
	while (root->right != NULL) {
		root = root->right;
	}
	return root;
}
BST BST_delete(unsigned int ip, unsigned int len, BST root)
{	//printf("del_ip=%u del_len=%u\n", root->ip,root->len);
	if (root == NULL) return NULL;
	if (ip > root->ip) root->right = BST_delete(ip, len, root->right);
	else if (ip < root->ip) root->left = BST_delete(ip, len, root->left);
	else {
		if (root->left == NULL && root->right == NULL) { //printf("case3\n");
			free(root);
			return NULL;
		}
		else if (root->left == NULL || root->right == NULL) {
			// printf("case1\n");
			BST temp;
			if (root->left == NULL) temp = root->right;
			else temp = root->left;
			free(root);
			return temp;
		}
		else {//printf("case2\n");
			BST temp = smallest_prefix(root->right);
			root->ip = temp->ip;
			root->len = temp->len;
			root->u_bound = temp->u_bound;
			root->port = temp->port;
			root->right = BST_delete(temp->ip, temp->len, root->right);
		}
	}
	return root;
}



void BFdelete(unsigned int ip, unsigned int len, forest root, int s)
{
	BST x, y, z, temp;
	unsigned int p_ip = ip, p_u_bound;
	unsigned int p_len = len;
	if (len == 0) p_u_bound = 0;
	else p_u_bound = ip + ((1 << (32 - len)) - 1);

	for (int i = 0; i <= s; ++i) {
		x = root[i];
		while (x != NULL) {
			if ( x->ip == p_ip && x->len == p_len) {
				if (i == s) {
					root[i] = BST_delete(p_ip, p_len, root[i]);
					return;
				}
				temp = search_cover(p_ip, p_u_bound, root[i + 1]);

				if (temp == NULL) {
					root[i] = BST_delete(p_ip, p_len, root[i]);
					return;
				}
				else {
					if (x->right != NULL) y = smallest_prefix(x->right);
					if (y != NULL && cover(temp->ip, temp->u_bound, y->ip, y->u_bound)) {
						root[i] = BST_delete(p_ip, p_len, root[i]);
						return;
					}
					if (x->left != NULL) z = largest_prefix(x->left);
					if (z != NULL && cover(temp->ip, temp->u_bound, z->ip, z->u_bound)) {
						root[i] = BST_delete(p_ip, p_len, root[i]);
						return;
					}

					x->ip = temp->ip;
					x->len = temp->len;
					x->u_bound = temp->u_bound;

					p_ip = temp->ip;
					p_len = temp->len;
					p_u_bound = temp->u_bound;
					break;
				}
			}
			if (cover(p_ip, p_u_bound, x->ip, x->u_bound)) break;
			if (cover(x->ip, x->u_bound, p_ip, p_u_bound)) return;
			if (p_ip < x->ip) {
				y = x;
				x = x->left;
			}
			else {
				z = x;
				x = x->right;
			}
		}
		if (i < s) continue;
		return;
	}

}
////////////////////////////////////////////////////////////////////////////////////


void shuffle(struct ENTRY *array, int n) {
	srand((unsigned)time(NULL));
	struct ENTRY *temp = (struct ENTRY *)malloc(sizeof(struct ENTRY));

	for (int i = 0; i < n - 1; i++) {
		size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
		temp->ip = array[j].ip;
		temp->len = array[j].len;
		temp->port = array[j].port;
		array[j].ip = array[i].ip;
		array[j].len = array[i].len;
		array[j].port = array[i].port;
		array[i].ip = temp->ip;
		array[i].len = temp->len;
		array[i].port = temp->port;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void init_seg_table()
{
	seg_table = (segment_table)malloc(65536 * sizeof(seg));
	for (int i = 0; i < 65536; ++i) {
		seg_table[i].max_level = -1;
		seg_table[i].port = 256;
		seg_table[i].ptr = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	int i, j;
	set_table(argv[1]);

	init_seg_table();
	build_BF16();
	set_table(argv[2]);
	// create();
	// printf("max_level=%d\n", max_level);


//////////////// search ////////////////
	int count = 0;
	shuffle(table, num_entry);
	for (i = 0; i < num_entry; i++) clock1[i] = 10000000;
	for (j = 0; j < 100; j++) {
		for (i = 0; i < num_entry; i++) {
			begin = rdtsc();
			count += search(table[i].ip, seg_table[(table[i].ip >> 16)].ptr, seg_table[(table[i].ip >> 16)].max_level);
			end = rdtsc();
			if (clock1[i] > (end - begin))
				clock1[i] = (end - begin);

		}
	}
	total = 0;
	for (j = 0; j < num_entry; j++)
		total += clock1[j];

	printf("Avg. Search: %llu\n", total / num_entry);
	printf("success : %d\n", count);
	CountClock();
	printf("# of nodes: %d\n", num_BFnode);
	printf("memory: %llu\n", ((65536 * 6) + num_BFnode * 14) / 1024);

	int level_node_count[20]={0};
	for (i = 0; i <= 65535; ++i) {
		if (seg_table[i].ptr != NULL) {
			for (j = 0; j <= seg_table[i].max_level; ++j)
				level_node_count[j] += countnode(seg_table[i].ptr[j]);

		}
	}
	for (i = 0; i < 20; i++) {
		printf("level %d:\t%d\n", i, level_node_count[i]);
	}

	//printf("level %d: %d\n", i,countnode(Bforest[i]));
////////////////delete////////////////
	/*
		for(int i=0;i<num_entry;i++) clock1[i]=10000000;
		for(int i=0;i<num_entry;i++){
			begin=rdtsc();
			BFdelete(table[i].ip, table[i].len, Bforest, max_level);
			end=rdtsc();
			if(clock1[i]>(end-begin))
				clock1[i]=(end-begin);
		}
		total=0;
		for(int j=0;j<num_entry;j++)
			total+=clock1[j];

		printf("Avg. delete: %llu\n",total/num_entry);
		CountClock();
	////////////////////////////////
		double total_m=num_entry*14/1024;
		printf("num node =%d \n", num_entry);
		printf("memory %f KB\n",  total_m);


		// for (int i = 0; i <=max_level; ++i) printf("level %d: %d\n", i,countnode(Bforest[i]));

	*/
///////////////////////////////////////////	
	return 0;
}
