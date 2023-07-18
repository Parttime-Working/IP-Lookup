#include<stdlib.h>
#include<stdio.h>
#include<string.h>

////////////////////////////////////////////////////////////////////////////////////
struct ENTRY{
	unsigned long long int ip1;
	unsigned long long int ip2;
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
int match_case = 0, loss_case = 0;
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
void add_node(unsigned long long int ip1, unsigned long long int ip2,unsigned char len,unsigned char nexthop){
	btrie ptr=root;
	int i, ipv6_count = 0;
	unsigned long long int int_one = 1;
	unsigned long long int int_zero = 0;
	if(len == 0)
		ptr -> port = nexthop;
	for(i=0;i<len;i+=4){
		if(i<64){
			if(ip1 & (int_one << 63-i) && ip1 & (int_one << 63 - (i+1)) && ip1 & (int_one << 63 - (i+2)) && ip1 & (int_one << 63 - (i+3))){
				if(ptr -> node_1111 == NULL)
					ptr -> node_1111 = create_node();
				ptr = ptr -> node_1111;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip1 & (int_one << 63-i) && ip1 & (int_one << 63 - (i+1)) && ip1 & (int_one << 63 - (i+2)) && ~(ip1 ^ (int_zero << 63 - (i+3)))){
				if(ptr -> node_1110 == NULL)
					ptr -> node_1110 = create_node();
				ptr = ptr -> node_1110;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip1 & (int_one << 63-i) && ip1 & (int_one << 63 - (i+1)) && ~(ip1 ^ (int_zero << 63 - (i+2))) && ip1 & (int_one << 63 - (i+3))){
				if(ptr -> node_1101 == NULL)
					ptr -> node_1101 = create_node();
				ptr = ptr -> node_1101;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip1 & (int_one << 63-i) && ip1 & (int_one << 63 - (i+1)) && ~(ip1 ^ (int_zero << 63 - (i+2))) && ~(ip1 ^ (int_zero << 63 - (i+3)))){
				if(ptr -> node_1100 == NULL)
					ptr -> node_1100 = create_node();
				ptr = ptr -> node_1100;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip1 & (int_one << 63-i) && ~(ip1 ^ (int_zero << 63 - (i+1))) && ip1 & (int_one << 63 - (i+2)) && ip1 & (int_one << 63 - (i+3))){
				if(ptr -> node_1011 == NULL)
					ptr -> node_1011 = create_node();
				ptr = ptr -> node_1011;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip1 & (int_one << 63-i) && ~(ip1 ^ (int_zero << 63 - (i+1))) && ip1 & (int_one << 63 - (i+2)) && ~(ip1 ^ (int_zero << 63 - (i+3)))){
				if(ptr -> node_1010 == NULL)
					ptr -> node_1010 = create_node();
				ptr = ptr -> node_1010;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip1 & (int_one << 63-i) && ~(ip1 ^ (int_zero << 63 - (i+1))) && ~(ip1 ^ (int_zero << 63 - (i+2))) && ip1 & (int_one << 63 - (i+3))){
				if(ptr -> node_1001 == NULL)
					ptr -> node_1001 = create_node();
				ptr = ptr -> node_1001;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip1 & (int_one << 63-i) && ~(ip1 ^ (int_zero << 63 - (i+1))) && ~(ip1 ^ (int_zero << 63 - (i+2))) && ~(ip1 ^ (int_zero << 63 - (i+3)))){
				if(ptr -> node_1000 == NULL)
					ptr -> node_1000 = create_node();
				ptr = ptr -> node_1000;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip1 ^ (int_zero << 63-i)) && ip1 & (int_one << 63 - (i+1)) && ip1 & (int_one << 63 - (i+2)) && ip1 & (int_one << 63 - (i+3))){
				if(ptr -> node_0111 == NULL)
					ptr -> node_0111 = create_node();
				ptr = ptr -> node_0111;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip1 ^ (int_zero << 63-i)) && ip1 & (int_one << 63 - (i+1)) && ip1 & (int_one << 63 - (i+2)) && ~(ip1 ^ (int_zero << 63 - (i+3)))){
				if(ptr -> node_0110 == NULL)
					ptr -> node_0110 = create_node();
				ptr = ptr -> node_0110;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip1 ^ (int_zero << 63-i)) && ip1 & (int_one << 63 - (i+1)) && ~(ip1 ^ (int_zero << 63 - (i+2))) && ip1 & (int_one << 63 - (i+3))){
				if(ptr -> node_0101 == NULL)
					ptr -> node_0101 = create_node();
				ptr = ptr -> node_0101;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip1 ^ (int_zero << 63-i)) && ip1 & (int_one << 63 - (i+1)) && ~(ip1 ^ (int_zero << 63 - (i+2))) && ~(ip1 ^ (int_zero << 63 - (i+3)))){
				if(ptr -> node_0100 == NULL)
					ptr -> node_0100 = create_node();
				ptr = ptr -> node_0100;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip1 ^ (int_zero << 63-i)) && ~(ip1 ^ (int_zero << 63 - (i+1))) && ip1 & (int_one << 63 - (i+2)) && ip1 & (int_one << 63 - (i+3))){
				if(ptr -> node_0011 == NULL)
					ptr -> node_0011 = create_node();
				ptr = ptr -> node_0011;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip1 ^ (int_zero << 63-i)) && ~(ip1 ^ (int_zero << 63 - (i+1))) && ip1 & (int_one << 63 - (i+2)) && ~(ip1 ^ (int_zero << 63 - (i+3)))){
				if(ptr -> node_0010 == NULL)
					ptr -> node_0010 = create_node();
				ptr = ptr -> node_0010;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip1 ^ (int_zero << 63-i)) && ~(ip1 ^ (int_zero << 63 - (i+1))) && ~(ip1 ^ (int_zero << 63 - (i+2))) && ip1 & (int_one << 63 - (i+3))){
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
		else{
			if(ip2 & (int_one << 63-(i-64)) && ip2 & (int_one << 63 - (i-64+1)) && ip2 & (int_one << 63 - (i-64+2)) && ip2 & (int_one << 63 - (i-64+3))){
				if(ptr -> node_1111 == NULL)
					ptr -> node_1111 = create_node();
				ptr = ptr -> node_1111;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip2 & (int_one << 63-(i-64)) && ip2 & (int_one << 63 - (i-64+1)) && ip2 & (int_one << 63 - (i-64+2)) && ~(ip2 ^ (int_zero << 63 - (i-64+3)))){
				if(ptr -> node_1110 == NULL)
					ptr -> node_1110 = create_node();
				ptr = ptr -> node_1110;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip2 & (int_one << 63-(i-64)) && ip2 & (int_one << 63 - (i-64+1)) && ~(ip2 ^ (int_zero << 63 - (i-64+2))) && ip2 & (int_one << 63 - (i-64+3))){
				if(ptr -> node_1101 == NULL)
					ptr -> node_1101 = create_node();
				ptr = ptr -> node_1101;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip2 & (int_one << 63-(i-64)) && ip2 & (int_one << 63 - (i-64+1)) && ~(ip2 ^ (int_zero << 63 - (i-64+2))) && ~(ip2 ^ (int_zero << 63 - (i-64+3)))){
				if(ptr -> node_1100 == NULL)
					ptr -> node_1100 = create_node();
				ptr = ptr -> node_1100;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip2 & (int_one << 63-(i-64)) && ~(ip2 ^ (int_zero << 63 - (i-64+1))) && ip2 & (int_one << 63 - (i-64+2)) && ip2 & (int_one << 63 - (i-64+3))){
				if(ptr -> node_1011 == NULL)
					ptr -> node_1011 = create_node();
				ptr = ptr -> node_1011;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip2 & (int_one << 63-(i-64)) && ~(ip2 ^ (int_zero << 63 - (i-64+1))) && ip2 & (int_one << 63 - (i-64+2)) && ~(ip2 ^ (int_zero << 63 - (i-64+3)))){
				if(ptr -> node_1010 == NULL)
					ptr -> node_1010 = create_node();
				ptr = ptr -> node_1010;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip2 & (int_one << 63-(i-64)) && ~(ip2 ^ (int_zero << 63 - (i-64+1))) && ~(ip2 ^ (int_zero << 63 - (i-64+2))) && ip2 & (int_one << 63 - (i-64+3))){
				if(ptr -> node_1001 == NULL)
					ptr -> node_1001 = create_node();
				ptr = ptr -> node_1001;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(ip2 & (int_one << 63-(i-64)) && ~(ip2 ^ (int_zero << 63 - (i-64+1))) && ~(ip2 ^ (int_zero << 63 - (i-64+2))) && ~(ip2 ^ (int_zero << 63 - (i-64+3)))){
				if(ptr -> node_1000 == NULL)
					ptr -> node_1000 = create_node();
				ptr = ptr -> node_1000;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip2 ^ (int_zero << 63-(i-64))) && ip2 & (int_one << 63 - (i-64+1)) && ip2 & (int_one << 63 - (i-64+2)) && ip2 & (int_one << 63 - (i-64+3))){
				if(ptr -> node_0111 == NULL)
					ptr -> node_0111 = create_node();
				ptr = ptr -> node_0111;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip2 ^ (int_zero << 63-(i-64))) && ip2 & (int_one << 63 - (i-64+1)) && ip2 & (int_one << 63 - (i-64+2)) && ~(ip2 ^ (int_zero << 63 - (i-64+3)))){
				if(ptr -> node_0110 == NULL)
					ptr -> node_0110 = create_node();
				ptr = ptr -> node_0110;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip2 ^ (int_zero << 63-(i-64))) && ip2 & (int_one << 63 - (i-64+1)) && ~(ip2 ^ (int_zero << 63 - (i-64+2))) && ip2 & (int_one << 63 - (i-64+3))){
				if(ptr -> node_0101 == NULL)
					ptr -> node_0101 = create_node();
				ptr = ptr -> node_0101;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip2 ^ (int_zero << 63-(i-64))) && ip2 & (int_one << 63 - (i-64+1)) && ~(ip2 ^ (int_zero << 63 - (i-64+2))) && ~(ip2 ^ (int_zero << 63 - (i-64+3)))){
				if(ptr -> node_0100 == NULL)
					ptr -> node_0100 = create_node();
				ptr = ptr -> node_0100;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip2 ^ (int_zero << 63-(i-64))) && ~(ip2 ^ (int_zero << 63 - (i-64+1))) && ip2 & (int_one << 63 - (i-64+2)) && ip2 & (int_one << 63 - (i-64+3))){
				if(ptr -> node_0011 == NULL)
					ptr -> node_0011 = create_node();
				ptr = ptr -> node_0011;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip2 ^ (int_zero << 63-(i-64))) && ~(ip2 ^ (int_zero << 63 - (i-64+1))) && ip2 & (int_one << 63 - (i-64+2)) && ~(ip2 ^ (int_zero << 63 - (i-64+3)))){
				if(ptr -> node_0010 == NULL)
					ptr -> node_0010 = create_node();
				ptr = ptr -> node_0010;
				if((i + 4 >= len) && (ptr->port == 256))
					ptr -> port = nexthop;
			}
			else if(~(ip2 ^ (int_zero << 63-(i-64))) && ~(ip2 ^ (int_zero << 63 - (i-64+1))) && ~(ip2 ^ (int_zero << 63 - (i-64+2))) && ip2 & (int_one << 63 - (i-64+3))){
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
}
////////////////////////////////////////////////////////////////////////////////////

int getIndexOfSigns(char ch)
{
    if(ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if(ch >= 'A' && ch <='F') 
    {
        return ch - 'A' + 10;
    }
    if(ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    return -1;
}


unsigned long long int hexToDec(char *source)
{
    unsigned long long int sum = 0;
    unsigned long long int t = 1;
    int i, len;
 
    len = strlen(source);
    for(i=len-1; i>=0; i--)
    {
        sum += t * getIndexOfSigns(*(source + i));
        t *= 16;
    }
	//printf("sum : %llu\n", sum);
    return sum;
}

void read_table(char *str,unsigned long long int *ip1,unsigned long long int *ip2, int *len,unsigned int *nexthop){
	*ip1 = 0;
	*ip2 = 0;
	char tok[]="/";
	char buf[100], buf2[100],*str1;
	sprintf(buf,"%s\0",strtok(str,tok));
	strcpy(buf2, buf);

	str1=(char *)strtok(NULL,tok);
	if(str1!=NULL){
		sprintf(buf,"%s\0",str1);
		*len=atoi(buf);
		//printf("%d\n", *len);
	}

	//printf("%s, %d\n", buf2, strlen(buf2));
	int ipv6_count = 0;
	char * cut_buf = NULL;
	cut_buf = strtok(buf2,":");
	//sprintf(buf,"%s\0",strtok(buf2,":"));
	
	while(1){
		//printf("in while\n");
		if(cut_buf == NULL){
			
			if(ipv6_count < 64){
				*ip1 = *ip1 << (64 - ipv6_count);
			}
			else
			{
				*ip2 = *ip2 << (128 - ipv6_count);
			}
			
			break;
		}
			
		//printf("cut_buf : %s\n", cut_buf);
		if(ipv6_count < 64){
			*ip1 = *ip1 << 16;
			ipv6_count += 16;
			*ip1 += hexToDec(cut_buf);
		}
		else
		{
			*ip2 = *ip2 << 16;
			ipv6_count += 16;
			*ip2 += hexToDec(cut_buf);
		}
		cut_buf = strtok(NULL,":");
		//sprintf(buf,"%s\0",strtok(NULL,":"));
		//printf("out while\n");
	}
	//printf("%llu, %llu\n", *ip1, *ip2);
	*nexthop = 39;
}
////////////////////////////////////////////////////////////////////////////////////
void search(unsigned long long int ip1, unsigned long long int ip2){
	int j;
	unsigned long long int int_one = 1;
	unsigned long long int int_zero = 0;
	btrie current=root,temp=NULL;
	for(j=0; j<128; j++){
		if(current==NULL)
			break;
		if(current->port!=256)
			temp=current;
		if(j < 64){
			if(ip1 & (int_one << 63-j) && ip1 & (int_one << (63-(j+1))) && ip1 & (int_one << (63-(j+2))) && ip1 & (int_one << (63-(j+3)))){
				current = current -> node_1111;
			}
			else if(ip1 & (int_one << 63-j) && ip1 & (int_one << (63-(j+1))) && ip1 & (int_one << (63-(j+2))) && ~(ip1 ^ (int_zero << (63-(j+3))))){
				current = current -> node_1110;
			}
			else if(ip1 & (int_one << 63-j) && ip1 & (int_one << (63-(j+1))) && ~(ip1 ^ (int_zero << (63-(j+2)))) && ip1 & (int_one << (63-(j+3)))){
				current = current -> node_1101;
			}
			else if(ip1 & (int_one << 63-j) && ip1 & (int_one << (63-(j+1))) && ~(ip1 ^ (int_zero << (63-(j+2)))) && ~(ip1 ^ (int_zero << (63-(j+3))))){
				current = current -> node_1100;
			}
			else if(ip1 & (int_one << 63-j) && ~(ip1 ^ (int_zero << (63-(j+1)))) && ip1 & (int_one << (63-(j+2))) && ip1 & (int_one << (63-(j+3)))){
				current = current -> node_1011;
			}
			else if(ip1 & (int_one << 63-j) && ~(ip1 ^ (int_zero << (63-(j+1)))) && ip1 & (int_one << (63-(j+2))) && ~(ip1 ^ (int_zero << (63-(j+3))))){
				current = current -> node_1010;
			}
			else if(ip1 & (int_one << 63-j) && ~(ip1 ^ (int_zero << (63-(j+1)))) && ~(ip1 ^ (int_zero << (63-(j+2)))) && ip1 & (int_one << (63-(j+3)))){
				current = current -> node_1001;
			}
			else if(ip1 & (int_one << 63-j) && ~(ip1 ^ (int_zero << (63-(j+1)))) && ~(ip1 ^ (int_zero << (63-(j+2)))) && ~(ip1 ^ (int_zero << (63-(j+3))))){
				current = current -> node_1000;
			}
			else if(~(ip1 ^ (int_zero << 63-j)) && ip1 & (int_one << (63-(j+1))) && ip1 & (int_one << (63-(j+2))) && ip1 & (int_one << (63-(j+3)))){
				current = current -> node_0111;
			}
			else if(~(ip1 ^ (int_zero << 63-j)) && ip1 & (int_one << (63-(j+1))) && ip1 & (int_one << (63-(j+2))) && ~(ip1 ^ (int_zero << (63-(j+3))))){
				current = current -> node_0110;
			}
			else if(~(ip1 ^ (int_zero << 63-j)) && ip1 & (int_one << (63-(j+1))) && ~(ip1 ^ (int_zero << (63-(j+2)))) && ip1 & (int_one << (63-(j+3)))){
				current = current -> node_0101;
			}
			else if(~(ip1 ^ (int_zero << 63-j)) && ip1 & (int_one << (63-(j+1))) && ~(ip1 ^ (int_zero << (63-(j+2)))) && ~(ip1 ^ (int_zero << (63-(j+3))))){
				current = current -> node_0100;
			}
			else if(~(ip1 ^ (int_zero << 63-j)) && ~(ip1 ^ (int_zero << (63-(j+1)))) && ip1 & (int_one << (63-(j+2))) && ip1 & (int_one << (63-(j+3)))){
				current = current -> node_0011;
			}
			else if(~(ip1 ^ (int_zero << 63-j)) && ~(ip1 ^ (int_zero << (63-(j+1)))) && ip1 & (int_one << (63-(j+2))) && ~(ip1 ^ (int_zero << (63-(j+3))))){
				current = current -> node_0010;
			}
			else if(~(ip1 ^ (int_zero << 63-j)) && ~(ip1 ^ (int_zero << (63-(j+1)))) && ~(ip1 ^ (int_zero << (63-(j+2)))) && ip1 & (int_one << (63-(j+3)))){
				current = current -> node_0001;
			}
			else {
				current = current -> node_0000;
			}
		}
		else{
			if(ip2 & (int_one << 63-(j-64)) && ip2 & (int_one << (63-(j-64+1))) && ip2 & (int_one << (63-(j-64+2))) && ip2 & (int_one << (63-(j-64+3)))){
				current = current -> node_1111;
			}
			else if(ip2 & (int_one << 63-(j-64)) && ip2 & (int_one << (63-(j-64+1))) && ip2 & (int_one << (63-(j-64+2))) && ~(ip2 ^ (int_zero << (63-(j-64+3))))){
				current = current -> node_1110;
			}
			else if(ip2 & (int_one << 63-(j-64)) && ip2 & (int_one << (63-(j-64+1))) && ~(ip2 ^ (int_zero << (63-(j-64+2)))) && ip2 & (int_one << (63-(j-64+3)))){
				current = current -> node_1101;
			}
			else if(ip2 & (int_one << 63-(j-64)) && ip2 & (int_one << (63-(j-64+1))) && ~(ip2 ^ (int_zero << (63-(j-64+2)))) && ~(ip2 ^ (int_zero << (63-(j-64+3))))){
				current = current -> node_1100;
			}
			else if(ip2 & (int_one << 63-(j-64)) && ~(ip2 ^ (int_zero << (63-(j-64+1)))) && ip2 & (int_one << (63-(j-64+2))) && ip2 & (int_one << (63-(j-64+3)))){
				current = current -> node_1011;
			}
			else if(ip2 & (int_one << 63-(j-64)) && ~(ip2 ^ (int_zero << (63-(j-64+1)))) && ip2 & (int_one << (63-(j-64+2))) && ~(ip2 ^ (int_zero << (63-(j-64+3))))){
				current = current -> node_1010;
			}
			else if(ip2 & (int_one << 63-(j-64)) && ~(ip2 ^ (int_zero << (63-(j-64+1)))) && ~(ip2 ^ (int_zero << (63-(j-64+2)))) && ip2 & (int_one << (63-(j-64+3)))){
				current = current -> node_1001;
			}
			else if(ip2 & (int_one << 63-(j-64)) && ~(ip2 ^ (int_zero << (63-(j-64+1)))) && ~(ip2 ^ (int_zero << (63-(j-64+2)))) && ~(ip2 ^ (int_zero << (63-(j-64+3))))){
				current = current -> node_1000;
			}
			else if(~(ip2 ^ (int_zero << 63-(j-64))) && ip2 & (int_one << (63-(j-64+1))) && ip2 & (int_one << (63-(j-64+2))) && ip2 & (int_one << (63-(j-64+3)))){
				current = current -> node_0111;
			}
			else if(~(ip2 ^ (int_zero << 63-(j-64))) && ip2 & (int_one << (63-(j-64+1))) && ip2 & (int_one << (63-(j-64+2))) && ~(ip2 ^ (int_zero << (63-(j-64+3))))){
				current = current -> node_0110;
			}
			else if(~(ip2 ^ (int_zero << 63-(j-64))) && ip2 & (int_one << (63-(j-64+1))) && ~(ip2 ^ (int_zero << (63-(j-64+2)))) && ip2 & (int_one << (63-(j-64+3)))){
				current = current -> node_0101;
			}
			else if(~(ip2 ^ (int_zero << 63-(j-64))) && ip2 & (int_one << (63-(j-64+1))) && ~(ip2 ^ (int_zero << (63-(j-64+2)))) && ~(ip2 ^ (int_zero << (63-(j-64+3))))){
				current = current -> node_0100;
			}
			else if(~(ip2 ^ (int_zero << 63-(j-64))) && ~(ip2 ^ (int_zero << (63-(j-64+1)))) && ip2 & (int_one << (63-(j-64+2))) && ip2 & (int_one << (63-(j-64+3)))){
				current = current -> node_0011;
			}
			else if(~(ip2 ^ (int_zero << 63-(j-64))) && ~(ip2 ^ (int_zero << (63-(j-64+1)))) && ip2 & (int_one << (63-(j-64+2))) && ~(ip2 ^ (int_zero << (63-(j-64+3))))){
				current = current -> node_0010;
			}
			else if(~(ip2 ^ (int_zero << 63-(j-64))) && ~(ip2 ^ (int_zero << (63-(j-64+1)))) && ~(ip2 ^ (int_zero << (63-(j-64+2)))) && ip2 & (int_one << (63-(j-64+3)))){
				current = current -> node_0001;
			}
			else {
				current = current -> node_0000;
			}
		}
		
	}
	if(temp==NULL){
		//printf("default: %llu, %llu\n", ip1, ip2);
		loss_case++;
	}else{
		match_case++;
	}
	  
	  /*else
	  printf("%u\n",temp->port);*/
}
////////////////////////////////////////////////////////////////////////////////////
void set_table(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned long long int ip1, ip2;
	unsigned int nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2,&len,&nexthop);
		num_entry++;
	}
	rewind(fp);
	table=(struct ENTRY *)malloc(num_entry*sizeof(struct ENTRY));
	num_entry=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2,&len,&nexthop);
		table[num_entry].ip1=ip1;
		table[num_entry].ip2=ip2;
		table[num_entry].port=nexthop;
		table[num_entry++].len=len;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void set_query(char *file_name){
	FILE *fp;
	int len;
	char string[100];
	unsigned long long int ip1, ip2;
	unsigned int nexthop;
	fp=fopen(file_name,"r");
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2,&len,&nexthop);
		num_query++;
	}
	rewind(fp);
	query=(struct ENTRY *)malloc(num_query*sizeof(struct ENTRY));
	clock=(unsigned long long int *)malloc(num_query*sizeof(unsigned long long int));
	num_query=0;
	while(fgets(string,50,fp)!=NULL){
		read_table(string,&ip1, &ip2 ,&len,&nexthop);
		query[num_query].ip1=ip1;
		query[num_query].ip2=ip2;
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
	for(i=0;i<num_entry/10*9;i++)
		add_node(table[i].ip1,table[i].ip2, table[i].len,table[i].port);
	end=rdtsc();

	printf("Avg. Build: %llu\n",(end-begin)/(num_entry/10*9));


	begin=rdtsc();
	for(i=num_entry/10*9;i<num_entry;i++)
		add_node(table[i].ip1,table[i].ip2, table[i].len,table[i].port);
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

        temp->ip1=array[j].ip1;
		temp->ip2=array[j].ip2;
        temp->len=array[j].len;
        temp->port=array[j].port;
        array[j].ip1 = array[i].ip1;
		array[j].ip2 = array[i].ip2;
        array[j].len = array[i].len;
        array[j].port = array[i].port;
        array[i].ip1 = temp->ip1;
		array[i].ip2 = temp->ip2;
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
	//printf("end create\n");
	//////////////
	/*
	int prefix_array[129] = {0};
	for(i = 0; i < num_entry; ++i){
		prefix_array[table[i].len]++;
	}
	for(i = 0; i < 129; ++i){
		printf("%u\n", prefix_array[i]);
	}
	*/
	//////////////

	printf("number of nodes: %d\n",num_node);
	printf("num entry : %d\n", num_entry);
	printf("Total memory requirement: %d KB\n",((num_node*sizeof(struct list))/1024));

	shuffle(query, num_query);
	//printf("end shuffle\n"); 
	////////////////////////////////////////////////////////////////////////////
	for(j=0;j<100;j++){
		for(i=0;i<num_query;i++){
			begin=rdtsc();
			search(query[i].ip1, query[i].ip2);
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
	printf("loss case: %d, match case : %d\n", loss_case, match_case);
	////////////////////////////////////////////////////////////////////////////
	//count_node(root);
	//printf("There are %d nodes in binary trie\n",N);
	return 0;
}
