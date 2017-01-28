//Game of Life homework by Nikita Georgiou

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ALIVE 'a'
#define DOOMED 'd' 
#define ISALIVE(neighbor) ((neighbor == '*') || (neighbor == DOOMED))

static inline int validateArg(char* arg);
static inline FILE* validateFile(char* filename);
static inline char** createGrid(int rows, int cols);
static inline void* safeMalloc(size_t size);
static inline void updateDisplay(char** grid, int rows, int cols, int generation);
static inline void deleteGrid(char** grid, int rows);

void populateGrid(char** grid, FILE* inputFile, int rows, int cols);
void animate(char** grid, int generations, int rows, int cols);
int checkAdjacencies(char** grid, int row, int col);
void cull(char** grid, int rows, int cols);

int main(int argc, char** argv){
	int rows = 10, cols = 10, generations = 10;
	FILE * inputFile;
	char* filename = "life.txt";
	char** grid;

	//switch to digest command line args
	switch(argc){
		case 5: generations = validateArg(argv[4]);
		case 4: filename = argv[3];
		case 3: cols = validateArg(argv[2]);
		case 2: rows = validateArg(argv[1]);
		case 1: break;
		default: 
			dprintf(2, "Usage: life rows columns filename generations\n");
			exit(EXIT_FAILURE);
	}			

	//padding the grid to avoid checking out of bounds
	rows += 2;
	cols += 2;
	
	//open file, check if we opened correctly
	inputFile = validateFile(filename);

	//malloc grid for game of life
	grid = createGrid(rows, cols);
	
	//populate grid with data from file
	//no more use for the file, so close it
	//not really concerned with fclose failing
	populateGrid(grid, inputFile, rows, cols);
	fclose(inputFile);

	//animate!
	animate(grid, generations, rows, cols);

	//free heap
	deleteGrid(grid, rows);
}

//makes sure numeric command line arguments
//are indeed numbers or not zero. 
//returns parsed number or exits process
static inline int validateArg(char* arg){
	int number;
	if((number = strtol(arg, NULL, 0)) == 0){
		(arg[0] == '0') ? 
			fprintf(stderr, "Error: Row or column cannot be 0\n")
		       	: perror("Error");
		exit(EXIT_FAILURE);
	}
	return number;
}

//validates that the generating file opens successfully
//returns file stream pointer or exits process
static inline FILE* validateFile(char* filename){
	FILE* inFile;
	if((inFile = fopen(filename, "r")) == NULL){
		perror("Error");
		exit(EXIT_FAILURE);
	}
	return inFile;
}

//creates 2D array on the heap, whose length and height
//is specified with rows and cols by user (default = 10).
//Also populates grid cells with 'dead' spaces (space character).
//Each row is null terminated for later printing
//Cannot fail on its own, thoguh safeMalloc might
//returns pointer to 'start' of grid.
static inline char** createGrid(int rows, int cols){
	char** grid = (char**)(safeMalloc(rows * sizeof(char*)));
	for(int i = 0; i < rows; i++){
		grid[i] = (char*)(safeMalloc(cols));
		for(int j = 0; j < cols - 1; j++)
			grid[i][j] = '-';
		grid[i][cols - 1] = '\0';
	}
	return grid;
}

//Allocates memory or dies trying
//returns poiunter to memory on success
static inline void* safeMalloc(size_t size){
	void* mem;
	if((mem = malloc(size)) == NULL){
		perror("Error: Couldn't create grid. ");
		exit(EXIT_FAILURE);
	}
	return mem;
}

//deleting 2d matricies sucks tbh
//im not about to provide error checking Ritchie didn't
static inline void deleteGrid(char** grid, int rows){
	for(int i = 0; i < rows; i++) free(grid[i]);
	free(grid);
}

//Reads generating file character by character replacing dead spaces
//in grid with asterisks in corresponding location to the file.
//Keeps reading unless either the maximum size of the grid 
//or the end of file has been reached.
//Stops reading a row early if a newline is encountered. 
void populateGrid(char** grid, FILE* inputFile, int rows, int cols){
	char charFromFile;
	for(int i = 1; i < rows; i++){
		for(int j = 1; j < cols; j++){
			if((charFromFile = getc(inputFile)) == EOF) return;
			else if(charFromFile == '\n') break;
			if(charFromFile == '*'){
				grid[i][j] = '*';
			}
		}
	}
}

//Scans each 'valid' cell (ignoring perimiter cells)
//and checks its adjacencies. Acts according to GoL's rules. 
//Cells that will die the following turn are marked DOOMED
//and dead cells that are to come alive are marked ALIVE
//ISALIVE checks if the cell is alive, or marked to die the following
//turn (but still alive as of this turn). 
//prints updates to the console with updateDisplay()
void animate(char** grid, int generations, int rows, int cols){
	int neighbors;

	for(int genCounter = 0; genCounter < generations; genCounter++){
		updateDisplay(grid, rows, cols, genCounter);
		for(int i = 1; i < rows - 1; i++){
			for(int j = 1; j < cols - 1; j++){
				neighbors = checkAdjacencies(grid, i, j);
				if((neighbors < 2 || neighbors > 3) 
				&& ISALIVE(grid[i][j])) 
					grid[i][j] = DOOMED;
				else if(neighbors == 3 && grid[i][j] == '-')
					grid[i][j] = ALIVE;
		}
	}

	cull(grid, rows, cols);
	usleep(500000); //Not entirely portable as arg is <1000000
	}}


//Prints each row of the grid to the console
//along with a bar to seperate the different generations
static inline void updateDisplay(char** grid, int rows, int cols, int generation){
	printf("Generation %d:\n", generation + 1);
	for(int i = 1; i < rows - 1; i++)
		printf("%s\n", grid[i] + 1);
	for(int i = 1; i < cols - 1; i++) printf("=");
	printf("\n");
}

//Scans the grid for cells marked as DOOMED or ALIVE
//and changes that be a - or *, respectively
//so that we can update the game state without
//modifying the grid while also checking for neighbors
void cull(char** grid, int rows, int cols){
	for(int i = 1; i < rows - 1; i++)
		for(int j = 1; j < cols - 1; j++)
			if(grid[i][j] == DOOMED)
				grid[i][j] = '-';
			else if(grid[i][j] == ALIVE)
				grid[i][j] = '*';
}

//hard coded checks for all eight positions around a 
//particular cell specified by row and col.
//Unless this game changes dimension, this should be
//fine for now. 
//returns the number of adjacent live cells found. 
int checkAdjacencies(char** grid, int row, int col){
	int neighbors = 0;
	if(ISALIVE(grid[row - 1][col - 1])) neighbors++;
	if(ISALIVE(grid[row - 1][col])) neighbors++;
	if(ISALIVE(grid[row - 1][col + 1])) neighbors++;
	if(ISALIVE(grid[row][col - 1])) neighbors++;
	if(ISALIVE(grid[row][col + 1])) neighbors++;
	if(ISALIVE(grid[row + 1][col - 1])) neighbors++;
	if(ISALIVE(grid[row + 1][col])) neighbors++;
	if(ISALIVE(grid[row + 1][col + 1])) neighbors++;
	return neighbors;
}
