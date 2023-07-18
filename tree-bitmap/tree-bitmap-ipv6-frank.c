#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>

////////////////////////////////////////////////////////////////////////////////////
struct ENTRY{
	unsigned long long int ip1;
	unsigned long long int ip2;
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

int supernode_num = 0, memory_access_time = 0;
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
	if(input_node -> rule_array == NULL){
		input_node -> rule_array = create_rule_list();
		input_node -> rule_array -> port = 1;
	}
	else
	{
		struct rule_lsit * temp = input_node -> rule_array;
		while(temp -> next_link != NULL){
			temp = temp -> next_link;
		}
		temp -> next_link = create_rule_list();
		temp -> next_link -> port = 1;
	}
}
////////////////////////////////////////////////////////////////////////////////////
struct list{//structure of binary trie
	unsigned int port;
	struct list *left,*right;
};
typedef struct list node;
typedef node *btrie;
int match_case = 0, loss_case = 0;
unsigned short int short_1 = 1;
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
int num_entry=0;
int num_query=0;
int num_update = 0;
struct ENTRY *table, *query, *update_table;
int N=0;//number of nodes
unsigned long long int time_begin,time_end,total=0;
unsigned long long int *time_clock, *update_clock, *delete_clock;
int num_node=0;//total number of nodes in the binary trie
////////////////////////////////////////////////////////////////////////////////////
btrie create_node(){
	btrie temp;
	//num_node++;
	temp=(btrie)malloc(sizeof(node));
	temp->right=NULL;
	temp->left=NULL;
	temp->port=256;//default port
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned long long int ip1, unsigned long long int ip2,unsigned char len,unsigned char nexthop){
	btrie ptr=root;
	int i, ipv6_count = 0;
	unsigned long long int int_one = 1;
	if(len == 0)
		ptr -> port = nexthop;
	for(i=0;i<len;i++){
		if(i<64){
			if(ip1&(int_one<<(63-i))){
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
		else{
			if(ip2&(int_one<<(63-(i-64)))){
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
}
////////////////////////////////////////////////////////////////////////////////////

int getIndexOfSigns(char ch)
{
    if(ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if(ch >= 'A' && ch <='F') 
    {
        return ch - 'A' + 10;
    }
    if(ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    return -1;
}


unsigned long long int hexToDec(char *source)
{
    unsigned long long int sum = 0;
    unsigned long long int t = 1;
    int i, len;
 
    len = strlen(source);
    for(i=len-1; i>=0; i--)
    {
        sum += t * getIndexOfSigns(*(source + i));
        t *= 16;
    }
	//printf("sum : %llu\n", sum);
    return sum;
}

void read_table(char *str, unsigned long long int *ip_upper, unsigned long long int *ip_lower, int *len, unsigned int *nexthop) {
	int i, j = 0;
	int seg_count = 0;
	int gap;
	char tok[] = "/";
	char tok2[] = ":";
	char buf[100];
	char temp[100];
	char *p;

	sprintf(buf, "%s\0", strtok(str, tok));
	strcpy(temp,buf);
	// get len
	sprintf(buf, "%s\0", strtok(NULL, tok));
	*len = atoi(buf);

	//count have number segment
	p = strtok(temp, tok2);
	while (p != NULL){
		seg_count++;
		p = strtok(NULL, tok2);
	}

	//ip expend
	memset(temp, '\0', 100);
	sprintf(buf, "%s\0", strtok(str, tok));
	char *now = buf, *next = buf;

	while (*next != '\0'){
		next++;
		//check double colon
		if (*now == ':' && *next == ':') {
			for (i = j; i < j + (8 - seg_count) * 4; i++)
				temp[i] = '0';
			j += (8 - seg_count) * 4;
			now += 2;
			next++;
		}
		else {
			while (*next != ':' && *next != '\0')
				next++;
			while (*now == ':')
				now++;
			gap = next - now;
			for (i = 4 - gap; i > 0; i--, j++)
				temp[j] = '0';
			for (i = 0; i < gap; i++, j++, now++)
				temp[j] = *now;
		}
	}
	//convert to byte expression
	*ip_upper = 0;
	for (i = 0; i < 16; i++) {
		*ip_upper <<= 4;
		*ip_upper += (temp[i] > '9') ? (temp[i] - 'a' + 10) : (temp[i] - '0');
	}
	
	*ip_lower = 0;
	for (i = 16; i < 32; i++) {
		*ip_lower <<= 4;
		*ip_lower += (temp[i] > '9') ? (temp[i] - 'a' + 10) : (temp[i] - '0');
	}
	
	*nexthop = 39;
}
////////////////////////////////////////////////////////////////////////////////////
void search(unsigned long long int ip1, unsigned long long int ip2){
	int j;
	unsigned long long int int_one = 1;
	btrie current=root,temp=NULL;
	for(j=0; j<128; j++){
		if(current==NULL)
			break;
		if(current->port!=256)
			temp=current;
		if(j < 64){
			if(ip1 & (int_one<<(63-j))){
				current=current->right;
			}
			else{
				current=current->left; 
			}
		}
		else{
			if(ip2 & (int_one<<(63-(j - 64)))){
				current=current->right;
			}
			else{
				current=current->left; 
			}
		}
		
	}
	if(temp==NULL){
		//printf("default: %llu, %llu\n", ip1, ip2);
		loss_case++;
	}else{
		match_case++;
	}
	  
	  /*else
	  printf("%u\n",temp->port);*/
}
////////////////////////////////////////////////////////////////////////////////////
void set_table(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned long long int ip1, ip2;
	unsigned int nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2,&len,&nexthop);
		num_entry++;
	}
	rewind(fp);
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
	num_entry=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2,&len,&nexthop);
		//printf("ip1 : %u, ip2 : %u, len : %d\n", ip1, ip2, len);
		table[num_entry].ip1=ip1;
		table[num_entry].ip2=ip2;
		table[num_entry].port=nexthop;
		table[num_entry++].len=len;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_query(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned long long int ip1, ip2;
	unsigned int nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2,&len,&nexthop);
		num_query++;
	}
	rewind(fp);
	query=(struct ENTRY *)malloc(num_query*sizeof(struct ENTRY));
	time_clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2 ,&len,&nexthop);
		query[num_query].ip1=ip1;
		query[num_query].ip2=ip2;
		query[num_query].len=len;
		query[num_query].port=nexthop;
		time_clock[num_query++]=10000000;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_update(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned long long int ip1, ip2;
	unsigned int nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2,&len,&nexthop);
		num_update++;
	}
	rewind(fp);
	update_table=(struct ENTRY *)malloc(num_update*sizeof(struct ENTRY));
	update_clock=(unsigned long long int *)malloc(num_update*sizeof(unsigned long long int));
	delete_clock=(unsigned long long int *)malloc(num_update*sizeof(unsigned long long int));
	num_update=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2,&len,&nexthop);
		update_table[num_update].ip1=ip1;
		update_table[num_update].ip2=ip2;
		update_table[num_update].port=nexthop;
		update_table[num_update].len=len;
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
		add_node(table[i].ip1,table[i].ip2, table[i].len,table[i].port);
	time_end=rdtsc();

	printf("Avg. Build: %llu\n",(time_end-time_begin)/(num_entry));

	/*
	time_begin=rdtsc();
	for(i=num_entry/10*9;i<num_entry;i++)
		add_node(table[i].ip1,table[i].ip2, table[i].len,table[i].port);
	time_end=rdtsc();

	printf("Avg. Insert: %llu\n",(time_end-time_begin)/(num_entry/10));
	*/
}
////////////////////////////////////////////////////////////////////////////////////
void count_node(struct Supernode *r){
	int i,j=0;
	/*
	if(r==NULL)
		return;
		*/
	num_node++;
	if(r->external_node){
		for(i=0;i<16;i++)
			if(r->external_node&(1<<i))
				j++;
		for(i=0;i<j;i++)
			count_node(&r->down_link[i]);
	}
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

        temp->ip1=array[j].ip1;
		temp->ip2=array[j].ip2;
        temp->len=array[j].len;
        temp->port=array[j].port;
        array[j].ip1 = array[i].ip1;
		array[j].ip2 = array[i].ip2;
        array[j].len = array[i].len;
        array[j].port = array[i].port;
        array[i].ip1 = temp->ip1;
		array[i].ip2 = temp->ip2;
        array[i].len = temp->len;
        array[i].port = temp->port;
    }
	free(temp);
}
////////////////////////////////////////////////////////////////////////////////////
void insert_supernode(btrie ptr, int level, unsigned long long int value1, unsigned long long int value2){
	//printf("level %d, ip1 : %llu, ip2 : %llu\n", level, value1, value2);
	struct Supernode * super_ptr = root_supernode;
	if(ptr == NULL)
		return;
	unsigned long long int int_one = 1;
	//printf("a\n");
	if(ptr->port != 256){
		int prefix_length = level;
		int pre_node = 0;
		
		unsigned long long int value_cp1 = value1;
		unsigned long long int value_cp2 = value2;
		if(level < 64){
			value_cp1 = value1 << (64 - level);
		}
		else{
			value_cp2 = value2 << (64 - (level - 64));
		}
		int IPV6_count = 0;

		

		//printf("b\n");
		while(prefix_length >= 0){
			if(IPV6_count < 64){
					//printf("c\n");
				//printf("%u\n", (value_cp & (1 << 31)) != 0);
				int first_bit = (value_cp1 & (int_one << 63)) != 0;
				int second_bit = (value_cp1 & (int_one << 62)) != 0;
				int third_bit = (value_cp1 & (int_one << 61)) != 0;
				int foruth_bit = (value_cp1 & (int_one << 60)) != 0;

				int array_index = 0;

				if(prefix_length == 0){
					//printf("10\n");
					super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
					add_rule_list(super_ptr);
					//printf("internel up index: %d\n", array_index);
					//printf("11\n");
				}
				else if(prefix_length == 1){
					//printf("20\n");
					array_index = first_bit + 1; 
					super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
					add_rule_list(super_ptr);
					//printf("internel up index: %d\n", array_index);
					//printf("21\n");
				}
				else if(prefix_length == 2){
					//printf("30\n");
					array_index = 2 * first_bit + second_bit + 3;
					super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
					add_rule_list(super_ptr);
					//printf("internel up index: %d\n", array_index);
					//printf("31\n");
				}
				else if(prefix_length == 3){
					//printf("40\n");
					array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
					super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
					add_rule_list(super_ptr);
					//printf("internel up index: %d\n", array_index);
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
					
					//printf("01\n");
				}
				
				prefix_length -= 4;
				value_cp1 = value_cp1 << 4;
			}
			else
			{
				int first_bit = (value_cp2 & (int_one << 63)) != 0;
				int second_bit = (value_cp2 & (int_one << 62)) != 0;
				int third_bit = (value_cp2 & (int_one << 61)) != 0;
				int foruth_bit = (value_cp2 & (int_one << 60)) != 0;

				int array_index = 0;

				if(prefix_length == 0){
					//printf("10\n");
					super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
					add_rule_list(super_ptr);
					//printf("internel down index: %d\n", array_index);
					//printf("11\n");
				}
				else if(prefix_length == 1){
					//printf("20\n");
					array_index = first_bit + 1; 
					super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
					add_rule_list(super_ptr);
					//printf("internel down index: %d\n", array_index);
					//printf("21\n");
				}
				else if(prefix_length == 2){
					//printf("30\n");
					array_index = 2 * first_bit + second_bit + 3;
					super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
					add_rule_list(super_ptr);
					//printf("internel down index: %d\n", array_index);
					//printf("31\n");
				}
				else if(prefix_length == 3){
					//printf("40\n");
					array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
					super_ptr -> internal_node =  super_ptr -> internal_node | short_1 << array_index;
					add_rule_list(super_ptr);
					//printf("internel down index: %d\n", array_index);
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
					
					//printf("01\n");
				}
				
				prefix_length -= 4;
				value_cp2 = value_cp2 << 4;
			}
			
			
			IPV6_count += 4;
			//printf("d\n");
		}

	}
	//printf("e\n");
	if(level < 64){
		if(ptr->left != NULL){
			insert_supernode(ptr->left, level + 1, value1 << 1, value2);
		}
		if(ptr->right != NULL){
			insert_supernode(ptr->right, level + 1, (value1 << 1) + int_one, value2);
		}
	}
	else
	{
		if(ptr->left != NULL){
			insert_supernode(ptr->left, level + 1, value1, value2 << 1);
		}
		if(ptr->right != NULL){
			insert_supernode(ptr->right, level + 1, value1, (value2 << 1) + int_one);
		}
	}
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



void update_super_node(int len, unsigned long long int input_ip1, unsigned long long int input_ip2){
	struct Supernode * super_ptr = root_supernode;

	//printf("a\n");

	int prefix_length = len;
	int pre_node = 0;
	unsigned long long int value_cp1 = input_ip1;
	unsigned long long int value_cp2 = input_ip2;
	int IPV6_count = 0;
	if(len < 64){
		value_cp1 = input_ip1 << (63 - len);
	}
	else{
		value_cp2 = input_ip2 << (63 - (len - 64));
	}

	unsigned long long int int_one = 1;
	//printf("b\n");
	while(prefix_length >= 0){
		if(IPV6_count < 64){
				//printf("c\n");
			//printf("%u\n", (value_cp & (1 << 31)) != 0);
			int first_bit = (value_cp1 & (int_one << 63)) != 0;
			int second_bit = (value_cp1 & (int_one << 62)) != 0;
			int third_bit = (value_cp1 & (int_one << 61)) != 0;
			int foruth_bit = (value_cp1 & (int_one << 60)) != 0;

			int array_index = 0;

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
				
				//printf("01\n");
			}
			
			prefix_length -= 4;
			value_cp1 = value_cp1 << 4;
			//printf("d\n");
		}
		else
		{
				//printf("c\n");
			//printf("%u\n", (value_cp & (1 << 31)) != 0);
			int first_bit = (value_cp2 & (int_one << 63)) != 0;
			int second_bit = (value_cp2 & (int_one << 62)) != 0;
			int third_bit = (value_cp2 & (int_one << 61)) != 0;
			int foruth_bit = (value_cp2 & (int_one << 60)) != 0;

			int array_index = 0;

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
				
				//printf("01\n");
			}
			
			prefix_length -= 4;
			value_cp2 = value_cp2 << 4;
			//printf("d\n");
		}
		
		
		IPV6_count += 4;
	}
}

void search_super_node(unsigned long long int input_ip1, unsigned long long int input_ip2, int numcheck){
	//printf("ip1 : %llu, ip2 : %llu\n", input_ip1, input_ip2);
	int search_level = 0;
	struct Supernode * super_ptr = root_supernode;
	memory_access_time++;
	unsigned long long int ip1 = input_ip1;
	unsigned long long int ip2 = input_ip2;
	int find_answer = 0;
	unsigned long long int int_one = 1;
	int IPV6_count = 0;

	for (int i = 32; i >= 0; i--)
	{
		if(IPV6_count < 64){
			int first_bit = (ip1 & (int_one << 63)) != 0;
			int second_bit = (ip1 & (int_one << 62)) != 0;
			int third_bit = (ip1 & (int_one << 61)) != 0;
			int foruth_bit = (ip1 & (int_one << 60)) != 0;
			
			int array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;
			
			if(super_ptr->has_pre_node != 0 && find_answer == 0){
				find_answer = 1;
				//return;
				//printf("set true\n");
			}
			//printf("externel up index: %d\n", array_index);
			if(super_ptr -> external_node & short_1 << array_index){
				int cut_num = 0;
				for(int j = 0; j <= array_index; ++j){
					if(super_ptr -> external_node & short_1 << (j))
						++ cut_num;
				}
				//printf("array index : %d, cut_num : %d, down number %d\n", array_index, cut_num, super_ptr -> down_link_number);
				super_ptr = &super_ptr -> down_link[cut_num-1];
				memory_access_time++;
				ip1 = ip1 << 4;
				search_level += 4;
				IPV6_count += 4;

				if(super_ptr -> has_pre_node != 0)
				{
					find_answer = 1;
					//return;
					//printf("set true at change\n");
				}
				//printf("down\n");
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
						//printf("find\n");
						struct rule_lsit * temp = super_ptr -> rule_array;
						
						for(int j = 0; j < rule_count; ++j){
							if(temp == NULL){
								break;
							}
							else if(j == rule_count -1){
								find_answer = 1;
								break;
							}
							temp = temp -> next_link;
						}
					}
				}
				memory_access_time++;
				//printf("not here at up\n");
				break;
			}
				
				
		}
		else
		{
			int first_bit = (ip2 & (int_one << 63)) != 0;
			int second_bit = (ip2 & (int_one << 62)) != 0;
			int third_bit = (ip2 & (int_one << 61)) != 0;
			int foruth_bit = (ip2 & (int_one << 60)) != 0;
			
			int array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;

			//printf("not down\n");
			if(~(super_ptr -> external_node & short_1 << array_index)){

			}
			

			if(super_ptr->has_pre_node != 0 && find_answer == 0){
				find_answer = 1;
				//return;
				//printf("set true\n");
			}
			//printf("externel down index: %d\n", array_index);
			if(super_ptr -> external_node & short_1 << array_index){
				int cut_num = 0;
				for(int j = 0; j <= array_index; ++j){
					if(super_ptr -> external_node & short_1 << (j))
						++ cut_num;
				}
				//printf("array index : %d, cut_num : %d, down number %d\n", array_index, cut_num, super_ptr -> down_link_number);
				super_ptr = &super_ptr -> down_link[cut_num-1];
				memory_access_time++;
				ip2 = ip2 << 4;
				search_level += 4;
				IPV6_count += 4;

				if(super_ptr -> has_pre_node != 0)
				{
					find_answer = 1;
					//return;
					//printf("set true at change\n");
				}
				//printf("down\n");
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
						//printf("find\n");
						struct rule_lsit * temp = super_ptr -> rule_array;
						
						for(int j = 0; j < rule_count; ++j){
							if(temp == NULL){
								break;
							}
							else if(j == rule_count -1){
								find_answer = 1;
								break;
							}
							temp = temp -> next_link;
						}
					}
				}
				memory_access_time++;
				break;
			}
				
		}
		
		
	}
	
	if(find_answer == 0){
		loss_case++;
		//printf("not found, ip is = %llu, %llu,  count is %d, num: %d\n", input_ip1, input_ip2, IPV6_count, numcheck);
	}
	else
	{
		match_case++;
	}
	
		
}
void delete_super_node(int len, unsigned long long int input_ip1, unsigned long long int input_ip2){
	struct Supernode * super_ptr = root_supernode;

	//printf("a\n");

	int prefix_length = len;
	unsigned long long int value_cp1 = input_ip1;
	unsigned long long int value_cp2 = input_ip2;
	int IPV6_count = 0;
	if(len < 64){
		value_cp1 = input_ip1 << (63 - len);
	}
	else{
		value_cp2 = input_ip2 << (63 - (len - 64));
	}

	unsigned long long int int_one = 1;
	//printf("b\n");
	while(prefix_length >= 0){
		if(IPV6_count < 64){
				//printf("c\n");
			//printf("%u\n", (value_cp & (1 << 31)) != 0);
			int first_bit = (value_cp1 & (int_one << 63)) != 0;
			int second_bit = (value_cp1 & (int_one << 62)) != 0;
			int third_bit = (value_cp1 & (int_one << 61)) != 0;
			int foruth_bit = (value_cp1 & (int_one << 60)) != 0;

			int array_index = 0;

			if(prefix_length == 0){
				//printf("10\n");
				super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
				traversal_delete(super_ptr, len);
				return;
				//printf("11\n");
			}
			else if(prefix_length == 1){
				//printf("20\n");
				array_index = first_bit + 1; 
				super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
				traversal_delete(super_ptr, len);
				return;
				//printf("21\n");
			}
			else if(prefix_length == 2){
				//printf("30\n");
				array_index = 2 * first_bit + second_bit + 3;
				super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
				traversal_delete(super_ptr, len);
				return;
				//printf("31\n");
			}
			else if(prefix_length == 3){
				//printf("40\n");
				array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
				super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
				traversal_delete(super_ptr, len);
				return;
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
				
				//printf("01\n");
			}
			
			prefix_length -= 4;
			value_cp1 = value_cp1 << 4;
			//printf("d\n");
		}
		else
		{
				//printf("c\n");
			//printf("%u\n", (value_cp & (1 << 31)) != 0);
			int first_bit = (value_cp2 & (int_one << 63)) != 0;
			int second_bit = (value_cp2 & (int_one << 62)) != 0;
			int third_bit = (value_cp2 & (int_one << 61)) != 0;
			int foruth_bit = (value_cp2 & (int_one << 60)) != 0;

			int array_index = 0;

			if(prefix_length == 0){
				//printf("10\n");
				super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
				traversal_delete(super_ptr, len);
				return;
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
				return;
				//printf("31\n");
			}
			else if(prefix_length == 3){
				//printf("40\n");
				array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
				super_ptr -> internal_node =  super_ptr -> internal_node ^ short_1 << array_index;
				traversal_delete(super_ptr, len);
				return;
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
				
				//printf("01\n");
			}
			
			prefix_length -= 4;
			value_cp2 = value_cp2 << 4;
			//printf("d\n");
		}
		
		
		IPV6_count += 4;
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
	//printf("after read\n");
	create();
	//printf("end create\n");
	//////////////
	/*
	int prefix_array[129] = {0};
	for(i = 0; i < num_entry; ++i){
		prefix_array[table[i].len]++;
	}
	for(i = 0; i < 129; ++i){
		printf("%u\n", prefix_array[i]);
	}
	*/
	//////////////

	//printf("number of nodes: %d\n",num_node);
	printf("num entry : %d\n", num_entry);
	
	unsigned long long int int_zero = 0;
	root_supernode = create_Supernode();
	time_begin=rdtsc();
	insert_supernode(root, 0, int_zero, int_zero);
	time_end=rdtsc();
	count_node(root_supernode);
	printf("tree bit map total size : %d\n", 13 * num_node / 1024);
	printf("tree bit map node_num : %d\n", num_node);
	printf("tree bit map total size : %d\n", 13 * supernode_num / 1024);
	printf("tree bit map node_num : %d\n", supernode_num);
	//shuffle(query, num_query);
	//printf("end shuffle\n"); 
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			time_begin=rdtsc();
			search_super_node(query[i].ip1, query[i].ip2, i);
			time_end=rdtsc();
			if(time_clock[i]>(time_end-time_begin))
				time_clock[i]=(time_end-time_begin);
		}
	}
	total=0;
	for(j=0;j<num_query;j++)
		total+=time_clock[j];
	printf("Avg. Search: %llu\n",total/num_query);
	printf("Avg. Memory access: %f\n",(double)memory_access_time/((double)num_query * 100));
	CountClock();
	printf("loss case: %d, match case : %d\n", loss_case, match_case);

	if(num_update > 0){
		total = 0;
		for(i=0;i<num_update;i++){
			time_begin=rdtsc();
			//printf("---------------------\n");
			update_super_node(update_table[i].len,update_table[i].ip1, update_table[i].ip2);
			time_end=rdtsc();
			if(update_clock[i]>(time_end-time_begin))
				update_clock[i]=(time_end-time_begin);
			total += (time_end-time_begin);
		}
			
		

		printf("Avg. Insert: %llu\n",total/num_update);
		Count_update_Clock();

		total = 0;
		for(i=0;i<num_update;i++){
			time_begin=rdtsc();
			//printf("i is : %d\n", i);
			delete_super_node(update_table[i].len,update_table[i].ip1, update_table[i].ip2);
			time_end=rdtsc();
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
