#include<stdlib.h>
#include<stdio.h>
#include<string.h>

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
struct prefix_node{
	unsigned int ip;
	int len;
	int port;
};

struct prefix_node* create_prefix_node(){
	struct prefix_node* temp = NULL;
	temp = (struct prefix_node*)malloc(sizeof(struct prefix_node));
	temp -> ip = 0;
	temp -> len = 0;
	temp -> port = 0;
	return temp;
}
struct prefix_node* binary_array;
////////////////////////////////////////////////////////////////////////////////////
struct list{//structure of binary trie
	unsigned int port;
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
int lost_case = 0, match_case = 0, total_case = 0;
int num_leaf_node = 0;
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
	if(len == 0)
		ptr -> port = nexthop;
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
		if(current->port!=256){
			temp=current;
			return;
		}
			
		if(ip&(1<<j)){
			current=current->right;
		}
		else{
			current=current->left; 
		}
	}
	if(temp==NULL){
		printf("default: %u\n", ip);
	}
	  
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
	clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		query[num_query].ip=ip;
		query[num_query].len=len;
		query[num_query].port=nexthop;
		clock[num_query++]=10000000;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void create(){
	int i;
	root=create_node();
	begin=rdtsc();
	for(i=0;i<num_entry;i++)
		add_node(table[i].ip,table[i].len,table[i].port);
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
	if(ptr -> port != 256){
		if(ptr -> left != NULL && ptr -> right == NULL){
			if(ptr -> left -> port == 256){
				ptr -> left -> port = ptr -> port;
			}
			ptr -> right = create_node();
			ptr -> right -> port = ptr -> port;
			ptr -> port = 256;
				
		}
		else if(ptr -> right != NULL && ptr -> left == NULL){
			if(ptr -> right -> port == 256){
				ptr -> right -> port = ptr -> port;
			}
			ptr -> left = create_node();
			ptr -> left -> port = ptr -> port;
			ptr -> port = 256;
		}
		else if(ptr -> right != NULL && ptr -> left != NULL){
			if(ptr -> right -> port == 256){
				ptr -> right -> port = ptr -> port;
			}
			if(ptr -> left -> port == 256){
				ptr -> left -> port = ptr -> port;
			}
			ptr -> port = 256;
		}
	}
	leaf_push(ptr -> left);
	leaf_push(ptr -> right);
}

void count_leaf_node(btrie ptr){
	if(ptr == NULL)
		return;
	if(ptr -> port != 256 && ptr -> left == NULL && ptr -> right == NULL)	
		num_leaf_node++;
	count_leaf_node(ptr -> right);
	count_leaf_node(ptr -> left);


}

void rotation(btrie ptr, int level){

	if(ptr == NULL)	return;
	if(level == 31){
		if(ptr -> left != NULL && ptr -> right == NULL){
			if(ptr -> left -> port != 256){
				ptr -> port = ptr -> left -> port;
				ptr -> right = create_node();
				ptr -> right -> port = ptr -> left -> port;
				free(ptr -> left);
				ptr -> left = NULL;
				num_leaf_node++;
			}
		}
		else if(ptr -> right != NULL && ptr -> left == NULL){
			if(ptr -> right -> port != 256){
				ptr -> port = ptr -> right -> port;
				num_leaf_node++;
			}
		}
		else if(ptr -> right != NULL && ptr -> left != NULL){
			if(ptr -> left -> port != 256){
				ptr -> port = ptr -> left -> port;
				free(ptr -> left);
				ptr -> left = NULL;
			}
		}
		return;
	}

	rotation(ptr -> left, level + 1);
	rotation(ptr -> right, level + 1);
}

int build_array_index = 0;
void inorder_build_array(btrie ptr, int level, unsigned int value){
	if(ptr == NULL){
		return;
	}
	inorder_build_array(ptr -> left, level + 1, value << 1);

	if (ptr -> port != 256)
	{	if(level != 32){
			unsigned int temp_ip = ((value << 1) + 1) << (31 - level);
			binary_array[build_array_index].ip = temp_ip;
			binary_array[build_array_index].len = level;
			binary_array[build_array_index].port = ptr -> port;
			++build_array_index; 
		}
		else
		{
			binary_array[build_array_index].ip = 0;
			binary_array[build_array_index].len = level;
			binary_array[build_array_index].port = ptr -> port;
			++build_array_index;
		}
	}

	inorder_build_array(ptr -> right, level + 1, (value << 1) + 1);
}

void search_prefix_n(unsigned int input_ip, int left_index, int right_index){
	unsigned int left_ip, right_ip, middle_ip;
	int middle;
	do
	{
		left_ip = binary_array[left_index].ip;
		right_ip = binary_array[right_index].ip;

		if(left_ip == 0 && left_index != 0){
			left_ip = binary_array[left_index - 1].ip;
		}
		

		if(right_ip == 0){
			right_ip = binary_array[right_index - 1].ip;
		}
		
		
		if(left_index + 1 == right_index){
			if(binary_array[left_index].len > binary_array[right_index].len){
				if(left_ip >> (32 - binary_array[left_index].len) == input_ip >> (32 - binary_array[left_index].len))	return;
				if(right_ip >> (32 - binary_array[right_index].len) == input_ip >> (32 - binary_array[right_index].len))	return;
			}
			else
			{
				if(right_ip >> (32 - binary_array[right_index].len) == input_ip >> (32 - binary_array[right_index].len))	return;
				if(left_ip >> (32 - binary_array[left_index].len) == input_ip >> (32 - binary_array[left_index].len))	return;
			}
			//printf("not found\n");
			//printf("input ip : %u, len for check : %d\n", input_ip, len_for_check);
			lost_case++;
			return;
		}
		middle = (left_index + right_index)/2;
		middle_ip = binary_array[middle].ip;
		if(middle_ip == 0){
			middle_ip = binary_array[middle-1].ip;
		}

		
		if(middle_ip >> (32 - binary_array[middle].len) == input_ip >> (32 - binary_array[middle].len))	return;
		if(middle_ip >> (32 - binary_array[middle].len) < input_ip >> (32 - binary_array[middle].len)){
			left_index = middle;
			//search_prefix_n(input_ip, middle, right_index);
		} 
		else
		{
			right_index = middle;
			//search_prefix_n(input_ip, left_index, middle);
		}
	} while (1);
	

	
	return;

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


	begin = rdtsc();
	leaf_push(root);
	count_leaf_node(root);
	rotation(root, 0);

	printf("num leaf node : %d\n", num_leaf_node);
	binary_array = (struct prefix_node*)malloc(sizeof(struct prefix_node) * num_leaf_node);
	printf("Total memory requirement: %d KB\n",(((sizeof(struct prefix_node) - sizeof(int)) * num_leaf_node)/1024));

	inorder_build_array(root, 0, 0);
	end = rdtsc();
	printf("build_array_index : %d\n", build_array_index);
	printf("Avg. Build: %llu\n",(end-begin)/num_entry);
	printf("build end\n");

	shuffle(query, num_entry); 
	////////////////////////////////////////////////////////////////////////////
	total_case = 100 * num_entry;
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			begin=rdtsc();
			search_prefix_n(query[i].ip, 0, build_array_index-1);
			end=rdtsc();
			if(clock[i]>(end-begin))
				clock[i]=(end-begin);
		}
	}
	total=0;
	for(j=0;j<num_query;j++)
		total+=clock[j];
	printf("Avg. Search: %llu\n",total/num_query);
	printf("lost case : %d, match_case : %d, total case : %d\n", lost_case, match_case, total_case);
	CountClock();
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
