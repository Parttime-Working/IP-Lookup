#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<vector>
#include<time.h>


using namespace std;
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
class  Supernode
{
	public:
		bool has_pre_node;
		bool internal_node[15];
		bool external_node[16];
		vector<int> rule_array;
		Supernode * down[16];
		Supernode(){
			for (int i = 0; i < 15; i++)
			{
				internal_node[i] = false;
			}
			for (int i = 0; i < 16; i++)
			{
				external_node[i] = false;
				down[i] = NULL;
			}
			has_pre_node = false;
			rule_array.clear();
		}

};
int supernode_num = 1;
Supernode * root_supernode = new Supernode();
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
int num_entry=0;
int num_query=0;
struct ENTRY *table, *query;
int N=0;//number of nodes
unsigned long long int time_begin,time_end,total=0;
unsigned long long int *time_clock;
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
		table[num_entry].port=nexthop;
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
		time_clock[num_query++]=10000000;
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
	Supernode * super_ptr = root_supernode;
	if(ptr == NULL)
		return;

	//printf("a\n");
	if(ptr->port != 256){
		int prefix_length = level;
		bool pre_node = false;
		unsigned int value_cp = value << (32 - level);
		//printf("b\n");
		while(prefix_length >= 0){
			//printf("c\n");
			//printf("%u\n", (value_cp & (1 << 31)) != 0);
			bool first_bit = (value_cp & (1 << 31)) != 0;
			bool second_bit = (value_cp & (1 << 30)) != 0;
			bool third_bit = (value_cp & (1 << 29)) != 0;
			bool foruth_bit = (value_cp & (1 << 28)) != 0;

			int array_index = 0;

			if(prefix_length == 0){
				//printf("10\n");
				super_ptr -> internal_node[array_index] = true;
				super_ptr -> rule_array.push_back(1);
				//printf("11\n");
			}
			else if(prefix_length == 1){
				//printf("20\n");
				array_index = first_bit + 1; 
				super_ptr -> internal_node[array_index] = true;
				super_ptr -> rule_array.push_back(1);
				//printf("21\n");
			}
			else if(prefix_length == 2){
				//printf("30\n");
				array_index = 2 * first_bit + second_bit + 3;
				super_ptr -> internal_node[array_index] = true;
				super_ptr -> rule_array.push_back(1);
				//printf("31\n");
			}
			else if(prefix_length == 3){
				//printf("40\n");
				array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
				super_ptr -> internal_node[array_index] = true;
				super_ptr -> rule_array.push_back(1);
				//printf("41\n");
			}
			else
			{
				//printf("00\n");
				array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;
				if(super_ptr -> internal_node[4 * first_bit + 2 * second_bit + third_bit + 7] == true ||
					super_ptr -> internal_node[2 * first_bit + second_bit + 3] == true ||
					super_ptr -> internal_node[first_bit + 1] == true ||
					super_ptr -> internal_node[0] == true )
				{
					pre_node = true;
				}
				if(super_ptr -> has_pre_node == true)
					pre_node = true;
				super_ptr->external_node[array_index] = true;
				if(super_ptr->down[array_index] == NULL){
					super_ptr -> down[array_index] = new Supernode();
					supernode_num++;
					//printf("set pre node = %d\n", super_ptr->has_pre_node);
				}
				if(pre_node == true)
					super_ptr -> down[array_index]-> has_pre_node = true;
				super_ptr = super_ptr -> down[array_index];
				
				//printf("01\n");
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

void update_super_node(int len, unsigned int input_ip){
	Supernode * super_ptr = root_supernode;

	//printf("a\n");

	int prefix_length = len;
	bool pre_node = false;
	unsigned int value_cp = input_ip << (32 - len);
	//printf("b\n");
	while(prefix_length >= 0){
		//printf("c\n");
		//printf("%u\n", (value_cp & (1 << 31)) != 0);
		bool first_bit = (value_cp & (1 << 31)) != 0;
		bool second_bit = (value_cp & (1 << 30)) != 0;
		bool third_bit = (value_cp & (1 << 29)) != 0;
		bool foruth_bit = (value_cp & (1 << 28)) != 0;

		int array_index = 0;

		if(prefix_length == 0){
			//printf("10\n");
			super_ptr -> internal_node[array_index] = true;
			super_ptr -> rule_array.push_back(1);
			//printf("11\n");
		}
		else if(prefix_length == 1){
			//printf("20\n");
			array_index = first_bit + 1; 
			super_ptr -> internal_node[array_index] = true;
			super_ptr -> rule_array.push_back(1);
			//printf("21\n");
		}
		else if(prefix_length == 2){
			//printf("30\n");
			array_index = 2 * first_bit + second_bit + 3;
			super_ptr -> internal_node[array_index] = true;
			super_ptr -> rule_array.push_back(1);
			//printf("31\n");
		}
		else if(prefix_length == 3){
			//printf("40\n");
			array_index = 4 * first_bit + 2 * second_bit + third_bit + 7;
			super_ptr -> internal_node[array_index] = true;
			super_ptr -> rule_array.push_back(1);
			//printf("41\n");
		}
		else
		{
			//printf("00\n");
			array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;
			if(super_ptr -> internal_node[4 * first_bit + 2 * second_bit + third_bit + 7] == true ||
				super_ptr -> internal_node[2 * first_bit + second_bit + 3] == true ||
				super_ptr -> internal_node[first_bit + 1] == true ||
				super_ptr -> internal_node[0] == true )
			{
				pre_node = true;
			}
			if(super_ptr -> has_pre_node == true)
				pre_node = true;
			super_ptr->external_node[array_index] = true;
			if(super_ptr->down[array_index] == NULL){
				super_ptr -> down[array_index] = new Supernode();
				supernode_num++;
				//printf("set pre node = %d\n", super_ptr->has_pre_node);
			}
			if(pre_node == true)
				super_ptr -> down[array_index]-> has_pre_node = true;
			super_ptr = super_ptr -> down[array_index];
			
			//printf("01\n");
		}
		
		prefix_length -= 4;
		value_cp = value_cp << 4;
		//printf("d\n");
	}
}
////////////////////////////////////////////////////////////////////////////////////
void search_super_node(unsigned int input_ip){
	int search_level = 0;
	Supernode * super_ptr = root_supernode;
	unsigned int ip = input_ip;
	bool find_answer = false;
	for (int i = 8; i >= 0; i--)
	{
		bool first_bit = (ip & (1 << 31)) != 0;
		bool second_bit = (ip & (1 << 30)) != 0;
		bool third_bit = (ip & (1 << 29)) != 0;
		bool foruth_bit = (ip & (1 << 28)) != 0;
		
		int array_index = 8 * first_bit + 4 * second_bit + 2 * third_bit + foruth_bit;

		//printf("not down\n");
		int temp_index[4] = {4 * first_bit + 2 * second_bit + third_bit + 7,
							2 * first_bit + second_bit + 3,
							first_bit + 1,
							0
							};

		for(int k = 0; k < 4; ++k){
			if(super_ptr -> internal_node[temp_index[k]] == true){
				int rule_count = 0;
				search_level += k;
				for(int j = temp_index[k]; j >= 0; --j){
					if(super_ptr -> internal_node[j] == true)
						++ rule_count;
				}
				//printf("find\n");
				if(super_ptr->rule_array.size() >= rule_count - 1){
					//printf("set true\n");
					find_answer = true;
					return;
				}
				else{
					printf("not set true\n");
				}
			}
		}

		if(super_ptr->has_pre_node == true && find_answer == false){
			find_answer = true;
			return;
			//printf("set true\n");
		}
		if(super_ptr -> external_node[array_index] == true){
			if(super_ptr->down[array_index] == NULL)
				printf("error occur");
			super_ptr = super_ptr -> down[array_index];
			ip = ip << 4;
			search_level += 4;
			if(super_ptr -> has_pre_node == true)
			{
				find_answer = true;
				return;
				//printf("set true at change\n");
			}
			//printf("down\n");
		}

		else if(super_ptr -> external_node[array_index] == false)
			break;
			
	}
	
	//if(find_answer == false)
	//	printf("not found, ip is = %u, search level is %d\n", input_ip, search_level);
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
	create();
	
	printf("number of nodes: %d\n",num_node);
	printf("Total memory requirement: %d KB\n",((num_node*sizeof(struct node_list))/1024));

	time_begin=rdtsc();
	insert_supernode(root, 0, 0);
	time_end=rdtsc();

	printf("supernode total size : %d\n", sizeof(Supernode) * supernode_num / 1024);

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
	total=0;
	for(j=0;j<num_query;j++)
		total+=time_clock[j];
	printf("Avg. Search: %llu\n",total/num_query);
	CountClock();

	time_begin=rdtsc();
	for(i=num_entry/10*9;i<num_entry;i++)
		update_super_node(table[i].len,table[i].ip);
	time_end=rdtsc();
	
	printf("Avg. Insert: %llu\n",(time_end-time_begin)/(num_entry/10));
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
