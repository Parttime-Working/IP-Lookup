#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
////////////////////////////////////////////////////////////////////////////////////
struct ENTRY
{
	unsigned int ip;
	unsigned char len;
	unsigned char port;
};
////////////////////////////////////////////////////////////////////////////////////
static __inline__ unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__("rdtsc"
						 : "=a"(lo), "=d"(hi));
	return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}
////////////////////////////////////////////////////////////////////////////////////
int global_node_num = 0;
class layer_node
{
public:
	unsigned int ip;
	int lenght;
	bool open;
	layer_node *left, *right, *parent;
	layer_node()
	{
		ip = 0;
		lenght = 0;
		open = true;
		left = NULL;
		right = NULL;
		parent = NULL;
		++global_node_num;
	}
};
layer_node *root_layer_node[32];
////////////////////////////////////////////////////////////////////////////////////
struct node_list
{ //structure of binary trie
	unsigned int port;
	struct node_list *left, *right;
};
typedef struct node_list node;
typedef node *btrie;
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
int num_entry = 0;
int num_query = 0;
struct ENTRY *table, *query;
int N = 0; //number of nodes
unsigned long long int time_begin, time_end, total = 0;
unsigned long long int *time_clock;
int num_node = 0; //total number of nodes in the binary trie
////////////////////////////////////////////////////////////////////////////////////
btrie create_node()
{
	btrie temp;
	num_node++;
	temp = (btrie)malloc(sizeof(node));
	temp->right = NULL;
	temp->left = NULL;
	temp->port = 256; //default port
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
int min(int a, int b)
{
	return a < b ? a : b;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned int ip, unsigned char len, unsigned char nexthop)
{
	btrie ptr = root;
	int i;
	for (i = 0; i < len; i++)
	{
		if (ip & (1 << (31 - i)))
		{
			if (ptr->right == NULL)
				ptr->right = create_node(); // Create Node
			ptr = ptr->right;
			if ((i == len - 1) && (ptr->port == 256))
				ptr->port = nexthop;
		}
		else
		{
			if (ptr->left == NULL)
				ptr->left = create_node();
			ptr = ptr->left;
			if ((i == len - 1) && (ptr->port == 256))
				ptr->port = nexthop;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////
void read_table(char *str, unsigned int *ip, int *len, unsigned int *nexthop)
{
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
	if (str1 != NULL)
	{
		sprintf(buf, "%s\0", str1);
		*len = atoi(buf);
	}
	else
	{
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
void search(unsigned int ip)
{
	int j;
	btrie current = root, temp = NULL;
	for (j = 31; j >= (-1); j--)
	{
		if (current == NULL)
			break;
		if (current->port != 256)
			temp = current;
		if (ip & (1 << j))
		{
			current = current->right;
		}
		else
		{
			current = current->left;
		}
	}
	if (temp == NULL)
		printf("default\n");
	/*else
	  printf("%u\n",temp->port);*/
}
////////////////////////////////////////////////////////////////////////////////////
void set_table(char *file_name)
{
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip, nexthop;
	fp = fopen(file_name, "r");
	while (fgets(string, 50, fp) != NULL)
	{
		read_table(string, &ip, &len, &nexthop);
		num_entry++;
	}
	rewind(fp);
	table = (struct ENTRY *)malloc(num_entry * sizeof(struct ENTRY));
	num_entry = 0;
	while (fgets(string, 50, fp) != NULL)
	{
		read_table(string, &ip, &len, &nexthop);
		table[num_entry].ip = ip;
		table[num_entry].port = nexthop;
		table[num_entry++].len = len;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_query(char *file_name)
{
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip, nexthop;
	fp = fopen(file_name, "r");
	while (fgets(string, 50, fp) != NULL)
	{
		read_table(string, &ip, &len, &nexthop);
		num_query++;
	}
	rewind(fp);
	query = (struct ENTRY *)malloc(num_query * sizeof(struct ENTRY));
	time_clock = (unsigned long long int *)malloc(num_query * sizeof(unsigned long long int));
	num_query = 0;
	while (fgets(string, 50, fp) != NULL)
	{
		read_table(string, &ip, &len, &nexthop);
		query[num_query].ip = ip;
		query[num_query].len = len;
		query[num_query].port = nexthop;
		time_clock[num_query++] = 10000000;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void create()
{
	int i;
	root = create_node();
	time_begin = rdtsc();
	for (i = 0; i < num_entry; i++)
		add_node(table[i].ip, table[i].len, table[i].port);
	time_end = rdtsc();
}
////////////////////////////////////////////////////////////////////////////////////
int compare_cover(unsigned int input_ip1, unsigned int input_ip2, int len1, int len2)
{

	int minlen = min(len1, len2);
	bool different_cover = false;
	unsigned int ip1 = input_ip1, ip2 = input_ip2;
	ip1 = ip1 >> (32 - minlen);
	ip2 = ip2 >> (32 - minlen);

	if (ip1 > ip2)
	{
		different_cover = true;
		return 1; // bigger -> find right
	}
	else if (ip1 < ip2)
	{
		different_cover = true;
		return 2; // smaller -> find left
	}

	if (different_cover == false)
	{
		if (len1 > len2)
		{
			return 3; //put old to next tree
		}
		else if (len1 < len2)
		{
			return 4; //put the new to next tree;
		}
		else if(len1 == len2){
			return 0;
		}
	}
	printf("at compare cover,  this can not happed, ip1 is %u, ip2 is %u, len1 is %d, len2 is %d\n", input_ip1, input_ip2, len1, len2);
}
////////////////////////////////////////////////////////////////////////////////////
void add_layer_node(unsigned int input_ip, int input_len)
{
	unsigned int temp_ip = input_ip, temp_ip2;
	int temp_len = input_len, temp_len2;
	for (int i = 0; i < 32; i++)
	{
		//if(i == 31)	printf("enter 31\n");
		if (root_layer_node[i] == NULL)
		{
			//printf("create root at i = %d\n", i);
			root_layer_node[i] = new layer_node();
			root_layer_node[i]->ip = temp_ip;
			root_layer_node[i]->lenght = temp_len;
			return;
		}
		else
		{
			layer_node *temp = root_layer_node[i];
			bool have_insert = false;
			while (have_insert == false)
			{
				int compare_result = compare_cover(temp_ip, temp->ip, temp_len, temp->lenght);
				if (compare_result == 1)
				{
					//printf("enter compare_result 1\n");
					if (temp->right == NULL)
					{
						temp->right = new layer_node();
						temp->right->ip = temp_ip;
						temp->right->lenght = temp_len;
						temp->right->parent = temp;
						have_insert = true;
						
						if (temp->parent != NULL && temp->left == NULL)
						{
							if (temp->parent->left != NULL && temp->parent->right == NULL)
							{
								layer_node *node_layer_1 = temp->parent, *node_layer_2 = temp, *node_layer_3 = temp->right;

								temp_ip2 = node_layer_1 -> ip;
								temp_len2 = node_layer_1 -> lenght;
								node_layer_1->ip = node_layer_3 -> ip;
								node_layer_1->lenght = node_layer_3 -> lenght;
								node_layer_3 -> ip = temp_ip2;
								node_layer_3 -> lenght = temp_len2;

								node_layer_1 -> right = node_layer_3;
								node_layer_2 -> right = NULL;
								node_layer_3 -> parent = node_layer_1;
							}
							else if (temp->parent->left == NULL && temp->parent->right != NULL)
							{
								layer_node *node_layer_1 = temp->parent, *node_layer_2 = temp, *node_layer_3 = temp->right;

								temp_ip2 = node_layer_1 -> ip;
								temp_len2 = node_layer_1 -> lenght;
								node_layer_1->ip = node_layer_2 -> ip;
								node_layer_1->lenght = node_layer_2 -> lenght;
								node_layer_2 -> ip = temp_ip2;
								node_layer_2 -> lenght = temp_len2;

								node_layer_2 -> right = NULL;
								node_layer_1 -> left = node_layer_2;
								node_layer_1 -> right = node_layer_3;
								node_layer_3 -> parent = node_layer_1;
							}
						}
						
						return;
					}
					else
					{
						temp = temp->right;
					}
					//printf("leave compare_result 1\n");
				}
				else if (compare_result == 2)
				{
					//printf("enter compare_result 2\n");
					if (temp->left == NULL)
					{
						temp->left = new layer_node();
						temp->left->ip = temp_ip;
						temp->left->lenght = temp_len;
						temp->left->parent = temp;
						have_insert = true;
						
						if (temp->parent != NULL && temp->right == NULL)
						{
							if (temp->parent->left != NULL && temp->parent->right == NULL)
							{
								layer_node *node_layer_1 = temp->parent, *node_layer_2 = temp, *node_layer_3 = temp->left;

								temp_ip2 = node_layer_1 -> ip;
								temp_len2 = node_layer_1 -> lenght;
								node_layer_1->ip = node_layer_2 -> ip;
								node_layer_1->lenght = node_layer_2 -> lenght;
								node_layer_2 -> ip = temp_ip2;
								node_layer_2 -> lenght = temp_len2;

								node_layer_2 -> left = NULL;
								node_layer_1 -> right = node_layer_2;
								node_layer_1 -> left = node_layer_3;
								node_layer_3 -> parent = node_layer_1;

							}
							else if (temp->parent->left == NULL && temp->parent->right != NULL)
							{
								layer_node *node_layer_1 = temp->parent, *node_layer_2 = temp, *node_layer_3 = temp->left;

								temp_ip2 = node_layer_1 -> ip;
								temp_len2 = node_layer_1 -> lenght;
								node_layer_1->ip = node_layer_3 -> ip;
								node_layer_1->lenght = node_layer_3 -> lenght;
								node_layer_3 -> ip = temp_ip2;
								node_layer_3 -> lenght = temp_len2;

								node_layer_1 -> left = node_layer_3;
								node_layer_2 -> left = NULL;
								node_layer_3 -> parent = node_layer_1;
							}
						}
						
						return;
					}
					else
					{
						temp = temp->left;
					}
					//printf("leave compare_result 2\n");
				}
				else if (compare_result == 3)
				{
					//printf("enter compare_result 3\n");
					temp_ip2 = temp->ip;
					temp_len2 = temp->lenght;
					temp->ip = temp_ip;
					temp->lenght = temp_len;
					temp_ip = temp_ip2;
					temp_len = temp_len2;
					//printf("leave compare_result 3\n");
					break;
				}
				else if (compare_result == 4)
				{
					//printf("enter & leave compare_result 4\n");
					break;
				}
				else if(compare_result == 0){
					printf("no need to add node\n");
					temp -> open = true;
					return;
				}
			}
		}
	}
	printf("at add layer node, this can not happen, ip is %u, len is %d\n", input_ip, input_len);
}
////////////////////////////////////////////////////////////////////////////////////
void create_layer_node(btrie ptr, int level, unsigned int value)
{
	if(ptr == NULL)
		return;
	int i;
	if(ptr -> port != 256){
		add_layer_node(value << (32 - level), level);
	}

	create_layer_node(ptr->left, level + 1, value << 1);
	create_layer_node(ptr->right, level + 1, (value << 1) + 1);
}
////////////////////////////////////////////////////////////////////////////////////
void search_layer_node(unsigned int input_ip, int len_for_check){
	//printf("len_for_check %d\n", len_for_check);
	unsigned int ip = input_ip;
	unsigned int len = 32;

	for(int i=0; i < 32; ++i){
		layer_node * temp = root_layer_node[i];
		bool have_find = false;
		while(have_find == false){
			if(temp != NULL){
				int compare_result = compare_cover(ip, temp->ip, len, temp->lenght);

				if(compare_result == 1){
					if(temp->right != NULL){
						temp = temp -> right;
					}
					else
					{
						break;
					}
					
				}
				else if(compare_result == 2){
					if(temp->left != NULL){
						temp = temp -> left;
					}
					else
					{
						break;
					}
				}
				else if(compare_result == 3){
					have_find = true;
					return;
				}
				else if(compare_result == 4){
					//go to next tree to find
					printf("may be it will not happened in here\n");
					break;
				}
				else if(compare_result == 0){
					have_find = true;
					return;
				}
				else
				{
					 printf("this can not happened at search\n");
					 return;
				}
				
			}
			else
			{
				printf("error, root not found in i = %d, len for check is %d\n", i, len_for_check);
			}
			
		}
		
	}

	printf("not found, ip is %u, len is %d\n", input_ip, len_for_check);
}
////////////////////////////////////////////////////////////////////////////////////

void count_node(btrie r)
{
	if (r == NULL)
		return;
	count_node(r->left);
	N++;
	count_node(r->right);
}
////////////////////////////////////////////////////////////////////////////////////
void CountClock()
{
	unsigned int i;
	unsigned int *NumCntClock = (unsigned int *)malloc(50 * sizeof(unsigned int));
	for (i = 0; i < 50; i++)
		NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for (i = 0; i < num_query; i++)
	{
		if (time_clock[i] > MaxClock)
			MaxClock = time_clock[i];
		if (time_clock[i] < MinClock)
			MinClock = time_clock[i];
		if (time_clock[i] / 5000 < 50)
			NumCntClock[time_clock[i] / 5000]++;
		else
			NumCntClock[49]++;
	}
	printf("(MaxClock, MinClock) = (%5llu, %5llu)\n", MaxClock, MinClock);

	for (i = 0; i < 50; i++)
	{
		printf("%d\n", NumCntClock[i]);
	}
	return;
}

void shuffle(struct ENTRY *array, int n)
{

	srand((unsigned)time(NULL));
	struct ENTRY *temp = (struct ENTRY *)malloc(sizeof(struct ENTRY));

	for (int i = 0; i < n - 1; i++)
	{
		ssize_t j = i + rand() / (RAND_MAX / (n - i) + 1);

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
	free(temp);
}

////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	/*if(argc!=3){
		printf("Please execute the file as the following way:\n");
		printf("%s  routing_table_file_name  query_table_file_name\n",argv[0]);
		exit(1);
	}*/
	int i, j;
	//set_query(argv[2]);
	//set_table(argv[1]);

	set_query(argv[1]);
	//for (i = 0; i < num_query; i++)
	//	printf("query[i].len = %d", query[i].len);
	set_table(argv[1]);
	create();
	time_begin = rdtsc();
	create_layer_node(root, 0, 0);
	time_end = rdtsc();
	printf("end create_layer_node\n");
	printf("Avg. Insert: %llu\n", (time_end - time_begin) / num_entry);
	printf("number of nodes: %d\n", num_node);
	printf("Total memory requirement: %d KB\n", ((num_node * sizeof(struct node_list)) / 1024));
	
	shuffle(query, num_entry);
	////////////////////////////////////////////////////////////////////////////
	for (i = 0; i < num_query; i++)
	{
		if(i%10000 == 0)
			printf("num_query= %d\n", i);
		time_begin = rdtsc();
		search_layer_node(query[i].ip, query[i].len);
		time_end = rdtsc();
		if (time_clock[i] > (time_end - time_begin))
			time_clock[i] = (time_end - time_begin);
	}
	

	total = 0;
	for (j = 0; j < num_query; j++)
		total += time_clock[j];
	printf("Avg. Search: %llu\n", total / num_query);
	CountClock();
	////////////////////////////////////////////////////////////////////////////
	shuffle(query, num_entry);
	time_begin = rdtsc();
	for (i = 0; i < num_query; i++)
	{
		if(i%10000 == 0)
			printf("num_query= %d\n", i);
		search_layer_node(query[i].ip, query[i].len);
	}
	time_end = rdtsc();

	printf("avg delete = %d\n", (time_end - time_begin) / (num_entry));
	printf("total memeory cost = %d\n",  sizeof(layer_node) * global_node_num);
	printf("global_node_num = %d\n", global_node_num);
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
