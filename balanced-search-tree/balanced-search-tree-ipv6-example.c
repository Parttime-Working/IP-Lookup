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
	unsigned long long int ip[2];
	unsigned int len;
	unsigned long long int u_bound[2];
	unsigned int port;
	struct list *left, *right;
	int height;
};
typedef struct list node;
typedef node *BST;
typedef BST *forest;
forest Bforest;

int num_node = 0;
int max_level = -1;

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

///////////////////////////////////////

/*
int overlap(unsigned int L1,unsigned int U1,unsigned int L2,unsigned int U2);
void search(unsigned int ip, BST *root, int max_level);
void BST_del(char *p, BST root);
BST search_a_tree_for_enclosure(char *p,BST *root,int level);
void delete(char *p, BST *root, int max_level);
void insert(char *p, BST *root, int max_level);
void BST_delete(unsigned int ip,unsigned int len,BST root);
BST search_cover(unsigned int ip,unsigned int u_bound, BST root);
BST smallest_prefix(BST root);
BST largest_prefix(BST root);
*/
void CountClock();

void read_table(char *str, unsigned long long int *upper_ip, unsigned long long int *lower_ip, int *len,unsigned int *nexthop){
	char tok[]="/";
	char tok2[]=":";
	int count = 0;
	char buf[100],*str1;
	unsigned long long int n[8] = {0};
	char *cur = str;
	int check = 0;
	int skip_point = 0;
	int c = 0;
	while( *cur != '\0' ){
		if( *cur == ':' ){
			cur++;
			check = 1;
			c++;
		}
		if( *cur == ':' && check){
			skip_point = c;
		}
		else{
			check = 0;
			cur++;
		}
	}

	char *substr = strtok(str, tok);
	sprintf(buf,"%s\0",strtok(NULL,tok));
	*len = atoi(buf);
	substr = strtok(str, tok2);
	
     
    do{
    	sprintf(buf,"%s\0",substr);
    	n[count] = strtol(buf,NULL,16);
        count++;
        substr = strtok(NULL, tok2);
    }while (substr != NULL);

    
    unsigned long long int group[8] = {0};
    for( int i = 0; i < skip_point; i++ ){
    	group[i] = n[i];
    }
    for( int i = skip_point;  i + 8- count < 8; i++ ){
    	group[i+8-count] = n[i];
    	n[i] = 0;
    }

    for( int i = 0; i < 8; i++ ){
    	if( i < 4 ){  	
			*upper_ip <<= 16;
			*upper_ip += group[i];    		
    	}
    	else{
    		*lower_ip <<= 16;
    		*lower_ip += group[i];
    	}

    }
    *nexthop = 1;

}

void set_query(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		//read_table(string,&ip,&len,&nexthop);
		num_query++;
	}
	rewind(fp);
	query = (struct q_entry *)malloc(num_query * sizeof(struct q_entry));
	clock1 = (unsigned long long int *)malloc(num_query * sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&upper_ip,&lower_ip,&len,&nexthop);

		query[num_query].ip[0]=upper_ip;
		query[num_query].ip[1]=lower_ip;
		clock1[num_query++] = 10000000;
	}
}


void set_table(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		//read_table(string,&ip,&len,&nexthop);
		num_entry++;
	}
	rewind(fp);
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));

	num_entry=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&upper_ip,&lower_ip,&len,&nexthop);
		int shift=0;

		table[num_entry].ip[0]=upper_ip;
		table[num_entry].ip[1]=lower_ip;
	
		table[num_entry].port=1;
		table[num_entry++].len=len;
	}
}
BST create_node()
{
	BST temp;
	temp = (BST)malloc(sizeof(node));
	temp->ip[0] = 0;
	temp->ip[1] = 0;
	temp->len = 0;
	temp->u_bound[0] = 0;
	temp->u_bound[1] = 0;
	temp->right = NULL;
	temp->left = NULL;
	temp->port = 256;
	temp->height = 0;
	num_node++;
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

BST AVL_insert(BST root, unsigned long long int ip0, unsigned long long int ip1, unsigned int len, unsigned long long int u_bound0, unsigned long long int u_bound1, unsigned int port)
{

	if (root == NULL) {
		root = create_node();
		root->ip[0] = ip0;
		root->ip[1] = ip1;
		root->len = len;
		root->u_bound[0] = u_bound0;
		root->u_bound[1] = u_bound1;
		root->port = port;
	}
	else {
		if (ip0 > root->ip[0]) {
			root->right = AVL_insert(root->right, ip0, ip1, len, u_bound0, u_bound1, port);
			if (BF(root) == -2) {
				if (ip0 > root->right->ip[0]) {
					root = RR(root);
				}
				else if (ip0 < root->right->ip[0]) {
					root = RL(root);
				}
				else if (ip0 == root->right->ip[0]) {
					if (ip1 > root->right->ip[1]) root = RR(root);
					else root = RL(root);
				}
			}

		}
		else if (ip0 < root->ip[0]) {
			root->left = AVL_insert(root->left, ip0, ip1, len, u_bound0, u_bound1, port);
			if (BF(root) == 2) {
				if (ip0 < root->left->ip[0]) {
					root = LL(root);
				}
				else if (ip0 > root->left->ip[0]) {
					root = LR(root);
				}
				else if (ip0 == root->left->ip[0]) {
					if (ip1 > root->left->ip[1]) root = LR(root);
					else root = LL(root);
				}
			}
		}
		else if (ip0 == root->ip[0]) {
			if (ip1 > root->ip[1]) {
				root->right = AVL_insert(root->right, ip0, ip1, len, u_bound0, u_bound1, port);
				if (BF(root) == -2) {
					if (ip0 > root->right->ip[0]) {
						root = RR(root);
					}
					else if (ip0 < root->right->ip[0]) {
						root = RL(root);
					}
					else if (ip0 == root->right->ip[0]) {
						if (ip1 > root->right->ip[1]) root = RR(root);
						else root = RL(root);
					}
				}
			}
			else {
				root->left = AVL_insert(root->left, ip0, ip1, len, u_bound0, u_bound1, port);
				if (BF(root) == 2) {
					if (ip0 < root->left->ip[0]) {
						root = LL(root);
					}
					else if (ip0 > root->left->ip[0]) {
						root = LR(root);
					}
					else if (ip0 == root->left->ip[0]) {
						if (ip1 > root->left->ip[1]) root = LR(root);
						else root = LL(root);
					}
				}
			}
		}
	}
	root->height = height(root);
	return root;
}
// range-1 cover range-2
int cover(unsigned long long int L11, unsigned long long int L12, unsigned long long int U11, unsigned long long int U12, unsigned long long int L21, unsigned long long int L22, unsigned long long int U21, unsigned long long int U22)
{
	if ( U11 >= U21 && L11 <= L21) {
		if (U11 == U21 &&  L11 == L21) {
			if ( U12 >= U22 && L12 <= L22) return 1;
			else return 0;
		}
		else return 1;
	}
	else return 0;

}
void insert(unsigned long long int ip0, unsigned long long int ip1, unsigned int len, unsigned int nexthop, forest root, int s)
{
	BST x, q;
	unsigned long long int p_ip0 = ip0, p_ip1 = ip1, p_u_bound0, p_u_bound1;
	unsigned int p_len = len, p_port = nexthop;
	int lh, rh;
	int shift;
	int i;
	// unsigned long long int temp=1;
	if (len == 0) { p_u_bound0 = -1ULL; p_u_bound1 = -1ULL;}
	else {
		if (len <= 64) {
			shift = 64 - len;
			p_u_bound0 = ip0 + ((1ULL << shift) - 1);
			p_u_bound1 = -1ULL;
		}
		else {
			p_u_bound0 = ip0;
			shift = 128 - len;
			p_u_bound1 = ip1 + ((1ULL << shift) - 1);
		}
	}
	for (i = 0; i <= s; ++i) {
		x = root[i];

		while (x != NULL) {
			if ( (p_ip0 == x->ip[0]) && (p_ip1 == x->ip[1]) && (p_len == x->len)) return;
			if ( cover(x->ip[0], x->ip[1], x->u_bound[0], x->u_bound[1], p_ip0, p_ip1, p_u_bound0, p_u_bound1)) {
				q = create_node();
				num_node--;
				q->ip[0] = x->ip[0];
				q->ip[1] = x->ip[1];
				q->len = x->len;
				q->u_bound[0] = x->u_bound[0];
				q->u_bound[1] = x->u_bound[1];
				q->port = x->port;

				x->ip[0] = p_ip0;
				x->ip[1] = p_ip1;
				x->len = p_len;
				x->u_bound[0] = p_u_bound0;
				x->u_bound[1] = p_u_bound1;
				x->port = p_port;

				p_ip0 = q->ip[0];
				p_ip1 = q->ip[1];
				p_len = q->len;
				p_u_bound0 = q->u_bound[0];
				p_u_bound1 = q->u_bound[1];
				p_port = q->port;
				free(q);
				q = NULL;
				break;
			}
			if ( cover(p_ip0, p_ip1, p_u_bound0, p_u_bound1, x->ip[0], x->ip[1], x->u_bound[0], x->u_bound[1])) break;

			if ( p_ip0 > x->ip[0]) {
				if (x->right == NULL) {
					root[i] = AVL_insert(root[i], p_ip0, p_ip1, p_len, p_u_bound0, p_u_bound1, p_port);
					return;
				}
				else {

					x = x->right;
				}

			}
			else if (p_ip0 < x->ip[0]) {
				if (x->left == NULL) {
					root[i] = AVL_insert(root[i], p_ip0, p_ip1, p_len, p_u_bound0, p_u_bound1, p_port);
					return;
				}
				else {
					x = x->left;
				}
			}
			else {
				if ( p_ip1 > x->ip[1]) {
					if (x->right == NULL) {
						root[i] = AVL_insert(root[i], p_ip0, p_ip1, p_len, p_u_bound0, p_u_bound1, p_port);
						return;
					}
					else x = x->right;
				}
				else if (p_ip1 == x->ip[1]) return;
				else {
					if (x->left == NULL) {
						root[i] = AVL_insert(root[i], p_ip0, p_ip1, p_len, p_u_bound0, p_u_bound1, p_port);
						return;
					}
					else x = x->left;
				}
			}

		}
	}
	max_level = max_level + 1;

	root[max_level] = create_node();
	root[max_level]->ip[0] = p_ip0;
	root[max_level]->ip[1] = p_ip1;
	root[max_level]->height = 0;
	root[max_level]->len = p_len;
	root[max_level]->u_bound[0] = p_u_bound0;
	root[max_level]->u_bound[1] = p_u_bound1;
	root[max_level]->port = p_port;
	Bforest[max_level]=root[max_level];
	return;
}
void create()
{
	max_level = -1;
	Bforest = (BST *)malloc(20 * sizeof(BST));
	int _10percent_index = 82785;
	for (int i = 0; i < 20; ++i) Bforest[i] = NULL;

	for (int i = 0; i < num_entry; i++) clock1[i] = 10000000;
	// for (int i = 0; i < num_entry; ++i){
	for (int i = 0; i < 92582; ++i) {
		begin = rdtsc();
		insert(table[i].ip[0], table[i].ip[1], table[i].len, table[i].port, Bforest, max_level);
		end = rdtsc();
		if (clock1[i] > (end - begin))
			clock1[i] = (end - begin);
		// printf("rule %d success\n", i);
		// if(i==_10percent_index-1) printf("build node:%d\n", num_node);
	}
	total = 0;

	for (int j = _10percent_index; j < 92582; j++)
		total += clock1[j];

	printf("Avg. insert: %llu\n", total / (unsigned int)(92582 - _10percent_index));
	// printf("Avg. insert: %llu\n",total/(unsigned int)(num_entry*0.1));
	// CountClock();
}



int search(unsigned long long int ip0, unsigned long long int ip1, forest root, int s)
{
	int i;
	BST x;
	for (i = 0; i <= s; ++i) {
		x = root[i];
		while (x != NULL) {

			if(x->len<65){
				if(x->ip[0]<=ip0 && ip0<=x->u_bound[0]) return 1;
			}
			else{
				if(x->ip[0]==ip0){
					if(x->ip[1]<=ip1 && ip1<=x->u_bound[1]) return 1;
				}
				return 1;
			}

			if (ip0 < x->ip[0]) x = x->left;
			else if (ip0 > x->ip[0]) x = x->right;
			else {
				if (ip1 < x->ip[1]) x = x->left;
				else if (ip1 > x->ip[1]) x = x->right;
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
		printf("%d : %d\n", (i + 1) * 100, NumCntClock[i]);
	}
	return;
}
//////////////////////delete//////////////////////////////////////////////////////////////

BST search_cover(unsigned int long long ip0, unsigned int long long ip1, unsigned int long long u_bound0, unsigned int long long u_bound1, BST root)
{
	if (root == NULL) return NULL;
	if (cover(root->ip[0], root->ip[1], root->u_bound[0], root->u_bound[1], ip0, ip1, u_bound0, u_bound1)) return root;
	else if (ip0 > root->ip[0]) return search_cover(ip0, ip1, u_bound0, u_bound1, root->right);
	else if (ip0 < root->ip[0]) return search_cover(ip0, ip1, u_bound0, u_bound1, root->left);
	else {
		if (ip1 > root->ip[1]) return search_cover(ip0, ip1, u_bound0, u_bound1, root->right);
		else if (ip1 < root->ip[1]) return search_cover(ip0, ip1, u_bound0, u_bound1, root->left);
	}
}

BST smallest_prefix(BST root)
{
	if (root == NULL) return NULL;
	else if (root->left != NULL) return smallest_prefix(root->left);
	return root;
}

BST largest_prefix(BST root)
{
	if (root == NULL) return NULL;
	else if (root->right != NULL) return largest_prefix(root->right);
	return root;
}
BST BST_delete(unsigned int long long ip0, unsigned int long long ip1, unsigned int len, BST root)
{
	if (root == NULL) return NULL;
	if (ip0 > root->ip[0]) root->right = BST_delete(ip0, ip1, len, root->right);
	else if (ip0 < root->ip[0]) root->left = BST_delete(ip0, ip1, len, root->left);
	else {
		if (ip1 > root->ip[1]) root->right = BST_delete(ip0, ip1, len, root->right);
		else if (ip1 < root->ip[1]) root->left = BST_delete(ip0, ip1, len, root->left);
		else {
			if (root->left == NULL && root->right == NULL) {
				free(root);
				return NULL;
			}
			else if (root->left == NULL || root->right == NULL) {
				BST temp;
				if (root->left == NULL) temp = root->right;
				else temp = root->left;
				free(root);
				return temp;
			}
			else {
				BST temp = smallest_prefix(root->right);
				root->ip[0] = temp->ip[0];
				root->ip[1] = temp->ip[1];
				root->len = temp->len;
				root->u_bound[0] = temp->u_bound[0];
				root->u_bound[1] = temp->u_bound[1];
				root->port = temp->port;
				root->right = BST_delete(temp->ip[0], temp->ip[1], temp->len, root->right);
			}
		}
	}
	return root;
}
/////////////////////////////////////////////////////////////////////////////////////////


void BFdelete(unsigned int long long ip0, unsigned int long long ip1, unsigned int len, forest root, int s)
{
	BST x, y, z, temp;
	unsigned long long int p_ip0 = ip0, p_ip1 = ip1, p_u_bound0, p_u_bound1;
	unsigned int p_len = len;
	int shift;
	unsigned long long one = 1;
	if (len == 0) { p_u_bound0 = 0; p_u_bound1 = 0;}
	else {
		if (len <= 64) {
			shift = 64 - len;
			p_u_bound0 = ip0 + ((one << shift) - 1);
			p_u_bound1 = -1ULL;
		}
		else {
			p_u_bound0 = ip0;
			shift = 128 - len;
			p_u_bound1 = ip1 + ((one << shift) - 1);
		}
	}


	for (int i = 0; i <= s; ++i) {
		x = root[i];
		while (x != NULL) {
			if ( x->ip[0] == p_ip0 && x->ip[1] == p_ip1 && x->len == p_len) {
				if (i == s) {
					root[i] = BST_delete(p_ip0, p_ip1, p_len, root[i]);
					return;
				}
				temp = search_cover(p_ip0, p_ip1, p_u_bound0, p_u_bound1, root[i + 1]);
				if (temp == NULL) root[i] = BST_delete(p_ip0, p_ip1, p_len, root[i]);
				else {
					if (x->right != NULL) y = smallest_prefix(x->right);
					if (y != NULL && cover(temp->ip[0], temp->ip[1], temp->u_bound[0], temp->u_bound[1], y->ip[0], y->ip[1], y->u_bound[0], y->u_bound[1])) {
						root[i] = BST_delete(p_ip0, p_ip1, p_len, root[i]);
						return;
					}
					if (x->left != NULL) z = largest_prefix(x->left);
					if (z != NULL && cover(temp->ip[0], temp->ip[1], temp->u_bound[0], temp->u_bound[1], z->ip[0], z->u_bound[0], z->ip[1], z->u_bound[1])) {
						root[i] = BST_delete(p_ip0, p_ip1, p_len, root[i]);
						return;
					}

					x->ip[0] = temp->ip[0];
					x->ip[1] = temp->ip[1];
					x->len = temp->len;
					x->u_bound[0] = temp->u_bound[0];
					x->u_bound[1] = temp->u_bound[1];

					p_ip0 = temp->ip[0];
					p_ip1 = temp->ip[1];
					p_len = temp->len;
					p_u_bound0 = temp->u_bound[0];
					p_u_bound1 = temp->u_bound[1];
					break;
				}
			}
			if (cover(p_ip0, p_ip1, p_u_bound0, p_u_bound1, x->ip[0], x->ip[1], x->u_bound[0], x->u_bound[1])) break;
			if (cover(x->ip[0], x->ip[1], x->u_bound[0], x->u_bound[1] , p_ip0, p_ip1, p_u_bound0, p_u_bound1)) return;
			if (p_ip0 < x->ip[0]) {
				y = x;
				x = x->left;
			}
			else if (p_ip0 > x->ip[0]) {
				z = x;
				x = x->right;
			}
			else {
				if (p_ip1 < x->ip[1]) {
					y = x;
					x = x->left;
				}
				else {
					z = x;
					x = x->right;
				}
			}
		}
		if (i < s) continue;
		return;
	}

}
////////////////////////////////////////////////////////////////////////////////////
int countnode(BST root)
{
	if (root == NULL) return 0;
	return 1 + countnode(root->left) + countnode(root->right);
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
////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	int i,j;
	set_table(argv[1]);
	set_query(argv[2]);
	// num_entry=num_entry-1;
	// printf("%d \n", num_entry);
	// set_table("test.txt");

	create();

	printf("max_level=%d\n", max_level);

	int count;
	count = 0;
	for (i = 0; i <= max_level; ++i)
	{
		count = countnode(Bforest[i]);
		printf("level %d = %d\n", i, count );
	}

	// printf("num_query:%d\n", num_query);

	shuffle(query, num_query);

	printf("-----------------\n" );
	printf("%llu %llu\n", query[0].ip[0],query[0].ip[1]);
	int suc = 0;
	for (i = 0; i < num_query; i++) clock1[i] = 10000000;
	for (j = 0; j < 1; j++) {
		for (i = 0; i < num_query; ++i) {
			begin = rdtsc();
			suc += search(query[i].ip[0], query[i].ip[1], Bforest, max_level);
			end = rdtsc();
			if (clock1[i] > (end - begin))
				clock1[i] = (end - begin);
		}
	}
	total = 0;
	for (i = 0; i < num_entry; i++)
		total += clock1[i];


	printf("Avg. Search: %llu\n", total / num_entry);
	CountClock();
	printf("success %d\n", suc);

	/* success test
		count+=search(1677721600, Bforest, max_level);
		count+=search(1677787136, Bforest, max_level);
	*/
	/* 	fail test
		count+=search(167772160, Bforest, max_level);
		count+=search(83886080, Bforest, max_level);
	*/
	/*
		for (int i = 0; i < num_entry; ++i){
			count+=search(table[i].ip, Bforest, max_level);
		}
		count=0;
		for (int i = 0; i < num_entry; ++i){
			count+=search(table[i].ip, Bforest, max_level);
		}
		printf("count=%d\n", count);
	*/
	/*
		for(int j=0;j<10;j++){
			for(int i=0;i<num_entry;i++){
				begin=rdtsc();
				search(table[i].ip[0], table[i].ip[1], table[i].ip[2], table[i].ip[3], Bforest, max_level);
				end=rdtsc();
				if(clock[i]>(end-begin))
					clock[i]=(end-begin);

			}
		}
		total=0;

		for(int j=0;j<num_entry;j++)
			total+=clock[j];

		printf("Avg. Search: %llu\n",total/num_entry);
		CountClock();
	*/
	/*
		for(int i=0;i<num_entry;i++) clock1[i]=10000000;
		for(int i=0;i<num_entry;i++){
			begin=rdtsc();
			BFdelete(table[i].ip[0], table[i].ip[1], table[i].len, Bforest, max_level);
			end=rdtsc();
			if(clock1[i]>(end-begin))
				clock1[i]=(end-begin);
		}
		total=0;

		for(int j=0;j<num_entry;j++)
			total+=clock1[j];

		printf("Avg. delete: %llu\n",total/num_entry);
		// CountClock();
		int level=max_level;
		for (int i = level; i >=0; --i)
		{
			printf("level %d # of node= %d\n", i, countnode(Bforest[i]));
		}
	*/
	double total_m = num_entry * 26 / 1024;
	printf("memory %f KB\n",  total_m);
	return 0;
}
