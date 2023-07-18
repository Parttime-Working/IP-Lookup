#ifndef MULTILAYER_H
#define MULTILAYER_H

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


typedef struct list{//structure of binary trie
	unsigned int port;
	struct list *left,*right,*parent;
	unsigned int layer;
	unsigned int ip;
	unsigned int len;
	unsigned int layer0_count;
	int level;
	int toplayer;
}node;
typedef node *btrie;

typedef struct Layer0_S{
	unsigned int ip;
	unsigned int len;
	unsigned int layer;
}Layer0_struct;

struct btreeNode {
  Layer0_struct *item;
  int count;
  struct btreeNode *link[MAX + 1]; 
};

struct Upper_BTreeNode {
  Layer0_struct *item;
  Layer0_struct *eset;
  int count;
  int eset_num;
  int max_len;
  struct Upper_BTreeNode * nextlevel_root[MAX + 1];
  struct Upper_BTreeNode *linker[MAX + 1];
};

typedef struct Top_S{
	unsigned int ip;
	unsigned int len;
	int num;
	btrie trie_node;
	struct Top_S * nextlevel;
	struct Upper_BTreeNode * nextlevel_root;
}Top_struct;

struct ENTRY{
	unsigned int ip;
	unsigned int len;
	unsigned int port;
	unsigned int layer;
	btrie root;
};

struct segment_array{
	unsigned int ip;
	unsigned int len;
	unsigned int port;
	unsigned int len_list;
};

typedef struct list_element{
	struct btreeNode *B_root;
	struct ENTRY * list_table;
	int num;
} tentry;


#endif 