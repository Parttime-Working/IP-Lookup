#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

////////////////////////////////////////////////////////////////////////////////////
struct ENTRY{
	unsigned int ip;
	unsigned char len;
	int port;
};
////////////////////////////////////////////////////////////////////////////////////
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
////////////////////////////////////////////////////////////////////////////////////
struct range_node{
	unsigned int end_index;
	int port;
};
struct range_node* range_node_array;
int lost_case = 0;
////////////////////////////////////////////////////////////////////////////////////
struct list{//structure of binary trie
	int port;
	struct list *left,*right;
};
typedef struct list node;
typedef node *btrie;
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
int num_entry=0;
int num_query=0;
struct ENTRY *table, *query;
int N=0;//number of nodes
unsigned long long int begin,end,total=0;
unsigned long long int *clock;
int num_node=0;//total number of nodes in the binary trie
////////////////////////////////////////////////////////////////////////////////////
btrie create_node(){
	btrie temp;
	num_node++;
	temp=(btrie)malloc(sizeof(node));
	temp->right=NULL;
	temp->left=NULL;
	temp->port=-1;//default port
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned int ip,unsigned char len,unsigned char nexthop){
	btrie ptr=root;
	int i;
	if(len == 0)
		ptr -> port = nexthop;
	for(i=0;i<len;i++){
		if(ip&(1<<(31-i))){
			if(ptr->right==NULL)
				ptr->right=create_node(); // Create Node
			ptr=ptr->right;
			if((i==len-1)&&(ptr->port==-1))
				ptr->port=nexthop;
		}
		else{
			if(ptr->left==NULL)
				ptr->left=create_node();
			ptr=ptr->left;
			if((i==len-1)&&(ptr->port==-1))
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
		if(current->port!=-1)
			temp=current;
		if(ip&(1<<j)){
			current=current->right;
		}
		else{
			current=current->left; 
		}
	}
	if(temp==NULL)
	  printf("default: %u\n", ip);
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
		table[num_entry].port=num_entry;
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
	clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		query[num_query].ip=ip;
		query[num_query].len = len;
		query[num_query].port = num_query;
		clock[num_query++]=10000000;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void create(){
	int i;
	root=create_node();
	begin=rdtsc();
	for(i=0;i<num_entry;i++){
		add_node(table[i].ip,table[i].len,table[i].port);
	}
		
	end=rdtsc();
	//printf("Avg. Build: %llu\n",(end-begin)/num_entry - 100);
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
		if(clock[i] > MaxClock) MaxClock = clock[i];
		if(clock[i] < MinClock) MinClock = clock[i];
		if(clock[i] / 100 < 50) NumCntClock[clock[i] / 100]++;
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
void leaf_push(btrie ptr){
	if(ptr == NULL)
		return;
	if(ptr -> port != -1){
		if(ptr -> left != NULL && ptr -> right == NULL){
			if(ptr -> left -> port == -1){
				ptr -> left -> port = ptr -> port;
			}
			ptr -> right = create_node();
			ptr -> right -> port = ptr -> port;
			ptr -> port = -1;
		}
		else if(ptr -> right != NULL && ptr -> left == NULL){
			if(ptr -> right -> port == -1){
				ptr -> right -> port = ptr -> port;
			}
			ptr -> left = create_node();
			ptr -> left -> port = ptr -> port;
			ptr -> port = -1;
		}
		else if(ptr -> right != NULL && ptr -> left != NULL){
			if(ptr -> right -> port == -1){
				ptr -> right -> port = ptr -> port;
			}
			if(ptr -> left -> port == -1){
				ptr -> left -> port = ptr -> port;
			}
			ptr -> port = -1;
		}
	}
	leaf_push(ptr -> left);
	leaf_push(ptr -> right);
}

int num_leaf_node = 0;
unsigned int pre_ip_right = 0;
int pre_port = -1;
void count_leaf_node(btrie ptr, int level, unsigned int value){
	if(ptr == NULL)
		return;
	count_leaf_node(ptr -> left, level + 1, value << 1);
	if(ptr -> port != -1 && ptr -> left == NULL && ptr -> right == NULL){
		unsigned int ip_left = value << (32 - level);
		
		if(pre_ip_right < ip_left - 1){
			num_leaf_node++;
		}
		num_leaf_node++;
		pre_ip_right = value;
		for(int i = 0; i < (32 - level); ++i){
			pre_ip_right = (pre_ip_right << 1) + 1;
		}
		
		//printf("ptr -> port %d\n", ptr ->port);
	}	
		
	count_leaf_node(ptr -> right, level + 1, (value << 1) + 1);
}

int range_node_index = 0;
void create_range_node(btrie ptr, int level, unsigned int value){
	if(ptr == NULL)
		return;

	create_range_node(ptr -> left, level + 1, value << 1);
	if(ptr -> port != -1 && ptr -> left == NULL && ptr -> right == NULL){
		unsigned int ip_left = value << (32 - level);
		if(ip_left == 0) ++ ip_left;
		//printf("ip left %u\n", ip_left);
		if(pre_ip_right < ip_left - 1){
			range_node_array[range_node_index].end_index = ip_left-1;
			range_node_array[range_node_index].port = -1;
			++range_node_index;
		}
		
		
		pre_ip_right = value;
		for(int i = 0; i < (32 - level); ++i){
			pre_ip_right = (pre_ip_right << 1) + 1;
		}
		range_node_array[range_node_index].end_index = pre_ip_right;
		//range_node_array[range_node_index].end_index = ((value + 1) << (32 - level)) - 1;
		range_node_array[range_node_index].port = ptr->port;
		++range_node_index;
	}	
		
	create_range_node(ptr -> right, level + 1, (value << 1) + 1);
}

void search_range(unsigned int input_ip, int left_index, int right_index, int len_for_check){
	//printf("part1, right index: %d, left index : %d\n", right_index, left_index);
	if(input_ip <= range_node_array[left_index].end_index){
		//printf("part2\n");
		if(range_node_array[left_index].port == -1){
			lost_case++;
			//printf("input ip :%u, left index : %d, right index : %d, len = %d\n", input_ip, left_index, right_index, len_for_check);
		}
			
		//printf("part3\n");
		return;
	}
	//printf("part4, right index: %d, left index : %d\n", right_index, left_index);
	//int middle = (int)ceil(((double)(left_index + right_index))/2);
	int middle = (left_index + right_index) / 2;
	//printf("part middle : %d\n", middle);
	//printf("range_node_array[middle].end_index : %d\n", range_node_array[middle].end_index);
	if (range_node_array[middle].end_index < input_ip){
		//printf("part5\n");
		search_range(input_ip, middle + 1, right_index, len_for_check);
		//printf("part6\n");
		return;
	}
	else
	{
		//printf("part7\n");
		search_range(input_ip, left_index, middle, len_for_check);
		//printf("part8\n");
		return;
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

	set_query(argv[1]);
	set_table(argv[1]);
	create();
	printf("number of nodes: %d\n",num_node);
	printf("num entry : %d\n", num_entry);
	
	begin = rdtsc();
	leaf_push(root);
	count_leaf_node(root, 0, 0);
	printf("num leaf node : %d\n", num_leaf_node);
	range_node_array = (struct range_node*)malloc(sizeof(struct range_node) * num_leaf_node);
	printf("Total memory requirement: %d KB\n",(sizeof(struct range_node) * num_leaf_node/1024));
	pre_ip_right = 0;
	pre_port = -1;
	create_range_node(root, 0, 0);
	end = rdtsc();
	printf("Avg. Build: %llu\n",(end-begin)/num_entry);
	printf("after create range node\n");
	printf("range_node_index : %d\n", range_node_index);
	//printf("%d, %d, %d, %d\n", range_node_array[0].port, range_node_array[1].port, range_node_array[2].port, range_node_array[3].port);
	//printf("%u, %u, %u, %u\n", range_node_array[0].end_index, range_node_array[1].end_index, range_node_array[2].end_index, range_node_array[3].end_index);
	shuffle(query, num_entry); 
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			begin=rdtsc();
			search_range(query[i].ip, 0, range_node_index-1, query[i].len);
			end=rdtsc();
			if(clock[i]>(end-begin))
				clock[i]=(end-begin);
		}
	}
	total=0;
	for(j=0;j<num_query;j++)
		total+=clock[j];
	printf("Avg. Search: %llu\n",total/num_query);
	printf("lost case : %d\n", lost_case);
	CountClock();
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
