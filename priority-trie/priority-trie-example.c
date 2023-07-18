#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#define rdtscp __builtin_ia32_rdtscp
unsigned int *a;
////////////////////////////////////////////////////////////////////////////////////
struct ENTRY{
	unsigned int ip;
	unsigned int  low;
	unsigned int  up;
	unsigned char len;
	unsigned char port;
};
////////////////////////////////////////////////////////////////////////////////////
/*inline unsigned long long int rdtsc(){
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
	int len,type;
	unsigned int ip;
	unsigned int up;
	unsigned int low;
	struct list *left,*right;
};
typedef struct list node;
typedef node *btrie;

typedef struct list_element{
	unsigned int port;
	btrie  root;
	struct ENTRY * list_table;
	int num;
} tentry;
tentry * segement_table;
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
btrie root;
unsigned int *query;
int num_entry=0;
int num_entry2=0;
int num_query=0;
int max_segment_size=0;
struct ENTRY *table;
struct ENTRY *table2;
struct ENTRY *table_insert;
int *temp_len[33];
int count[33]={0};
int N=0;//number of nodes
unsigned int mask_value[33];
unsigned long long int begin,end,total=0;
unsigned long long int begin2,end2;
unsigned long long int *clock1;
int max_len=0;
int success=0,fail=0;
int priority_case=0,ordinary_case=0;
int num_node=0;//total number of nodes in the binary trie
int mem_access=0;
unsigned long long int t_begin, t_end;
double total_time=0;
////////////////////////////////////////////////////////////////////////////////////
btrie create_node(){
	btrie temp;
	num_node++;
	temp=(btrie)malloc(sizeof(node));
	temp->right=NULL;
	temp->left=NULL;
	temp->ip=0;
	temp->low=0;
	temp->up=4294967295;
	temp->type=-1; // type = 1 as priority node , type = 0 as ordinary node 
	temp->len=33; // default len
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
void buildNode(btrie current,unsigned int ip,unsigned int low,unsigned int up,unsigned char len,unsigned char nexthop,int level){
	if(current == NULL)
		return;
	if(current->port == 256){ // node is non-prefix node 
		current->len = len;
		current->port = nexthop;
		current->ip = ip;
		current->low = low;
		current->up = up;
		if(len > level )
			current->type=1; // mark node as priority node 
		else
			current->type=0; // mark node as ordinary node 
		return;
	}
	else{  // node if prefix node 
		if(len == level && current->type==1){
			btrie temp=create_node();
			temp->len = current->len;
			temp->port = current->port;
			temp->ip =current->ip;
			temp->low = current->low;
			temp->up = current->up;
			
			current->len = len;
			current->port = nexthop;
			current->low = low;
			current->up = up;
			current->ip = ip;
			
			len = temp->len;
			nexthop = temp->port;
			ip = temp->ip;
			low = temp->low;
			up = temp->up;
			
			current->type=0;
			num_node--;
		}
		else if(len>current->len  && ( current->up >= up && current->low <= low)  && current->type==1){
			btrie temp=create_node();
			temp->len = current->len;
			temp->port = current->port;
			temp->ip =current->ip;
			temp->low = current->low;
			temp->up = current->up;
			
			current->len = len;
			current->port = nexthop;
			current->low = low;
			current->up = up;
			current->ip = ip;
			
			len = temp->len;
			nexthop = temp->port;
			ip = temp->ip;
			low = temp->low;
			up = temp->up;
			num_node--;
		}
		
		if(ip&(1<<(31-level))){
			if(current->right==NULL)
				current->right=create_node(); // Create Node
			current=current->right;
		}
		else{
			if(current->left==NULL)
				current->left=create_node();
			current=current->left;
		}
		level++;
		buildNode(current,ip,low,up,len,nexthop,level);
	}
	
}
void build_priority_trie(){
	/*allready sort*/
	int i;
	root = create_node();
	begin2=rdtsc();
	for(i=0;i<num_entry;i++)
		buildNode(root,table2[i].ip,table2[i].low,table2[i].up,table2[i].len,table2[i].port,0);
	end2=rdtsc();
	
}
unsigned long long int insert_time[65536]={0};
void build_segment_create( int pos ){
	int i;

	segement_table[pos].root = create_node();
	//root=create_node();
	for(i=0;i<segement_table[pos].num;i++){
		buildNode( segement_table[pos].root, segement_table[pos].list_table[i].ip,segement_table[pos].list_table[i].low,segement_table[pos].list_table[i].up,segement_table[pos].list_table[i].len, segement_table[pos].list_table[i].port,16);
	}
}
int segement_array[65536];
void build_segement_table(){
	for(int i=0;i<65536;i++)
		segement_array[i]=256;
	int num[65536]={0};
	unsigned int ip = 0;
	unsigned int ip_max = 0;
	int prefix_dis[16] = {0};
	for(int i=0;i<num_entry;i++){
		unsigned int temp_min = 0,temp_max=0;
		if(table2[i].len > 16){
			for(int k = 0; k < 16; k++ ) {
				if( table2[i].ip & (1<< (31-k)) ){
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
		if( table2[i].len > 16 ){ 
			ip = table2[i].ip>>16;
			segement_table[ip].list_table[segement_table[ip].num].len = table2[i].len;
			segement_table[ip].list_table[segement_table[ip].num].ip = table2[i].ip;
			segement_table[ip].list_table[segement_table[ip].num].low = table2[i].low;
			segement_table[ip].list_table[segement_table[ip].num].up = table2[i].up;
			segement_table[ip].num++;
		}
		else if(table2[i].len == 16){
			ip = table2[i].ip>>16;
			segement_array[ip]=1;
		}
		else{
			ip = table2[i].ip>>16;
			ip_max = ip +(1<<(16-table2[i].len))-1;
			for(int j=ip;j<=ip_max;j++){
				/*segement_table[j].list_table[segement_table[j].num].len = table2[i].len;
				segement_table[j].list_table[segement_table[j].num].ip = table2[i].ip;
				segement_table[j].list_table[segement_table[j].num].low = table2[i].low;
				segement_table[j].list_table[segement_table[j].num].up = table2[i].up;
				segement_table[j].num++;*/
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
	printf("empty_entry =%d , non_empty_entry =%d\n",empty_entry,non_empty_entry);

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
int tree_level(btrie current){
	
	int templ,tempr;
	if(current == NULL)
		return 0;
	
	templ = tree_level(current->left);
	tempr = tree_level(current->right);
	
	if(templ >= tempr){
		return templ+1;
	}
	else{
		return tempr+1;
	}
	
}
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
int is_match( unsigned int ip,  unsigned int ip_rule, unsigned int len){
	if( ( ip_rule & mask_value[len]) ^ (ip & mask_value[len]) )
		return 0;
	else
		return 1;
}
////////////////////////////////////////////////////////////////////////////////////
void search_segment(unsigned int ip,btrie x,int ip_temp,int success_flag){
	if( x == NULL ){
		return ;
	}
	int level=16;
	do{
		mem_access++;
		if(is_match(ip,x->low,x->len)){
			if(success_flag==0){
				success++;
				success_flag=1;
			}
			if(x->type == 1){
				priority_case++;
				break;
			}
		}
		// if( x->up >= ip && x->low <=ip){
		// 	// match 
		// 	if(success_flag==0){
		// 		success++;
		// 		success_flag=1;
		// 	}
		// 	if(x->type == 1){
		// 		priority_case++;
		// 		break;
		// 		//break;
		// 	}
		// 	//break;
				
		// }
		if(ip&(1<<(31-level))){
			x=x->right;
		}
		else{
			x=x->left;
		}
		level++;
	}while(x != NULL);

	return;

}
void search(unsigned int ip){
	unsigned int ip_temp;
	ip_temp = ip>>16;
	int success_flag=0;
	if(segement_array[ip_temp]==1){
		success_flag=1;
		success++;
	}
	mem_access++;
	search_segment(ip,segement_table[ip_temp].root,ip_temp,success_flag);


	

}
void search2(unsigned int ip,btrie x){
	int level=0;
	int first=0;
	do{
		if( x->up >= ip && x->low <=ip){
			// match 
			if(first==0){
				success++;
				first=1;
			}
			if(x->type == 1){
				priority_case++;
				break;
				
			}
				
		}
		
		if(ip&(1<<(31-level))){
			x=x->right;
		}
		else{
			x=x->left;
		}
		level++;
	}while(x != NULL);
	return;
	
}
void search3(unsigned int ip){
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
}
int ccount [33]={0};
int less16=0,equal16=0;
void set_table2(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip,nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		num_entry2++;
	}
	rewind(fp);
	table_insert=(struct ENTRY *)malloc(num_entry2*sizeof(struct ENTRY));
	num_entry2=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		//if( len < 16 ){
		table_insert[num_entry2].ip=ip;
		table_insert[num_entry2].up=ip+(1<<(32-len))-1;
		table_insert[num_entry2].port=nexthop;
		table_insert[num_entry2++].len=len;
		//}
	}
}
void set_table(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int ip,nexthop;
	int i,j;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		num_entry++;
	}
	rewind(fp);
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
	table2=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
	num_entry=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip,&len,&nexthop);
		if(len<16)
			less16++;
		else if(len==16)
			equal16++;
		ccount[len]++;
		table[num_entry].ip=ip;
		table[num_entry].low=ip;
		if(len==0)
				table[num_entry].up=4294967295;
		else
			table[num_entry].up=ip+(1<<(32-len))-1;
		table[num_entry].port=nexthop;
		table[num_entry++].len=len;
	}
	for(i=0;i<33;i++){
		if(ccount[i]>max_len)
			max_len = ccount[i];
	}

	for(i=0;i<33;i++){    //initial layer min/max 
		temp_len[i] = (int *)malloc((max_len+1)*sizeof(int));
		for(j=0;j<max_len+1;j++){
			temp_len[i][j]=-1;
		}
	}
	for(i=0;i<num_entry;i++){
		for(j=0;j<33;j++){
			if(table[i].len == j){
				temp_len[j][count[j]++]=i;
			}
		}
	}
	int k=0;
	for(i=32; i>=0 ;i--){
		j=0;
		while(temp_len[i][j]!=-1){
			table2[k].ip=table[temp_len[i][j]].ip;
			table2[k].low=table[temp_len[i][j]].low;
			table2[k].up=table[temp_len[i][j]].up;
			table2[k].port=table[temp_len[i][j]].port;
			table2[k++].len=table[temp_len[i][j]].len;
			j++;
		}
	}
	printf("k= %d\n",k);
	for(i=0;i<33;i++){    //free 
		free(temp_len[i]);
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
	begin2=rdtsc();
	for(i=0;i<num_entry;i++)
		add_node(table[i].ip,table[i].len,table[i].port);
	end2=rdtsc();
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
void Countclock1(){
	unsigned int i;
	unsigned int* NumCntclock1 = (unsigned int* )malloc(50 * sizeof(unsigned int ));
	for(i = 0; i < 50; i++) NumCntclock1[i] = 0;
	unsigned long long Minclock1 = 10000000, Maxclock1 = 0;
	for(i = 0; i < num_query; i++)
	{
		if(clock1[i] > Maxclock1) Maxclock1 = clock1[i];
		if(clock1[i] < Minclock1) Minclock1 = clock1[i];
		if(clock1[i] / 100 < 50) NumCntclock1[clock1[i] / 100]++;
		else NumCntclock1[49]++;
	}
	printf("(Maxclock1, Minclock1) = (%5llu, %5llu)\n", Maxclock1, Minclock1);
	FILE *fp;

	fp=fopen("cycle.txt","w");
 
	for(i = 0; i < 50; i++)
	{
		fprintf(fp,"%d\n",NumCntclock1[i]);
		printf("%d\n", NumCntclock1[i]);
	}
	fclose(fp);
	return;
}
void shuffle(unsigned int *array, int n){
    
    srand((unsigned)time(NULL));
    for (int i = 0; i < n - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t;
        t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}
////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char *argv[]){
	a=(unsigned int *)malloc(sizeof(unsigned int));
	mask_value[0]=0;
	mask_value[32]=-1;
	for(int i=1;i<32;i++){
		mask_value[i] = ~( (1 << (32-i) )-1);
	}
	int i,j,treehight;
	unsigned long long int total_insert=0;
	set_query(argv[2]);
	set_table(argv[1]);
	set_table2(argv[3]);
	/*set_query("final_output.txt");
	set_table("final_output.txt");*/
	printf("# of prefix = %d\n",num_entry);
	printf("# of prefix is len < 16 = %d\n",less16);
	printf("# of prefix is len = 16 = %d\n",equal16);
	printf("# of search = %d\n",num_query);
	shuffle(query, num_query);
	build_segement_table();
	//build_priority_trie();
	//create();
	treehight = tree_level(root);
	printf("tree-hight: %d\n",treehight);
	printf("number of nodes: %d\n",num_node);
	
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			// begin=rdtsc();
			t_begin = rdtscp(a);
			search(query[i]); //segment
			//search2(query[i],root); //non-segment
			t_end = rdtscp(a);
			if(clock1[i]>(t_end-t_begin))
				clock1[i]=(t_end-t_begin);
			// end=rdtsc();
			// if(clock1[i]>(end-begin))
			// 	clock1[i]=(end-begin);
		}
	}
	total=0;
	printf("success = %d , fail = %d\n",success/100,num_query-success/100);
	printf("mem_access =%d, Avg mem_access = %f\n", mem_access,(float)mem_access/(float)num_query );
	total_time=0;
	for(j=0;j<num_query;j++){
		total_time += (double)clock1[j];
		// total+=clock[j];
	}
	printf("Avg. Search: %f\n",total_time/num_query);
	// for(j=0;j<num_query;j++)
	// 	total+=clock1[j];
	// printf("Avg. Search: %llu\n",total/num_query);
	// Countclock1();

	for(int i=0;i<num_entry2;i++)
		clock1[i]=10000000;

	for( int i=0;i<num_entry2;i++){
		unsigned int ip = table_insert[i].ip>>16;
		
		// begin2=rdtsc();
		t_begin = rdtscp(a);
		if(table_insert[i].len==16)
			segement_array[ip]=1;
		else if(table_insert[i].len<16){
			unsigned int ip_max = ip +(1<<(16-table_insert[i].len))-1;
			for(int j=ip;j<=ip_max;j++){
				segement_array[j]=1;
			}
		}
		else{
			if(segement_table[ip].root==NULL)
				segement_table[ip].root = create_node();
			buildNode( segement_table[ip].root, table_insert[i].ip,table_insert[i].ip,table_insert[i].up,table_insert[i].len, table_insert[i].port,16);
		}
		t_end = rdtscp(a);
		clock1[i]=(t_end-t_begin);
		// end2=rdtsc();
		// if(clock1[i]>(end2-begin2))
		// 	clock1[i]=(end2-begin2);

	}
	//end2=rdtsc();
	total=0;
	total_time=0;
	for(j=0;j<num_entry2;j++){
		total_time += (double)clock1[j];
		// total+=clock[j];
	}
	printf("Avg. Insert: %f\n",total_time/num_entry2);
	// for(int j=0;j<num_entry2;j++)
	// 	total+=clock1[j];
	// printf("Avg. Insert: %llu\n",total/num_entry2);

	

	success=0;
	for(i=0;i<num_query;i++){
		begin=rdtsc();
		search(query[i]); //segment
		//search2(query[i],root); //non-segment
		end=rdtsc();
		if(clock1[i]>(end-begin))
			clock1[i]=(end-begin);
	}
	printf("success = %d , fail = %d\n",success,num_query-success);
	//printf("Total memory requirement: %d KB\n",((num_node*14)/1024));
	printf("Total memory requirement: %d KB\n",((num_node*14+7*65536)/1024));// len=1bytes,port=2bytes,pointer=4bytes
	
	////////////////////////////////////////////////////////////////////////////
	
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
