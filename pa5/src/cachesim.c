#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


struct node {
	int num;
	long int tag;
	struct node * next;
	struct node * prev;
};

// creates nodes
struct node * createNode (int x, long int t, struct node *n, struct node *p) {

	struct node * ptr = (struct node *)malloc(sizeof(struct node));
	if (!ptr) return NULL;

	ptr->num = x;
	ptr->tag = t;
	ptr->next = n;
	ptr->prev = p;

	return ptr;

}

//free nodes
void freeNode(struct node * ptr) {


	struct node * prev = ptr;

	/*go to end of list*/
	while(ptr!=NULL) {
		prev = ptr;
		ptr=ptr->next;
	}

	/*free the list backwards, so not too much in stack*/
	ptr = prev;
	prev = ptr->prev;

	while (ptr!=NULL) {
		free(ptr);
		ptr = prev;
		if (ptr!=NULL) {
			prev = ptr->prev;
		}
	}

}

//frees cache
void freedom(struct node ** cache, int size) {

	if (cache==NULL) return;

	int i=0;

	for (i=0; i<size; i++) {
		freeNode(cache[i]);
	}

	free(cache);


}


/*check if in cache.
	If yes return 0;
	If no, node into cache, return 1;
*/
int check (struct node ** cache, long int address, int policy, int bSize, int iSize, int lines, int pre) {

	if (cache == NULL) return 1;


	int res=1;
	long int tag, index, blockOffset;
	

	blockOffset = address%bSize;
	

	long int maybe = (address - blockOffset)%(iSize*bSize);
	index = maybe/bSize;

	tag = address-blockOffset-(index*iSize*bSize);

	int number = 0;
	int a = 0;

	/*index contains the set number, tag contains the size*/
	struct node * start = cache[index];
	struct node * ptr = cache[index];
	struct node * prev = NULL;
	struct node * meh;


	while (ptr!=NULL) {
		if (a!=1) {
			if (ptr->tag == tag) {
				if (policy==1) {
					return 0;
				}else if (pre!=1) {
					prev->next = ptr->next;
					if (ptr->next!=NULL) {
						struct node * t = ptr->next;
						t->prev = prev;
					}
					meh = ptr;
					a=1;
					meh->next = NULL;
				/*save it so it can be moved to the end of the list since it is the most recently used*/
				} else {
					return 0;
				}
			}
		}
		if (ptr->num>number) {
			number=ptr->num;
		}
		prev = ptr;
		ptr = ptr->next;
	}
	
	
	if (a==1 && pre!=1) {
		prev->next = meh;
		meh->prev = prev;
		meh->next = NULL;
		meh->num = number;
		return 0;
	}

	/*if it gets here, the address was not in the cache*/

	number++;

	struct node * temp = createNode(number, tag, NULL, prev);
	prev->next = temp;

	/*if the set is full, get rid of first item in the list*/
	if (number>lines) {
		meh = start->next;
		ptr = meh->next;
		start->next = ptr;
		ptr->prev = start;
		free(meh);
	}else{
        //free(meh);
    }


	return res;

}

//converts string of decimal number into an int of cache and block size
int toString (char * string) {

	int i =0, j=0;
	int result =0;
	for (i=0; string[i]!='\0'; i++) {
	}

	for (j=0; j<i; j++) {
		result = result * 10;
		result +=  (int)(string[j]) - 48;
	}

	return result;

}

//takes a number and then returns its base 2. Returns -1 if not a power of 2 
int baseTwo (int num) {

	int i=0;

	while (num>1) {
		if (num%2!=0) return -1;
		num = num/2;
		i++;
	}


	return i;
}






int main (int argc, char ** argv) {

	int bSize=0, bPower = 0, cSize=0, cPower =0, i=0, j=0, size=0;
	int type, policy, numSets, lines; 

	if (argc!=6) {
		printf("Error: invalid number or arguments.");
		return 1;
	} 

	




	while (argv[5][size]!='\0'){
		size++;
	}
	size+=1;

	
	char * filename = (char *)malloc(sizeof(char *) * size);
	strcpy(filename, argv[5]);

	FILE * fp;
	fp = fopen(filename, "r");
	if (!fp) return 1;


	//if power of 2
	cSize = toString(argv[1]);
	cPower = baseTwo(cSize);
	if (cPower == -1) return 1;

	//if power of 2
	bSize = toString(argv[4]);
	bPower = baseTwo(bSize);
	if (bPower == -1) return 1;

	//policy
	if (strcmp("fifo", argv[3])==0) {
		policy =1;
	} else {
		policy=2;
	}

	//type
	for (i=0; argv[2][i]!='\0'; i++) {}
	if (i==6) {
		type=0;
		numSets = cSize/bSize;
		lines = 1;
	}else if (i==5) {
		type=1;
		numSets = 1;
		lines = cSize/bSize;
	} else {
		int a = i-6;
		char * temp = (char *)malloc(sizeof(char) * a+1);
		for (j=6; argv[2][j]!='\0'; j++) {
			temp[j-6] = argv[2][j];
		}
		temp[j-6] = '\0';
		type = toString(temp);
		free(temp);
		if (baseTwo(type)==-1) return -1;

		numSets = cSize/(bSize * type);
		lines = type;
	}


	


	struct node ** regular = (struct node **) malloc (sizeof(struct node *) * numSets);
	struct node ** prefetch = (struct node **) malloc (sizeof(struct node *) * numSets);
	if (!regular || !prefetch) return 1;
	for (i=0; i<numSets; i++) {

		struct node * a = createNode(0, -1, NULL, NULL);
		regular[i] = a;

		struct node * b = createNode(0, -1, NULL, NULL);
		prefetch[i] = b;

	}

	long int address=0;
	int regHits=0, regMisses =0; 
	int regReads =0, regWrites = 0;
	int preHits =0, preMisses =0;
	int preReads =0, preWrites =0;

	char instruction=' ';
	char * meh = (char *) malloc (sizeof(char)* 100);


	

	//calls cache
	while (fscanf(fp, "%s ", meh)==1) {
		if (strcmp("#eof", meh)==0) break;
		if (fscanf(fp, "%c %lx", &instruction, &address)!=2) return 1;
		

		//runs address through regular
		if (check(regular, address, policy, bSize, numSets, lines, 0)==0) {
			regHits ++;
			if (instruction == 'W') {
				regWrites ++;
			}
		}else {
			regMisses ++;
			if (instruction == 'R') {
				regReads ++;
			}else {
				regReads ++;
				regWrites ++;
			}
		}

		//runs address through prefetch
		if (check(prefetch, address, policy, bSize, numSets, lines, 0)==0) {
			preHits++;
			if (instruction == 'W') {
				preWrites ++;
			}
		}else {
			preMisses++;
			if (instruction == 'R') {
				preReads ++;
			}else {
				preReads++;
				preWrites ++;
			}
			if (check(prefetch, address+bSize, policy, bSize, numSets, lines, 1)==0) {
			}else {
				preReads ++;
			}
		}

	}

	printf("Prefetch 0\n");
	
	printf("Memory reads: %d\n", regReads);
	printf("Memory writes: %d\n", regWrites);

    printf("Cache hits: %d\n", regHits);
	printf("Cache misses: %d\n", regMisses);

	printf("Prefetch 1\n");

    printf("Memory reads: %d\n", preReads);
	printf("Memory writes: %d\n", preWrites);
    
	printf("Cache hits: %d\n", preHits);
	printf("Cache misses: %d\n", preMisses);
	


	freedom(regular, numSets);
	freedom(prefetch, numSets);
    free(meh);
    
	fclose(fp);
    free(filename);

	return 0;
}