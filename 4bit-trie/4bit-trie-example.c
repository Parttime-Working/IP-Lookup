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
struct list{//structure of binary trie
	unsigned int port;
	struct list *link[16];
};
typedef struct list node;
typedef node *btrie;
////////////////////////////////////////////////////////////////////////////////////
struct segTable{//structure of segmentation table
	unsigned int port;
	char len;
	btrie *ptr;
};
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
struct ENTRY *query;
int num_entry=0;
int num_query=0;
struct ENTRY *table;
struct ENTRY *ins;
int num_insert=0;
int N=0;//number of nodes
unsigned long long int begin,end,total=0;
unsigned long long int *clock;
int num_node=0;//total number of nodes in the binary trie
int memory_access=0;
struct segTable *sTable;
////////////////////////////////////////////////////////////////////////////////////
btrie create_node(){
	btrie temp;
	num_node++;
	temp=(btrie)malloc(sizeof(node));
	for(int i=0;i<16;i++){
		temp->link[i] = NULL;
	}
	temp->port=256;//default port
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned int ip,unsigned char len,unsigned char nexthop){
	int i, x;
	unsigned int index;
	btrie tmp;
	index = ip>>16;
	if(len <= 16){
		index = index >> (16-len);
		unsigned int index_down = index<<(16-len);
		unsigned int index_up = index;
		for(i=0; i<16-len; i++){
			index_up = (index_up<<1) + 1;
		}
		for(i=index_down; i<=index_up; i++){
			if(sTable[i].port == 256){
				sTable[i].port = nexthop;
				sTable[i].len = len;
			}
			else{
				if(sTable[i].len < len){
					sTable[i].port = nexthop;
					sTable[i].len = len;
				}
			}
		}
	}
	else{
		if(sTable[index].ptr == NULL) 
			sTable[index].ptr = create_node();
		tmp = sTable[index].ptr;
		for(i=0;i<len-16;i+=4){
			x = ((ip & 0x0000FFFF) >> (12-i)) & 0x0000000F;
			if(tmp->link[x]==NULL)
				tmp->link[x]=create_node();
			tmp = tmp->link[x];
			if(((i+4)>=(len-16))&&(tmp->port==256))
				tmp->port=nexthop;	
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
	int j, x;
	int index = ip >> 16;
	btrie current = sTable[index].ptr;
	memory_access++;
	int temp = 256;
	if(current != NULL){
		for(j=16;j>0;j-=4){
			if(current == NULL) break;
			memory_access++;
			x = (ip>>(j-4)) & 0x0000000F;
			current = current->link[x];
			if((current != NULL) && (current->port != 256)) 
				temp=current->port;
		}
	}
	if(temp == 256 && sTable[index].port != 256)
		temp = sTable[index].port;
	/*if(temp == 256)
		printf("default\n");*/
	/*else
	  printf("%u\n",temp->port);*/
}
////////////////////////////////////////////////////////////////////////////////////
void insert(unsigned int ip,unsigned char len,unsigned char nexthop){
	int i, j, x;
	int index = ip >> 16;
	int over = 0;
	if(len <= 16){
		index = index >> (16-len);
		unsigned int index_down = index<<(16-len);
		unsigned int index_up = index;
		for(i=0; i<16-len; i++){
			index_up = (index_up<<1) + 1;
		}
		for(i=index_down; i<=index_up; i++){
			if(sTable[i].port == 256){
				sTable[i].port = nexthop;
				sTable[i].len = len;
			}
			else{
				if(sTable[i].len < len){
					sTable[i].port = nexthop;
					sTable[i].len = len;
				}
			}
		}
	}
	else{
		if(sTable[index].ptr == NULL) 
			sTable[index].ptr = create_node();
		btrie tmp = sTable[index].ptr;
		for(i=0;i<len-16;i+=4){
			x = ((ip & 0x0000FFFF) >> (12-i)) & 0x0000000F;
			if(tmp->link[x]==NULL)
				tmp->link[x]=create_node();
			tmp = tmp->link[x];
			if(((i+4)>=(len-16))&&(tmp->port==256))
				tmp->port=nexthop;
		}
	}
}		
////////////////////////////////////////////////////////////////////////////////////
void set_insert(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip,nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		num_insert++;
	}
	rewind(fp);
	ins=(struct ENTRY *)malloc(num_insert*sizeof(struct ENTRY));
	num_insert=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		ins[num_insert].ip=ip;
		ins[num_insert].port=nexthop;
		ins[num_insert++].len=len;
	}
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
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY ));
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
	query=(struct ENTRY *)malloc(num_query*sizeof(struct ENTRY ));
	clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		query[num_query].ip=ip;
		clock[num_query++]=10000000;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void create(){
	int i;
	root=create_node();
	sTable = (struct segTable *)malloc(sizeof(struct segTable) * 65536);
	for(i=0; i<65536; i++){
		sTable[i].ptr = NULL;
		sTable[i].port = 256;
	}
	begin=rdtsc();
	for(i=0;i<num_entry;i++)
		add_node(table[i].ip,table[i].len,table[i].port);
	end=rdtsc();
}
////////////////////////////////////////////////////////////////////////////////////
/*void count_node(btrie r){
	if(r==NULL)
		return;
	count_node(r->link);
	N++;
	count_node(r->link);
}*/
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
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
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
}
////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char *argv[]){
	/*if(argc!=3){
		printf("Please execute the file as the following way:\n");
		printf("%s  routing_table_file_name  query_table_file_name\n",argv[0]);
		exit(1);
	}*/
	int i,j;
	set_table(argv[1]);
	set_insert(argv[1]);
	set_query(argv[2]);
	create();
	printf("Avg. Insert: %llu\n",(end-begin)/num_entry);
	printf("number of nodes: %d\n",num_node); 
	printf("Total memory requirement: %d KB\n",((65*num_node)+(6*65536))/1024);

	//shuffle(query, num_entry);
	////////////////////////////////////////////////////////////////////////////
	begin=rdtsc();
	for(i=0;i<num_insert;i++)
		insert(ins[i].ip, ins[i].len, ins[i].port);
	end=rdtsc();
	printf("Avg. update: %llu\n",(end-begin)/num_insert);
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<1;j++){
		for(i=0;i<num_query;i++){
			begin=rdtsc();
			search(query[i].ip);
			end=rdtsc();
			if(clock[i]>(end-begin))
				clock[i]=(end-begin);
		}
	}
	total=0;
	for(j=0;j<num_query;j++)
		total+=clock[j];
	printf("Avg. Search: %llu\n",total/num_query);
	printf("memory_access = %f \n",(float)memory_access/((float)num_query));
	CountClock();
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
