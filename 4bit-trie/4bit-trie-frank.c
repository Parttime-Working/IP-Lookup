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
	struct list *node_0000,*node_0001, *node_0010, *node_0011, *node_0100, *node_0101, *node_0110, *node_0111, *node_1000, *node_1001, *node_1010, *node_1011, *node_1100, *node_1101, *node_1110, *node_1111;
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
	temp->node_0000=NULL;
	temp->node_0001=NULL;
	temp->node_0010=NULL;
	temp->node_0011=NULL;
	temp->node_0100=NULL;
	temp->node_0101=NULL;
	temp->node_0110=NULL;
	temp->node_0111=NULL;
	temp->node_1000=NULL;
	temp->node_1001=NULL;
	temp->node_1010=NULL;
	temp->node_1011=NULL;
	temp->node_1100=NULL;
	temp->node_1101=NULL;
	temp->node_1110=NULL;
	temp->node_1111=NULL;
	temp->port=256;//default port
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned int ip,unsigned char len,unsigned char nexthop){
	btrie ptr=root;
	int i;
	for(i=0;i<len;i+=4){
		
		if(ip & (1 << 31-i) && ip & (1 << 31 - (i+1)) && ip & (1 << 31 - (i+2)) && ip & (1 << 31 - (i+3))){
			if(ptr -> node_1111 == NULL)
				ptr -> node_1111 = create_node();
			ptr = ptr -> node_1111;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(ip & (1 << 31-i) && ip & (1 << 31 - (i+1)) && ip & (1 << 31 - (i+2)) && ~(ip ^ (0 << 31 - (i+3)))){
			if(ptr -> node_1110 == NULL)
				ptr -> node_1110 = create_node();
			ptr = ptr -> node_1110;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(ip & (1 << 31-i) && ip & (1 << 31 - (i+1)) && ~(ip ^ (0 << 31 - (i+2))) && ip & (1 << 31 - (i+3))){
			if(ptr -> node_1101 == NULL)
				ptr -> node_1101 = create_node();
			ptr = ptr -> node_1101;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(ip & (1 << 31-i) && ip & (1 << 31 - (i+1)) && ~(ip ^ (0 << 31 - (i+2))) && ~(ip ^ (0 << 31 - (i+3)))){
			if(ptr -> node_1100 == NULL)
				ptr -> node_1100 = create_node();
			ptr = ptr -> node_1100;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(ip & (1 << 31-i) && ~(ip ^ (0 << 31 - (i+1))) && ip & (1 << 31 - (i+2)) && ip & (1 << 31 - (i+3))){
			if(ptr -> node_1011 == NULL)
				ptr -> node_1011 = create_node();
			ptr = ptr -> node_1011;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(ip & (1 << 31-i) && ~(ip ^ (0 << 31 - (i+1))) && ip & (1 << 31 - (i+2)) && ~(ip ^ (0 << 31 - (i+3)))){
			if(ptr -> node_1010 == NULL)
				ptr -> node_1010 = create_node();
			ptr = ptr -> node_1010;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(ip & (1 << 31-i) && ~(ip ^ (0 << 31 - (i+1))) && ~(ip ^ (0 << 31 - (i+2))) && ip & (1 << 31 - (i+3))){
			if(ptr -> node_1001 == NULL)
				ptr -> node_1001 = create_node();
			ptr = ptr -> node_1001;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(ip & (1 << 31-i) && ~(ip ^ (0 << 31 - (i+1))) && ~(ip ^ (0 << 31 - (i+2))) && ~(ip ^ (0 << 31 - (i+3)))){
			if(ptr -> node_1000 == NULL)
				ptr -> node_1000 = create_node();
			ptr = ptr -> node_1000;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(~(ip ^ (0 << 31-i)) && ip & (1 << 31 - (i+1)) && ip & (1 << 31 - (i+2)) && ip & (1 << 31 - (i+3))){
			if(ptr -> node_0111 == NULL)
				ptr -> node_0111 = create_node();
			ptr = ptr -> node_0111;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(~(ip ^ (0 << 31-i)) && ip & (1 << 31 - (i+1)) && ip & (1 << 31 - (i+2)) && ~(ip ^ (0 << 31 - (i+3)))){
			if(ptr -> node_0110 == NULL)
				ptr -> node_0110 = create_node();
			ptr = ptr -> node_0110;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(~(ip ^ (0 << 31-i)) && ip & (1 << 31 - (i+1)) && ~(ip ^ (0 << 31 - (i+2))) && ip & (1 << 31 - (i+3))){
			if(ptr -> node_0101 == NULL)
				ptr -> node_0101 = create_node();
			ptr = ptr -> node_0101;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(~(ip ^ (0 << 31-i)) && ip & (1 << 31 - (i+1)) && ~(ip ^ (0 << 31 - (i+2))) && ~(ip ^ (0 << 31 - (i+3)))){
			if(ptr -> node_0100 == NULL)
				ptr -> node_0100 = create_node();
			ptr = ptr -> node_0100;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(~(ip ^ (0 << 31-i)) && ~(ip ^ (0 << 31 - (i+1))) && ip & (1 << 31 - (i+2)) && ip & (1 << 31 - (i+3))){
			if(ptr -> node_0011 == NULL)
				ptr -> node_0011 = create_node();
			ptr = ptr -> node_0011;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(~(ip ^ (0 << 31-i)) && ~(ip ^ (0 << 31 - (i+1))) && ip & (1 << 31 - (i+2)) && ~(ip ^ (0 << 31 - (i+3)))){
			if(ptr -> node_0010 == NULL)
				ptr -> node_0010 = create_node();
			ptr = ptr -> node_0010;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else if(~(ip ^ (0 << 31-i)) && ~(ip ^ (0 << 31 - (i+1))) && ~(ip ^ (0 << 31 - (i+2))) && ip & (1 << 31 - (i+3))){
			if(ptr -> node_0001 == NULL)
				ptr -> node_0001 = create_node();
			ptr = ptr -> node_0001;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
		}
		else {
			if(ptr -> node_0000 == NULL)
				ptr -> node_0000 = create_node();
			ptr = ptr -> node_0000;
			if((i + 4 >= len) && (ptr->port == 256))
				ptr -> port = nexthop;
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
	int i, check = 0;
	btrie current=root,temp=NULL;
	for(i=31;i>=(-1);i-=4){
		if(current==NULL)
			break;
		if(current->port!=256)
			check = 1;

		if(ip & (1 << i) && ip & (1 << (i-1)) && ip & (1 << (i-2)) && ip & (1 << (i-3))){
			current = current -> node_1111;
		}
		else if(ip & (1 << i) && ip & (1 << (i-1)) && ip & (1 << (i-2)) && ~(ip ^ (0 << (i-3)))){
			current = current -> node_1110;
		}
		else if(ip & (1 << i) && ip & (1 << (i-1)) && ~(ip ^ (0 << (i-2))) && ip & (1 << (i-3))){
			current = current -> node_1101;
		}
		else if(ip & (1 << i) && ip & (1 << (i-1)) && ~(ip ^ (0 << (i-2))) && ~(ip ^ (0 << (i-3)))){
			current = current -> node_1100;
		}
		else if(ip & (1 << i) && ~(ip ^ (0 << (i-1))) && ip & (1 << (i-2)) && ip & (1 << (i-3))){
			current = current -> node_1011;
		}
		else if(ip & (1 << i) && ~(ip ^ (0 << (i-1))) && ip & (1 << (i-2)) && ~(ip ^ (0 << (i-3)))){
			current = current -> node_1010;
		}
		else if(ip & (1 << i) && ~(ip ^ (0 << (i-1))) && ~(ip ^ (0 << (i-2))) && ip & (1 << (i-3))){
			current = current -> node_1001;
		}
		else if(ip & (1 << i) && ~(ip ^ (0 << (i-1))) && ~(ip ^ (0 << (i-2))) && ~(ip ^ (0 << (i-3)))){
			current = current -> node_1000;
		}
		else if(~(ip ^ (0 << i)) && ip & (1 << (i-1)) && ip & (1 << (i-2)) && ip & (1 << (i-3))){
			current = current -> node_0111;
		}
		else if(~(ip ^ (0 << i)) && ip & (1 << (i-1)) && ip & (1 << (i-2)) && ~(ip ^ (0 << (i-3)))){
			current = current -> node_0110;
		}
		else if(~(ip ^ (0 << i)) && ip & (1 << (i-1)) && ~(ip ^ (0 << (i-2))) && ip & (1 << (i-3))){
			current = current -> node_0101;
		}
		else if(~(ip ^ (0 << i)) && ip & (1 << (i-1)) && ~(ip ^ (0 << (i-2))) && ~(ip ^ (0 << (i-3)))){
			current = current -> node_0100;
		}
		else if(~(ip ^ (0 << i)) && ~(ip ^ (0 << (i-1))) && ip & (1 << (i-2)) && ip & (1 << (i-3))){
			current = current -> node_0011;
		}
		else if(~(ip ^ (0 << i)) && ~(ip ^ (0 << (i-1))) && ip & (1 << (i-2)) && ~(ip ^ (0 << (i-3)))){
			current = current -> node_0010;
		}
		else if(~(ip ^ (0 << i)) && ~(ip ^ (0 << (i-1))) && ~(ip ^ (0 << (i-2))) && ip & (1 << (i-3))){
			current = current -> node_0001;
		}
		else {
			current = current -> node_0000;
		}
	}
	//if(check==0)
	//  printf("not here\n");
	 /*else
	  printf("here\n");*/
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
		clock[num_query++]=10000000;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void create(){
	int i;
	root=create_node();
	begin=rdtsc();
	for(i=0;i<num_entry/10*9;i++)
		add_node(table[i].ip,table[i].len,table[i].port);
	end=rdtsc();
	printf("Avg. Build: %llu\n",(end-begin)/(num_entry/10*9));

	begin=rdtsc();
	for(i=num_entry/10*9;i<num_entry;i++)
		add_node(table[i].ip,table[i].len,table[i].port);
	end=rdtsc();
	
	printf("Avg. Insert: %llu\n",(end-begin)/(num_entry/10));

}
////////////////////////////////////////////////////////////////////////////////////
void count_node(btrie r){
	if(r==NULL)
		return;
	N++;
	count_node(r->node_1111);
	count_node(r->node_1110);
	count_node(r->node_1101);
	count_node(r->node_1100);
	count_node(r->node_1011);
	count_node(r->node_1010);
	count_node(r->node_1001);
	count_node(r->node_1000);
	count_node(r->node_0111);
	count_node(r->node_0110);
	count_node(r->node_0101);
	count_node(r->node_0100);
	count_node(r->node_0011);
	count_node(r->node_0010);
	count_node(r->node_0001);
	count_node(r->node_0000);
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
	printf("Total memory requirement: %d KB\n",((num_node*sizeof(struct list))/1024));

	shuffle(query, num_entry); 
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<100;j++){
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
	CountClock();
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
