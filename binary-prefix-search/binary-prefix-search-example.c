#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<time.h>
////////////////////////////////////////////////////////////////////////////////////
struct ENTRY {
	unsigned int ip;
	unsigned char len;
	unsigned char port;
};
////////////////////////////////////////////////////////////////////////////////////
inline unsigned long long int rdtsc()
{
	unsigned long long int x;
	asm   volatile ("rdtsc" : "=A" (x));
	return x;
}
////////////////////////////////////////////////////////////////////////////////////
struct list { //structure of binary trie
	unsigned int port;
	int real;
	int level;
	int length;
	unsigned int ipcount;
	struct list *left, *right, *pre;
};

typedef struct {
	unsigned int pre;
	unsigned int out_port;
	int prefix_length;
} arry;

unsigned int prefix_mask_array[33];

typedef struct list node;
typedef node *btrie;

unsigned long long int one = 1;

////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
btrie segRoot[65536];
unsigned int *query;
int split = 0;
int num_entry = 0;
int num_query = 0;
int num_query2 = 0;
struct ENTRY *table;
struct ENTRY *table_search;
int N = 0; //number of nodes
unsigned long long int begin, end, total_search, total = 0;
unsigned long long int *clock1;
int num_node = 0; //total number of nodes in the binary trie
bool check;
int maxnumber[65536] = {0};
arry **prefix_array;
////////////////////////////////////////////////////////////////////////////////////
btrie create_node(btrie prenode, int nowlevel, unsigned int value) {
	btrie temp;
	num_node++;
	temp = (btrie)malloc(sizeof(node));
	temp->right = NULL;
	temp->left = NULL;
	temp->pre = prenode;
	temp->port = 256; //default port
	temp->real = 1;
	temp->level = nowlevel;
	temp->ipcount = value;
	return temp;
}
btrie create_node_have_nexthop(unsigned int ip, btrie prenode, int nowlevel, unsigned int value, int length) {
	btrie temp;
	num_node++;
	temp = (btrie)malloc(sizeof(node));
	temp->right = NULL;
	temp->left = NULL;
	temp->pre = prenode;
	temp->port = 2; 
	temp->real = 0;
	temp->level = nowlevel;
	temp->ipcount = value;
	temp->length = length;
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned int ip, unsigned char len, unsigned char nexthop) {
	btrie ptr = root;
	int i;
	unsigned int control_ipcount = 0;
	for (i = 0; i < len; i++) {
		if (ip & (1 << (31 - i))) {
			control_ipcount += (one << (31 - i));


			if (ptr->right == NULL)
				ptr->right = create_node(ptr, ptr->level + 1, control_ipcount); // Create Node
			ptr = ptr->right;
			if ((i == len - 1) && (ptr->port == 256)) {
				ptr->length = len;
				ptr->port = nexthop;
			}
		}
		else {
			if (ptr->left == NULL)
				ptr->left = create_node(ptr, ptr->level + 1, control_ipcount);
			ptr = ptr->left;
			if ((i == len - 1) && (ptr->port == 256)) {
				ptr->length = len;
				ptr->port = nexthop;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////
void read_table(char *str, unsigned int *ip, int *len, unsigned int *nexthop) {
	char tok[] = "./";
	char buf[100], *str1;
	unsigned int n[4];
	sprintf(buf, "%s\0", strtok(str, tok));
	n[0] = atoi(buf);
	sprintf(buf, "%s\0", strtok(NULL, tok));
	n[1] = atoi(buf);
	sprintf(buf, "%s\0", strtok(NULL, tok));
	n[2] = atoi(buf);
	sprintf(buf, "%s\0", strtok(NULL, tok));
	n[3] = atoi(buf);
	*nexthop = n[2];
	str1 = (char *)strtok(NULL, tok);
	if (str1 != NULL) {
		sprintf(buf, "%s\0", str1);
		*len = atoi(buf);
	}
	else {
		if (n[1] == 0 && n[2] == 0 && n[3] == 0)
			*len = 8;
		else if (n[2] == 0 && n[3] == 0)
			*len = 16;
		else if (n[3] == 0)
			*len = 24;
	}
	*ip = n[0];
	*ip <<= 8;
	*ip += n[1];
	*ip <<= 8;
	*ip += n[2];
	*ip <<= 8;
	*ip += n[3];
}
////////////////////////////////////////////////////////////////////////////////////
void search(unsigned int ip) {
	int j;
	btrie current = root, temp = NULL;
	for (j = 31; j >= (-1); j--) {
		if (current == NULL)
			break;
		if (current->port != 256)
			temp = current;
		if (ip & (1 << j)) {
			current = current->right;
		}
		else {
			current = current->left;
		}
	}
	/*if(temp==NULL)
	  printf("default\n");
	  else
	  printf("%u\n",temp->port);*/
}
////////////////////////////////////////////////////////////////////////////////////
void set_table(char *file_name) {
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip, nexthop;
	fp = fopen(file_name, "r");
	while (fgets(string, 50, fp) != NULL) {
		read_table(string, &ip, &len, &nexthop);
		num_entry++;
	}
	rewind(fp);
	table = (struct ENTRY *)malloc(num_entry * sizeof(struct ENTRY));
	num_entry = 0;
	while (fgets(string, 50, fp) != NULL) {
		read_table(string, &ip, &len, &nexthop);
		table[num_entry].ip = ip;
		table[num_entry].port = nexthop;
		table[num_entry++].len = len;
	}
}

void set_table2(char *file_name) {
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip, nexthop;
	fp = fopen(file_name, "r");
	while (fgets(string, 50, fp) != NULL) {
		read_table(string, &ip, &len, &nexthop);
		num_query2++;
	}
	rewind(fp);
	table_search = (struct ENTRY *)malloc(num_query2 * sizeof(struct ENTRY));
	num_query2 = 0;
	while (fgets(string, 50, fp) != NULL) {
		read_table(string, &ip, &len, &nexthop);
		table_search[num_query2].ip = ip;
		table_search[num_query2].port = nexthop;
		table_search[num_query2++].len = len;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_query(char *file_name) {
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip, nexthop;
	fp = fopen(file_name, "r");
	while (fgets(string, 50, fp) != NULL) {
		read_table(string, &ip, &len, &nexthop);
		num_query++;
	}
	rewind(fp);
	query = (unsigned int *)malloc(num_query * sizeof(unsigned int));
	clock1 = (unsigned long long int *)malloc(num_query * sizeof(unsigned long long int));
	num_query = 0;
	while (fgets(string, 50, fp) != NULL) {
		read_table(string, &ip, &len, &nexthop);
		query[num_query] = ip;
		clock1[num_query++] = 10000000;
	}
}


////////////////////////////////////////////////////////////////////////////////////
void create() {
	int i;
	root = create_node(NULL, 0, 0);
	begin = rdtsc();
	for (i = 0; i < num_query; i++)
		add_node(table[i].ip, table[i].len, table[i].port);
	end = rdtsc();
}
////////////////////////////////////////////////////////////////////////////////////
void count_node(btrie r) {
	if (r == NULL)
		return;
	count_node(r->left);
	N++;
	count_node(r->right);
}
////////////////////////////////////////////////////////////////////////////////////
void Countclock1()
{
	unsigned int i;
	unsigned int* NumCntclock1 = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for (i = 0; i < 50; i++) NumCntclock1[i] = 0;
	unsigned long long Minclock1 = 10000000, Maxclock1 = 0;
	for (i = 0; i < num_query; i++)
	{
		if (clock1[i] > Maxclock1) Maxclock1 = clock1[i];
		if (clock1[i] < Minclock1) Minclock1 = clock1[i];
		if (clock1[i] / 100 < 50) NumCntclock1[clock1[i] / 100]++;
		else NumCntclock1[49]++;
	}
	printf("(Maxclock1, Minclock1) = (%5llu, %5llu)\n", Maxclock1, Minclock1);

	for (i = 0; i < 50; i++)
	{
		printf(" %d\n",  NumCntclock1[i]);
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////////
void complete_binary_trie(btrie CurrentNode) {
	unsigned int control_ipcount = 0;
	if (CurrentNode == NULL)return;


	if (CurrentNode->port != 256) {

		int tmport = CurrentNode->port;
		if (CurrentNode->left == NULL && CurrentNode->right != NULL ) {
			CurrentNode->left = create_node_have_nexthop(CurrentNode->port, CurrentNode, CurrentNode->level + 1, CurrentNode->ipcount,  CurrentNode->length);

		}
		else if (CurrentNode->left != NULL && CurrentNode->right == NULL ) {
			control_ipcount += (one << (63 - CurrentNode->level));
			CurrentNode->right = create_node_have_nexthop(CurrentNode->port, CurrentNode, CurrentNode->level + 1, control_ipcount + CurrentNode->ipcount, CurrentNode->length + 1);
			CurrentNode->port = 256;
		}
		if (CurrentNode->left != NULL && CurrentNode->right != NULL) {

			/*if (CurrentNode->left->port == 256) {
				CurrentNode->left->port = tmport;
				CurrentNode->left->length = CurrentNode->length;
				CurrentNode->port = 256;
			}
			if (CurrentNode->right->port == 256) {
				CurrentNode->right->port = tmport;
				CurrentNode->right->length = CurrentNode->length+1;
				CurrentNode->port = 256;
			}
			*/
		}
	}
	complete_binary_trie(CurrentNode->left);
	complete_binary_trie(CurrentNode->right);

}
void merge(btrie CurrentNode, int aux, btrie keep_Node) {
	unsigned long long int tmp = 0;

	if (CurrentNode == NULL)return;
	merge(CurrentNode->left, aux, keep_Node);

	if (CurrentNode->real == 0) {
		if (aux == 1) {
			btrie A = CurrentNode;
			int done = 0;
			while (A->pre != NULL && !done ) {
				A = A->pre;
				btrie B = keep_Node;
				while (B->pre != NULL) {
					if (B == A) {
						A->port = CurrentNode->port;
						A->length = CurrentNode->length;
						keep_Node->port = 256;
						CurrentNode->port = 256;
						done = 1;
						break;
					}
					B = B->pre;
				}

			}

		} else {
			aux = 1;
			keep_Node = CurrentNode;
		}
	} else {
		aux = 0;
	}
	merge(CurrentNode->right, aux, keep_Node);


}
/*void rotate(btrie CurrentNode) {
	unsigned int control_ipcount = 0;
	if (CurrentNode == NULL) return ;
	if (CurrentNode->left != NULL) {

		rotate(CurrentNode->left);

	}

	if (CurrentNode->right != NULL) {

		rotate(CurrentNode->right);
	}
	if (CurrentNode->level == 31 && CurrentNode->port == 256 ) {
		if (CurrentNode->left != NULL && CurrentNode->right != NULL) {
			if (CurrentNode->left->port != 256 && CurrentNode->right->port != 256) {
				CurrentNode->port = CurrentNode->left->port;
			}
		}
		else if (CurrentNode->left != NULL && CurrentNode->right == NULL) {
			btrie temp = CurrentNode;
			while (temp->port != 256)
				temp = temp->pre;
			if ((31 - CurrentNode->level) != 0)
				control_ipcount += pow(2, 31 - CurrentNode->level);
			else
				control_ipcount += 1;

			CurrentNode->right = create_node_have_nexthop(temp->port, CurrentNode, CurrentNode->level + 1, control_ipcount + CurrentNode->ipcount);
			num_node++;
			CurrentNode->port = CurrentNode->left->port;
		}
		else if (CurrentNode->left == NULL && CurrentNode->right != NULL) {
			btrie temp = CurrentNode;
			while (temp->port != 256)
				temp = temp->pre;
			CurrentNode->port = temp->port;
		}
	}
	else if (CurrentNode->level == 31 && CurrentNode->port != 256) {
		if (CurrentNode->left != NULL && CurrentNode->right == NULL) {
			if ((31 - CurrentNode->level) != 0)
				control_ipcount += pow(2, 31 - CurrentNode->level);
			else
				control_ipcount += 1;

			CurrentNode->right = create_node_have_nexthop(CurrentNode->port, CurrentNode, CurrentNode->level + 1, control_ipcount + CurrentNode->ipcount);
			num_node++;
			CurrentNode->port = CurrentNode->left->port;
		}
	}

}*/
int c = 0;
void binarysearch(unsigned int search, int index) {

	int low = 0;
	int high = maxnumber[index] - 1;
	int mid = 0;
	int out_port = 256;
	while (low <= high)
	{

		mid = (low + high) >> 1;
		unsigned int tmp_search = search;
		tmp_search = tmp_search & prefix_mask_array[prefix_array[index][mid].prefix_length];

		if (prefix_array[index][mid].pre == tmp_search )// =
		{
			out_port = prefix_array[index][mid].out_port;
			if (search & (one << (31 - prefix_array[index][mid].prefix_length ))) {
				low = mid + 1;
			}
			else {
				high = mid - 1;
			}
		}
		else if (prefix_array[index][mid].pre > search)//>
		{
			high = mid - 1;
		}
		else if (prefix_array[index][mid].pre < search) // <
		{
			low = mid + 1;
		}
	}


}
int segNumber = 0;
void Bulid_prefix_array(btrie CurrentNode, int index) {
	unsigned int control_ipcount = 0;
	if (CurrentNode == NULL)return ;
	if (CurrentNode->left != NULL) {
		Bulid_prefix_array(CurrentNode->left, index);
	}
	if (CurrentNode->port != 256 ) {
		prefix_array[index][maxnumber[index]].pre = CurrentNode->ipcount;
		prefix_array[index][maxnumber[index]].out_port = CurrentNode->port;
		prefix_array[index][maxnumber[index]].prefix_length = CurrentNode->length;
		maxnumber[index]++;
		segNumber++;
	}

	if (CurrentNode->right != NULL) {
		Bulid_prefix_array(CurrentNode->right, index);
	}
}

int smallthan16_prefixnode = 0;
int eqiu16_prefixnode = 0;
int nonEmpty_seg = 0;

void makeSeg(btrie tmp) {
	if (tmp == NULL) return ;
	else {
		if (tmp->level == 16) {
			if (tmp->port != 256  )
				eqiu16_prefixnode++;
			if (tmp->left != NULL || tmp->right != NULL)	{
				nonEmpty_seg++;
				segRoot[tmp->ipcount >> 16] = tmp;
				segRoot[tmp->ipcount >> 16]->port = 256;
				segRoot[tmp->ipcount >> 16]->pre = NULL;
			}
		}
		else if (tmp->port != 256 && tmp->level < 16)
			smallthan16_prefixnode++;
	}
	makeSeg(tmp->left);
	makeSeg(tmp->right);
}

void shuffle2(struct ENTRY *array, int n) {
	srand(time(NULL));
	if (n > 1) {
		for (int i = 0; i < n; i++) {
			size_t j = rand() / (RAND_MAX / (n));
			unsigned int t = array[j].ip;
			int t1 = array[j].len;
			int t2 = array[j].port;
			array[j].ip = array[i].ip;
			array[j].len = array[i].len;
			array[j].port = array[i].port;
			array[i].ip = t;
			array[i].len = t1;
			array[i].port = t2;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

	prefix_mask_array[0] = 0;
	for (int i = 1; i < 33; i++) {
		prefix_mask_array[i] = 0;
		prefix_mask_array[i] = prefix_mask_array[i - 1] + (one << (32 - i ));

	}

	int i, j;
	set_query(argv[1]);
	set_table(argv[1]);
	set_table2(argv[2]);
	create();
	makeSeg(root);
	printf("nonEmpty_seg:%d\n", nonEmpty_seg );
	printf("=16_prefix_node:%d\n", eqiu16_prefixnode );
	printf("<16_prefix_node:%d\n", smallthan16_prefixnode );
	printf("Seg_finish\n");
	for (int i = 0; i < 65536; i++)
		complete_binary_trie(segRoot[i]);
	//printf("node_count:%d\n", num_node );

	/*for (int i = 0; i < 65536; i++)
		merge(segRoot[i], 0, segRoot[i]);
	printf("merge_finish\n");*/

	prefix_array = ( arry **)malloc(65536 * sizeof(arry*));
	for (int i = 0; i < 65536; i++)
		prefix_array[i] = ( arry *)malloc(num_node * sizeof(arry));
	for (int i = 0; i < 65536; i++)
		Bulid_prefix_array(segRoot[i], i);

	printf("totalmemory:%llu\n", (segNumber * 6 + 65536 * 6) / 1024 );
	printf("array_finish\n");
	printf("prefix_array_count:%d\n", segNumber );


	shuffle2(table, num_query);
	shuffle2(table_search, num_query2);

	for (j = 0; j < 30; j++) {
		for (i = 0; i < num_query2; i++) {
			int index = table_search[i].ip >> 16;
			begin = rdtsc();
			binarysearch( table_search[i].ip, index);
			end = rdtsc();
			if (clock1[i] > (end - begin))
				clock1[i] = (end - begin);
		}
	}

	total = 0;

	for (j = 0; j < num_query2; j++) {
		total += clock1[j];
	}
	printf("Avg. Search: %llu\n", total / num_query2);
	Countclock1();


	////////////////////////////////////////////////////////////////////////////

	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
