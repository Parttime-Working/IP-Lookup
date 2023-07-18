#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>


////////////////////////////////////////////////////////////////////////////////////
struct ENTRY{
	unsigned int ip;
	unsigned char len;
	unsigned char port;
};
////////////////////////////////////////////////////////////////////////////////////
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

////////////////////////////////////////////////////////////////////////////////////
struct node_list{//structure of binary trie
	unsigned int port;
	struct node_list *left,*right;
};
typedef struct node_list node;
typedef node *btrie;
////////////////////////////////////////////////////////////////////////////////////
struct rule_lsit{
	int port;
	struct rule_lsit * next_link;
};
struct rule_lsit * create_rule_list(){
	struct rule_lsit * temp = (struct rule_lsit *)malloc(sizeof(struct rule_lsit));
	temp -> port = 256;
	temp ->next_link = NULL;
	return temp;
}


int loss_case = 0, update_malloc_num = 0, supernode_num = 0, match_case = 0, memory_access_time = 0;;
struct Supernode
{
	int has_pre_node;
	int down_link_number;
	unsigned short int internal_node;
	unsigned short int external_node;
	struct rule_lsit * rule_array;
	//struct Supernode * down[16];
	struct Supernode * down_link;
};

struct Supernode * create_downlink(int size){
	struct Supernode * new_array;
	new_array = (struct Supernode *)malloc(sizeof(struct Supernode) * size);
	for(int j = 0; j < size; ++j){
		new_array[j].down_link = NULL;
		new_array[j].down_link_number = 0;
		new_array[j].external_node = 0;
		new_array[j].has_pre_node = 0;
		new_array[j].internal_node = 0;
		new_array[j].rule_array = NULL;
	}
	supernode_num+=size;

	return new_array;
};

struct Supernode * create_Supernode(){
	struct Supernode * temp = (struct Supernode *)malloc(sizeof(struct Supernode));
	temp -> internal_node = 0;
	temp -> external_node = 0;
	temp -> down_link_number = 0;
	temp -> down_link = NULL;
	/*
	for (int i = 0; i < 16; i++)
	{
		temp -> down[i] = NULL;
	}
	*/

	temp -> has_pre_node = 0;
	supernode_num++;
	temp -> rule_array = NULL;
	return temp;
};

struct Supernode * root_supernode;
////////////////////////////////////////////////////////////////////////////////////
void add_rule_list(struct Supernode * input_node){
	//printf("00\n");
	if(input_node -> rule_array == NULL){
		//printf("10\n");
		input_node -> rule_array = create_rule_list();
		input_node -> rule_array -> port = 1;
		//printf("11\n");
	}
	else
	{
		//printf("20\n");
		struct rule_lsit * temp =  create_rule_list();
		//printf("201\n");
		temp -> next_link = input_node -> rule_array;
		//printf("202\n");
		temp -> port = 1;
		input_node -> rule_array = temp;
		//printf("21\n");
	}
}
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
int num_entry=0;
int num_query=0;
int num_update = 0;
struct ENTRY *table, *query, *update_table;
int N=0;//number of nodes
unsigned long long int time_begin,time_end,total=0;
unsigned long long int *time_clock, * update_clock, * delete_clock;
int num_node=0;//total number of nodes in the binary trie
////////////////////////////////////////////////////////////////////////////////////
btrie create_node(){
	btrie temp;
	num_node++;
	temp=(btrie)malloc(sizeof(node));
	temp->right=NULL;
	temp->left=NULL;
	temp->port=256;//default port
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned int ip,unsigned char len,unsigned char nexthop){
	btrie ptr=root;
	int i;
	for(i=0;i<len;i++){
		if(ip&(1<<(31-i))){
			if(ptr->right==NULL)
				ptr->right=create_node(); // Create Node
			ptr=ptr->right;
			if((i==len-1)&&(ptr->port==256))
				ptr->port=nexthop;
		}
		else{
			if(ptr->left==NULL)
				ptr->left=create_node();
			ptr=ptr->left;
			if((i==len-1)&&(ptr->port==256))
				ptr->port=nexthop;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////
void read_table(char *str,unsigned int *ip,int *len,unsigned int *nexthop){
	char tok[]="./";
	char buf[100],*str1;
	unsigned int n[4];
	sprintf(buf,"%s\0",strtok(str,tok));
	n[0]=atoi(buf);
	sprintf(buf,"%s\0",strtok(NULL,tok));
	n[1]=atoi(buf);
	sprintf(buf,"%s\0",strtok(NULL,tok));
	n[2]=atoi(buf);
	sprintf(buf,"%s\0",strtok(NULL,tok));
	n[3]=atoi(buf);
	*nexthop=n[2];
	str1=(char *)strtok(NULL,tok);
	if(str1!=NULL){
		sprintf(buf,"%s\0",str1);
		*len=atoi(buf);
	}
	else{
		if(n[1]==0&&n[2]==0&&n[3]==0)
			*len=8;
		else
			if(n[2]==0&&n[3]==0)
				*len=16;
			else
				if(n[3]==0)
					*len=24;
	}
	*ip=n[0];
	*ip<<=8;
	*ip+=n[1];
	*ip<<=8;
	*ip+=n[2];
	*ip<<=8;
	*ip+=n[3];
}
////////////////////////////////////////////////////////////////////////////////////
void search(unsigned int ip){
	int j;
	btrie current=root,temp=NULL;
	for(j=31;j>=(-1);j--){
		if(current==NULL)
			break;
		if(current->port!=256)
			temp=current;
		if(ip&(1<<j)){
			current=current->right;
		}
		else{
			current=current->left; 
		}
	}
	if(temp==NULL)
	  printf("default\n");
	  /*else
	  printf("%u\n",temp->port);*/
}
////////////////////////////////////////////////////////////////////////////////////
void set_table(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip,nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		num_entry++;
	}
	rewind(fp);
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
	num_entry=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		table[num_entry].ip=ip;
		table[num_entry].port=len;
		table[num_entry++].len=len;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_query(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip,nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		num_query++;
	}
	rewind(fp);
	query=(struct ENTRY *)malloc(num_query*sizeof(struct ENTRY));
	time_clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		query[num_query].ip=ip;
		query[num_query].len=len;
		query[num_query].port=len;
		time_clock[num_query++]=10000000;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_update(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip,nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		num_update++;
	}
	rewind(fp);
	update_table=(struct ENTRY *)malloc(num_update*sizeof(struct ENTRY));
	update_clock=(unsigned long long int *)malloc(num_update*sizeof(unsigned long long int));
	delete_clock=(unsigned long long int *)malloc(num_update*sizeof(unsigned long long int));
	num_update=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		update_table[num_update].ip=ip;
		update_table[num_update].len=len;
		update_table[num_update].port=len;
		delete_clock[num_update] = 10000000;
		update_clock[num_update++]=10000000;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void create(){
	int i;
	root=create_node();
	time_begin=rdtsc();
	for(i=0;i<num_entry;i++)
		add_node(table[i].ip,table[i].len,table[i].port);
	time_end=rdtsc();
}
////////////////////////////////////////////////////////////////////////////////////
void count_node(btrie r){
	if(r==NULL)
		return;
	count_node(r->left);
	N++;
	count_node(r->right);
}
////////////////////////////////////////////////////////////////////////////////////
void CountClock()
{
	unsigned int i;
	unsigned int* NumCntClock = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for(i = 0; i < 50; i++) NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for(i = 0; i < num_query; i++)
	{
		if(time_clock[i] > MaxClock) MaxClock = time_clock[i];
		if(time_clock[i] < MinClock) MinClock = time_clock[i];
		if(time_clock[i] / 100 < 50) NumCntClock[time_clock[i] / 100]++;
		else NumCntClock[49]++;
	}
	printf("(MaxClock, MinClock) = (%5llu, %5llu)\n", MaxClock, MinClock);
	
	for(i = 0; i < 50; i++)
	{
		printf("%d\n",NumCntClock[i]);
	}
	return;
}
void Count_update_Clock()
{
	unsigned int i;
	unsigned int* NumCntClock = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for(i = 0; i < 50; i++) NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for(i = 0; i < num_update; i++)
	{
		if(update_clock[i] > MaxClock) MaxClock = update_clock[i];
		if(update_clock[i] < MinClock) MinClock = update_clock[i];
		if(update_clock[i] / 100 < 50) NumCntClock[update_clock[i] / 100]++;
		else NumCntClock[49]++;
	}
	printf("(MaxClock, MinClock) = (%5llu, %5llu)\n", MaxClock, MinClock);
	
	for(i = 0; i < 50; i++)
	{
		printf("%d\n",NumCntClock[i]);
	}
	return;
}
void Count_delete_Clock()
{
	unsigned int i;
	unsigned int* NumCntClock = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for(i = 0; i < 50; i++) NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for(i = 0; i < num_update; i++)
	{
		if(delete_clock[i] > MaxClock) MaxClock = delete_clock[i];
		if(delete_clock[i] < MinClock) MinClock = delete_clock[i];
		if(delete_clock[i] / 100 < 50) NumCntClock[delete_clock[i] / 100]++;
		else NumCntClock[49]++;
	}
	printf("(MaxClock, MinClock) = (%5llu, %5llu)\n", MaxClock, MinClock);
	
	for(i = 0; i < 50; i++)
	{
		printf("%d\n",NumCntClock[i]);
	}
	return;
}

void shuffle(struct ENTRY *array, int n) {
	
    srand((unsigned)time(NULL));
    struct ENTRY *temp=(struct ENTRY *)malloc(sizeof(struct ENTRY));
    
    for (int i = 0; i < n - 1; i++) {
        ssize_t j = i + rand() / (RAND_MAX / (n - i) + 1);

        temp->ip=array[j].ip;
        temp->len=array[j].len;
        temp->port=array[j].port;
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
void insert_supernode(btrie ptr, int level, unsigned int value){
	struct Supernode * super_ptr = root_supernode;
	if(ptr == NULL)
		return;

	//printf("a\n");
	if(ptr->port != 256){
		int prefix_length = level;
		int pre_node = 0;
		unsigned int value_cp = value << (32 - level);
		//printf("b\n");
		while(prefix_length >= 0){
			//printf("c\n");
			//printf("%u\n", (value_cp & (1 << 31)) != 0);
			int first_bit = (value_cp & (1 << 31)) != 0;
			int second_bit = (value_cp & (1 << 30)) != 0;
			int third_bit = (value_cp & (1 << 29)) != 0;
			int foruth_bit = (value_cp & (1 << 28)) != 0;

			int array_index = 0;
			unsigned short int short_1 = 1;
			if(prefix_length == 0){
				//printf("10\n");
				super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
				add_rule_list(super_ptr);
				//printf("11\n");
			}
			else if(prefix_length == 1){
				//printf("20\n");
				array_index = first_bit + 1; 
				super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
				add_rule_list(super_ptr);
				//printf("21\n");
			}
			else if(prefix_length == 2){
				//printf("30\n");
				array_index = 2 * first_bit + second_bit + 3;
				super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
				add_rule_list(super_ptr);
				//printf("31\n");
			}
			else if(prefix_length == 3){
				//printf("40\n");
				array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
				super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
				add_rule_list(super_ptr);
				//printf("41\n");
			}
			else
			{
				//printf("00\n");
				array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;
				if(super_ptr -> internal_node & short_1 << (4 * first_bit + 2 * second_bit + third_bit + 7) ||
					super_ptr -> internal_node & short_1 << (2 * first_bit + second_bit + 3) ||
					super_ptr -> internal_node & short_1 << (first_bit + 1) ||
					super_ptr -> internal_node & short_1 << (0) )
				{
					pre_node = 1;
				}

				if(super_ptr -> has_pre_node != 0)
					pre_node = 1;

				super_ptr->external_node = super_ptr->external_node | short_1 << array_index;
				int exter_count = 0;
				int cut_num = 0;
				//printf("array_index : %d\n", array_index);
				for(int j = 0; j <= 15; ++j){
					if(super_ptr -> external_node & short_1 << (j)){
						++ exter_count;
						if(j <=array_index){
							cut_num++;
						}
					}
				}
				//printf("exter : %d, cut : %d\n", exter_count, cut_num);

				if(exter_count != super_ptr -> down_link_number){
					struct Supernode * old_array = super_ptr -> down_link;
					struct Supernode * new_array = create_downlink(exter_count);

					
					for(int j = 0; j < cut_num-1; ++j){
						new_array[j].down_link = old_array[j].down_link;
						new_array[j].down_link_number = old_array[j].down_link_number;
						new_array[j].external_node = old_array[j].external_node;
						new_array[j].has_pre_node = old_array[j].has_pre_node;
						new_array[j].internal_node = old_array[j].internal_node;
						new_array[j].rule_array = old_array[j].rule_array;
					}
					for(int j = cut_num; j < exter_count; ++j){
						new_array[j].down_link = old_array[j-1].down_link;
						new_array[j].down_link_number = old_array[j-1].down_link_number;
						new_array[j].external_node = old_array[j-1].external_node;
						new_array[j].has_pre_node = old_array[j-1].has_pre_node;
						new_array[j].internal_node = old_array[j-1].internal_node;
						new_array[j].rule_array = old_array[j-1].rule_array;
					}
					
					free(super_ptr -> down_link);
					supernode_num-=exter_count-1;
					super_ptr -> down_link = new_array;

					super_ptr -> down_link_number = exter_count;
				}

				if(pre_node == 1)
					super_ptr -> down_link[cut_num-1].has_pre_node = level;
				super_ptr = &super_ptr -> down_link[cut_num-1];

			}
			
			prefix_length -= 4;
			value_cp = value_cp << 4;
			//printf("d\n");
		}

	}
	//printf("e\n");
	if(ptr->left != NULL){
		insert_supernode(ptr->left, level + 1, value << 1);
	}
	if(ptr->right != NULL){
		insert_supernode(ptr->right, level + 1, (value << 1) + 1);
	}

}
////////////////////////////////////////////////////////////////////////////////////
void search_super_node(unsigned int input_ip){

	int search_level = 0;
	struct Supernode * super_ptr = root_supernode;
	memory_access_time++;
	unsigned int ip = input_ip;
	int find_answer = 0;
	unsigned short int short_1 = 1;
	for (int i = 8; i >= 0; i--)
	{
		int first_bit = (ip & (1 << 31)) != 0;
		int second_bit = (ip & (1 << 30)) != 0;
		int third_bit = (ip & (1 << 29)) != 0;
		int foruth_bit = (ip & (1 << 28)) != 0;
		
		int array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;
	 	//printf("array_index : %d\n", array_index);

		if(super_ptr->has_pre_node != 0 && find_answer == 0){
			find_answer = 1;
			//return;
			//printf("set true\n");
		}
		if(super_ptr -> external_node & short_1 << array_index){

			int cut_num = 0;
			for(int j = 0; j <= array_index; ++j){
				if(super_ptr -> external_node & short_1 << (j))
					++ cut_num;
			}
			//printf("array index : %d, cut_num : %d, down number %d\n", array_index, cut_num, super_ptr -> down_link_number);
			super_ptr = &super_ptr -> down_link[cut_num-1];
			//printf("after \n");
			
			memory_access_time++;
			ip = ip << 4;
			search_level += 4;
			if(super_ptr -> has_pre_node != 0)
			{
				find_answer = 1;
				//return;
				//printf("set true at change\n");
			}
			//printf("down\n");
			//printf("------------------------------------------ \n");
		}
		else if(~(super_ptr -> external_node & short_1 << array_index)){

			int temp_index[4] = {4 * first_bit + 2 * second_bit + third_bit + 7,
					2 * first_bit + second_bit + 3,
					first_bit + 1,
					0
					};

			for(int k = 0; k < 4; ++k){
				if(super_ptr -> internal_node & short_1 << (temp_index[k])){
					int rule_count = 0;
					search_level += k;
					for(int j = temp_index[k]; j >= 0; --j){
						if(super_ptr -> internal_node & short_1 << (j))
							++ rule_count;
					}
					
					struct rule_lsit * temp = super_ptr -> rule_array;
					
					for(int j = 0; j < rule_count; ++j){
						//printf("find\n");
						if(temp == NULL){
							//printf("find1.1\n");
							break;
						}
						else if(j == rule_count -1){
							//printf("find1.2\n");
							find_answer = 1;
							break;
						}
						temp = temp -> next_link;
						//printf("find2\n");
					}
				}
				
			}
			//printf("loop out\n");
			memory_access_time++;
			break;
		}
			
			
	}
	
	if(find_answer == 0){
		loss_case++;
		//printf("loss_case++\n");
		//printf("not found, ip is = %u, search level is %d\n", input_ip, search_level);
	}
	else
	{
		match_case++;
		//printf("match++\n");
	}
	
	//	
}

void traversal(struct Supernode * super_ptr, int pre_port){
	return;
	if(super_ptr == NULL)
		return;
	if(pre_port > super_ptr -> has_pre_node){
		super_ptr -> has_pre_node = pre_port;
	}

	for(int i = 0; i < super_ptr ->down_link_number ; ++i){
		if(super_ptr -> down_link[i].has_pre_node < pre_port){
			traversal(&super_ptr -> down_link[i], pre_port);
		}

	}
}

void traversal_delete(struct Supernode * super_ptr, int pre_port){
	return;
	if(super_ptr == NULL)
		return;
	if(pre_port == super_ptr -> has_pre_node){
		super_ptr -> has_pre_node = 0;
	}

	for(int i = 0; i < super_ptr ->down_link_number ; ++i){
		if(super_ptr -> down_link[i].has_pre_node == pre_port){
			traversal(&super_ptr -> down_link[i], pre_port);
		}

	}
}

void update_supernode(unsigned int input_ip, int len){

	struct Supernode * super_ptr = root_supernode;
	if(super_ptr == NULL)
		return;


	int prefix_length = len;
	int pre_node = 0;
	unsigned int value_cp = (input_ip >> (32 - len)) << (32 - len);
	//printf("b\n");
	while(prefix_length >= 0){
		//printf("c\n");
		//printf("%u\n", (value_cp & (1 << 31)) != 0);
		int first_bit = (value_cp & (1 << 31)) != 0;
		int second_bit = (value_cp & (1 << 30)) != 0;
		int third_bit = (value_cp & (1 << 29)) != 0;
		int foruth_bit = (value_cp & (1 << 28)) != 0;

		int array_index = 0;
		unsigned short int short_1 = 1;
		if(prefix_length == 0){
			//printf("10\n");
			super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
			add_rule_list(super_ptr);
			traversal(super_ptr, len);
			//printf("11\n");
		}
		else if(prefix_length == 1){
			//printf("20\n");
			array_index = first_bit + 1; 
			super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
			add_rule_list(super_ptr);
			traversal(super_ptr, len);
			//printf("21\n");
		}
		else if(prefix_length == 2){
			//printf("30\n");
			array_index = 2 * first_bit + second_bit + 3;
			super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
			add_rule_list(super_ptr);
			traversal(super_ptr, len);
			//printf("31\n");
		}
		else if(prefix_length == 3){
			//printf("40\n");
			array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
			super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
			add_rule_list(super_ptr);
			traversal(super_ptr, len);
			//printf("41\n");
		}
		else
		{
			//printf("00\n");
			array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;
			if(super_ptr -> internal_node & short_1 << (4 * first_bit + 2 * second_bit + third_bit + 7) ||
				super_ptr -> internal_node & short_1 << (2 * first_bit + second_bit + 3) ||
				super_ptr -> internal_node & short_1 << (first_bit + 1) ||
				super_ptr -> internal_node & short_1 << (0) )
			{
				pre_node = 1;
			}

			if(super_ptr -> has_pre_node != 0)
				pre_node = 1;

			super_ptr->external_node = super_ptr->external_node | short_1 << array_index;
			int exter_count = 0;
			int cut_num = 0;
			//printf("array_index : %d\n", array_index);
			for(int j = 0; j <= 15; ++j){
				if(super_ptr -> external_node & short_1 << (j)){
					++ exter_count;
					if(j <=array_index){
						cut_num++;
					}
				}
			}
			//printf("exter : %d, cut : %d\n", exter_count, cut_num);

			if(exter_count != super_ptr -> down_link_number){
				struct Supernode * old_array = super_ptr -> down_link;
				struct Supernode * new_array = create_downlink(exter_count);

				
				for(int j = 0; j < cut_num-1; ++j){
					new_array[j].down_link = old_array[j].down_link;
					new_array[j].down_link_number = old_array[j].down_link_number;
					new_array[j].external_node = old_array[j].external_node;
					new_array[j].has_pre_node = old_array[j].has_pre_node;
					new_array[j].internal_node = old_array[j].internal_node;
					new_array[j].rule_array = old_array[j].rule_array;
				}
				for(int j = cut_num; j < exter_count; ++j){
					new_array[j].down_link = old_array[j-1].down_link;
					new_array[j].down_link_number = old_array[j-1].down_link_number;
					new_array[j].external_node = old_array[j-1].external_node;
					new_array[j].has_pre_node = old_array[j-1].has_pre_node;
					new_array[j].internal_node = old_array[j-1].internal_node;
					new_array[j].rule_array = old_array[j-1].rule_array;
				}
				//printf("02\n");
				//free(super_ptr -> down_link);
				//printf("03\n");
				supernode_num-=exter_count-1;
				super_ptr -> down_link = new_array;

				super_ptr -> down_link_number = exter_count;
			}

			if(pre_node == 1)
				super_ptr -> down_link[cut_num-1].has_pre_node = len;
			super_ptr = &super_ptr -> down_link[cut_num-1];
			//printf("11\n");
		}
		
		prefix_length -= 4;
		value_cp = value_cp << 4;
		//printf("d\n");
	}

	
}

void delete_supernode(unsigned int input_ip, int len){

	struct Supernode * super_ptr = root_supernode;
	if(super_ptr == NULL)
		return;


	int prefix_length = len;
	int pre_node = 0;
	unsigned int value_cp = (input_ip >> (32 - len)) << (32 - len);
	//printf("b\n");
	while(prefix_length >= 0){
		//printf("c\n");
		//printf("%u\n", (value_cp & (1 << 31)) != 0);
		int first_bit = (value_cp & (1 << 31)) != 0;
		int second_bit = (value_cp & (1 << 30)) != 0;
		int third_bit = (value_cp & (1 << 29)) != 0;
		int foruth_bit = (value_cp & (1 << 28)) != 0;

		int array_index = 0;
		unsigned short int short_1 = 1;
		if(prefix_length == 0){
			//printf("10\n");
			super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
			traversal_delete(super_ptr, len);
			//printf("11\n");
		}
		else if(prefix_length == 1){
			//printf("20\n");
			array_index = first_bit + 1; 
			super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
			traversal_delete(super_ptr, len);
			//printf("21\n");
		}
		else if(prefix_length == 2){
			//printf("30\n");
			array_index = 2 * first_bit + second_bit + 3;
			super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
			traversal_delete(super_ptr, len);
			//printf("31\n");
		}
		else if(prefix_length == 3){
			//printf("40\n");
			array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
			super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
			traversal_delete(super_ptr, len);
			//printf("41\n");
		}
		else
		{
			//printf("00\n");
			array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;
			int exter_count = 0;
			int cut_num = 0;
			//printf("array_index : %d\n", array_index);
			for(int j = 0; j <= 15; ++j){
				if(super_ptr -> external_node & short_1 << (j)){
					++ exter_count;
					if(j <=array_index){
						cut_num++;
					}
				}
			}
			//printf("exter : %d, cut : %d\n", exter_count, cut_num);

			super_ptr = &super_ptr -> down_link[cut_num-1];
			//printf("11\n");
		}
		
		prefix_length -= 4;
		value_cp = value_cp << 4;
		//printf("d\n");
	}

	
}
////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char *argv[]){
	/*if(argc!=3){
		printf("Please execute the file as the following way:\n");
		printf("%s  routing_table_file_name  query_table_file_name\n",argv[0]);
		exit(1);
	}*/
	int i,j;
	//set_query(argv[2]);
	//set_table(argv[1]);

	set_query(argv[2]);
	set_table(argv[1]);
	set_update(argv[3]);
	create();
	
	//printf("number of nodes: %d\n",num_node);
	//printf("Total memory requirement: %d KB\n",((num_node*sizeof(struct node_list))/1024));
	root_supernode = create_Supernode();
	time_begin=rdtsc();
	insert_supernode(root, 0, 0);
	time_end=rdtsc();

	//printf("supernode total size : %d\n", sizeof(Supernode) * supernode_num / 1024);
	printf("tree bit map total size : %d\n", 69 * supernode_num / 1024);
	printf("tree bit map node_num : %d\n", supernode_num);
	printf("Avg. Build: %llu\n",(time_end-time_begin)/num_entry);

	
	shuffle(query, num_query); 
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			time_begin=rdtsc();
			search_super_node(query[i].ip);
			time_end=rdtsc();
			if(time_clock[i]>(time_end-time_begin))
				time_clock[i]=(time_end-time_begin);
		}
	}
	printf("hello\n");
	total=0;
	for(j=0;j<num_query;j++)
		total+=time_clock[j];
	printf("Avg. Search: %llu\n",total/num_query);
	printf("Avg. Memory access: %f\n",(double)memory_access_time/((double)num_query * 100));
	CountClock();
	printf("loss case : %d, match case : %d\n", loss_case, match_case);

	////////////////////////////////////////////////////////////////////////////
	if(num_update > 0){
		total = 0;
		for(i = 0; i < num_update; ++i){
			time_begin = rdtsc();
			update_supernode(update_table[i].ip, update_table[i].len);
			time_end = rdtsc();
			if(update_clock[i]>(time_end-time_begin))
				update_clock[i]=(time_end-time_begin);
			total += (time_end-time_begin);
		}
		
		printf("Avg. Insert: %llu\n",total/num_update);
		Count_update_Clock();

		total = 0;
		for(i = 0; i < num_update; ++i){
			time_begin = rdtsc();
			delete_supernode(update_table[i].ip, update_table[i].len);
			time_end = rdtsc();
			if(delete_clock[i]>(time_end-time_begin))
				delete_clock[i]=(time_end-time_begin);
			total += (time_end-time_begin);
		}
		
		printf("Avg. Delete: %llu\n",total/num_update);
		Count_delete_Clock();
	}
	


	
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
