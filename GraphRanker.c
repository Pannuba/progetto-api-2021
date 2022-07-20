#define _GNU_SOURCE
#define LINE_BUF 2401
#define GRAPHS_BUF 1441791
#define BLOCK_SIZE 720
#define MIN(a,b) (((a)<(b))?(a):(b))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Togliere i "long" e il passaggio di parametro inutile in TopK ha risparmiato 56.75 milioni di istruzioni nel test 1!

// Usare le funzioni tokenize al posto di strtok_r ha risparmiato 773.65 milioni di istruzioni nel test 1!

typedef enum {false, true} bool;

typedef struct
{
	char index[12];		// Dovrei trovare un modo per rendere l'array della dimensione del numero (+ spazio) e basta
	int totalSum;
} GraphDict;

typedef struct
{
	int *closeTo;	// Array con le distanze da quel nodo a tutti gli altri, in ordine di indice
	bool isVisited;	// Se è il nodo è già stato visitato o no
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

int strToIntLast(char *str)	// Creare un "strToInt" per il caso dell'ultimo numero ha risparmiato 606 milioni di istruzioni nel test 1!!!!
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
	
	for (i = 0; string[i] != ','; i++)		// Fare così toglie 65.4 milioni di istruzioni nel test 1!
		newStr[i] = string[i];
		
	newStr[i] = '\0';
	
	*oldIndex = strlen(newStr) + 1;
	
	return strToInt(newStr);
}

int tokenize_middle(char* string, int* oldIndex, int* newIndex)
{
	char newStr[8];
	int j;
	
	// Fare così invece di if (..) .. break; risparmia 158.2 milioni di istruzioni nel test 1!
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
	
	for (int i = 0; i < strleng - 1 - *newIndex; i++)		// Faccio il primo numero
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

void insertionSort(GraphDict* grafi, int left, int right)
{
	for (int i = left + 1; i <= right; i++)
	{
		GraphDict temp = grafi[i];
		int j = i - 1;
		
		while (j >= left && grafi[j].totalSum > temp.totalSum)
		{
			grafi[j + 1] = grafi[j];
			j--;
		}
		
		grafi[j + 1] = temp;
	}
}

void merge(GraphDict* grafi, int left, int middle, int right)
{

	int len1 = middle - left + 1, len2 = right - middle;
	GraphDict leftSubArr[len1], rightSubArr[len2];
	
	for (int i = 0; i < len1; i++)
		leftSubArr[i] = grafi[left + i];
		
	for (int i = 0; i < len2; i++)
		rightSubArr[i] = grafi[middle + 1 + i];
 
	int i = 0, j = 0, k = left;

	while (i < len1 && j < len2)
	{
		if (leftSubArr[i].totalSum <= rightSubArr[j].totalSum)
		{
			grafi[k] = leftSubArr[i];
			i++;
		}
		
		else
		{
			grafi[k] = rightSubArr[j];
			j++;
		}
		
		k++;
	}

	while (i < len1)
	{
		grafi[k] = leftSubArr[i];
		k++;
		i++;
	}

	while (j < len2)
	{
		grafi[k] = rightSubArr[j];
		k++;
		j++;
	}
}

void timSort(GraphDict* grafi, int n)
{	
	for (int i = 0; i < n; i += BLOCK_SIZE)
		insertionSort(grafi, i, MIN((i + BLOCK_SIZE - 1), (n - 1)));

	for (int size = BLOCK_SIZE; size < n; size *= 2)
	{
		for (int left = 0; left < n; left += size * 2)
		{
			int mid = left + size - 1;
			int right = MIN((left + 2 * size - 1), (n - 1));
			
			if(mid < right)
				merge(grafi, left, mid, right);
		}
	}
}

int calcolaLunghezza(Graph g, int nodi)	// Calcola la somma dei percorsi più brevi dal nodo 0 a tutti gli altri nodi raggiungibili
{
	int temp = 0;
	int totalSum = 0, i = 0, visitedNodes = 0;	// Partendo dal nodo 0, controllo gli altri nodi

	while (visitedNodes != nodi)		// Invece di allNodesVisited, questo risparma 85.8 milioni di istruzioni nel test 1!
	{
		for (int j = 0; j < nodi; j++)
		{
			if (i == j)		// Inutile controllare la distanza tra il nodo x e il nodo x
				continue;
			
			temp = g.nodes[i].distanceFromZero + g.nodes[i].closeTo[j];
			
			if ((g.nodes[j].distanceFromZero == -1 || g.nodes[j].distanceFromZero >= temp) && g.nodes[i].closeTo[j] != 0)
				g.nodes[j].distanceFromZero = temp;

		}

		g.nodes[i].isVisited = true;

		int val = __INT_MAX__;
		visitedNodes++;
		
		for (int k = 1; k < nodi; k++)	// i = 1 perché il nodo 0 è il primo che viene visitato
		{
			// Invertire gli if risparmia 23 milioni di istruzioni nel test 1!
			
			if (g.nodes[k].distanceFromZero < val && g.nodes[k].distanceFromZero != -1 && g.nodes[k].isVisited == false)
			{
				val = g.nodes[k].distanceFromZero;
				i = k;
			}
			
			if (visitedNodes == nodi - 1 && g.nodes[k].distanceFromZero == -1)
				goto END;
		}
	}
	
	END:

	for (int i = 1; i < nodi; i++)
		totalSum += g.nodes[i].distanceFromZero;
		
	// Settare totalSum a 0 se è < 0 risparmia 11.2 milioni di istruzioni nel test 1!

	return (totalSum > 0 ? totalSum : 0);
}

void topK(int k, GraphDict* grafi, int numGrafi)		// Rinominare grafi in listaGrafi
{	
	if (numGrafi < 500000)		// Se ci sono molti grafi il radixSort è meglio, se no timSort è più veloce o uguale
		timSort(grafi, numGrafi);
	
	else
		radixSort(grafi, numGrafi);

	if (k > numGrafi)
		k = numGrafi;
	
	// Salvare solo il numero nella stringa e stampare tutto separatamente (putc ' ') è più veloce nel test 1, ma più lento nel test 6
	// Mettere lo spazio prima del numero nell'indice e tagliare il primo carattere (invece del contrario) risparmia 2.1 milioni di istruzioni nel test 6!
	
	fputs_unlocked(grafi[0].index + 1, stdout);
	
	for (int i = 1; i < k; i++)		// MOLTO più veloce di strcat() e fputs
		fputs_unlocked(grafi[i].index, stdout);
		
	fputs_unlocked("\n", stdout);
	
	// Mettere fputs fuori dal for per l'ultimo indice usa meno istruzioni...
}

// Fare tutto nel main senza usare ottieniInfoGrafo() risparmia 204 milioni di istruzioni nel test 1!

int main()
{
	char string[LINE_BUF], c;
	int i, row, nodi, k, numGrafi = 0;
	int oldIndex, newIndex, strLength;
	
	GraphDict *grafi = malloc(GRAPHS_BUF * sizeof(GraphDict));		// Preallocare invece di ridimensionare l'array a ogni grafo risparmia 145 milioni di istruzioni nel test 1!!
	Graph *g = malloc(sizeof(Graph));	// Metterlo qui e fuori dal while risparmia 186 milioni di istruzioni nel test 1!
	
	if (fscanf(stdin, "%d%d\n", &nodi, &k)) { }		// NON SERVE SEGUIRE LA FORMATTAZIONE DELLA RIGA
	
	c = fgetc_unlocked(stdin);
	while (fgetc_unlocked(stdin) != '\n');		// Anche per la prima riga, controllare il primo carattere risparmia 7.2 milioni di istruzioni nel test 1!
	
	if (c == 'A')		// Mettere il controllo della prima riga a parte risparmia 2.67 milioni di istruzioni nel test 6
		goto AGGIUNGI_GRAFO;
		
	else
		fputs_unlocked("\n", stdout);	// fputs usa meno istruzioni di fputc
	
	// Anziché usare strcmp, basta controllare il primo carattere. fgetc + scorri fino a \n invece di fgets Risparmia 44.3 milioni di istruzioni nel test 1!	
	while ((c = fgetc_unlocked(stdin)) != EOF)		// Si può capire la lunghezza della linea dal file?
	{
		while (fgetc_unlocked(stdin) != '\n');	// Va avanti a leggere la riga fino alla fine (\n)
		
		if (c == 'A')
		{
			AGGIUNGI_GRAFO: ;
			
			g->nodes = calloc(nodi, sizeof(GraphNode));		// Forse quando Graph aveva la lista di liste faceva segfault perché non allocavo la memoria all'interno di ciascuna lista?
			
			GraphDict gd;	// Trovare la quantità di caratteri in numGrafi e fare malloc su index fa figo ma ci mette di più
			sprintf(gd.index, " %d", numGrafi);
			
			if (fgets_unlocked(string, LINE_BUF, stdin)) { }
			//printf("got string %s\n", string);
			
			g->nodes[0].distanceFromZero = 0;	// Avercelo o no non cambia niente
			g->nodes[0].closeTo = malloc(nodi * sizeof(int));	// Nodo 0 processato a parte per evitare il controllo (if i != 0), risparmia 400k istruzioni nel test 1!
			
			oldIndex = 0; newIndex = 0;
			strLength = (int) strlen(string);
				
			g->nodes[0].closeTo[0] = tokenize_first(string, &oldIndex);
			
			newIndex = oldIndex;
			
			for (i = 1; i < nodi - 1; i++)
				g->nodes[0].closeTo[i] = tokenize_middle(string, &oldIndex, &newIndex);
			
			g->nodes[0].closeTo[nodi - 1] = tokenize_last(string, strLength, &newIndex);
			
			for (row = 1; row < nodi; row++)		// Passa le altre linee dopo la prima che è a parte
			{
				if (fgets_unlocked(string, LINE_BUF, stdin)) { }
				
				g->nodes[row].distanceFromZero = -1;
				g->nodes[row].closeTo = malloc(nodi * sizeof(int));
				
				oldIndex = 0; newIndex = 0;
				strLength = (int) strlen(string);
				
				g->nodes[row].closeTo[0] = tokenize_first(string, &oldIndex);
				newIndex = oldIndex;
				
				for (i = 1; i < nodi - 1; i++)
					g->nodes[row].closeTo[i] = tokenize_middle(string, &oldIndex, &newIndex);
				
				g->nodes[row].closeTo[nodi - 1] = tokenize_last(string, strLength, &newIndex);
			}
			
			gd.totalSum = calcolaLunghezza(*g, nodi);
	
			for (i = 0; i < nodi; i++)		// Meno istruzioni quando faccio free()??
				free(g->nodes[i].closeTo);
			
			free(g->nodes);
			
			grafi[numGrafi] = gd;
			numGrafi++;
		}
		// risparmiate 5.76 milioni di istruzioni nel test 1 per aver invertito gli if e messo else!
		else
			topK(k, grafi, numGrafi);
	}
	
	free(g);
	free(grafi);
	
	return 0;
}
