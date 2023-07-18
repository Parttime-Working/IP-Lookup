#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
////////////////////////////////////////////////////////////////////////////////////
struct ENTRY {
    unsigned int ip;
    unsigned char len;
    unsigned char port;
};
////////////////////////////////////////////////////////////////////////////////////
static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}
////////////////////////////////////////////////////////////////////////////////////
struct balanced_node {
    unsigned int port;
    int layer;
    int highest_layer;  // -1: left higher or 1: right higher
    unsigned char len;
    unsigned int ip;
    struct balanced_node *left, *right, *parent;
};
////////////////////////////////////////////////////////////////////////////////////
/*global variables*/
struct list *root;
int num_entry = 0;
int num_query = 0;
struct ENTRY *table, *query;
int N = 0;  // number of nodes
unsigned long long int begin, end, total = 0;
unsigned long long int *my_clock;
int num_node = 0;  // total number of nodes in the binary trie
struct balanced_node **bt_root;  // an array of pointer
int total_layer = 1;             // {0, 1}
void print_bt(struct balanced_node *node, int layer);
void print_BF(struct balanced_node *node, int layer);
////////////////////////////////////////////////////////////////////////////////////
int compare_prefix(unsigned int a,
                   unsigned char len_a,
                   unsigned int b,
                   unsigned char len_b)
{
    unsigned char len = (len_a > len_b) ? len_b : len_a;
    a >>= (32 - len);
    b >>= (32 - len);
    return a - b;
    // 0: (a cover b) or (b cover a) or (a == b)
    // >0: a > b
    // <0: a < b
}
////////////////////////////////////////////////////////////////////////////////////
struct balanced_node *start, *start_parent;
void cal_layer(int layer, struct balanced_node *node)
{
    struct balanced_node *child = node;
    start = NULL;
    while (node != bt_root[layer]) {
        node = node->parent;
        if ((node->right == child &&
             (!node->left || node->left->layer < node->right->layer)) ||
            (node->left == child &&
             (!node->right || node->right->layer < node->left->layer))) {
            node->layer += 1;
        }
        if (node->right && node->left) {
            if (node->right->layer - node->left->layer == 2 ||
                node->right->layer - node->left->layer == -2) {
                start = node;
            }
        } else if (node->right) {
            if (node->right->layer == 2) {
                start = node;
            }
        } else if (node->left) {
            if (node->left->layer == 2) {
                start = node;
            }
        }
        child = node;
    }
}
struct balanced_node *find_highest_wrong_BF(struct balanced_node *node,
                                            struct balanced_node *target)
{
    struct balanced_node *now = node;
    while (now->left != target && now->right != target) {
        if (now->left && now->right) {
            if ((now->left->layer - now->right->layer > 1) ||
                (now->left->layer - now->right->layer < -1)) {
                return now;
            }
            if (target->ip > now->ip) {
                now = now->right;
            } else if (target->ip < now->ip) {
                now = now->left;
            } else {  // target == root
                perror("impossible! target == root\n");
                exit(1);
            }
        } else if (now->left) {
            if (now->left->layer > 0)
                return now;
            now = now->left;
        } else if (now->right) {
            if (now->right->layer > 0)
                return now;
            now = now->right;
        }
    }
    return NULL;
}
struct balanced_node *find_parent(int layer, struct balanced_node *target)
{
    if (!target) {
        return NULL;
    }
    if (bt_root[layer] == target) {
        return bt_root[layer];
    }
    struct balanced_node *parent = bt_root[layer];
    while (parent->left != target && parent->right != target) {
        if (parent->left && parent->ip > target->ip) {
            parent = parent->left;
        } else if (parent->right && parent->ip < target->ip) {
            parent = parent->right;
        } else {
            perror("Find parent failed!!!\n");
            exit(-1);
        }
    }
    return parent;
}
void rotate(int layer, struct balanced_node *target)
{
    cal_layer(layer, target);
    struct balanced_node *node2, *node3, *mid;
    /*start = find_highest_wrong_BF(bt_root[layer], target);*/

    if (start != NULL) {
        /*parent = find_parent(layer, start);*/
        /*parent = start_parent;*/
        if (start->left && start->ip > target->ip) {
            node2 = start->left;
            if (start->left->left && start->left->ip > target->ip) {
                // LL
                start->left = node2->right;
                node2->right = start;
                if (node2->right)
                    node2->right->parent = start;

                node2->parent = start->parent;
                start->parent = node2;

                mid = node2;
            } else if (start->left->right && start->left->ip < target->ip) {
                // LR
                node3 = node2->right;

                node2->right = node3->left;
                if (node3->left)
                    node3->left->parent = node2;
                start->left = node3->right;
                if (node3->right)
                    node3->right->parent = start;

                node3->left = node2;
                node3->right = start;

                node3->parent = start->parent;
                node2->parent = node3;
                start->parent = node3;

                mid = node3;
            }
        } else if (start->right && start->ip < target->ip) {
            node2 = start->right;
            if (start->right->left && start->right->ip > target->ip) {
                // RL
                node3 = node2->left;

                node2->left = node3->right;
                if (node3->right)
                    node3->right->parent = node2;
                start->right = node3->left;
                if (node3->left)
                    node3->left->parent = start;

                node3->right = node2;
                node3->left = start;

                node3->parent = start->parent;
                node2->parent = node3;
                start->parent = node3;

                mid = node3;
            } else if (start->right->right && start->right->ip < target->ip) {
                // RR
                start->right = node2->left;
                node2->left = start;
                if (node2->left)
                    node2->left->parent = start;

                node2->parent = start->parent;
                start->parent = node2;
                mid = node2;
            }
        } else {
            perror("rotate error\n");
            printf("target->ip: %x, len: %d\n", target->ip, target->len);
            exit(-1);
        }

        if (mid->parent == NULL) {  // start == root
            bt_root[layer] = mid;
        } else if (mid->parent->left == start) {
            mid->parent->left = mid;
        } else if (mid->parent->right == start) {
            mid->parent->right = mid;
        } else {
            printf("start: %x\n", start->ip);
            printf("mid:   %x\n", mid->ip);
            printf("mid->parent:   %x\n", mid->parent->ip);
            perror("Find parent failed\n");
            exit(-1);
        }
    }
}
char *convert_to_prefix(unsigned int ip, unsigned int len)
{
    int i;
    char *prefix = (char *) malloc(len + 1);
    for (i = 0; i < len; i++) {
        prefix[i] = ((ip >> (31 - i)) & 1) ? '1' : '0';
    }
    prefix[i] = '\0';
    return prefix;
}
void print_BF(struct balanced_node *node, int layer)
{
    if (!node) {
        return;
    }
    int i;
    for (i = 0; i < layer; i++) {
        printf("\t");
    }
    printf("%d", node->layer);
    printf("\n");
    print_BF(node->left, layer + 1);
    print_BF(node->right, layer + 1);
}
void insert_to_balanced_tree(int layer,
                             unsigned int ip,
                             unsigned char len,
                             unsigned char nexthop)
{
    /*printf("BF: --- %d\n", bt_root[layer]->BF);*/
    /*print_BF(bt_root[layer], 0);*/
    if (layer > total_layer) {
        bt_root = (struct balanced_node **) realloc(
            bt_root, sizeof(struct balanced_node *) * (layer + 1));
        bt_root[layer] =
            (struct balanced_node *) malloc(sizeof(struct balanced_node));
        printf("realloc : %d layer\n", layer);
        total_layer = layer;
        bt_root[layer]->port = 256;
    }
    if (bt_root[layer]->port == 256) {
        bt_root[layer]->port = nexthop;
        bt_root[layer]->ip = ip;
        bt_root[layer]->len = len;
        bt_root[layer]->left = NULL;
        bt_root[layer]->right = NULL;
        bt_root[layer]->parent = NULL;
        bt_root[layer]->layer = 0;
        bt_root[layer]->highest_layer = 0;
        return;
    }
    struct balanced_node *now;
    now = bt_root[layer];
    struct balanced_node *new =
        (struct balanced_node *) malloc(sizeof(struct balanced_node));
    new->port = nexthop;
    new->ip = ip;
    new->len = len;
    new->left = NULL;
    new->right = NULL;
    new->parent = NULL;
    new->layer = 0;
    new->highest_layer = 0;
    int res;
    while (now) {
        res = compare_prefix(ip, len, now->ip, now->len);
        // cover exist node -> level up
        if (res == 0 && len < now->len) {
            free(new);
            insert_to_balanced_tree(layer + 1, ip, len, nexthop);
            return;
        } else if (res == 0 && len > now->len) {
            free(new);
            insert_to_balanced_tree(layer + 1, now->ip, now->len, now->port);
            now->ip = ip;
            now->len = len;
            now->port = nexthop;
            return;
        } else if (res < 0) {
            if (!now->left || !now->left->len) {
                now->left = new;
                new->parent = now;
                rotate(layer, new);
                return;
            } else {
                now = now->left;
            }
        } else if (res > 0) {
            if (!now->right || !now->right->len) {
                now->right = new;
                new->parent = now;
                rotate(layer, new);
                return;
            } else {
                now = now->right;
            }
        } else {
            printf("insert same ip?\n");
            return;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
void read_table(char *str, unsigned int *ip, int *len, unsigned int *nexthop)
{
    char tok[] = "./";
    char buf[100], *str1;
    unsigned int n[4];
    sprintf(buf, "%s", strtok(str, tok));
    n[0] = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok));
    n[1] = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok));
    n[2] = atoi(buf);
    sprintf(buf, "%s", strtok(NULL, tok));
    n[3] = atoi(buf);
    *nexthop = n[2];
    str1 = (char *) strtok(NULL, tok);
    if (str1 != NULL) {
        sprintf(buf, "%s", str1);
        *len = atoi(buf);
    } else {
        if (n[1] == 0 && n[2] == 0 && n[3] == 0)
            *len = 8;
        else if (n[2] == 0 && n[3] == 0)
            *len = 16;
        else if (n[3] == 0)
            *len = 24;
    }
    *ip = n[0];
    *ip <<= 8;
    *ip += n[1];
    *ip <<= 8;
    *ip += n[2];
    *ip <<= 8;
    *ip += n[3];
}
////////////////////////////////////////////////////////////////////////////////////
void create()
{
    int i;

    bt_root =
        (struct balanced_node **) malloc(sizeof(struct balanced_node *) * 2);
    bt_root[0] = (struct balanced_node *) malloc(sizeof(struct balanced_node));
    bt_root[1] = (struct balanced_node *) malloc(sizeof(struct balanced_node));
    bt_root[0]->port = 256;
    bt_root[1]->port = 256;
    begin = rdtsc();
    for (i = 0; i < num_entry; i++) {
        if (i % 10000 == 0) {
            printf("(%4d/%d)\n", i, num_entry);
        }
        insert_to_balanced_tree(0, table[i].ip, table[i].len, table[i].port);
    }

    end = rdtsc();
    free(table);
}
////////////////////////////////////////////////////////////////////////////////////
int search(unsigned int ip)
{
    int j;
    struct balanced_node *now;
    int res;

    for (j = 0; j <= total_layer; j++) {
        now = bt_root[j];

        while (now) {
            // res = compare_prefix(ip, 32, now->ip, now->len);

            if (ip > now->ip)  // res == 1)
                now = now->right;
            else if (ip < now->ip)  // res == -1)
                now = now->left;
            else
                return now->port;
        }
    }
    return 256;
    /*if(temp==NULL)
      printf("default\n");
      else
      printf("%u\n",temp->port);*/
}
////////////////////////////////////////////////////////////////////////////////////
void set_table(char *file_name)
{
    FILE *fp;
    int len;
    char string[100];
    unsigned int ip, nexthop;
    fp = fopen(file_name, "r");
    while (fgets(string, 50, fp) != NULL) {
        read_table(string, &ip, &len, &nexthop);
        num_entry++;
    }
    rewind(fp);
    table = (struct ENTRY *) malloc(num_entry * sizeof(struct ENTRY));
    num_entry = 0;
    while (fgets(string, 50, fp) != NULL) {
        read_table(string, &ip, &len, &nexthop);
        table[num_entry].ip = ip;
        table[num_entry].port = nexthop;
        table[num_entry++].len = len;
    }
}
////////////////////////////////////////////////////////////////////////////////////
void set_query(char *file_name)
{
    FILE *fp;
    int len;
    char string[100];
    unsigned int ip, nexthop;
    fp = fopen(file_name, "r");
    while (fgets(string, 50, fp) != NULL) {
        read_table(string, &ip, &len, &nexthop);
        num_query++;
    }
    rewind(fp);
    query = (struct ENTRY *) malloc(num_query * sizeof(struct ENTRY));
    my_clock = (unsigned long long int *) malloc(
        num_query * sizeof(unsigned long long int));
    num_query = 0;
    while (fgets(string, 50, fp) != NULL) {
        read_table(string, &ip, &len, &nexthop);
        query[num_query].ip = ip;
        my_clock[num_query++] = 10000000;
    }
}
////////////////////////////////////////////////////////////////////////////////////
/*void count_node(struct list *r)
{
    if (r == NULL)
        return;
    count_node(r->left);
    N++;
    count_node(r->right);
}*/
////////////////////////////////////////////////////////////////////////////////////
void CountClock()
{
    unsigned int i;
    unsigned int *NumCntClock =
        (unsigned int *) malloc(50 * sizeof(unsigned int));
    for (i = 0; i < 50; i++)
        NumCntClock[i] = 0;
    unsigned long long MinClock = 10000000, MaxClock = 0;
    for (i = 0; i < num_query; i++) {
        if (my_clock[i] > MaxClock)
            MaxClock = my_clock[i];
        if (my_clock[i] < MinClock)
            MinClock = my_clock[i];
        if (my_clock[i] / 100 < 50)
            NumCntClock[my_clock[i] / 100]++;
        else
            NumCntClock[49]++;
    }
    printf("(MaxClock, MinClock) =\t(%5llu, %5llu)\n", MaxClock, MinClock);

    for (i = 0; i < 50; i++) {
        printf("%d\n", NumCntClock[i]);
    }
    return;
}

void shuffle(struct ENTRY *array, int n)
{
    srand((unsigned) time(NULL));
    struct ENTRY *temp = (struct ENTRY *) malloc(sizeof(struct ENTRY));

	int i;
    for (i = 0; i < n - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        temp->ip = array[j].ip;
        temp->len = array[j].len;
        temp->port = array[j].port;
        array[j].ip = array[i].ip;
        array[j].len = array[i].len;
        array[j].port = array[i].port;
        array[i].ip = temp->ip;
        array[i].len = temp->len;
        array[i].port = temp->port;
    }
}
////////////////////////////////////////////////////////////////////////////////////
void print_bt(struct balanced_node *node, int layer)
{
    if (!node || node->port == 256) {
        return;
    }
    int i;
    for (i = 0; i < layer; i++) {
        printf("\t");
    }
    printf("%x", node->ip);
    printf("\n");
    print_bt(node->left, layer + 1);
    print_bt(node->right, layer + 1);
}
////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    /*if(argc!=3){
        printf("Please execute the file as the following way:\n");
        printf("%s  routing_table_file_name  query_table_file_name\n",argv[0]);
        exit(1);
    }*/
    int i, j;
    // set_query(argv[2]);
    // set_table(argv[1]);
    set_query(argv[1]);
    set_table(argv[1]);
    create();
    printf("Avg. Insert:\t%llu\n", (end - begin) / num_entry);

    /* for (i = 0; i <= total_layer; i++) {*/
    /*printf("layer%d ---------\n", i);*/
    /*print_bt(bt_root[i], 0);*/
    /*}*/

    shuffle(query, num_entry);
    char *bit_string;
    ////////////////////////////////////////////////////////////////////////////
    int miss, max_miss = 0, min_miss = num_entry;
    for (j = 0; j < 100; j++) {
        miss = 0;
        for (i = 0; i < num_query; i++) {
            begin = rdtsc();
            if (search(query[i].ip) == 256) {
                miss++;
                printf("Not Found: %x\n", query[i].ip);
            }
            end = rdtsc();
            if (my_clock[i] > (end - begin))
                my_clock[i] = (end - begin);
            free(bit_string);
        }
        // printf("Loss rate: %f\n", (float) miss / (float) num_entry);
        if (miss > max_miss) {
            max_miss = miss;
        }
        if (miss < min_miss)
            min_miss = miss;
    }
    printf("Max Loss rate: %f\n", (float) max_miss / (float) num_entry);
    printf("Min Loss rate: %f\n", (float) min_miss / (float) num_entry);
    total = 0;
    for (j = 0; j < num_query; j++)
        total += my_clock[j];
    num_node = num_query;
    printf("Avg. Search:\t%llu\n", total / num_query);
    printf("Total memory requirement:\t%ld KB\n",
           ((num_entry * sizeof(struct balanced_node)) / 1024));
    CountClock();
    ////////////////////////////////////////////////////////////////////////////
    // count_node(root);
    // printf("There are %d nodes in binary trie\n",N);
    return 0;
}
