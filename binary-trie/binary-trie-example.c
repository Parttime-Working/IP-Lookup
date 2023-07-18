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
/*inline unsigned long long int rdtsc()
{
	unsigned long long int x;
	asm   volatile ("rdtsc" : "=A" (x));
	return x;
}*/
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
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
typedef struct list_element{
	unsigned int port;
	btrie root;
	struct ENTRY * list_table;
	int num;
} tentry;
tentry * segement_table;
int segement_array[65536];
unsigned int *query;
int num_entry=0;
int num_query=0;
struct ENTRY *table;
int N=0;//number of nodes
int max_segment_size=0;
unsigned long long int begin,end,total=0;
unsigned long long int *clock1;
int count=0;
int num_node=0;//total number of nodes in the binary trie
int mem_access=0;
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
void add_node(unsigned int ip,unsigned char len,unsigned char nexthop){  //16-bits segment
	btrie ptr=root;
	int i = 0;
	len = len - 16;
	for(i=0;i<len;i++){
		if(ip&(1<<(15-i))){
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
// void add_node(unsigned int ip,unsigned char len,unsigned char nexthop){ // non-segment
// 	btrie ptr=root;
// 	int i;
// 	for(i=0;i<len;i++){
// 		if(ip&(1<<(31-i))){
// 			if(ptr->right==NULL)
// 				ptr->right=create_node(); // Create Node
// 			ptr=ptr->right;
// 			if((i==len-1)&&(ptr->port==256)){
// 			//	count++;
// 				ptr->port=nexthop;
// 			}
				
// 		}
// 		else{
// 			if(ptr->left==NULL)
// 				ptr->left=create_node();
// 			ptr=ptr->left;
// 			if((i==len-1)&&(ptr->port==256)){
// 			//		count++;
// 				ptr->port=nexthop;
// 			}
				
// 		}
// 	}
// }
////////////////////////////////////////////////////////////////////////////////////
unsigned long long int insert_time[65536]={0};
void build_segment_create( int pos ){
	int i;
	root=create_node();
	begin=rdtsc();
	for(i=0;i<segement_table[pos].num;i++){
		add_node(segement_table[pos].list_table[i].ip,segement_table[pos].list_table[i].len,segement_table[pos].list_table[i].port);
	}
	end=rdtsc();
	insert_time[pos]=end-begin;
	segement_table[pos].root = root;
}
////////////////////////////////////////////////////////////////////////////////////
void build_segement_table(){
	for(int i=0;i<65536;i++)
		segement_array[i]=256;
	int num[65536]={0};
	unsigned int ip = 0;
	unsigned int ip_max = 0;
	int prefix_dis[16] = {0};
	for(int i=0;i<num_entry;i++){
		unsigned int temp_min = 0,temp_max=0;
		if(table[i].len > 16){
			for(int k = 0; k < 16; k++ ) {
				if( table[i].ip & (1<< (31-k)) ){
					temp_min += 1 << (15-k);
				}
			}
			num[temp_min]++;
		}
	}
	max_segment_size = num[0];
	int max_index;
	for(int i=1;i<65536;i++){
		if(num[i] > max_segment_size){
			max_segment_size = num[i];
			max_index=i;
		}
	}
	printf("max_segment_size =%d, index = %d\n",max_segment_size,max_index);
	inition();
	for(int i = 0; i < num_entry; i++ ){
		if( table[i].len > 16 ){ 
			ip = table[i].ip>>16;
			segement_table[ip].list_table[segement_table[ip].num].len = table[i].len;
			segement_table[ip].list_table[segement_table[ip].num].ip = table[i].ip;
			segement_table[ip].num++;
		}
		else if(table[i].len == 16){
			ip = table[i].ip>>16;
			segement_array[ip]=1;
		}
		else{ 
			ip = table[i].ip>>16;
			ip_max = ip +(1<<(16-table[i].len))-1;
			for(int j=ip;j<=ip_max;j++){
				/*segement_table[j].list_table[segement_table[j].num].len = table[j].len;
				segement_table[j].list_table[segement_table[j].num].ip = table[j].ip;
				segement_table[j].num++;*/
				segement_array[j]=1;
			}
			//segement_array[ip]=1;
		}


	}
	int non_empty_entry = 0;
	int empty_entry = 0;

	for(int i = 1; i < 65536; i++ ){
		if ( segement_table[i].num == 0 )
			empty_entry++;
		else{
			non_empty_entry++;
			build_segment_create(i);
		}
	}
	printf("empty_segment =%d , non_empty_segment =%d\n",empty_entry,non_empty_entry);

}
void inition(){
	segement_table= (tentry*)malloc( 65536* sizeof(tentry) );
	int i;
	for( i = 0; i < 65536; i++ ){
		segement_table[i].num = 0;
		segement_table[i].port = 256;
		segement_table[i].root = NULL;
		segement_table[i].list_table = (struct ENTRY *)malloc( (max_segment_size+1) * sizeof(struct ENTRY));
	}
}
///////////////////////////////////////////////////////////////////////////////////////
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
int find,success=0,fail=0;
void search(unsigned int ip,btrie root){
	int j;
	find=0;
	btrie current=root,temp=NULL;
	for(j=31;j>=(-1);j--){
		if(current==NULL)
			break;
		if(current->port!=256){
			temp=current;
			find=1;
		}
		if(ip&(1<<j)){
			current=current->right;
		}
		else{
			current=current->left; 
		}
	}
	
}
void search_segment(unsigned int ip){
	unsigned int ip_temp;
	int j;
	find=0;
	ip_temp = ip>>16;
	btrie current=segement_table[ip_temp].root,temp=NULL;
	mem_access++;
	for(j=15;j>=(-1);j--){
		if(current==NULL)
			break;
		if(current->port!=256){
			temp=current;
			find=1;
		}
		if(ip&(1<<j)){
			current=current->right;
		}
		else{
			current=current->left; 
		}
		mem_access++;
	}
	if(find)
		success++;
	else{
		if(segement_array[ip_temp]!=256)
			success++;
		else
			fail++;
	}

}
////////////////////////////////////////////////////////////////////////////////////
int less16=0,equal16=0;
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
		if(len<16)
			less16++;
		else if(len==16)
			equal16++;
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
	query=(unsigned int *)malloc(num_query*sizeof(unsigned int));
	clock1=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		query[num_query]=ip;
		clock1[num_query++]=10000000;
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
void count_node(btrie r){ //­pºânexthopªºnode­Ó¼Æ 
//	printf("aa\n");
	if(r==NULL)
		return;
	count_node(r->left);
	count_node(r->right);
	if(r->port!=256)
		count++;
	
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
		if(clock1[i] > MaxClock) MaxClock = clock1[i];
		if(clock1[i] < MinClock) MinClock = clock1[i];
		if(clock1[i] / 100 < 50) NumCntClock[clock1[i] / 100]++;
		else NumCntClock[49]++;
	}
	printf("(MaxClock, MinClock) = (%5llu, %5llu)\n", MaxClock, MinClock);
	FILE *fp;
	fp=fopen("cycle.txt","w");
	for(i = 0; i < 50; i++)
	{	
		fprintf(fp,"%d\n",NumCntClock[i]);
		printf("%d\n", NumCntClock[i]);
	}
	fclose(fp);
	return;
}
void shuffle(int *array, size_t n){
    //亂數前置
    srand(time(NULL));
    if (n > 1){
        size_t i;
        for (i = 0; i<n; i++){
            size_t j = rand()/(RAND_MAX/(n));
            unsigned int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char *argv[]){


	unsigned long long int total_insert=0;
	int i,j;
	/*set_query("final_output.txt");
	set_table("final_output.txt");*/
	set_query(argv[2]);
	set_table(argv[1]);
	printf("# of prefix = %d\n",num_entry);
	printf("# of prefix is len < 16 = %d\n",less16);
	printf("# of prefix is len = 16 = %d\n",equal16);
	//create();
	build_segement_table();
	//btrie ptr=root;
	//count_node(ptr);
	for(int i=1;i<65536;i++){
		total_insert+=insert_time[i];
	}
	shuffle(query, num_query);
	printf("Avg. Inseart: %llu\n",total_insert/num_entry);
	printf("Avg. Inseart: %llu\n",(end-begin)/num_entry);
	printf("number of nodes: %d\n",num_node);
	printf("Total memory requirement: %d KB\n",((num_node*12)/1024));
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			begin=rdtsc();
			//search(query[i],root);
			search_segment(query[i]);
			end=rdtsc();
			if(clock1[i]>(end-begin))
				clock1[i]=(end-begin);
		}
	}
	printf("mem_access =%d, Avg mem_access = %f\n", mem_access,(float)mem_access/(float)num_query );
	total=0;
	//printf("success= %d, fail =%d\n",success,fail);
	for(j=0;j<num_query;j++)
		total+=clock1[j];
	printf("Avg. Search: %llu\n",total/num_query);
	CountClock();
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
