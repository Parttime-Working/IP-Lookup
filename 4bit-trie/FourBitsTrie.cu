#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// CUDA runtime
#include <cuda_runtime.h>

typedef struct entry
{
	unsigned int IP;
	unsigned char len;
	unsigned char nexthop;
}ENTRY;

typedef struct trie
{
	unsigned int* ArrayNode;
	unsigned int* ArrayNHP;
	unsigned char* ArrayLen;
}TRIE;

ENTRY* TableEntry;
unsigned int num_Entry = 0;
unsigned int num_Query = 0;
unsigned int NumNode = 0;

TRIE root;
unsigned int* D_ArrayNode;
unsigned int* D_ArrayNHP;
unsigned int* H_TableQuery;
unsigned int* H_TableQueryNHP;
unsigned int* D_TableQuery;
unsigned int* D_TableQueryNHP;

ENTRY ReadTable(char* str)
{
	char token[] = "./";
	char buf[256];
	char* strTemp;
	ENTRY entryTemp;
	
	sprintf(buf, "%s\0", strtok(str, token));
	entryTemp.IP = atoi(buf);
	sprintf(buf, "%s\0", strtok(NULL, token));
	entryTemp.IP = (entryTemp.IP << 8) + atoi(buf);
	sprintf(buf, "%s\0", strtok(NULL, token));
	entryTemp.IP = (entryTemp.IP << 8) + atoi(buf);
	sprintf(buf, "%s\0", strtok(NULL, token));
	entryTemp.IP = (entryTemp.IP << 8) + atoi(buf);
	entryTemp.nexthop = (entryTemp.IP >> 8) % 256;
	strTemp = (char* )strtok(NULL, token);
	
	unsigned int i;
	if (strTemp != NULL)
	{
		sprintf(buf, "%s\0", strTemp);
		entryTemp.len = atoi(buf);
	}
	else
	{
		if (entryTemp.IP >> 24 == 0 && (entryTemp.IP >> 16) % 256 == 0 && (entryTemp.IP >> 8) % 256 == 0 && entryTemp.IP % 256 == 0) entryTemp.len = 0;
		else if((entryTemp.IP >> 16) % 256 == 0 && (entryTemp.IP >> 8) % 256 == 0 && entryTemp.IP % 256 == 0)
		{
			i = 0;
			while(1)
			{
				if (entryTemp.IP >> 24 & (1 << i)) i++;
				else break;
			}
			entryTemp.len = 8 - i;
		}
		else if ((entryTemp.IP >> 8) % 256 == 0 && entryTemp.IP % 256 == 0)
		{
			i = 0;
			while(1)
			{
				if ((entryTemp.IP >> 8) % 256 & (1 << i)) i++;
				else break;
			}
			entryTemp.len = 16 - i;
		}
		else if (entryTemp.IP % 256 == 0)
		{
			i = 0;
			while(1)
			{
				if ((entryTemp.IP >> 8) % 256 & (1 << i)) i++;
				else break;
			}
			entryTemp.len = 24 - i;
		}		
		else
		{
			i = 0;
			while(1)
			{
				if (entryTemp.IP % 256 & (1 << i)) i++;
				else break;
			}
			entryTemp.len = 32 - i;
		}
	}
	return entryTemp;
}

void SetEntryTable(char* FileName)
{
	FILE* ipt = fopen(FileName, "r");
	char buf[256];
	
	num_Entry = 0;
	while(fgets(buf, 256, ipt) != NULL) num_Entry++;
	TableEntry = (ENTRY* )malloc(num_Entry * sizeof(ENTRY));
	
	rewind(ipt);
	num_Entry = 0;
	while(fgets(buf, 256, ipt) != NULL)
	{
		TableEntry[num_Entry] = ReadTable(buf);
		num_Entry++;
	}
	
	fclose(ipt);
	return;
}

TRIE CreateNode()
{
	unsigned int i;
	TRIE TrieTemp;
	if (NumNode == 0)
	{
		TrieTemp.ArrayNode = (unsigned int* )malloc(16 * sizeof(unsigned int));
		TrieTemp.ArrayNHP = (unsigned int* )malloc(sizeof(unsigned int));
		TrieTemp.ArrayLen = (unsigned char* )malloc(sizeof(unsigned char));
	}
	else
	{
		TrieTemp.ArrayNode = (unsigned int* )realloc(root.ArrayNode, 16 * (NumNode + 1) * sizeof(unsigned int));
		TrieTemp.ArrayNHP = (unsigned int* )realloc(root.ArrayNHP, (NumNode + 1) * sizeof(unsigned int));
		TrieTemp.ArrayLen = (unsigned char* )realloc(root.ArrayLen, (NumNode + 1) * sizeof(unsigned char));
	}
	for(i = 0; i < 16; i++) TrieTemp.ArrayNode[16 * NumNode + i] = 0;
	TrieTemp.ArrayNHP[NumNode] = 256;
	TrieTemp.ArrayLen[NumNode] = 0;
	NumNode++;
	return TrieTemp;
}

void AddNode(ENTRY info)
{
	unsigned int curPos = 0;
	unsigned int i, limit, remainder, tmp;
	
	remainder = info.len % 4;
	limit = info.len / 4;
	
	for(i = 0; i < limit; i++)
	{
		tmp = (info.IP >> (28 - i * 4)) % 16;
		if(root.ArrayNode[16 * curPos + tmp] == 0) 
		{
			root = CreateNode();
			root.ArrayNode[16 * curPos + tmp] = NumNode - 1;
		}
		curPos = root.ArrayNode[16 * curPos + tmp];
	}
	if (remainder)
	{
		tmp = (info.IP >> (28 - limit * 4)) % 16;
		unsigned int count = 1 << (4 - remainder);
		for(i = 0; i < count; i++)
		{
			if (root.ArrayNode[16 * curPos + tmp + i] == 0)
			{
				root = CreateNode();
				root.ArrayNode[16 * curPos + tmp + i] = NumNode - 1;
			}
			if (root.ArrayLen[root.ArrayNode[16 * curPos + tmp + i]] < info.len)
			{
				root.ArrayNHP[root.ArrayNode[16 * curPos + tmp + i]] = info.nexthop;
				root.ArrayLen[root.ArrayNode[16 * curPos + tmp + i]] = info.len;
			}
		}
	}
	else
	{
		root.ArrayNHP[curPos] = info.nexthop;
		root.ArrayLen[curPos] = info.len;
	}
	return;
}

void CreateTrie()
{
	unsigned int i = 0;
	root = CreateNode();
	//BeginTime = rdtsc();
	for(i = 0; i < num_Entry; i++)
	{
		if (TableEntry[i].len != 0) AddNode(TableEntry[i]);
		else 
		{
			if (NumNode == 0) root = CreateNode();
			root.ArrayNHP[0] = 0;
			root.ArrayLen[0] = 0;
		}
	}
	//EndTime = rdtsc();
	return;
}

void SetTrieToDevice()
{
	cudaError_t error;
	error = cudaMalloc((void** ) &D_ArrayNode, 16 * NumNode * sizeof(unsigned int));
	if (error != cudaSuccess)
    {
        printf("cudaMalloc D_ArrayNode returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	
	error = cudaMemcpy(D_ArrayNode, root.ArrayNode, 16 * NumNode * sizeof(unsigned int), cudaMemcpyHostToDevice);
	if (error != cudaSuccess)
    {
        printf("cudaMemcpy (D_ArrayNode, root.ArrayNode) returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	
	error = cudaMalloc((void** ) &D_ArrayNHP, NumNode * sizeof(unsigned int));
	if (error != cudaSuccess)
    {
        printf("cudaMalloc D_ArrayNHP returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	
	error = cudaMemcpy(D_ArrayNHP, root.ArrayNHP, NumNode * sizeof(unsigned int), cudaMemcpyHostToDevice);
	if (error != cudaSuccess)
    {
        printf("cudaMemcpy (D_ArrayNHP, root.ArrayNHP) returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	return;
}

void SetQueryTable(char* FileName)
{
	FILE* ipt = fopen(FileName, "r");
	char buf[256];
	char str[256];
	
	num_Query = 0;
	while(fgets(buf, 256, ipt) != NULL) num_Query++;
	H_TableQuery = (unsigned int* )malloc(num_Query * sizeof(unsigned int));
	H_TableQueryNHP = (unsigned int* )malloc(num_Query * sizeof(unsigned int));
	
	rewind(ipt);
	num_Query = 0;
	while(fgets(buf, 256, ipt) != NULL)
	{
		sprintf(str, "%s\0", strtok(buf, "./"));
		H_TableQuery[num_Query] = atoi(str);
		sprintf(str, "%s\0", strtok(NULL, "./"));
		H_TableQuery[num_Query] = (H_TableQuery[num_Query] << 8) + atoi(str);
		sprintf(str, "%s\0", strtok(NULL, "./"));
		H_TableQuery[num_Query] = (H_TableQuery[num_Query] << 8) + atoi(str);
		sprintf(str, "%s\0", strtok(NULL, "./"));
		H_TableQuery[num_Query] = (H_TableQuery[num_Query] << 8) + atoi(str);
		H_TableQueryNHP[num_Query] = 256;
		num_Query++;
	}
	
	fclose(ipt);
	return;
}

void Shuffle()
{
	//ENTRY tmp;
	unsigned int tmp;
	unsigned int i, j;
	
	srand(time(0));
	
	for(i = 0; i < num_Query - 1; i++)
	{
		j = i + (rand() % (num_Query - i));
		tmp = H_TableQuery[i];
		H_TableQuery[i] = H_TableQuery[j];
		H_TableQuery[j] = tmp;
	}
	
	return;
}

void SetQueryTableToDevice()
{
	cudaError_t error;
	error = cudaMalloc((void** ) &D_TableQuery, num_Query * sizeof(unsigned int));
	if (error != cudaSuccess)
    {
        printf("cudaMalloc D_TableQuery returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	
	error = cudaMemcpy((void** ) D_TableQuery, H_TableQuery, num_Query * sizeof(unsigned int), cudaMemcpyHostToDevice);
	if (error != cudaSuccess)
    {
        printf("cudaMemcpy (D_TableQuery, H_TableQuery) returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	
	error = cudaMalloc((void** ) &D_TableQueryNHP, num_Query * sizeof(unsigned int));
	if (error != cudaSuccess)
    {
        printf("cudaMalloc D_TableQueryNHP returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	
	error = cudaMemcpy((void** ) D_TableQueryNHP, H_TableQueryNHP, num_Query * sizeof(unsigned int), cudaMemcpyHostToDevice);
	if (error != cudaSuccess)
    {
        printf("cudaMemcpy (D_TableQueryNHP, H_TableQueryNHP) returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	return;
}

__global__ void Search(unsigned int* ArrayNode, unsigned int* ArrayNHP, unsigned int* TableQuery, unsigned int* ResultQuery, unsigned int N)
{
	const unsigned int tid_in_grid = blockDim.x * blockIdx.x + threadIdx.x;
	
	if (tid_in_grid < N)
	{
		ResultQuery[tid_in_grid] = ArrayNHP[0];
		for(unsigned int i = 0, CurPos = 0, tmp; i < 8; i++)
		{
			tmp = (TableQuery[tid_in_grid] >> (28 - i * 4)) % 16;
			if (ArrayNode[CurPos * 16 + tmp] == 0) break;
			CurPos = ArrayNode[CurPos * 16 + tmp];
			if (ArrayNHP[CurPos] != 256) ResultQuery[tid_in_grid] = ArrayNHP[CurPos];
		}
	}
	__syncthreads();
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("Please execute the file as the following way:\n");
		printf("%s  routing_table_file_name  query_table_file_name\n", argv[0]);
		exit(1);
	}
	
	SetEntryTable(argv[1]);
	CreateTrie();
	SetTrieToDevice();

	SetQueryTable(argv[2]);
	Shuffle();
	SetQueryTableToDevice();
	
	dim3 grid((num_Query + 512 - 1) / 512, 1, 1);
	dim3 thread(512, 1, 1);
	
	// Allocate CUDA events that we'll use for timing
    cudaEvent_t start;
    cudaError_t error = cudaEventCreate(&start);

    if (error != cudaSuccess)
    {
        fprintf(stderr, "Failed to create start event (error code %s)!\n", cudaGetErrorString(error));
        exit(EXIT_FAILURE);
    }

    cudaEvent_t stop;
    error = cudaEventCreate(&stop);

    if (error != cudaSuccess)
    {
        fprintf(stderr, "Failed to create stop event (error code %s)!\n", cudaGetErrorString(error));
        exit(EXIT_FAILURE);
    }

    // Record the start event
    error = cudaEventRecord(start, NULL);

    if (error != cudaSuccess)
    {
        fprintf(stderr, "Failed to record start event (error code %s)!\n", cudaGetErrorString(error));
        exit(EXIT_FAILURE);
    }
	
	Search<<< grid, thread>>>(D_ArrayNode, D_ArrayNHP, D_TableQuery, D_TableQueryNHP, num_Query);
	
	// Record the stop event
    error = cudaEventRecord(stop, NULL);

    if (error != cudaSuccess)
    {
        fprintf(stderr, "Failed to record stop event (error code %s)!\n", cudaGetErrorString(error));
        exit(EXIT_FAILURE);
    }

    // Wait for the stop event to complete
    error = cudaEventSynchronize(stop);

    if (error != cudaSuccess)
    {
        fprintf(stderr, "Failed to synchronize on the stop event (error code %s)!\n", cudaGetErrorString(error));
        exit(EXIT_FAILURE);
    }

    float msecTotal = 0.0f;
    error = cudaEventElapsedTime(&msecTotal, start, stop);

    if (error != cudaSuccess)
    {
        fprintf(stderr, "Failed to get time elapsed between events (error code %s)!\n", cudaGetErrorString(error));
        exit(EXIT_FAILURE);
    }
	
	// Compute and print the performance
    float msecPerMatrixMul = msecTotal / num_Query;
	printf("Time= %.10f msec\n", msecPerMatrixMul);
	
	error = cudaMemcpy(H_TableQueryNHP, D_TableQueryNHP, num_Query * sizeof(unsigned int), cudaMemcpyDeviceToHost);
	if (error != cudaSuccess)
    {
        printf("cudaMemcpy (H_TableQueryNHP, D_TableQueryNHP) returned error code %d, line(%d)\n", error, __LINE__);
        exit(EXIT_FAILURE);
    }
	
	free(H_TableQuery);
	free(H_TableQueryNHP);
	cudaFree(D_ArrayNode);
	cudaFree(D_ArrayNHP);
	cudaFree(D_TableQuery);
	cudaFree(D_TableQueryNHP);
	return 0;
}