#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
////////////////////////////////////////////////////////////////////////////////////
struct ENTRY{
	unsigned int ip;
	unsigned int len;
	unsigned int port;
};
////////////////////////////////////////////////////////////////////////////////////
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
////////////////////////////////////////////////////////////////////////////////////
struct small_box{
	int exist;
	int height;
};

struct box{
	int exist;
	struct small_box *down;
};
struct box big_box[65536];
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
int size_16_x = 0;
int size_16_X_compress = 0;
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
	temp->port=256;//default port
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
int max(int num1, int num2)
{
    return (num1 > num2 ) ? num1 : num2;
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
void search(struct ENTRY array){
	int i, j, answer = 0;
	//
	if(array.len <= 16){
		answer = big_box[(int)((array.ip) >> 16)].exist;
	}
	else
	{	
		if(big_box[(int)((array.ip) >> 16)].down != NULL){
			unsigned int index = (array.ip << 16 >> 16) >> (32 - array.len) << (32 - array.len) >> (16 - big_box[(int)((array.ip) >> 16)].down[0].height);
			answer = big_box[(int)((array.ip) >> 16)].down[(int)index].exist;
		}
		//unsigned int index = (array.ip << 16 >> 16) >> ((32 - array.len) + (32 - array.len - big_box[(array.ip) >> 16].down[0].height));
		//i = index;
		//if(index > pow(2, big_box[(array.ip) >> 16].down->height))
		//	printf("error occur\n");
	}
	

	/*
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
	*/
	/*
	if(answer==0){
 		printf("not here\n");
		printf("array.ip= %u\n", array.ip);
		printf("array.len= %u\n", array.len);
		printf("index= %d\n", i);
	}
	 */
	//else
	//	printf("here\n");
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
		query[num_query].port = nexthop;
		query[num_query].len = len;
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
int find_max_height(btrie ptr){
	if(ptr == NULL)
		return 0;
	return max(find_max_height(ptr->left) + 1, find_max_height(ptr->right) + 1);
}
////////////////////////////////////////////////////////////////////////////////////
void build_small_box(btrie ptr, int level, unsigned int index, int height, struct small_box * box){
	if(ptr == NULL)
		return;
	if(ptr->port != 256){
			int range_num = (index + 1) << (height - level);
			for (int i = index << (height - level); i < range_num; i++)
			{
				box[i].exist = 1;
			}
			size_16_X_compress += sizeof(struct small_box);
		}
		if(ptr->left != NULL){
			build_small_box(ptr->left, level + 1, index << 1, height, box);
		}
		if (ptr->right != NULL){
			build_small_box(ptr->right, level + 1, (index << 1) + 1, height, box);
		}
}
////////////////////////////////////////////////////////////////////////////////////
void build_box(btrie ptr, int level, unsigned int index){
	if(ptr == NULL)
		return;
	if(level < 16){
		if(ptr->port != 256){
			int range_num = (index + 1) << (16 - level);
			for (int i = index << (16 - level); i < range_num; i++)
			{
				big_box[i].exist = 1;
			}
		}
		if(ptr->left != NULL){
			build_box(ptr->left, level+1, index << 1);
		}
		if (ptr->right != NULL){
			build_box(ptr->right, level+1, (index << 1) + 1);
		}
		
		
	}
	else
	{
		int max_height = max(find_max_height(ptr->left), find_max_height(ptr->right));
		if(ptr->port != 256){
				int range_num = (index + 1) << (16 - level);
				for (int i = index << (16 - level); i < range_num; i++)
				{
					big_box[i].exist = 1;
				}
			}
		if(max_height > 0){
			int small_box_num = pow(2, max_height);
			
			if(big_box[(int)index].down == NULL){
				big_box[(int)index].down = (struct small_box *)malloc(small_box_num * sizeof(struct small_box));
				size_16_x += small_box_num * sizeof(struct small_box);
				for (int i = 0; i < small_box_num; i++)
				{
					big_box[(int)index].down[i].exist = 0;
					big_box[(int)index].down[i].height = max_height;
				}
			}
			if(ptr->left != NULL){
				build_small_box(ptr->left, 1, 0, max_height, big_box[(int)index].down);
			}
			if (ptr->right != NULL){
				build_small_box(ptr->right, 1, 1, max_height, big_box[(int)index].down);
			}
		}
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

	for (i = 0; i < 65536; i++)
	{
		big_box[i].down = NULL;
		big_box[i].exist = 0;
	}


	

	printf("Avg. Insert: %llu\n",(end-begin)/num_entry);
	printf("number of nodes: %d\n",num_node);
	printf("Total memory requirement: %d KB\n",((num_node*sizeof(struct list))/1024));

	begin =rdtsc();
	build_box(root, 0, 0);
	end = rdtsc();
	printf("16 x build time = %d\n", (end - begin)/num_entry);
	printf("size of 16 X = %d KB\n", size_16_x/1024);
	printf("size of 16 X compress = %d KB\n", size_16_X_compress/1024);

	shuffle(query, num_entry); 

	printf("end shuffle\n");
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			begin=rdtsc();
			search(query[i]);
			end=rdtsc();
			if(clock[i]>(end-begin))
				clock[i]=(end-begin);
		}
	}
	total=0;
	for(j=0;j<num_query;j++)
		total+=clock[j];
	printf("Avg. Search: %llu\n",total/num_query);
	CountClock();
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
