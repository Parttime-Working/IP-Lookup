#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "multilayerv6.h"
////////////////////////////////////////////////////////////////////////////////////
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
unsigned long long int mask_value[65];
unsigned long long int begin,end,total=0;
unsigned long long int begin2,end2;
unsigned long long int begin3,end3;
unsigned long long int *clock;
unsigned long long int *clock2;
unsigned long long int *magic_b;
int num_entry=0 , num_insert=0;
int num_query=0 , num_delete=0;
int num_node=0, N = 0, index_count = 0;
int hash_count=0,max_hash=0,count_48=0;
int top_layer_num = 0, not_layer0 = 0;
int layer0_count = 0, insert_count = 0;
int match_rule = 0 , no_match = 0;
int cover_layer0=0 , replace_layer0=0 , same_replace=0;
int match_flag = 0, layer0_flag = 0;
int node_upper = 0, node_layer0 = 0;
int mem_access = 0, delete_case = 0;
int replace_flag = 0, insertValue_flag = 0,splitNode_flag = 0 ;
int count_l = 0;
int root_num = 0, count_n = 0;
int layerArray[10] = {0}, toplayer_array[10] = {0};
arr_entry2 * upper_Btree_root;
btrie root;
tentry * hashtable;
tentry * index_array;
struct ENTRY *query;
struct ENTRY *table;
struct ENTRY *insert_table;
struct ENTRY *delete_table;
struct btreeNode *btree_root;
struct upper_BtreeNode * top_root;
struct upper_BtreeNode * allBtree_root;
arr_entry * layer0_table;
arr_entry * toplayer_table;
arr_entry layer0_insert[100000];
/////////////////////////////////////////////////////////
struct upper_BtreeNode *upper_createNode( unsigned long long int upper_ip, unsigned long long int lower_ip,unsigned int len, 
  struct upper_BtreeNode *child ) {
  struct upper_BtreeNode *newNode;
  newNode = (struct upper_BtreeNode *)malloc(sizeof(struct upper_BtreeNode));
  newNode->item = (struct ip_infor*)malloc( (MAX+1)* sizeof(struct ip_infor) );
  node_upper++;
  newNode->item[1].upper_ip = upper_ip;
  newNode->item[1].lower_ip = lower_ip;
  newNode->item[1].len = len;
  newNode->count = 1;
  newNode->linker[0] = top_root;
  newNode->linker[1] = child;
  for( int i = 0; i < MAX + 1; i++ ){
    newNode->nextlevel_root[i] = NULL;
    newNode->item[i].valid = 1;
  }
  return newNode;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct btreeNode *createNode(struct btreeNode *B_root, 
	unsigned long long int upper_ip, unsigned long long int lower_ip ,unsigned int len,struct btreeNode *child ) 
{
  struct btreeNode *newNode;
  node_layer0++;
  newNode = (struct btreeNode *)malloc(sizeof(struct btreeNode));
  newNode->item = (struct ip_infor*)malloc( (MAX+1)* sizeof(struct ip_infor) );
  newNode->item[1].upper_ip = upper_ip;
  newNode->item[1].lower_ip = lower_ip;
  newNode->item[1].len = len;
  newNode->count = 1;
  newNode->link[0] = B_root;
  newNode->link[1] = child;
  for( int i = 0; i < MAX + 1; i++ ){
    // newNode->nextlevel_root[i] = NULL;
    newNode->item[i].valid = 1;
  }
  return newNode;
}
////////////////////////////////////////////////////////////////////////////////////
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
	temp->toplayer = 0;
	return temp;
}
////////////////////////////////////////////////////////////////////////////////////
void add_node( unsigned long long int upper_ip, unsigned long long int lower_ip,unsigned int len,unsigned char nexthop){
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
			if(ptr->left==NULL)
				ptr->left=create_node();
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
////////////////////////////////////////////////////////////////////////////////////
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
int search( btrie current ){
	if( current == NULL ) return 0;
	int leftlayer = 0;
	int rightlayer = 0;
	leftlayer = search( current->left );
	rightlayer = search(  current->right );
	if( current->port != 256 ){
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
		if( leftlayer > rightlayer ) return leftlayer;
		else return rightlayer;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_delete(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		num_delete++;
	}
	rewind(fp);
	delete_table=(struct ENTRY *)malloc(num_delete*sizeof(struct ENTRY));
	num_delete=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&upper_ip,&lower_ip,&len,&nexthop);
		delete_table[num_delete].upper_ip=upper_ip;
		delete_table[num_delete].lower_ip=lower_ip;
		delete_table[num_delete].port=1;
		delete_table[num_delete++].len=len;
	}
}

void set_insert(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		num_insert++;
	}
	rewind(fp);
	insert_table=(struct ENTRY *)malloc(num_insert*sizeof(struct ENTRY));
	num_insert=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&upper_ip,&lower_ip,&len,&nexthop);
		insert_table[num_insert].upper_ip=upper_ip;
		insert_table[num_insert].lower_ip=lower_ip;
		insert_table[num_insert].port=1;
		insert_table[num_insert++].len=len;
	}
}
//////////////////////////////////////////////////////
void set_table(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		//read_table(string,&ip,&len,&nexthop);
		num_entry++;
	}
	rewind(fp);
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
	num_entry=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&upper_ip,&lower_ip,&len,&nexthop);
		
		table[num_entry].upper_ip=upper_ip;
		table[num_entry].lower_ip=lower_ip;
		table[num_entry].port=1;
		table[num_entry++].len=len;
		
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_query(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned int nexthop;
	unsigned long long int upper_ip = 0;
	unsigned long long int lower_ip = 0;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		//read_table(string,&ip,&len,&nexthop);
		num_query++;
	}
	rewind(fp);
	
	query=(struct ENTRY *)malloc(num_query*sizeof(struct ENTRY));
	clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	// magic_b =(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&upper_ip,&lower_ip,&len,&nexthop);
		query[num_query].len = len;
		query[num_query].upper_ip = upper_ip;
		query[num_query].lower_ip = lower_ip;
		clock[num_query]=10000000;
		num_query++;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void create(){
	int i;
	root=create_node();
	//begin=rdtsc();
	for(i=0;i<num_entry;i++){
		add_node(table[i].upper_ip, table[i].lower_ip,  table[i].len, table[i].port);
	//end=rdtsc();
	}
}
////////////////////////////////////////////////////////////////////////////////////
void layer0_to_Btree(btrie current){
	if (current == NULL) return;
	layer0_to_Btree(current->left);
	layer0_to_Btree(current->right);
	if ( current->port != 256 ) {
		if (current-> layer == 1 ) {		
			if(current->len==48){				
				build_hashtable(current->upper_ip,current->lower_ip,current->len);
				count_48++;
			}
			else{
				layer0_table[layer0_count].upper_ip = current->upper_ip;
				layer0_table[layer0_count].lower_ip = current->lower_ip;
				layer0_table[layer0_count].len = current->len;
				btree_insertion(&btree_root, current->upper_ip, current->lower_ip , current->len, 0);
				layer0_count++;
			}			
		}
	}
}
int is_match( unsigned long long int search_upper_ip , unsigned long long int search_lower_ip, unsigned long long int upper_ip, 
			  unsigned long long int lower_ip, unsigned int len)
{
	if(len <= 64){
		if(upper_ip ^ (search_upper_ip &  mask_value[len] ) ){
			return 0;
		}
		else{
			return 1;
		}
		
	}
	else{
		if(search_upper_ip ^ upper_ip){
			return 0;
		}
		else{
			if( (lower_ip & mask_value[len-64])  ^ (search_lower_ip &  mask_value[len-64]) ){
				return 0;
			}
			else
				return 1;
		}
	}

}

unsigned int hash(unsigned long long int upper48) {
	return (upper48 * 0x8100808881888088) >> 46;
}
void build_hashtable(unsigned long long int upper_ip, unsigned long long int lower_ip,int len){
	unsigned long long int upper48=upper_ip>>16;
	unsigned int hash_value;
	hash_value=hash(upper48); // 16bits
	if( hashtable[hash_value].port == 256 ){
		hashtable[hash_value].port = 1;
		hashtable[hash_value].upper_ip = upper_ip;
	}
	else{
		if(hashtable[hash_value].index==-1){
			hashtable[hash_value].index = index_count;
			index_array[index_count].port=1;
			index_array[index_count++].upper_ip=upper_ip;
		}
		else{
			int temp_index = hashtable[hash_value].index;
			while( index_array[temp_index].index!=-1 ){
				temp_index = index_array[temp_index].index;
			}
			index_array[temp_index].index = index_count;
			index_array[index_count].port=1;
			index_array[index_count++].upper_ip=upper_ip;
		}

	}
}
int search_hash(unsigned long long int upper_ip){		
	unsigned long long int temp = upper_ip>>16;
	unsigned int hash_value = hash(temp);
	mem_access++;
	if(hashtable[hash_value].port!=256){
		if(hashtable[hash_value].port==1){
			if(  !( (upper_ip &  mask_value[48] ) ^ (hashtable[hash_value].upper_ip)) ){
				return 1;
			}
		}
		int temp_index = hashtable[hash_value].index;
		while(temp_index != -1){
			mem_access++;
			if(index_array[temp_index].port==1){
				if(  !( (upper_ip &  mask_value[48] ) ^ (index_array[temp_index].upper_ip) ) ){
					return 1;
				}
			}
			temp_index = index_array[temp_index].index;
		}
		
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void insertValue(unsigned long long int upper_ip,unsigned long long int lower_ip , unsigned int len ,int pos, struct btreeNode *node,struct btreeNode *child) 
{
  int j = node->count;
  while (j > pos) {
    node->item[j+1].upper_ip = node->item[j].upper_ip;
    node->item[j+1].lower_ip = node->item[j].lower_ip;
    node->item[j+1].len = node->item[j].len;
    node->link[j+1] = node->link[j];
    j--;
  }
  node->item[j+1].upper_ip = upper_ip;
  node->item[j+1].lower_ip = lower_ip;
  node->item[j+1].len = len;
  node->link[j+1] = child;
  node->count++;
}

void splitNode(unsigned long long int upper_ip, unsigned long long int *pval, unsigned long long int lower_ip, unsigned long long int *pval2,
			   unsigned int len, unsigned int *pval3 ,int pos, struct btreeNode *node,struct btreeNode *child, struct btreeNode **newNode) 
{
  int median, j;
  if (pos > MIN)
    median = MIN + 1;
  else
    median = MIN;

  *newNode = (struct btreeNode *)malloc(sizeof(struct btreeNode));
  (*newNode)->item = (struct ip_infor*)malloc( (MAX+1)* sizeof(struct ip_infor) );
  for( int i = 0; i < MAX + 1; i++ ){
    (*newNode)->item[i].valid = 1;
  }
  node_layer0++;
  j = median + 1;
  while (j <= MAX) {
    (*newNode)->item[j-median].upper_ip = node->item[j].upper_ip;
    (*newNode)->item[j-median].lower_ip = node->item[j].lower_ip;
    (*newNode)->item[j-median].len = node->item[j].len;
    (*newNode)->link[j-median] = node->link[j];
    j++;
  }
  node->count = median;
  (*newNode)->count = MAX - median;

  if (pos <= MIN) {
    insertValue(upper_ip, lower_ip , len, pos, node, child);
  } else {
    insertValue(upper_ip, lower_ip , len, pos-median, *newNode, child);
  }
  *pval = node->item[node->count].upper_ip;
  *pval2 = node->item[node->count].lower_ip;
  *pval3 = node->item[node->count].len;
  (*newNode)->link[0] = node->link[node->count];
  node->count--;
}

int setNodeValue(unsigned long long int upper_ip,unsigned long long int lower_ip, unsigned int len, unsigned long long int *pval,
 				unsigned long long int *pval2, unsigned long long int *pval3, struct btreeNode *node, struct btreeNode **child,int flag)
{
  int pos;
  if (!node) {
    *pval=upper_ip;
    *pval2=lower_ip;
    *pval3=len;
    *child=NULL;
    return 1;
  }
  if(flag==0){
  	int flag2=0;
  	if (upper_ip < node->item[1].upper_ip) {
	    pos = 0;
	} 
	else if(upper_ip == node->item[1].upper_ip  ){
		if(lower_ip < node->item[1].lower_ip){
			pos = 0;
		}
		else{
			flag2 = 1;
		}	

	}
	else
		flag2 = 1;

	if(flag2){
		for (pos = node->count;( (upper_ip < node->item[pos].upper_ip || (upper_ip == node->item[pos].upper_ip && lower_ip <= node->item[pos].lower_ip) ) && pos > 1); pos--);
		if (upper_ip == node->item[pos].upper_ip && lower_ip == node->item[pos].lower_ip && len>=node->item[pos].len) {
		 	not_layer0++;
		 	return 2;
		}
	}
  }
  else{
  	int flag2=0;
  	if(upper_ip < node->item[1].upper_ip){
  		if(cover(upper_ip,lower_ip,len,node->item[1].upper_ip,node->item[1].lower_ip,node->item[1].len)){
  			not_layer0++;
  			cover_layer0++;
  			return 0;
  		}
  		pos = 0;
  	}
  	else if(upper_ip == node->item[1].upper_ip  ){

		if(lower_ip < node->item[1].lower_ip){
			if(cover(upper_ip,lower_ip,len,node->item[1].upper_ip,node->item[1].lower_ip,node->item[1].len)){
	  			not_layer0++;
				cover_layer0++;
				return 0;
	  		}
			pos = 0;
		}
		else flag2 = 1;
	}
  	else if(upper_ip > node->item[node->count].upper_ip){
  		if(cover(node->item[node->count].upper_ip,node->item[node->count].lower_ip,node->item[node->count].len,upper_ip,lower_ip,len)){
  			not_layer0++;
			replace_layer0++;
			replace_flag=1;
			insert_upper_Btree( 1, node->item[node->count].upper_ip, node->item[node->count].lower_ip, node->item[node->count].len, 1, allBtree_root->count, allBtree_root, NULL, 0);
			node->item[node->count].upper_ip = upper_ip;
			node->item[node->count].lower_ip = lower_ip;
			node->item[node->count].len = len;
			return 2;
  		}
		pos = node->count;
  	}
  	else if(upper_ip == node->item[node->count].upper_ip){
  		if(lower_ip > node->item[node->count].lower_ip ){
  			if(cover(node->item[node->count].upper_ip,node->item[node->count].lower_ip,node->item[node->count].len,upper_ip,lower_ip,len)){
  				not_layer0++;
				replace_layer0++;
				replace_flag=1;
				insert_upper_Btree( 1, node->item[node->count].upper_ip, node->item[node->count].lower_ip, node->item[node->count].len, 1, allBtree_root->count, allBtree_root, NULL, 0);
				node->item[node->count].upper_ip = upper_ip;
				node->item[node->count].lower_ip = lower_ip;
				node->item[node->count].len = len;
				return 2;
  			}
			pos = node->count;
		}
		else flag2=1;
  	}
  	else flag2=1;

  	if(flag2){
  		int cover_case;
  		int mid=0;
		int idxl=1;
		int idxr=node->count;
		while( idxl <= idxr ){
			mid = (idxl + idxr)/2;
			if(upper_ip == node->item[mid].upper_ip){
				if(lower_ip < node->item[mid].lower_ip){
					cover_case=2;
					idxr = mid-1;
					pos = mid-1;
				}
				else if(lower_ip > node->item[mid].lower_ip){
					cover_case=3;
					idxl = mid+1;
					pos = mid;
				}
				else{
					if( len > node->item[mid].len){
						not_layer0++;
						same_replace++;
						replace_flag=1;
						insert_upper_Btree( 1, node->item[mid].upper_ip, node->item[mid].lower_ip, node->item[mid].len, 1, allBtree_root->count, allBtree_root, NULL, 0);
						node->item[mid].upper_ip = upper_ip;
						node->item[mid].lower_ip = lower_ip;
						node->item[mid].len = len;
						return 2;
					}
					else{
						cover_layer0++;
						not_layer0++;
						return 0;
					}
					
				}
			}
			else if( upper_ip < node->item[mid].upper_ip ){
				cover_case=0;
				idxr = mid-1;
				pos = mid-1;
			}
			else{
				cover_case=1;
				idxl = mid+1;
				pos = mid;
			}
			if(idxl > idxr){
				if(cover_case==0){
					if(cover(upper_ip,lower_ip,len,node->item[mid].upper_ip,node->item[mid].lower_ip,node->item[mid].len)){
						not_layer0++;
						cover_layer0++;
						return 0;
					}
					else if(cover(node->item[mid-1].upper_ip,node->item[mid-1].lower_ip,node->item[mid-1].len,upper_ip,lower_ip,len)){
						not_layer0++;
						replace_layer0++;
						replace_flag=1;
						insert_upper_Btree( 1, node->item[mid-1].upper_ip, node->item[mid-1].lower_ip, node->item[mid-1].len, 1, allBtree_root->count, allBtree_root, NULL, 0);
						node->item[mid-1].upper_ip = upper_ip;
						node->item[mid-1].lower_ip = lower_ip;
						node->item[mid-1].len = len;
						return 2;
					}
				}
				else if(cover_case==1){
					if(cover(upper_ip,lower_ip,len,node->item[mid+1].upper_ip,node->item[mid+1].lower_ip,node->item[mid+1].len)){
						not_layer0++;
						cover_layer0++;
						return 0;
					}
					else if(cover(node->item[mid].upper_ip,node->item[mid].lower_ip,node->item[mid].len,upper_ip,lower_ip,len)){
						not_layer0++;
						replace_layer0++;
						replace_flag=1;
						insert_upper_Btree( 1, node->item[mid].upper_ip, node->item[mid].lower_ip, node->item[mid].len, 1, allBtree_root->count, allBtree_root, NULL, 0);
						node->item[mid].upper_ip = upper_ip;
						node->item[mid].lower_ip = lower_ip;
						node->item[mid].len = len;
						return 2;
					}
					
				}
				else if(cover_case==2){
					int re_place=0;
					if(cover(upper_ip,lower_ip,len,node->item[mid].upper_ip,node->item[mid].lower_ip,node->item[mid].len)){
						not_layer0++;
						cover_layer0++;
						return 0;
					}
					if(cover(node->item[mid-1].upper_ip,node->item[mid-1].lower_ip,node->item[mid-1].len,upper_ip,lower_ip,len)){
						not_layer0++;
						replace_layer0++;
						replace_flag=1;
						insert_upper_Btree( 1, node->item[mid-1].upper_ip, node->item[mid-1].lower_ip, node->item[mid-1].len, 1, allBtree_root->count, allBtree_root, NULL, 0);
						node->item[mid-1].upper_ip = upper_ip;
						node->item[mid-1].lower_ip = lower_ip;
						node->item[mid-1].len = len;
						return 2;
					}

				}
				else if(cover_case==3){
					int re_place=0;
					if(cover(node->item[mid].upper_ip,node->item[mid].lower_ip,node->item[mid].len,upper_ip,lower_ip,len)){
						not_layer0++;
						replace_layer0++;
						replace_flag=1;
						insert_upper_Btree( 1, node->item[mid].upper_ip, node->item[mid].lower_ip, node->item[mid].len, 1, allBtree_root->count, allBtree_root, NULL, 0);
						node->item[mid].upper_ip = upper_ip;
						node->item[mid].lower_ip = lower_ip;
						node->item[mid].len = len;
						return 2;
					}
					if(cover(upper_ip,lower_ip,len,node->item[mid+1].upper_ip,node->item[mid+1].lower_ip,node->item[mid+1].len)){
						not_layer0++;
						cover_layer0++;
						return 0;
					}

				}
			}
		}
  	}
  	
  }
  
  if (setNodeValue(upper_ip, lower_ip , len , pval, pval2, pval3, node->link[pos], child, flag)==1) {
    if (node->count < MAX) {
      insertValue_flag=1;
      insertValue(*pval, *pval2, *pval3 , pos, node, *child);
      return 2;
    } else {
      splitNode_flag=1;
      splitNode(*pval, pval, *pval2, pval2,*pval3, pval3, pos, node, *child, child);
      return 1;
    }
  }
  if(flag==0) return 2;
  if(flag==1){
  	if(replace_flag==1){
  		layer0_flag=1;
  	}
  	if(insertValue_flag==1 || splitNode_flag==1 || replace_flag==1) return 2;
  	else return 0;
  }
}

void btree_insertion(struct btreeNode **B_root ,unsigned long long int upper_ip,unsigned long long int lower_ip, unsigned int len,int insert_type) {
  int flag;
  replace_flag=0;
  insertValue_flag=0;
  splitNode_flag=0;
  unsigned long long int i,i2,i3;
  struct btreeNode *child;
  flag = setNodeValue(upper_ip,lower_ip,len,&i,&i2,&i3,*B_root,&child,insert_type);
  if (flag==1){
  	*B_root = createNode(*B_root, i, i2, i3,child);
  }
  else if(flag==0){
  	layer0_flag=1;
  	insert_upper_Btree( 1, upper_ip, lower_ip, len, 1, allBtree_root->count, allBtree_root, NULL, 0);
  }
}

int BtreeSearch(struct btreeNode *myNode, unsigned long long int upper_ip , unsigned long long int lower_ip ,int idxl,int idxr){
	int mid = 0;
	int bndl = 0;
	while( idxl <= idxr ){
		mem_access++;
		mid = (idxl + idxr)/2;
		if( is_match( upper_ip, lower_ip ,myNode->item[mid].upper_ip, myNode->item[mid].lower_ip ,myNode->item[mid].len) && myNode->item[mid].valid==1 ){
			match_rule++;
			return 1;
		}
		if( upper_ip < myNode->item[mid].upper_ip ){
			idxr = mid-1;
			bndl = mid-1;
		}
		else if(upper_ip == myNode->item[mid].upper_ip){
			if(lower_ip < myNode->item[mid].lower_ip){
				idxr = mid-1;
				bndl = mid-1;
			}
			else{
				idxl = mid + 1;
				bndl = mid;
			}
		}
		else{
			idxl = mid + 1;
			bndl = mid;
		}
		if(idxl > idxr && myNode->link[bndl]!=NULL){
			mem_access++;
			myNode = myNode->link[bndl];
			mid=0;
			bndl=0;
			idxl=1;
			idxr=myNode->count;
		}
	}
	return 0;
}
// upper layer code Build & search
////////////////////////////////////////////////////////////////////////////////////
void search_toplayer(btrie current, int toplayer){
	if( current == NULL )
		return;
	if( current->right == NULL  && current->left == NULL ){
		current->toplayer = 0;
		toplayer_array[0]++;
		return;
	}
	if( current->port != 256 ){
		toplayer_table[count_l].upper_ip = current->upper_ip;
		toplayer_table[count_l].lower_ip = current->lower_ip;
		toplayer_table[count_l++].len = current->len;
		current->port = 256;
		current->port = 1;	
		current->toplayer = toplayer;
		toplayer_array[toplayer]++;
		toplayer++;
	}
	search_toplayer(current->left, toplayer);
	search_toplayer(current->right, toplayer);
}
void build_btree( btrie current, arr_entry2 * Btree_current ){
	if( current == NULL ) return;
	if( current->right == NULL  && current->left == NULL )return;
	if( current->port != 256 ){
		Btree_current[count_n].upper_ip = current->upper_ip;
		Btree_current[count_n].lower_ip = current->lower_ip;
		Btree_current[count_n].trie_node = current;
		Btree_current[count_n++].len = current->len;
		return;
	}
	build_btree(current->left, Btree_current);
	build_btree(current->right, Btree_current);
}
void addValToNode(unsigned long long int upper_ip,unsigned long long int lower_ip, 
        unsigned int len , int pos, struct upper_BtreeNode *node,struct upper_BtreeNode *child) 
{
  int j = node->count;
  while (j > pos) {
    node->item[j+1].upper_ip = node->item[j].upper_ip;
    node->item[j+1].lower_ip = node->item[j].lower_ip;
    node->item[j+1].len = node->item[j].len;
    node->nextlevel_root[j+1] = node->nextlevel_root[j];
    node->linker[j+1] = node->linker[j];
    j--;
  }
  node->item[j+1].upper_ip = upper_ip;
  node->item[j+1].lower_ip = lower_ip;
  node->item[j+1].len = len;
  node->nextlevel_root[j+1] = NULL;
  node->linker[j+1] = child;
  node->count++;
}

void upper_splitNode(unsigned long long int upper_ip, unsigned long long int *pval, unsigned long long int lower_ip, unsigned long long int *pval2,
         unsigned int len, unsigned int *pval3,  int pos, struct upper_BtreeNode *node,struct upper_BtreeNode *child, struct upper_BtreeNode **newNode) 
{
  int median, j;
  if (pos > MIN)
    median = MIN + 1;
  else
    median = MIN;

  *newNode = (struct upper_BtreeNode *)malloc(sizeof(struct upper_BtreeNode));
  (*newNode)->item = (struct ip_infor*)malloc( (MAX+1)* sizeof(struct ip_infor) );
  node_upper++;
  for( int i = 0; i < MAX + 1; i++ ){
    (*newNode)->nextlevel_root[i] = NULL;
    (*newNode)->item[i].valid = 1;
  }
  j = median + 1;
  while (j <= MAX) {
    (*newNode)->item[j-median].upper_ip = node->item[j].upper_ip;
    (*newNode)->item[j-median].lower_ip = node->item[j].lower_ip;
    (*newNode)->item[j-median].len = node->item[j].len;
    (*newNode)->nextlevel_root[j-median] = node->nextlevel_root[j];
    (*newNode)->linker[j-median] = node->linker[j];
    j++;
  }
  node->count = median;
  (*newNode)->count = MAX - median;

  if (pos <= MIN) {
    addValToNode(upper_ip, lower_ip, len, pos, node, child);
  } else {
    addValToNode(upper_ip, lower_ip, len,  pos-median, *newNode, child);
  }
  *pval = node->item[node->count].upper_ip;
  *pval2 = node->item[node->count].lower_ip;
  *pval3 = node->item[node->count].len;
  (*newNode)->linker[0] = node->linker[node->count];
  node->count--;
}

int upper_setValueInNode(unsigned long long int upper_ip,unsigned long long int lower_ip, unsigned int len, 
  unsigned long long int *pval, unsigned long long int *pval2,  unsigned int *pval3, 
  struct upper_BtreeNode *node, struct upper_BtreeNode **child)
{
  int pos;
  int flag = 0;
  if (!node) {
    *pval=upper_ip;
    *pval2=lower_ip;
    *pval3=len;
    *child=NULL;
    return 1;
  }
  int flag2=0;
  if (upper_ip < node->item[1].upper_ip) {
    pos = 0;
  } 
  else if(upper_ip == node->item[1].upper_ip  ){
    if(lower_ip < node->item[1].lower_ip){
      pos = 0;
    }
    else{
      flag2 = 1;
    } 

  }
  else
    flag2 = 1;

  if(flag2){
    pos = node->count;
    while( pos > 1 ){
      if( upper_ip > node->item[pos].upper_ip )
        break;
      else if( upper_ip == node->item[pos].upper_ip ){
        if( lower_ip > node->item[pos].lower_ip )
          break;
      }
      pos--;
    }
    if (upper_ip == node->item[pos].upper_ip && lower_ip == node->item[pos].lower_ip && len>=node->item[pos].len) {
      return 0;
    }
  }

  if (upper_setValueInNode(upper_ip, lower_ip, len, pval, pval2, pval3, node->linker[pos], child)) {
    if (node->count < MAX) {
      addValToNode(*pval, *pval2,*pval3 ,pos, node, *child);
    } else {
      upper_splitNode(*pval, pval, *pval2, pval2, *pval3 , pval3, pos, node, *child, child);
      return 1;
    }
  }
  return 0;
}

void upper_insertion( unsigned long long int upper_ip, unsigned long long int lower_ip,unsigned int len,
  struct upper_BtreeNode * nextlevel_root ) {
  int flag;
  unsigned long long int i, i2, i3;
  struct upper_BtreeNode *child;
  flag = upper_setValueInNode(upper_ip,lower_ip,len, &i,&i2,&i3, top_root,&child);
  if (flag){
    top_root = upper_createNode( i, i2, i3, child);
  }
}
void Searching(struct upper_BtreeNode *myNode, unsigned long long int upper_ip , unsigned long long int lower_ip , int len, int idxl,int idxr){
 int mid = 0;
 int bndl = 0;
 if( myNode == NULL )
 	return;
 while( idxl <= idxr ){
	mid = (idxl + idxr)/2;
	if( upper_ip == myNode->item[mid].upper_ip && lower_ip == myNode->item[mid].lower_ip && len == myNode->item[mid].len ){
		myNode->nextlevel_root[mid] = top_root;
		// test_num++;
		return;
  	}
  	if( upper_ip < myNode->item[mid].upper_ip ){
   		idxr = mid-1;
   		bndl = mid-1;
  	}
  	else if(upper_ip == myNode->item[mid].upper_ip ){
   		if(lower_ip < myNode->item[mid].lower_ip){
   			idxr = mid-1;
    		bndl = mid-1;
  		}
   		else{
    		idxl = mid + 1;
    		bndl = mid;
   		}
 	}
  	else{
   		idxl = mid + 1;
   		bndl = mid;
  	}
  	if(idxl > idxr && myNode->linker[bndl]!=NULL){
	   myNode = myNode->linker[bndl];
	   mid=0;
	   bndl=0;
	   idxl=1;
	   idxr=myNode->count;
  	}
  }
}

void traversal_build_tree( int level, struct upper_BtreeNode * current_root, arr_entry2 * current, int num) {
  int i;
  int top_count= current_root->count;
  if (current) {
    for (i = 0; i < num; i++) {
    	if( current[i].nextlevel != NULL ){	
    		top_root = NULL;
        int u_len = 0;
        int l_len = 0;
    		for( int j = 0; j < current[i].num; j++ ){
    			upper_insertion(current[i].nextlevel[j].upper_ip, current[i].nextlevel[j].lower_ip, current[i].nextlevel[j].len, NULL);
    		}
    		Searching(current_root, current[i].upper_ip , current[i].lower_ip, current[i].len , 1, top_count);
    		current[i].nextlevel_root =  top_root;
    		traversal_build_tree( level+1, current[i].nextlevel_root, current[i].nextlevel, current[i].num);
    	}
    }		
  }
}
void push_prefix( btrie current ){
	if( current == NULL ) return;
	if( current->right == NULL  && current->left == NULL ) return;
	if( current->port != 256 ){
		if( current->toplayer == 1 ){
			upper_Btree_root[root_num].upper_ip = current->upper_ip;
			upper_Btree_root[root_num].lower_ip = current->lower_ip;
			upper_Btree_root[root_num].trie_node = current;
			upper_Btree_root[root_num++].len = current->len;
		}
	}
	push_prefix(current->left);
	push_prefix(current->right);
}

void build(){
	upper_Btree_root = (arr_entry2*)malloc( ( toplayer_array[1] + 1000 ) * sizeof(arr_entry2) );
	for( int i = 0; i <  root_num; i++ ){
		upper_Btree_root[i].num = 0;
		upper_Btree_root[i].nextlevel = NULL;
	}
	push_prefix(root);
	arr_entry2 ** build_queue = ( arr_entry2 **) malloc( ( num_entry + 10000 ) *sizeof(arr_entry2*));
	count_n = 0;
	int q_size = 0;
	for( int j = 0; j < toplayer_array[1]; j++ ){
		count_n = 0;
		upper_Btree_root[j].nextlevel = (arr_entry2*)malloc( 120 * sizeof(arr_entry2) );
		for( int k = 0; k < 50; k++ ){
			upper_Btree_root[j].nextlevel[k].num = 0;
			upper_Btree_root[j].nextlevel[k].nextlevel = NULL;
		}
		upper_Btree_root[j].trie_node->port = 256;
		build_btree( upper_Btree_root[j].trie_node, upper_Btree_root[j].nextlevel );
		upper_Btree_root[j].trie_node->port = 1;
		upper_Btree_root[j].num = count_n;
		if( count_n != 0 ){
			build_queue[q_size++] = upper_Btree_root[j].nextlevel;
			upper_Btree_root[j].nextlevel[0].num = count_n;
		}
		else{
			free(upper_Btree_root[j].nextlevel);
			upper_Btree_root[j].nextlevel = NULL;
		}
	}
	for( int i = 0; i < q_size; i++ ){
		int n_num = build_queue[i][0].num;
		arr_entry2 * temp = build_queue[i];
		for( int j = 0; j < n_num; j++ ){
			count_n = 0;
			temp[j].nextlevel = (arr_entry2*)malloc( 120 * sizeof(arr_entry2) );
			for( int k = 0; k < 50; k++ ){
				temp[j].nextlevel[k].num = 0;
				temp[j].nextlevel[k].nextlevel = NULL;
			}
			temp[j].trie_node->port = 256;
			build_btree( temp[j].trie_node, temp[j].nextlevel );
			temp[j].trie_node->port = 1;
			temp[j].num = count_n;
			if( count_n != 0 ){
				build_queue[q_size++] = temp[j].nextlevel;
				temp[j].nextlevel[0].num = count_n;
			}
			else{
				free(temp[j].nextlevel);
				temp[j].nextlevel = NULL;
			}
		}
	}
	for( int i = 0; i < top_layer_num; i++ ){
		upper_insertion( upper_Btree_root[i].upper_ip, upper_Btree_root[i].lower_ip, upper_Btree_root[i].len, NULL);
	}
	allBtree_root =  top_root;
	traversal_build_tree( 1, allBtree_root, upper_Btree_root, top_layer_num);
}
int cover(unsigned long long int L11, unsigned long long int L12, unsigned int len, 
	unsigned long long int L21, unsigned long long int L22,  unsigned int len2) {
	if (len < len2) {
		if( len <= 64 ){
			return ((L11 & mask_value[len]) ^ (L21 & mask_value[len])) ? 0 : 1;
		}
		else{
			if( L11 == L21 )
				return ((L12 & mask_value[len-64]) ^ (L22 & mask_value[len-64])) ? 0 : 1;
		}
	}
	return 0;
}
void search_upper_Btree(int i, struct upper_BtreeNode *myNode, unsigned long long int upper_ip , unsigned long long int lower_ip ,int idxl,int idxr){
  int mid = 0;
  int bndl = 0;
  if( myNode == NULL )
    return;
  struct upper_BtreeNode * current_root = myNode;
  while( idxl <= idxr ){
  	mem_access++;
    mid = (idxl + idxr)/2;
    if( is_match( upper_ip, lower_ip, myNode->item[mid].upper_ip, myNode->item[mid].lower_ip, myNode->item[mid].len ) ){ 
      if( myNode->item[mid].valid )
      	match_flag = 1;
      mem_access++;
      if( myNode->nextlevel_root[mid] != NULL ){
        search_upper_Btree(i+1, myNode->nextlevel_root[mid], upper_ip , lower_ip , 1, myNode->nextlevel_root[mid]->count);
        return;
      }
      else
        return;
    }
  
    if( upper_ip < myNode->item[mid].upper_ip ){
      idxr = mid-1;
      bndl = mid-1;
    }
    else if(upper_ip == myNode->item[mid].upper_ip){
      if(lower_ip < myNode->item[mid].lower_ip){
        idxr = mid-1;
        bndl = mid-1;
      }
      else{
        idxl = mid + 1;
        bndl = mid;
      }
    }
    else{
      idxl = mid + 1;
      bndl = mid;
    }
    if(idxl > idxr ){
    	mem_access++;
    	if( myNode->linker[bndl]!=NULL ){
    		myNode = myNode->linker[bndl];
		    mid=0;
		    bndl=0;
		    idxl=1;
		    idxr=myNode->count;
    	}
    	else{
    		if(match_flag == 0){
    			for( int i = 0; i < current_root->eset_num; i++ ){
    				if( is_match( upper_ip, lower_ip, current_root->eset[i].upper_ip, current_root->eset[i].lower_ip, current_root->eset[i].len ) ){ 
    					if( myNode->item[mid].valid )
    						match_flag = 1;
    				}
    			}
    		}
    	}
      
    }
  }

}
//////////////////////////  upper Insert
void insert_upper_Btree( int level, unsigned long long int upper_ip, unsigned long long int lower_ip, 
             unsigned int len, int idxl,int idxr, struct upper_BtreeNode *current, struct upper_BtreeNode * pre_node, int pre_index)
{
  int mid = 0;
  int bndl = 0;
  struct upper_BtreeNode *current_root = current;
  int n_num;
  if( current == NULL )
    return;

  while( idxl <= idxr ){
    mid = (idxl + idxr)/2;
    if( upper_ip < current->item[mid].upper_ip ){
      idxr = mid-1;
      bndl = mid-1;
    }
    else if(upper_ip == current->item[mid].upper_ip){
        if(lower_ip < current->item[mid].lower_ip){
          idxr = mid-1;
          bndl = mid-1;
          // bndr = mid;
        }
        else{
          idxl = mid + 1;
          bndl = mid;
        }
    }
    else{
      idxl = mid + 1;
      bndl = mid;
    }
    if(idxl > idxr ){
      n_num = current->count;
      if( bndl != 0 && cover(current->item[bndl].upper_ip, current->item[bndl].lower_ip, current->item[bndl].len, upper_ip, lower_ip, len) ){ 
        if( current->nextlevel_root[bndl] != NULL ){
        	insert_upper_Btree( level+1, upper_ip , lower_ip, len, 1, current->nextlevel_root[bndl]->count, current->nextlevel_root[bndl], current, bndl);
          return;
        }
        else{
            top_root = NULL;
            upper_insertion(upper_ip, lower_ip, len, NULL);
            current->nextlevel_root[bndl] = top_root;
            return;
          }
      }
      else {
        if(  bndl+1 <= n_num && cover( upper_ip, lower_ip, len, current->item[bndl+1].upper_ip, current->item[bndl+1].lower_ip, current->item[bndl+1].len)){
 			if( current_root->eset_num == 0 ){
 				current_root->eset = (struct ip_infor*)malloc( 50 * sizeof(struct ip_infor) );
 			}
 			current_root->eset[current_root->eset_num].upper_ip = upper_ip;
 			current_root->eset[current_root->eset_num].lower_ip = lower_ip;
 			current_root->eset[current_root->eset_num].len = len;
 			current_root->eset_num++;
          	return;
        }
      }
      if( current->linker[bndl] != NULL ){
        current = current->linker[bndl];
        mid=0;
        bndl=0;
        idxl=1;
        idxr=current->count;
      }
      else{
        top_root = current_root;
        upper_insertion( upper_ip, lower_ip, len, NULL);
      }
    }
  }
}

void upper_btree_delete( int level, unsigned long long int upper_ip, unsigned long long int lower_ip, 
             unsigned int len, int idxl,int idxr, struct upper_BtreeNode *current)
{
  int mid = 0;
  int bndl = 0;
  struct upper_BtreeNode *current_root = current;
  int n_num;
  if( current == NULL )
    return;

  while( idxl <= idxr ){
    mid = (idxl + idxr)/2;
    if( upper_ip < current->item[mid].upper_ip ){
      idxr = mid-1;
      bndl = mid-1;
    }
    else if(upper_ip == current->item[mid].upper_ip){
        if(lower_ip < current->item[mid].lower_ip){
          idxr = mid-1;
          bndl = mid-1;
          // bndr = mid;
        }
        else{
          idxl = mid + 1;
          bndl = mid;
        }
    }
    else{
      idxl = mid + 1;
      bndl = mid;
    }
    if(idxl > idxr ){
      n_num = current->count;
      if( upper_ip == current->item[bndl].upper_ip && lower_ip == current->item[bndl].lower_ip && current->item[bndl].len == len){
      	current->item[bndl].valid = 0;
      	return;
      }

      if( bndl != 0 && cover(current->item[bndl].upper_ip, current->item[bndl].lower_ip, current->item[bndl].len, upper_ip, lower_ip, len) ){ 
        if( current->nextlevel_root[bndl] != NULL ){
        	upper_btree_delete( level+1, upper_ip , lower_ip, len, 1, current->nextlevel_root[bndl]->count, current->nextlevel_root[bndl]);
          	return;
        }
      }
      if( current->linker[bndl] != NULL ){
        current = current->linker[bndl];
        mid=0;
        bndl=0;
        idxl=1;
        idxr=current->count;
      }
      else{
      	for( int i = 0; i < current_root->eset_num; i++ ){
			if( upper_ip == current_root->eset[i].upper_ip && lower_ip == current_root->eset[i].lower_ip && current_root->eset[i].len == len){
		      	current_root->eset[i].valid = 0;
		      	return;
		    }
		}
      }
    }
  }
}

int delete_hashtable(unsigned long long int upper_ip,unsigned long long int lower_ip,unsigned long long int len){
	
	unsigned long long int temp = upper_ip>>16;
	unsigned int hash_value = hash(temp);
	if(hashtable[hash_value].port!=256){

		if(hashtable[hash_value].port ==1 )
			if(cover(hashtable[hash_value].upper_ip,0,48,upper_ip, lower_ip,len) ){
				hashtable[hash_value].port=0;
				insert_upper_Btree( 1, hashtable[hash_value].upper_ip, 0ULL, 48, 1, allBtree_root->count, allBtree_root, NULL, 0);
				return 1;
			}

		int temp_index = hashtable[hash_value].index;
		while(temp_index != -1){
			if(index_array[temp_index].port==1){
				if(cover(index_array[temp_index].upper_ip , 0ULL , 48 , upper_ip , lower_ip, len) ){
					index_array[temp_index].port=0;
					insert_upper_Btree( 1, index_array[temp_index].upper_ip , 0ULL, 48, 1, allBtree_root->count, allBtree_root, NULL, 0);
					return 1;
				}
			}
			temp_index = index_array[temp_index].index;
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
void count_node(struct btreeNode *myNode) {
  int i;
  if (myNode) {
  	node_layer0++;
    for (i = 0; i <= myNode->count; i++) {
      count_node(myNode->link[i]);
    }
  }
}

int btree_delete(struct btreeNode *myNode, unsigned long long int upper_ip , unsigned long long int lower_ip , int len ,int idxl,int idxr){
	if(len == 48){
		unsigned long long int temp = upper_ip>>16;
		unsigned int hash_value = hash(temp);

		if(hashtable[hash_value].port!=256){
			if(hashtable[hash_value].port ==1 )
				if(hashtable[hash_value].upper_ip == upper_ip){
					hashtable[hash_value].port=0;
					return 1;
				}

			int temp_index = hashtable[hash_value].index;
			// printf("hash_value %u\n",hash_value);
			while(temp_index != -1){
				// printf("temp_index %d\n",temp_index);
				if(index_array[temp_index].port == 1){
					if(index_array[temp_index].upper_ip ==upper_ip ){
						index_array[temp_index].port=0;
						return 1;
					}
				}
				temp_index = index_array[temp_index].index;
			}
		}
	}
	int mid = 0;
	int bndl = 0;
	while( idxl <= idxr ){
		mem_access++;
		mid = (idxl + idxr)/2;
		if(myNode->item[mid].upper_ip == upper_ip && myNode->item[mid].lower_ip == lower_ip &&  myNode->item[mid].len == len){
			// printf("%d\n", myNode->item[mid].valid );
			myNode->item[mid].valid=0;
			return 1;
		}
		if( upper_ip < myNode->item[mid].upper_ip ){
			idxr = mid-1;
			bndl = mid-1;
		}
		else if(upper_ip == myNode->item[mid].upper_ip){
			if(lower_ip < myNode->item[mid].lower_ip){
				idxr = mid-1;
				bndl = mid-1;
			}
			else{
				idxl = mid + 1;
				bndl = mid;
			}
		}
		else{
			idxl = mid + 1;
			bndl = mid;
		}
		if(idxl > idxr && myNode->link[bndl]!=NULL){
			mem_access++;
			myNode = myNode->link[bndl];
			mid=0;
			bndl=0;
			idxl=1;
			idxr=myNode->count;
		}
	}
	return 0;
}
void layer0_init(){
	layer0_table = (arr_entry*)malloc( layerArray[1]* sizeof(arr_entry) );
	// hashtable= (tentry*)malloc( 131072* sizeof(tentry) );
	hashtable= (tentry*)malloc( 262144* sizeof(tentry) );
	index_array= (tentry*)malloc( 8192* sizeof(tentry) );
	int i;
	for( i = 0; i < 262144; i++ ){
		hashtable[i].port = 256;
		hashtable[i].upper_ip=0;
		hashtable[i].index=-1;
	}

	for( i = 0; i < 8192; i++ ){
		index_array[i].port = 256;
		index_array[i].upper_ip=0;
		index_array[i].index=-1;
	}
}
void inition(){
	mask_value[0]=0;
	mask_value[64]=-1ULL;
	for(int i=1;i<64;i++)
		mask_value[i] = ~( (1ull << (64-i) )-1);
	toplayer_table = (arr_entry*)malloc( num_entry* sizeof(arr_entry) );
	search_toplayer(root, 1);
	top_layer_num = toplayer_array[1];
	build();
	layer0_init();
}
void search_performance(){
	clock2=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	for(int j=0;j<num_query;j++)
		clock2[j]=10000000;
	int hash_table_search , hash_succes=0 , fail_hash=0;
	no_match=0;
	int upper_match=0;
	printf("num_query = %d \n",num_query);
	for(int j=0;j<100;j++){
		for(int i=0;i<num_query;i++){
			begin=rdtsc();
			hash_table_search = search_hash(query[i].upper_ip);
			if(hash_table_search){
				hash_succes++;
				match_rule++;
			}
			else{
				fail_hash++;
				int search_flag = BtreeSearch( btree_root, query[i].upper_ip, query[i].lower_ip , 1, btree_root->count);
				if(!search_flag){
					match_flag = 0;
					search_upper_Btree(1, allBtree_root, query[i].upper_ip , query[i].lower_ip , 1, allBtree_root->count);
					if( match_flag ){
						upper_match++;
						match_rule++;
					}
					else{
						no_match++;
					}
				}
			}
			end=rdtsc();
			if(clock2[i]>(end-begin))
				clock2[i]=(end-begin);
		}
	}
	for(int j=0;j<num_query;j++)
		total+=clock2[j];
	printf("---------------------------------Avg. Search: %llu--------------------------------\n",total/num_query);
	printf("match_rule = %d , no_match=%d , upper_match =%d\n",match_rule,no_match,upper_match);
	//printf("match_rule = %d , no_match=%d , hash_succes =%d , fail_hash =%d \n",match_rule,no_match,hash_succes, fail_hash);
}
void memory_cal(){
	count_node(btree_root);
	printf("# of node = %d \n",node_upper + node_layer0);
	printf("node_upper = %d\n",node_upper);
	printf("node_layer0 = %d\n",node_layer0);
	int Memory = (node_upper+node_layer0) * ( MAX*(16+1+1+1) + (MAX+1)*4);
	printf("Memory = %f kiB\n",(float)(Memory/1024));
	printf("mem_access =%d, Avg mem_access = %f\n", mem_access, ((float)mem_access/(float)num_query )/100 );
}
void insert_performance(){
	clock2=(unsigned long long int *)malloc(num_insert*sizeof(unsigned long long int));
	total=0;
	for(int j=0;j<num_insert;j++)
		clock2[j]=10000000;
	for( int i=0;i<num_insert;i++){
		layer0_flag=0;
		begin=rdtsc();
		//btree_insertion( &btree_root ,layer0_insert[i].upper_ip, layer0_insert[i].lower_ip,layer0_insert[i].len, 1);
		btree_insertion( &btree_root ,insert_table[i].upper_ip, insert_table[i].lower_ip,insert_table[i].len, 1);
		if(insert_table[i].len > 48 && layer0_flag == 0){
			if(delete_hashtable(insert_table[i].upper_ip,insert_table[i].lower_ip,insert_table[i].len)){
				delete_case++;
			}
		}
		end=rdtsc();
		clock2[i]=(end-begin);
	}
	
	for(int j=0;j<num_insert;j++)
		total+=clock2[j];
	printf("---------------------------------Avg. Insert: %llu---------------------------------\n",total/num_insert);
	printf(" # of insert = %d\n",num_insert);
	printf("Insert in layer-0 = %d , not_layer0 = %d\n",num_insert - not_layer0, not_layer0);
	printf("replace_layer0 = %d , same_replace =%d ,cover_layer0 =%d\n",replace_layer0,same_replace,cover_layer0);
}
void delete_performance(){
	printf("-----------------------------------Delete Start-----------------------------\n");
	printf(" # of delete = %d\n",num_delete);
	int found_delete=0;
	for(int j=0;j<num_delete;j++)
		clock2[j]=10000000;
	for( int i=0;i<num_delete;i++){
		begin=rdtsc();
		if( btree_delete( btree_root ,delete_table[i].upper_ip, delete_table[i].lower_ip,delete_table[i].len,1,btree_root->count) )
			found_delete++;
		else
			upper_btree_delete( 1, delete_table[i].upper_ip, delete_table[i].lower_ip,delete_table[i].len, 1, allBtree_root->count, allBtree_root);

		end=rdtsc();
		clock2[i]=(end-begin);
	}
	total=0;
	for(int j=0;j<num_delete;j++)
		total+=clock2[j];
	printf("-----------------------------------Avg. Delete: %llu-----------------------------\n",total/num_delete);
}
int main(int argc,char *argv[]){

	set_table(argv[1]);
	set_query(argv[1]);
	set_insert(argv[2]); //insert table
	set_delete(argv[2]); //delete table
	create();
	search(root);
	inition();
	layer0_to_Btree(root);
	search_performance();
	memory_cal();
	insert_performance();
	delete_performance();
	
}
