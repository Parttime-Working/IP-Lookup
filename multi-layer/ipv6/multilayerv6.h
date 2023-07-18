#ifndef MULTILAYERV6_H
#define MULTILAYERV6_H
// #define MAX 256  
// #define MIN 128 
// #define MAX 128  
// #define MIN 64 
// #define MAX 64 
// #define MIN 32 
// #define MAX 32
// #define MIN 16
#define MAX 16
#define MIN 8
// #define MAX 8
// #define MIN 4

struct ENTRY{
	unsigned long long int upper_ip;
	unsigned long long int lower_ip;
	unsigned int len;
	unsigned int port;
	int coll;
};
typedef struct HASH{
	unsigned long long int upper_ip;
	unsigned int port;
	int index;
} tentry;
struct ip_infor{
	unsigned long long int upper_ip;
	unsigned long long int lower_ip;
	unsigned int len;
	unsigned int valid;
};
struct btreeNode {
  struct ip_infor *item;
  int count;
  struct btreeNode *link[MAX + 1];
};
struct upper_BtreeNode {
  struct ip_infor *item;
  struct ip_infor *eset;
  int count;
  int eset_num;
  struct upper_BtreeNode * nextlevel_root[MAX + 1];
  struct upper_BtreeNode *linker[MAX + 1];
};
//////////////////////////////////////////////////////////////////////////
typedef struct arr{
	unsigned long long int upper_ip;
	unsigned long long int lower_ip;
	unsigned int len;
	unsigned int layer;
}arr_entry;

typedef struct list{//structure of binary trie
	unsigned int port;
	struct list *left,*right;
	unsigned int layer;
	unsigned long long int upper_ip;
	unsigned long long int lower_ip;
	unsigned int len;
	int toplayer;
}node;
typedef node *btrie;
typedef struct arr2{
	unsigned long long int upper_ip;
	unsigned long long int lower_ip;
	unsigned int len;
	int num;
	btrie trie_node;
	struct upper_BtreeNode * nextlevel_root;
	struct arr2 * nextlevel;
}arr_entry2;
#endif