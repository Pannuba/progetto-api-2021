#define _GNU_SOURCE
#define LINE_BUF 2401
#define GRAPHS_BUF 1441791
#define BLOCK_SIZE 720
#define MIN(a,b) (((a)<(b))?(a):(b))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Removing "long" and the useless parameter in TopK has saved 56.75 million instructions in test 1!

// Using the tokenize functions instead of strtok_r has saved 773.65 million instructions in test 1!

typedef enum {false, true} bool;

typedef struct
{
	char index[12];
	int totalSum;
} GraphDict;

typedef struct
{
	int *closeTo;	// Array with the distances from that node to every other node, ordered by index
	bool isVisited;	// If the node has already been visited or not
	int distanceFromZero;
} GraphNode;

typedef struct
{
	GraphNode *nodes;
} Graph;

int strToInt(char *str)
{
	int val = 0;
	
	for (; *str != '\0'; str++)
		val = val * 10 + (*str - '0');
	
	return val;
}

int strToIntLast(char *str)	// Creating a "strToInt" for the last number has saved 606 million instructions in test 1!
{
	int val = 0;
	
	for (; *str != '\n'; str++)
		val = val * 10 + (*str - '0');
	
	return val;
}

int tokenize_first(char* string, int* oldIndex)
{
	char newStr[8];
	int i;
	
	for (i = 0; string[i] != ','; i++)		// Doing this saves 65.4 million instructions in test 1!
		newStr[i] = string[i];
		
	newStr[i] = '\0';
	
	*oldIndex = strlen(newStr) + 1;
	
	return strToInt(newStr);
}

int tokenize_middle(char* string, int* oldIndex, int* newIndex)
{
	char newStr[8];
	int j;
	
	// Doing this instead of if (...) .. break; saves 158.2 million instructions in test 1!
	for (j = *newIndex + 1; string[j] != ','; j++);

	*oldIndex = *newIndex;
	*newIndex = j;
	
	for (j = 0; j < *newIndex - *oldIndex; j++)
		newStr[j] = string[*oldIndex + j];
	
	newStr[*newIndex - *oldIndex] = '\0';
	*newIndex += 1;
	
	return strToInt(newStr);
}

int tokenize_last(char* string, int strleng, int* newIndex)
{
	char newStr[8];
	
	for (int i = 0; i < strleng - 1 - *newIndex; i++)		// First number
		newStr[i]= string[*newIndex + i];
		
	newStr[strleng - 1 - *newIndex] = '\0';
	
	return strToInt(newStr);
}

int maxValue(GraphDict arr[], int n)
{
	int mx = arr[0].totalSum;
	
	for (int i = 1; i < n; i++)
		if (arr[i].totalSum > mx)
			mx = arr[i].totalSum;
			
	return mx;
}

void countSort(GraphDict arr[], int n, int exp)
{
	GraphDict* output;
	output = malloc (n * sizeof(GraphDict));
	
	int i, count[10] = {0};

	for (i = 0; i < n; i++)
		count[(arr[i].totalSum / exp) % 10]++;

	for (i = 1; i < 10; i++)
		count[i] += count[i - 1];
 
	for (i = n - 1; i >= 0; i--)
	{
		output[count[(arr[i].totalSum / exp) % 10] - 1] = arr[i];
		count[(arr[i].totalSum / exp) % 10]--;
	}
 
	for (i = 0; i < n; i++)
		arr[i] = output[i];
		
	free(output);
}

void radixSort(GraphDict arr[], int n)
{
	int m = maxValue(arr, n);
	
	for (int exp = 1; m / exp > 0; exp *= 10)
		countSort(arr, n, exp);
}

void insertionSort(GraphDict* graphs, int left, int right)
{
	for (int i = left + 1; i <= right; i++)
	{
		GraphDict temp = graphs[i];
		int j = i - 1;
		
		while (j >= left && graphs[j].totalSum > temp.totalSum)
		{
			graphs[j + 1] = graphs[j];
			j--;
		}
		
		graphs[j + 1] = temp;
	}
}

void merge(GraphDict* graphs, int left, int middle, int right)
{

	int len1 = middle - left + 1, len2 = right - middle;
	GraphDict leftSubArr[len1], rightSubArr[len2];
	
	for (int i = 0; i < len1; i++)
		leftSubArr[i] = graphs[left + i];
		
	for (int i = 0; i < len2; i++)
		rightSubArr[i] = graphs[middle + 1 + i];
 
	int i = 0, j = 0, k = left;

	while (i < len1 && j < len2)
	{
		if (leftSubArr[i].totalSum <= rightSubArr[j].totalSum)
		{
			graphs[k] = leftSubArr[i];
			i++;
		}
		
		else
		{
			graphs[k] = rightSubArr[j];
			j++;
		}
		
		k++;
	}

	while (i < len1)
	{
		graphs[k] = leftSubArr[i];
		k++;
		i++;
	}

	while (j < len2)
	{
		graphs[k] = rightSubArr[j];
		k++;
		j++;
	}
}

void timSort(GraphDict* graphs, int n)
{	
	for (int i = 0; i < n; i += BLOCK_SIZE)
		insertionSort(graphs, i, MIN((i + BLOCK_SIZE - 1), (n - 1)));

	for (int size = BLOCK_SIZE; size < n; size *= 2)
	{
		for (int left = 0; left < n; left += size * 2)
		{
			int mid = left + size - 1;
			int right = MIN((left + 2 * size - 1), (n - 1));
			
			if(mid < right)
				merge(graphs, left, mid, right);
		}
	}
}

int calculateLength(Graph g, int nodes)	// Calculates the sum of the shortest paths from node 0 to every other reachable node
{
	int temp = 0;
	int totalSum = 0, i = 0, visitedNodes = 0;	// Starting from node 0, I check the other nodes

	while (visitedNodes != nodes)		// Instead of while(allNodesVisited) (a removed boolean function), this saves 85.8 million instructions in test 1!
	{
		for (int j = 0; j < nodes; j++)
		{
			if (i == j)		// It's pointless to check the distance between node x and node x
				continue;
			
			temp = g.nodes[i].distanceFromZero + g.nodes[i].closeTo[j];
			
			if ((g.nodes[j].distanceFromZero == -1 || g.nodes[j].distanceFromZero >= temp) && g.nodes[i].closeTo[j] != 0)
				g.nodes[j].distanceFromZero = temp;

		}

		g.nodes[i].isVisited = true;

		int val = __INT_MAX__;
		visitedNodes++;
		
		for (int k = 1; k < nodes; k++)	// i = 1 because node 0 is the first node that's visited
		{
			// Inverting these two ifs saves 23 million instructions in test 1!
			
			if (g.nodes[k].distanceFromZero < val && g.nodes[k].distanceFromZero != -1 && g.nodes[k].isVisited == false)
			{
				val = g.nodes[k].distanceFromZero;
				i = k;
			}
			
			if (visitedNodes == nodes - 1 && g.nodes[k].distanceFromZero == -1)
				goto END;
		}
	}
	
	END:

	for (int i = 1; i < nodes; i++)
		totalSum += g.nodes[i].distanceFromZero;
		
	// Setting totalSum = 0 if it's < 0 saves 11.2 million instructions in test 1!

	return (totalSum > 0 ? totalSum : 0);
}

void topK(int k, GraphDict* graphs, int numGraphs)
{	
	if (numGraphs < 500000)		// If there are many graphs radixSort is better, otherwise timSort is faster or equal
		timSort(graphs, numGraphs);
	
	else
		radixSort(graphs, numGraphs);

	if (k > numGraphs)
		k = numGraphs;
	
	// Only saving the string number and printing everything separately (putc ' ') is faster in test 1, but slower in test 6
	// Putting the space before the index's number and cutting the first character away (instead of the other way around) saves 2.1 million instructions in test 6!
	
	fputs_unlocked(graphs[0].index + 1, stdout);
	
	for (int i = 1; i < k; i++)		// MUCH faster than strcat() and fputs
		fputs_unlocked(graphs[i].index, stdout);
		
	fputs_unlocked("\n", stdout);
	
	// Putting fputs out of the for loop for the last index uses less instructions...
}

// Doing everything in main() without using obtainGraphInfo() saves 204 million instructions in test 1!

int main()
{
	char string[LINE_BUF], c;
	int i, row, nodes, k, numGraphs = 0;
	int oldIndex, newIndex, strLength;
	
	GraphDict *graphs = malloc(GRAPHS_BUF * sizeof(GraphDict));		// Preallocating instead of resizing the array for every graph saves 145 million instructions in test 1!
	Graph *g = malloc(sizeof(Graph));	// Putting it here and out of the while loop saves 186 million instructions in test 1!
	
	if (fscanf(stdin, "%d%d\n", &nodes, &k)) { }		// I DON'T HAVE TO FOLLOW THE ROW'S FORMATTING
	
	c = fgetc_unlocked(stdin);
	while (fgetc_unlocked(stdin) != '\n');		// Same for the first row, checking the first character only saves 7.2 million instructions in test 1!
	
	if (c == 'A')		// Putting this first row check 2.67 million instructions in test 6
		goto ADD_GRAPH;
		
	else
		fputs_unlocked("\n", stdout);	// fputs uses less instructions than fputc
	
	// Instead of using strcmp, I just need to check the first character. fgetc + read until \n instead of fgets saves 44 million instructions in test 1!	
	while ((c = fgetc_unlocked(stdin)) != EOF)
	{
		while (fgetc_unlocked(stdin) != '\n');	// Keeps on reading the row until the end (\n)
		
		if (c == 'A')
		{
			ADD_GRAPH: ;
			
			g->nodes = calloc(nodes, sizeof(GraphNode));		// Maybe it segfaulted when Graph was using a list-of-lists because I wasn't allocating the memory inside each list?
			
			GraphDict gd;	// Finding the number of characters in numGraphs and doing malloc on the index is cooler but also longer
			sprintf(gd.index, " %d", numGraphs);
			
			if (fgets_unlocked(string, LINE_BUF, stdin)) { }
			
			g->nodes[0].distanceFromZero = 0;	// Putting this or not changes nothing
			g->nodes[0].closeTo = malloc(nodes * sizeof(int));	// Node 0 is processed separately to avoid the (if i != 0) check, saves 400k instructions in test 1!
			
			oldIndex = 0; newIndex = 0;
			strLength = (int) strlen(string);
				
			g->nodes[0].closeTo[0] = tokenize_first(string, &oldIndex);
			
			newIndex = oldIndex;
			
			for (i = 1; i < nodes - 1; i++)
				g->nodes[0].closeTo[i] = tokenize_middle(string, &oldIndex, &newIndex);
			
			g->nodes[0].closeTo[nodes - 1] = tokenize_last(string, strLength, &newIndex);
			
			for (row = 1; row < nodes; row++)		// Passes the other rows after the first one, separately checked before
			{
				if (fgets_unlocked(string, LINE_BUF, stdin)) { }
				
				g->nodes[row].distanceFromZero = -1;
				g->nodes[row].closeTo = malloc(nodes * sizeof(int));
				
				oldIndex = 0; newIndex = 0;
				strLength = (int) strlen(string);
				
				g->nodes[row].closeTo[0] = tokenize_first(string, &oldIndex);
				newIndex = oldIndex;
				
				for (i = 1; i < nodes - 1; i++)
					g->nodes[row].closeTo[i] = tokenize_middle(string, &oldIndex, &newIndex);
				
				g->nodes[row].closeTo[nodes - 1] = tokenize_last(string, strLength, &newIndex);
			}
			
			gd.totalSum = calculateLength(*g, nodes);
	
			for (i = 0; i < nodes; i++)		// Less instructions when I'm using free()??
				free(g->nodes[i].closeTo);
			
			free(g->nodes);
			
			graphs[numGraphs] = gd;
			numGraphs++;
		}
		// Saved 5.76 million instructions in test 1 after inverting the ifs and adding else!
		else
			topK(k, graphs, numGraphs);
	}
	
	free(g);
	free(graphs);
	
	return 0;
}
