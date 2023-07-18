#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <stdint.h>
#define rdtscp __builtin_ia32_rdtscp
unsigned int *a;
struct ENTRY{
	unsigned long long int upper_ip;
	unsigned long long int lower_ip;
	unsigned char len;
	unsigned char port;
};
//#define uint128_t __uint128_t
struct ENTRY *table;
struct ENTRY *query;
struct list{
	unsigned int port;
	struct list *left,*right;
	unsigned long long int upper_ip;
	unsigned long long int lower_ip;
	unsigned char len;
	int layer;
};
unsigned long long int begin,end,total=0;
unsigned long long int begin2,end2;
unsigned long long int *clock;
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
typedef struct list node;
typedef node *btrie;
btrie root;


typedef struct list_element{
	unsigned int port;
	btrie * root;
	struct ENTRY * list_table;
	int num;
} tentry;
tentry * segement_table;
int segement_array[65536];
int length[129] = {0};
int num_entry = 0;
int num_query=0;
int num_node = 0;
int mem_access=0;
btrie create_node(){
	btrie temp;
	num_node++;
	temp=(btrie)malloc(sizeof(node));
	temp->right=NULL;
	temp->left=NULL;
	temp->port=256;//default port
	temp->layer = 0;
	temp->upper_ip = 0;
	temp->lower_ip = 0;
	temp->len = 0;
	return temp;
}
void CountClock()
{
	unsigned int i;
	unsigned int* NumCntClock = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for(i = 0; i < 50; i++) NumCntClock[i] = 0;
	unsigned long long MinClock = 10000000, MaxClock = 0;
	for(i = 0; i < num_entry; i++)
	{
		if(clock[i] > MaxClock) MaxClock = clock[i];
		if(clock[i] < MinClock) MinClock = clock[i];
		if(clock[i] / 100 < 50) NumCntClock[clock[i] / 100]++;
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
int dup = 0;
// void add_node(int entry, unsigned long long int upper_ip, unsigned long long int lower_ip,unsigned char len,unsigned char nexthop){
// 	btrie ptr=root;
// 	int i;
// 	unsigned long long int ip = 0;
// 	unsigned long long int temp = 1;
// 	int shift = 0;
// 	len = len - 16;
// 	for(i=0;i<len;i++){

// 		if( i > 47 ){
// 			ip = lower_ip;
// 			shift = 111 - i;
// 		}
// 		else{
// 			ip = upper_ip;
// 			shift = 63 - i;
// 			shift = shift-16;
// 		}

// 		if(ip&(temp<<shift)){
// 			//printf("1");
// 			if(ptr->right==NULL)
// 				ptr->right=create_node(); // Create Node
// 			ptr=ptr->right;
// 			if((i==len-1)&&(ptr->port==256)){
// 				ptr->upper_ip = upper_ip;
// 				ptr->lower_ip = lower_ip;
// 				ptr->len = len;
// 				ptr->port=nexthop;
// 			}
// 		}
// 		else{
// 			//printf("0");
// 			if(ptr->left==NULL)
// 				ptr->left=create_node(ip, len);
// 			ptr=ptr->left;
// 			if((i==len-1)&&(ptr->port==256)){
// 				ptr->upper_ip = upper_ip;
// 				ptr->lower_ip = lower_ip;
// 				ptr->len = len;
// 				ptr->port=nexthop;
// 			}
// 		}
// 	}
// }
void add_node(int entry, unsigned long long int upper_ip, unsigned long long int lower_ip,unsigned char len,unsigned char nexthop){
	btrie ptr=root;
	int i;
	unsigned long long int ip = 0;
	unsigned long long int temp = 1;
	int shift = 0;

	for(i=0;i<len;i++){

		if( i > 63 ){
			ip = lower_ip;
			shift = 127 - i;
		}
		else{
			ip = upper_ip;
			shift = 63 - i;
		}

		if(ip&(temp<<shift)){
			//printf("1");
			if(ptr->right==NULL)
				ptr->right=create_node(); // Create Node
			ptr=ptr->right;
			if((i==len-1)&&(ptr->port==256)){
				ptr->upper_ip = upper_ip;
				ptr->lower_ip = lower_ip;
				ptr->len = len;
				ptr->port=nexthop;
			}
		}
		else{
			//printf("0");
			if(ptr->left==NULL)
				ptr->left=create_node(ip, len);
			ptr=ptr->left;
			if((i==len-1)&&(ptr->port==256)){
				ptr->upper_ip = upper_ip;
				ptr->lower_ip = lower_ip;
				ptr->len = len;
				ptr->port=nexthop;
			}
		}
	}
}
void create(){
	int i;
	root=create_node();
	begin2=rdtsc();
	for(i=0;i<num_entry;i++){
		add_node( i, table[i].upper_ip, table[i].lower_ip ,table[i].len,table[i].port);
	}
	end2=rdtsc();
}
//////////////////////////////////////////////////////////////////////////////////////////
unsigned long long int insert_time[65536]={0};
void build_segment_create( int pos ){
	int i;
	root=create_node();
	begin=rdtsc();
	for(i=0;i<segement_table[pos].num;i++){
		add_node(i,segement_table[pos].list_table[i].upper_ip,segement_table[pos].list_table[i].lower_ip,segement_table[pos].list_table[i].len,segement_table[pos].list_table[i].port);
	}
	end=rdtsc();
	insert_time[pos]=end-begin;
	segement_table[pos].root = root;
}
//////////////////////////////////////////////////////////////////////////////////////////
int max_segment_size=0;
void build_segement_table(){
	for(int i=0;i<65536;i++)
		segement_array[i]=256;
	int num[65536]={0};
	unsigned long long int ip = 0;
	unsigned long long int ip_max = 0;
	unsigned long long int shift = 1;
	int prefix_dis[16] = {0};
	for(int i=0;i<num_entry;i++){
		unsigned long long int temp_min = 0,temp_max=0;
		if(table[i].len > 16){
			for(int k = 0; k < 16; k++ ) {
				if( table[i].upper_ip & (shift<< (63-k)) ){
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
			ip = table[i].upper_ip>>48;
			segement_table[ip].list_table[segement_table[ip].num].len = table[i].len;
			segement_table[ip].list_table[segement_table[ip].num].upper_ip = table[i].upper_ip;
			segement_table[ip].list_table[segement_table[ip].num].lower_ip = table[i].lower_ip;
			segement_table[ip].num++;
		}
		else{ 
			ip = table[i].upper_ip>>48;
			ip_max = ip +(1<<(16-table[i].len))-1;
			for(int j=ip;j<=ip_max;j++){
				segement_array[j]=1;
			}
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
		segement_table[i].list_table = (struct ENTRY *)malloc( (max_segment_size+1) * sizeof(struct ENTRY));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void read_table(char *str, unsigned long long int *upper_ip, unsigned long long int *lower_ip, int *len,unsigned int *nexthop){
	char tok[]="/";
	char tok2[]=":";
	int count = 0;
	char buf[100],*str1;
	unsigned long long int n[8] = {0};
	char *cur = str;
	int check = 0;
	int skip_point = 0;
	int c = 0;
	while( *cur != '\0' ){
		if( *cur == ':' ){
			cur++;
			check = 1;
			c++;
		}
		if( *cur == ':' && check){
			skip_point = c;
		}
		else{
			check = 0;
			cur++;
		}
	}

	char *substr = strtok(str, tok);
	sprintf(buf,"%s\0",strtok(NULL,tok));
	*len = atoi(buf);
	substr = strtok(str, tok2);
	
     
    do{
    	sprintf(buf,"%s\0",substr);
    	n[count] = strtol(buf,NULL,16);
        count++;
        substr = strtok(NULL, tok2);
    }while (substr != NULL);

    
    unsigned long long int group[8] = {0};
    for( int i = 0; i < skip_point; i++ ){
    	group[i] = n[i];
    }
    for( int i = skip_point;  i + 8- count < 8; i++ ){
    	group[i+8-count] = n[i];
    	n[i] = 0;
    }

    // for( int i = 0; i < 8; i++ ){
    // 	printf("%llu\n",group[i] );
    // }
     // if( num_entry == 5971 ){
     // 	for( int i = 0; i < 8; i++ ){
	    // 	printf("%llu\n",group[i] );
	    // }
     // }
    // 	exit(0);

    for( int i = 0; i < 8; i++ ){
    	if( i < 4 ){  	
			*upper_ip <<= 16;
			*upper_ip += group[i];    		
    	}
    	else{
    		*lower_ip <<= 16;
    		*lower_ip += group[i];
    	}

    }
    *nexthop = 1;

}
void set_table(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	unsigned int nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		//read_table(string,&upper_ip,  &lower_ip, &len,&nexthop);
		num_entry++;
	}
	rewind(fp);
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
	clock=(unsigned long long int *)malloc(num_entry*sizeof(unsigned long long int));
	num_entry=0;
	
	while(fgets(string,50,fp)!=NULL){

		read_table(string,&upper_ip, &lower_ip, &len,&nexthop);
		table[num_entry].upper_ip=upper_ip;
		table[num_entry].lower_ip=lower_ip;
		table[num_entry].port=nexthop;
		table[num_entry].len=len;
		clock[num_entry++]=10000000;
	}
}

void set_query(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		num_query++;
	}
	rewind(fp);
	query=(struct ENTRY *)malloc(num_query*sizeof(struct ENTRY));
	clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&upper_ip, &lower_ip, &len,&nexthop);
		query[num_query].upper_ip=upper_ip;
		query[num_query].lower_ip=lower_ip;
		query[num_query].port=nexthop;
		query[num_query].len=len;
		clock[num_query++]=10000000;
	}
}

int layerArray[15] = {0};
int prefix_num = 0;
int search( btrie current ){

	if( current == NULL ){
		return 0;
	}

	int leftlayer = 0;
	int rightlayer = 0;
	leftlayer = search( current->left );
	rightlayer = search(  current->right );
	if( current->port != 256 ){
		prefix_num++;
		if( leftlayer > rightlayer ){
			current->layer = leftlayer + 1;		
			layerArray[current->layer]++;
			return leftlayer + 1;
		}
		else if( leftlayer <= rightlayer ) {
			current->layer = rightlayer + 1;
			layerArray[current->layer]++;
			return rightlayer + 1;
		}
	}
	else{
		if( leftlayer > rightlayer )
			return leftlayer;
		else
			return rightlayer;
	}
	  
}

int pushcase[3]={0};
void leafpushing(btrie current){
	if(current ==NULL)
		return;
	if(current->left==NULL && current->right==NULL)
		return;
	int port_temp = current->port;
	if(current->port != 256 ){
		if( current->left !=NULL &&  current->right !=NULL){
			if( current->left->port != 256 && current->right ->port != 256){
				current->port = 256;
				pushcase[2]++;
			} 
			else if( current->left->port != 256 ) {
				current->right->port = current->port;
				current->port = 256;
				pushcase[1]++;
			}
			else if( current->right->port != 256 ) {
				current->left->port = current->port;
				current->port = 256;
				pushcase[1]++;
			}
			else 
			  pushcase[0]++;
		}

		else if(current->left ==NULL && current->right->port!=256 ){
			current->left = create_node();
			current->left ->port = port_temp;
			current -> port = 256;
			pushcase[1]++;
			
		}
		else if(current->right ==NULL && current->left->port!=256 ){
			current->right = create_node();
			current->right ->port = port_temp;
			current -> port = 256;
			pushcase[1]++;
		}
		else
			pushcase[0]++;
		
	}
	leafpushing(current->left);
	leafpushing(current->right);

}

int leaf = 0;
void search_case( btrie current ){

	if(current ==NULL)
		return;
	if(current->left==NULL && current->right==NULL){

		if(current->port != 256 )
			leaf++;
		return;
	}

	if(current->port != 256 ){
		if( current->left !=NULL &&  current->right !=NULL){
			if( current->left->port != 256 && current->right ->port != 256)
				pushcase[2]++;	
			else if( current->left->port != 256 ) 
				pushcase[1]++;
			else if( current->right->port != 256 ) 
				pushcase[1]++;
			else 
			  pushcase[0]++;
		}
		else if(current->left ==NULL && current->right->port!=256 ){
			pushcase[1]++;
		}		
		else if(current->right ==NULL && current->left->port!=256 )
			pushcase[1]++;
		else
			pushcase[0]++;
	}
	search_case(current->left);
	search_case(current->right);
}
int success=0,fail=0;
void search_segment( unsigned long long int upper_ip, unsigned long long int lower_ip){
	int find=0;
	unsigned long long int ip_temp;
	int i;
	unsigned long long int ip = 0;
	unsigned long long int temp = 1;
	int shift = 0;
	ip_temp = upper_ip>>48;
	begin=rdtsc();

	btrie current=segement_table[ip_temp].root;
	btrie rule=NULL;

	for(int i=0;i<112;i++){
		if(current==NULL)
			break;
		if(current->port!=256){
			rule=current;
			find=1;
		}
		if( i > 47 ){
			ip = lower_ip;
			shift = 111 - i;
		}
		else{
			ip = upper_ip;
			shift = 63 - i;
			shift = shift-16;
		}

		if(ip&(temp<<shift)){
			if( current->right != NULL ){
				current = current->right;
			}
		}
		else{
			if( current->left != NULL ){
				current = current->left;
			}
		}
	}
	if(find)
		success++;
	else{
		if(segement_array[ip_temp]!=256)
			success++;
		else
			fail++;
	}
	end=rdtsc();

}
int match_rule = 0;



void search_btrie_ipv6( unsigned long long int upper_ip, unsigned long long int lower_ip ){
	//printf("---------------------\n");
	btrie current=root;
	//printf("---------------------\n");
	int i;
	unsigned long long int ip = 0;
	unsigned long long int temp = 1;
	int find=0;
	int shift = 0;
	btrie rule = NULL;
	for(int i=0;i<128;i++){
		mem_access++;
		//printf("%d\n", i);
		if( i > 63 ){
			ip = lower_ip;
			shift = 127 - i;
		}
		else{
			ip = upper_ip;
			shift = 63 - i;
		}

		if(ip&(temp<<shift)){
			if( current->right != NULL ){
				current = current->right;
			}
			else break;
		}
		else{
			if( current->left != NULL ){
				current = current->left;
			}
			else break;
		}

		if( current == NULL )
			break;
		if(current->port != 256 ){
			find=1;
			rule = current;
		}
	}

	if(find)
		success++;
	else{
		fail++;
	}

}
void shuffle(struct ENTRY *array, int n) {
    srand((unsigned)time(NULL));
    struct ENTRY *temp=(struct ENTRY *)malloc(sizeof(struct ENTRY));
    
    for (int i = 0; i < n - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        temp->lower_ip=array[j].lower_ip;
        temp->upper_ip=array[j].upper_ip;
        temp->len=array[j].len;
        temp->port=array[j].port;
        array[j].lower_ip = array[i].lower_ip;
        array[j].upper_ip = array[i].upper_ip;
        array[j].len = array[i].len;
        array[j].port = array[i].port;
        array[i].lower_ip = temp->lower_ip;
        array[i].upper_ip = temp->upper_ip;
        array[i].len = temp->len;
        array[i].port = temp->port;
    }
}
void main(int argc, char const *argv[]){
	a=(unsigned int *)malloc(sizeof(unsigned int));
	set_table(argv[1]);
	set_query(argv[2]);
	// for( int i = 0; i < num_entry; i++ ){
	// 	printf("entry num: %d, upper: %llu, lower: %llu\n", i, table[i].upper_ip, table[i].lower_ip );
	// }
	
	//search( root );
	//leafpushing(root);
	/*for( int i  = 0; i < 3; i++  ){
		pushcase[i] = 0;
	}
	search_case( root );
	for( int i = 0; i < 3; i++ ){
		printf("case %d: %d\n",i, pushcase[i] );
	}
	printf("leaf node: %d\n", leaf );*/
	create();
	//build_segement_table();
	unsigned long long int t_begin, t_end;
	double total_time=0;

	shuffle(query, num_query);
	for(int j=0;j<100;j++){
		match_rule = 0;
		for(int i=0;i<num_query;i++){
			// begin=rdtsc();
			//printf("---------------------\n");
			t_begin = rdtscp(a);
			search_btrie_ipv6( query[i].upper_ip, query[i].lower_ip );
			t_end = rdtscp(a);
			//search_segment( query[i].upper_ip, query[i].lower_ip );
			//printf("zzzzzzzzzzzzzzz\n");
			// end=rdtsc();
			if(clock[i]>(t_end-t_begin))
				clock[i]=(t_end-t_begin);
		}
	}
	total=0;
	for(int j=0;j<num_query;j++){
		total_time += (double)clock[j]/2100;
		// total+=clock[j];
	}
	
	// CountClock();
	int total_insert=0;
	printf("number of nodes: %d\n",num_node);
	printf("Avg. Search: %f\n",total_time/num_query);
	printf("mem_access =%d, Avg mem_access = %f\n", mem_access,(float)mem_access/(float)num_query );
	//printf("success= %d, fail =%d\n",success,fail);
	for(int i=1;i<65536;i++){
		total_insert+=insert_time[i];
	}
	printf("Avg. Inseart: %llu\n",total_insert/num_entry);
	printf("Avg. Inseart: %llu\n",(end2-begin2)/num_entry);
	printf("Total memory requirement: %d KB\n",((num_node*9)/1024));
	printf("total query: %d\n", num_entry);
	// for( int i = 0; i < 15; i++ )
	// 	printf("layer%d : %d\n", i, layerArray[i] );

	// for( int i = 0; i < 15; i++ )
	// 	layerArray[i] = 0;

	/*printf("prefix　num :%d\n", prefix_num );
	prefix_num = 0;
	printf("after leaf pushing\n" );

	leafpushing(root);
	for( int i = 0; i < 15; i++ )
		layerArray[i] = 0;
	search( root );
	for( int i = 0; i < 15; i++ )
		printf("layer%d : %d\n", i, layerArray[i] );
	

	printf("num node: %d\n", num_node );
	printf("num entry: %d\n", num_entry );
	printf("prefix num: %d\n", prefix_num );*/
	// printf("dum: %d\n", dup );
}


