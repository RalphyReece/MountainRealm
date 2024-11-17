#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#define PERM_SIZE 256
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 12
#define NUM_COLUMNS 16
#define PERM_SIZE 256
#define X_SIZE 100
#define Y_SIZE 100
#define ZW_SIZE 10
#define Z_SIZE 20
#define NAME_LENGTH 50
#define MAX_PROFESSIONS 10
#define MAX_INVENTORY 10
#define MAX_PATH 300
#define FPS_UPDATE_INTERVAL 200.0 // Update FPS every second

typedef struct {
    int frameCount;          // Number of frames rendered
    double elapsedTime;     // Time since last FPS update
    clock_t lastTime;       // Last time FPS was updated
} FPSCounter;

// Initialize the FPSCounter
void initFPSCounter(FPSCounter *fpsCounter) {
    fpsCounter->frameCount = 0;
    fpsCounter->elapsedTime = 0.0;
    fpsCounter->lastTime = clock();
}

// Update the FPS counter and print FPS

typedef struct {
    char name[NAME_LENGTH];
    int age;
    int professions[MAX_PROFESSIONS];
    int inventory[MAX_INVENTORY];
    int free;
    int task;
    int material;
    int dx;
    int dy;
    int cx; // New field
    int cy; // New field
    int cz; // New field
    int path[MAX_PATH];
    int offset;
} Person;


typedef struct {
    Person *persons;
    size_t size;
    size_t capacity;
} PersonList;

// Initialize the PersonList
void initPersonList(PersonList *list, size_t initialCapacity) {
    list->persons = malloc(initialCapacity * sizeof(Person));
    list->size = 0;
    list->capacity = initialCapacity;
}

// Free the allocated memory for PersonList
void freePersonList(PersonList *list) {
    free(list->persons);
    list->persons = NULL;
    list->size = 0;
    list->capacity = 0;
}

// Append a new person to the list
void appendPerson(PersonList *list, const char *name, int age, int *professions, int *inventory, 
                  int free, int task, int material, int dx, int dy, int cx, int cy, int cz, int *path, int offset) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->persons = realloc(list->persons, list->capacity * sizeof(Person));
        if (list->persons == NULL) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
    }

    strncpy(list->persons[list->size].name, name, NAME_LENGTH);
    list->persons[list->size].age = age;
    memcpy(list->persons[list->size].professions, professions, MAX_PROFESSIONS * sizeof(int));
    memcpy(list->persons[list->size].inventory, inventory, MAX_INVENTORY * sizeof(int));
    list->persons[list->size].free = free;
    list->persons[list->size].task = task;
    list->persons[list->size].material = material;
    list->persons[list->size].dx = dx;
    list->persons[list->size].dy = dy;
    list->persons[list->size].cx = cx; // Set cx
    list->persons[list->size].cy = cy; // Set cy
    list->persons[list->size].cz = cz; // Set cz
    memcpy(list->persons[list->size].path, path, MAX_PATH * sizeof(int));
    list->persons[list->size].offset = offset; 

    list->size++;
}

// Display a single person's information
void displayPerson(const Person *p) {
    printf("Name: %s\n", p->name);
    printf("Age: %d\n", p->age);
    printf("Professions: ");
    for (int i = 0; i < MAX_PROFESSIONS; i++) {
        if (p->professions[i] != 0) // Assuming 0 means no profession
            printf("%d ", p->professions[i]);
    }
    printf("\nInventory: ");
    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (p->inventory[i] != 0) // Assuming 0 means no item
            printf("%d ", p->inventory[i]);
    }
    printf("\nFree: %d, Task: %d, Material: %d, dx: %d, dy: %d\n", 
           p->free, p->task, p->material, p->dx, p->dy);
}

// Display all persons in the list
void displayAllPersons(const PersonList *list) {
    for (size_t i = 0; i < list->size; i++) {
        displayPerson(&list->persons[i]);
        printf("\n");
    }
}


/*
PersonList list;
    initPersonList(&list, 2); // Initial capacity of 2

    // Sample data for professions and inventory
    int professions1[] = {1, 2, 0, 0, 0, 0, 0, 0, 0, 0}; // Assume 1 and 2 are profession IDs
    int inventory1[] = {10, 20, 0, 0, 0, 0, 0, 0, 0, 0}; // Assume 10 and 20 are item IDs

    // Append persons to the list
    appendPerson(&list, "Alice", 30, professions1, inventory1, 0, 1, 2, 3, 4, 12, 10, 4);
    
    int professions2[] = {3, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int inventory2[] = {30, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    appendPerson(&list, "Bob", 25, professions2, inventory2, 1, 0, 5, 6, 7, 10, 10, 4);

    // Display all persons
    displayAllPersons(&list);

    // Free memory
    freePersonList(&list);
*/
static int perm[PERM_SIZE * 2];

void init_permutation() {
    for (int i = 0; i < PERM_SIZE; i++) {
        perm[i] = i;
    }
    for (int i = 0; i < PERM_SIZE; i++) {
        int r = rand() % PERM_SIZE;
        int temp = perm[i];
        perm[i] = perm[r];
        perm[r] = temp;
    }
    for (int i = 0; i < PERM_SIZE; i++) {
        perm[PERM_SIZE + i] = perm[i];
    }
}

double fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double lerp(double a, double b, double t) {
    return a + t * (b - a);
}

int hash(int x) {
    return perm[x & (PERM_SIZE - 1)];
}

double grad(int hash, double x, double y) {
    int h = hash & 3; // Use only last 2 bits for 4 gradients
    double u = h < 2 ? x : y; // Choose x or y
    double v = h < 1 ? y : 0; // Choose y or 0
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

double perlin(double x, double y) {
    int xi = (int)floor(x) & 255;
    int yi = (int)floor(y) & 255;

    double xf = x - floor(x);
    double yf = y - floor(y);

    double u = fade(xf);
    double v = fade(yf);

    int aa = hash(hash(xi) + yi);
    int ab = hash(hash(xi) + yi + 1);
    int ba = hash(hash(xi + 1) + yi);
    int bb = hash(hash(xi + 1) + yi + 1);

    double x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
    double x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);

    return (lerp(x1, x2, v) + 1) / 2; // Scale to 0-1
}
void splitInteger(int number, char *result) {
    // Convert integer to string
    sprintf(result, "%d", number);
}
void Texter(SDL_Renderer *renderer, SDL_Texture *spriteSheet, 
                      int i, int j, char a, int scale, int cr, int cg, int cb) {
    int characterIndex = 0; // Default character index
    int ind = 97;
    if(a=='a') {
        ind = 97;
    } else if(a=='b') {
        ind = 98;
    } else if(a=='c') {
        ind = 99;
    } else if(a=='d') {
        ind = 100;
    } else if(a=='e') {
        ind = 101;
    } else if(a=='f') {
        ind = 102;
    } else if(a=='g') {
        ind = 103;
    } else if(a=='h') {
        ind = 104;
    } else if(a=='i') {
        ind = 105;
    } else if(a=='j') {
        ind = 106;
    } else if(a=='k') {
        ind = 107;
    } else if(a=='l') {
        ind = 108;
    } else if(a=='m') {
        ind = 109;
    } else if(a=='n') {
        ind = 110;
    } else if(a=='o') {
        ind = 111;
    } else if(a=='p') {
        ind = 112;
    } else if(a=='q') {
        ind = 113;
    } else if(a=='r') {
        ind = 114;
    } else if(a=='s') {
        ind = 115;
    } else if(a=='t') {
        ind = 116;
    } else if(a=='u') {
        ind = 117;
    } else if(a=='v') {
        ind = 118;
    } else if(a=='w') {
        ind = 119;
    } else if(a=='x') {
        ind = 120;
    } else if(a=='y') {
        ind = 121;
    } else if(a=='z') {
        ind = 122;
     
    } else if(a=='A') {
        ind = 65;
    } else if(a=='B') {
        ind = 66;
    } else if(a=='C') {
        ind = 67;
    } else if(a=='D') {
        ind = 68;
    } else if(a=='E') {
        ind = 69;
    } else if(a=='F') {
        ind = 70;
    } else if(a=='G') {
        ind = 71;
    } else if(a=='H') {
        ind = 72;
    } else if(a=='I') {
        ind = 73;
    } else if(a=='J') {
        ind = 74;
    } else if(a=='K') {
        ind = 75;
    } else if(a=='L') {
        ind = 76;
    } else if(a=='M') {
        ind = 77;
    } else if(a=='N') {
        ind = 78;
    } else if(a=='O') {
        ind = 79;
    } else if(a=='P') {
        ind = 80;
    } else if(a=='Q') {
        ind = 81;
    } else if(a=='R') {
        ind = 82;
    } else if(a=='S') {
        ind = 83;
    } else if(a=='T') {
        ind = 84;
    } else if(a=='U') {
        ind = 85;
    } else if(a=='V') {
        ind = 86;
    } else if(a=='W') {
        ind = 87;
    } else if(a=='X') {
        ind = 88;
    } else if(a=='Y') {
        ind = 89;
    } else if(a=='Z') {
        ind = 90;
    } else if(a=='0') {
        ind = 48;
    } else if(a=='1') {
        ind = 49;
    } else if(a=='2') {
        ind = 50;
    } else if(a=='3') {
        ind = 51;
    } else if(a=='4') {
        ind = 52;
    } else if(a=='5') {
        ind = 53;
    } else if(a=='6') {
        ind = 54;
    } else if(a=='7') {
        ind = 55;
    } else if(a=='8') {
        ind = 56;
    } else if(a=='9') {
        ind = 57;
    } else if(a==' ') {
        ind = 0;
    } else if(a=='-') {
        ind = 249;
    }
    characterIndex = ind;
    
    SDL_Rect srcRect = {
                (characterIndex % NUM_COLUMNS) * CHAR_WIDTH,
                (characterIndex / NUM_COLUMNS) * CHAR_HEIGHT,
                CHAR_WIDTH,
                CHAR_HEIGHT
            };

            // Calculate the destination rectangle for rendering
            SDL_Rect dstRect = { i, j, CHAR_WIDTH * scale, CHAR_HEIGHT * scale }; // Scale up for visibility

            // Set color modulation for the texture (optional)
            SDL_SetTextureColorMod(spriteSheet, cr, cg, cb); // Example: render white pixels as green

            // Render the character from the sprite sheet
            SDL_RenderCopy(renderer, spriteSheet, &srcRect, &dstRect);
}
void SheetPrint(SDL_Renderer *renderer, SDL_Texture *spriteSheet, 
                      int i, int j, int a, int scale, int cr, int cg, int cb) {
    int characterIndex = 0; // Default character index
    int ind = a;
    characterIndex = ind;
    
    SDL_Rect srcRect = {
                (characterIndex % NUM_COLUMNS) * CHAR_WIDTH,
                (characterIndex / NUM_COLUMNS) * CHAR_HEIGHT,
                CHAR_WIDTH,
                CHAR_HEIGHT
            };

            // Calculate the destination rectangle for rendering
            SDL_Rect dstRect = { i, j, CHAR_WIDTH * scale, CHAR_HEIGHT * scale }; // Scale up for visibility

            // Set color modulation for the texture (optional)
            SDL_SetTextureColorMod(spriteSheet, cr, cg, cb); // Example: render white pixels as green

            // Render the character from the sprite sheet
            SDL_RenderCopy(renderer, spriteSheet, &srcRect, &dstRect);
}
void EntityPrint(SDL_Renderer *renderer, SDL_Texture *spriteSheet, 
                      int i, int j, int a, int scale, int cr, int cg, int cb) {
    int characterIndex = 0; // Default character index
    int ind = 100;
    SheetPrint(renderer,spriteSheet, i,j,219,2,0,0,0);
    
    if (a==1) {
        ind = 1;
    }
    characterIndex = ind;
    SDL_Rect srcRect = {
                (characterIndex % NUM_COLUMNS) * CHAR_WIDTH,
                (characterIndex / NUM_COLUMNS) * CHAR_HEIGHT,
                CHAR_WIDTH,
                CHAR_HEIGHT
            };

            // Calculate the destination rectangle for rendering
            SDL_Rect dstRect = { i, j, CHAR_WIDTH * scale, CHAR_HEIGHT * scale }; // Scale up for visibility

            // Set color modulation for the texture (optional)
            SDL_SetTextureColorMod(spriteSheet, cr, cg, cb); // Example: render white pixels as green

            // Render the character from the sprite sheet
            SDL_RenderCopy(renderer, spriteSheet, &srcRect, &dstRect);
}
void updateFPS(FPSCounter *fpsCounter, int t) {
    fpsCounter->frameCount++;

    clock_t currentTime = clock();
    fpsCounter->elapsedTime += (double)(currentTime - fpsCounter->lastTime) / CLOCKS_PER_SEC;
    fpsCounter->lastTime = currentTime;

    if (fpsCounter->elapsedTime >= FPS_UPDATE_INTERVAL) {
        printf("FPS: %d, T: %d\n", fpsCounter->frameCount, t); // Output FPS and t
        fpsCounter->frameCount = 0; // Reset frame count
        fpsCounter->elapsedTime = 0.0; // Reset elapsed time
    }
}

int* getAllCxValues(const PersonList *list, size_t *count) {
    *count = list->size; // Set the count of cx values to the number of persons
    if (*count == 0) {
        return NULL; // Return NULL if there are no persons
    }

    int *cxValues = malloc(*count * sizeof(int)); // Allocate memory for the cx values
    if (cxValues == NULL) {
        perror("Failed to allocate memory for cx values");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < list->size; i++) {
        cxValues[i] = list->persons[i].cx; // Copy the cx values into the array
    }

    return cxValues; // Return the array of cx values
}
int* getAllCyValues(const PersonList *list, size_t *count) {
    *count = list->size; // Set the count of cx values to the number of persons
    if (*count == 0) {
        return NULL; // Return NULL if there are no persons
    }

    int *cyValues = malloc(*count * sizeof(int)); // Allocate memory for the cx values
    if (cyValues == NULL) {
        perror("Failed to allocate memory for cx values");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < list->size; i++) {
        cyValues[i] = list->persons[i].cy; // Copy the cx values into the array
    }

    return cyValues; // Return the array of cx values
}
int* getAllCzValues(const PersonList *list, size_t *count) {
    *count = list->size; // Set the count of cx values to the number of persons
    if (*count == 0) {
        return NULL; // Return NULL if there are no persons
    }

    int *czValues = malloc(*count * sizeof(int)); // Allocate memory for the cx values
    if (czValues == NULL) {
        perror("Failed to allocate memory for cx values");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < list->size; i++) {
        czValues[i] = list->persons[i].cz; // Copy the cx values into the array
    }

    return czValues; // Return the array of cx values
}
int* getAllFreeValues(const PersonList *list, size_t *count) {
    *count = list->size; // Set the count of cx values to the number of persons
    if (*count == 0) {
        return NULL; // Return NULL if there are no persons
    }

    int *freeValues = malloc(*count * sizeof(int)); // Allocate memory for the cx values
    if (freeValues == NULL) {
        perror("Failed to allocate memory for cx values");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < list->size; i++) {
        freeValues[i] = list->persons[i].free; // Copy the cx values into the array
    }

    return freeValues; // Return the array of cx values
}
int* getAllTaskValues(const PersonList *list, size_t *count) {
    *count = list->size; // Set the count of cx values to the number of persons
    if (*count == 0) {
        return NULL; // Return NULL if there are no persons
    }

    int *taskValues = malloc(*count * sizeof(int)); // Allocate memory for the cx values
    if (taskValues == NULL) {
        perror("Failed to allocate memory for cx values");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < list->size; i++) {
        taskValues[i] = list->persons[i].task; // Copy the cx values into the array
    }

    return taskValues; // Return the array of cx values
}
int getOneOffsetValue(const PersonList *list, size_t index) {
    return list->persons[index].offset; // Return the array of cx values
}
typedef struct {
    int x, y, z;
} Point;

typedef struct {
    Point *points;
    int front, rear, size;
} Queue;

// Function to create a queue
Queue* createQueue(int capacity) {
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    queue->points = (Point*)malloc(capacity * sizeof(Point));
    queue->front = 0;
    queue->rear = 0;
    queue->size = capacity;
    return queue;
}

bool isEmpty(Queue *queue) {
    return queue->front == queue->rear;
}

void enqueue(Queue *queue, Point point) {
    queue->points[queue->rear++] = point;
}

Point dequeue(Queue *queue) {
    return queue->points[queue->front++];
}

// BFS pathfinding
bool bfsPathfinding(int array[X_SIZE][Y_SIZE][Z_SIZE], Point start, Point end) {
    Queue *queue = createQueue(X_SIZE * Y_SIZE * Z_SIZE);
    bool visited[X_SIZE][Y_SIZE][Z_SIZE] = {{{false}}};

    int directions[6][3] = {
        {1, 0, 0}, {-1, 0, 0}, 
        {0, 1, 0}, {0, -1, 0}, 
        {0, 0, 1}, {0, 0, -1}
    };

    enqueue(queue, start);
    visited[start.x][start.y][start.z] = true;

    while (!isEmpty(queue)) {
        Point current = dequeue(queue);

        // Check if we reached the destination
        if (current.x == end.x && current.y == end.y && current.z == end.z) {
            free(queue->points);
            free(queue);
            return true; // Path found
        }

        for (int i = 0; i < 6; i++) {
            int newX = current.x + directions[i][0];
            int newY = current.y + directions[i][1];
            int newZ = current.z + directions[i][2];

            if (newX >= 0 && newX < X_SIZE && 
                newY >= 0 && newY < Y_SIZE && 
                newZ >= 0 && newZ < Z_SIZE &&
                array[newX][newY][newZ] >= 2 && 
                !visited[newX][newY][newZ]) {
                
                visited[newX][newY][newZ] = true;
                enqueue(queue, (Point){newX, newY, newZ});
            }
        }
    }

    free(queue->points);
    free(queue);
    return false; // No path found
}
void UpdateDwarfCx(PersonList *list, size_t index, int cx) {
    if (index >= list->size) {
        printf("Index out of bounds. Unable to update person.\n");
        return; // Handle invalid index
    }

    // Update the person's data
    /*
    strncpy(list->persons[index].name, name, NAME_LENGTH);
    list->persons[index].age = age;
    memcpy(list->persons[index].professions, professions, MAX_PROFESSIONS * sizeof(int));
    memcpy(list->persons[index].inventory, inventory, MAX_INVENTORY * sizeof(int));
    list->persons[index].free = free;
    list->persons[index].task = task;
    list->persons[index].material = material;
    list->persons[index].dx = dx;
    list->persons[index].dy = dy;
    */
    list->persons[index].cx = cx;
}
void UpdateDwarfCy(PersonList *list, size_t index, int cy) {
    if (index >= list->size) {
        printf("Index out of bounds. Unable to update person.\n");
        return; // Handle invalid index
    }
    list->persons[index].cy = cy;
}
void UpdateDwarfCz(PersonList *list, size_t index, int cz) {
    if (index >= list->size) {
        printf("Index out of bounds. Unable to update person.\n");
        return; // Handle invalid index
    }
    list->persons[index].cz = cz;
}
void UpdateDwarfOffset(PersonList *list, size_t index, int offset) {
    if (index >= list->size) {
        printf("Index out of bounds. Unable to update person.\n");
        return; // Handle invalid index
    }
    list->persons[index].offset = offset;
}
//UpdateDwarfCz(&list, 0, 99);
void PrintLine(SDL_Renderer *renderer, SDL_Texture *spriteSheet, char *test, int len, int scale, int sx, int sy, int r, int g, int b) {
    for (int i = 0; test[i] != '\0'; i++) {
        Texter(renderer, spriteSheet, sx + (scale * 8 * i), sy, test[i], scale, r, g, b);
    }

}
//PrintLine(renderer, spriteSheet, test, 9, 2, 500, 0, 200, 200, 200);//char-0-scale-startx-starty-r-g-b
int ***createWorldArray(size_t x, size_t y, size_t z) {
    int ***array = malloc(x * sizeof(int **));
    for (size_t i = 0; i < x; i++) {
        array[i] = malloc(y * sizeof(int *));
        for (size_t j = 0; j < y; j++) {
            array[i][j] = malloc(z * sizeof(int));
        }
    }
    return array;
}

void freeWorldArray(int ***array, size_t x, size_t y) {
    for (size_t i = 0; i < x; i++) {
        for (size_t j = 0; j < y; j++) {
            free(array[i][j]); 
        }
        free(array[i]); 
    }
    free(array); 
}
/*
3D pathfinding run example:
Point start = {0, 0, 0}; // Starting point
    Point end = {9, 9, 9};   // Ending point

    if (bfsPathfinding(worldarray, start, end)) {
        printf("Path found!\n");
    } else {
        printf("No path found.\n");
    }

    

*/
void menu0print(SDL_Renderer *renderer, SDL_Texture *spriteSheet) {
    char test[9] = "Main menu";
    PrintLine(renderer, spriteSheet, test, 9, 2, 500, 0, 200, 200, 200);//char-len-scale-startx-starty-r-g-b
    PrintLine(renderer, spriteSheet, "d-designation", 13, 2, 500, 30, 200, 200, 200);
    PrintLine(renderer, spriteSheet, "Space-Pause Game", 16, 2, 500, 64, 200, 200, 200);
}
void menu1print(SDL_Renderer *renderer, SDL_Texture *spriteSheet,int des) {
    char test[20] = "Designation Menu";
    PrintLine(renderer, spriteSheet, test, 9, 2, 500, 0, 200, 200, 200);//char-len-scale-startx-starty-r-g-b
    if (des == 1){
    	PrintLine(renderer, spriteSheet, "d-dig", 13, 2, 500, 30, 0, 200, 200);
    } else {
    	PrintLine(renderer, spriteSheet, "d-dig", 13, 2, 500, 30, 200, 200, 200);
    }
    if (des==2){
    	PrintLine(renderer, spriteSheet, "t-trees", 13, 2, 500, 64, 0, 200, 200);
    } else {
    	PrintLine(renderer, spriteSheet, "t-trees", 16, 2, 500, 64, 200, 200, 200);
    }
}
void menu2print(SDL_Renderer *renderer, SDL_Texture *spriteSheet) {
    char test[20] = "Designation Menu";
    PrintLine(renderer, spriteSheet, test, 9, 2, 500, 0, 200, 200, 200);//char-len-scale-startx-starty-r-g-b
    PrintLine(renderer, spriteSheet, "d-dig", 13, 2, 500, 30, 200, 200, 200);
    PrintLine(renderer, spriteSheet, "t-trees", 16, 2, 500, 64, 200, 200, 200);
}
int main(int argc, char *argv[]) {
	int designation = 0;
    int sdx;
    int sdy;
    int sdz;
    int edx;
    int edy;
    int edz;
    int dclicks=0;
    int numTile=0;
    char strTile[30] = "NO DATA AVAILABLE";
    int cursorx=0;
    int cursory=0;
    int cursor = 0;
    int menu = 0;
    PersonList list;
    initPersonList(&list, 2); // Initial capacity of 2

    int professions1[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int inventory1[] = {10, 20, 0, 0, 0, 0, 0, 0, 0, 0};
    int path[300] = {0};

    appendPerson(&list, "Woodcutter", 30, professions1, inventory1, 1, 1, 2, 3, 4, 6, 6, 0, path, 87);
    
    int professions2[] = {3, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int inventory2[] = {30, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    appendPerson(&list, "Urist", 25, professions2, inventory2, 1, 0, 5, 6, 7, 8, 9, 0, path, 10);
    


    int overx = 0;
    float biome_scaling = .01; //.01 = classic 
    int overy = 0;
    int paused = 1;
    int scx = 30;
    int scy = 20;
    int t = 0;
    int tasks[X_SIZE][Y_SIZE][Z_SIZE];
    int worldarray[X_SIZE][Y_SIZE][ZW_SIZE];
    FPSCounter fpsCounter;
    initFPSCounter(&fpsCounter); // Initialize the FPS counter

    // Initialize all values to -11
    for (int x = 0; x < X_SIZE; x++) {
        for (int y = 0; y < Y_SIZE; y++) {
            for (int z = 0; z < ZW_SIZE; z++) {
                worldarray[x][y][z] = -11;
            }
        }
    }
    int z_level = 0;
    srand(time(NULL));
    //////////////////////////////////
    
    init_permutation();
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            double noise = perlin(i * .05, j * .05);
            if(noise >.7) {
                worldarray[i][j][0] = 2; //grass
            } else if(noise >.6) {
                worldarray[i][j][1] = 2; //grass
            } else if(noise >.5) {
                worldarray[i][j][2] = 2; //grass
            } else if(noise >0) {
                worldarray[i][j][3] = 2; //grass
            }
            float rng = (float)rand() / (float)RAND_MAX; // This will be between 0.0 and 1.0

            if (rng > .96) {
                if (worldarray[i][j][0]==2) {
                    worldarray[i][j][0] = -12; //tree
                } else if (worldarray[i][j][1]==2) {
                    worldarray[i][j][1] = -12; //tree
                } else if (worldarray[i][j][2]==2) {
                    worldarray[i][j][2] = -12; //tree
                } else if (worldarray[i][j][3]==2) {
                    worldarray[i][j][3] = -12; //tree
                }
            }
            
        }
    }
    for (int i = 0; i < 100; i++) { 
        for (int j = 0; j < 100; j++) {
            for (int k = 4; k >= 0; k--) { 
                if(worldarray[i][j][k+1]==-12) {
                    worldarray[i][j][k]=-12;
                }
            }
        }
    } 
                
    for (int i = 0; i < 100; i++) { //Putting air in the world
        for (int j = 0; j < 100; j++) {
            for (int k = 0; k < 4; k++) {  
                if(worldarray[i][j][k+1]==2) {
                    worldarray[i][j][k]=1;
                } else if(worldarray[i][j][k+2]==2) {
                    worldarray[i][j][k]=1;
                } else if(worldarray[i][j][k+3]==2) {
                    worldarray[i][j][k]=1;
                } else if(worldarray[i][j][k+4]==2) {
                    worldarray[i][j][k]=1;
                } 
            }
        }
    }
    init_permutation();

    // Generate some noise values
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            double noise = perlin(i * .08, j * .08);
            if(noise >.63) {
                if(worldarray[i][j][0]==2) {
                    worldarray[i][j][0]=4; // grass 2
                } else if(worldarray[i][j][1]==2) {
                    worldarray[i][j][1]=4; // grass 2
                } else if(worldarray[i][j][2]==2) {
                    worldarray[i][j][2]=4; // grass 2
                } else if(worldarray[i][j][3]==2) {
                    worldarray[i][j][3]=4; // grass 2
                }
            }
        }
    }
    init_permutation();

    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            double noise = perlin(i * .2, j * .2);
            
            if(noise >.63) {
                if(worldarray[i][j][0]==2) {
                    worldarray[i][j][0]=5; // grass 3
                } else if(worldarray[i][j][1]==2) {
                    worldarray[i][j][1]=5; // grass 3
                } else if(worldarray[i][j][2]==2) {
                    worldarray[i][j][2]=5; // grass 3
                } else if(worldarray[i][j][3]==2) {
                    worldarray[i][j][3]=5; // grass 3
                }
            }
        }
    }
    
    
    
    //////////////////////////////////
    
    int array[X_SIZE][Y_SIZE][Z_SIZE];
    for (int x = 0; x < X_SIZE; x++) {
        for (int y = 0; y < Y_SIZE; y++) {
            for (int z = 0; z < Z_SIZE; z++) {
                if (z % 2 == 0) {  // Check if the index is even
                    array[x][y][z] = worldarray[x][y][z/2];  // Copy from worldarray
                } else {
                    array[x][y][z] = 0;  // Set odd index to 0
                }
            }
        }
    }
    //ramps
    for (int i = 1; i < X_SIZE-1; i++) {
        for (int j = 1; j < Y_SIZE-1; j++) {
            for (int k = 0; k < 10; k++) {
                if (array[i][j][k] >= 2 &&
                    array[i][j][k] != 6) {
                    if (array[i+1][j][k] == 1) {
                        if (array[i+1][j][k+2] >= 2) {
                            array[i+1][j][k] = 6;
                            array[i+1][j][k+1] = 6;
                            array[i+1][j][k+2] = 7;
                        }
                    } if (array[i-1][j][k] == 1) {
                        if (array[i-1][j][k+2] >= 2) {
                            array[i-1][j][k] = 6;
                            array[i-1][j][k+1] = 6;
                            array[i-1][j][k+2] = 7;
                        }
                    } if (array[i][j+1][k] == 1) {
                        if (array[i][j+1][k+2] >= 2) {
                            array[i][j+1][k] = 6;
                            array[i][j+1][k+1] = 6;
                            array[i][j+1][k+2] = 7;
                        }
                    } if (array[i][j-1][k] == 1) {
                        if (array[i][j-1][k+2] >= 2) {
                            array[i][j-1][k] = 6;
                            array[i][j-1][k+1] = 6;
                            array[i][j-1][k+2] = 7;
                        }
                    }
                }
            }
        }
    }
    //testing pathing
    Point start = {0, 0, 6}; // Starting point
    Point end = {6, 0, 6};   // Ending point

    if (bfsPathfinding(array, start, end)) {
        printf("Path found!\n");
    } else {
        printf("No path found.\n");
    }
    //gravity
    for(int i; i<10; i++) {
        size_t cxCount;
        int *cxValues = getAllCxValues(&list, &cxCount);
        size_t cyCount;
        int *cyValues = getAllCyValues(&list, &cyCount);
        size_t czCount;
        int *czValues = getAllCzValues(&list, &czCount);
        for (size_t j = 0; j < cxCount; j++) {
            if (array[cxValues[j]][cyValues[j]][czValues[j]] < 2) {
                UpdateDwarfCz(&list, j, czValues[j]+2);
            }
        }
        
    }
    
                
            
        
    

    /*
    init_permutation();

    // Generate some noise values
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            double noise = perlin(i * 0.1, j * 0.1);
            printf("Noise at (%d, %d): %f\n", i, j, noise);
        }
    }
    */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Emerald Lands", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load sprite sheet
    SDL_Texture *spriteSheet = IMG_LoadTexture(renderer, "Tileset/curses.bmp");
    if (!spriteSheet) {
        printf("Failed to load texture: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    // Set blend mode for transparency
    SDL_SetTextureBlendMode(spriteSheet, SDL_BLENDMODE_BLEND);

    // Main loop
    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        if (menu != 0) {
                            menu = 0;
                            printf("menu - 0\n");
                            
                        }
                        else if (menu == 0) {
                            running = 0; // Exit on ESC key press
                        }
                        break;
                    case SDLK_SPACE:
                        if(menu == 0) {
                            if(paused==0) {
                                paused++;
                            } else {
                                paused--;
                            }
                        }
                        break;
                    case SDLK_RIGHT:
                        if (menu == 0) {
                            if (cursor==1) {
                                cursorx++;
                            
                            } else if(overx < 70) {
                                overx = overx + 5;
                            } 
                        } else if (menu == 1) {
                            if (cursor==1) {
                                cursorx++;
                            
                            } else if(overx < 70) {
                                overx = overx + 5;
                            } 
                        }
                        break;
                    case SDLK_DOWN:
                        if (menu == 0) {
                            if (cursor==1) {
                                cursory++;
                            } else if(overy < 70) {
                                overy = overy + 5;
                            }
                        } else if (menu == 1) {
                            if (cursor==1) {
                                cursory++;
                            } else if(overy < 70) {
                                overy = overy + 5;
                            }
                        }
                        break;
                    case SDLK_LEFT:
                        if (menu == 0) {
                            if (cursor==1) {
                                cursorx--;
                            } else if(overx > 0) {
                                overx = overx - 5;
                            }
                        } else if (menu == 1) {
                            if (cursor==1) {
                                cursorx--;
                            } else if(overx > 0) {
                                overx = overx - 5;
                            }
                        }
                        break;
                    case SDLK_UP:
                        if (menu == 0) {
                            if (cursor==1) {
                                cursory--;
                            } else if(overy > 0) {
                                overy = overy - 5;
                            }
                        } else if (menu == 1) {
                        	if (cursor==1) {
                                cursory--;
                            } else if(overy > 0) {
                                overy = overy - 5;
                            }
                        }
                        	
                        break;
                    case SDLK_PERIOD:
                        if (menu == 0) {
                            if (SDL_GetModState() & KMOD_SHIFT) {
                                if(z_level<Z_SIZE) {
                                    z_level++;
                                    z_level++;
                                }
                            }
                        } 
                        break;
                    case SDLK_COMMA:
                        if (menu == 0) {
                            if (SDL_GetModState() & KMOD_SHIFT) {
                                if(z_level>0) {
                                    z_level--;
                                    z_level--;
                                }
                            } 
                        }
                        break;
                    case SDLK_d:
                        if (menu == 0) {
                            menu = 1;
                            printf("menu - 1\n");
                            cursor=1;
                        } else if (menu==1) {
                        	designation=1;
                        }
                        break;
                    case SDLK_k:
                        if (menu == 0 &&
                        cursor==0) {
                            cursor=1;
                        } else if (menu == 0 &&
                        cursor==1) {
                            cursor=0;
                        } 
                        break;
                    case SDLK_t:
                    	if (menu==1) {
                        	designation=2;
                        }
                        break;
                    case SDLK_RETURN:
                    	if (menu==1 && dclicks==0) {
                    		dclicks=1;
                    		sdx=cursorx-overx;
                    		sdy=cursory-overy;
                    		sdz=z_level;
                    		printf("%d\n",sdx);
                    	}
                    	
                    
                    // Add more keys as needed
                    
                    default:
                        break;
                
                }
            }
        }
        t++;
    
        

        // Clear the renderer to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render the characters based on the 3D array
    for (int i = 0; i < scx; i++) { 
        for (int j = 0; j < scy; j++) { 
            int characterIndex = 0; 
            int cr = 0;
            int cg = 0;
            int cb = 0;

            
            if (array[i+overx][j+overy][z_level] == 1) {
                characterIndex = 249; 
                cr = 100;
                cg = 100;
                cb = 100;
            } else if (array[i+overx][j+overy][z_level] == 2) {
                characterIndex = 44; 
                cr = 0;
                cg = 255;
                cb = 0;
            } else if (array[i+overx][j+overy][z_level] == -12) {
                characterIndex = 79; 
                cr = 139;
                cg = 69;
                cb = 19;
            } else if (array[i+overx][j+overy][z_level] == 4) {
                characterIndex = 34; // Change this index based on your logic
                cr = 7;
                cg = 150;
                cb = 10;
            } else if (array[i+overx][j+overy][z_level] == 5) {
                characterIndex = 46; // Change this index based on your logic
                cr = 160;
                cg = 140;
                cb = 10;
            } else if (array[i+overx][j+overy][z_level] == 6) {
                characterIndex = 31; // Change this index based on your logic
                cr = 0;
                cg = 200;
                cb = 20;
            } else if (array[i+overx][j+overy][z_level] == 7) {
                characterIndex = 30; // Change this index based on your logic
                cr = 0;
                cg = 200;
                cb = 20;
            } else if (array[i+overx][j+overy][z_level] == -11) {
                
                
                characterIndex = 0; // Change this index based on your logic
                if(array[i+overx+1][j+overy][z_level]>-10) {
                    characterIndex=219;   
                } else if(array[i+overx-1][j+overy][z_level]>-10) {
                    characterIndex=219;   
                } else if(array[i+overx][j+overy+1][z_level]>-10) {
                    characterIndex=219;   
                } else if(array[i+overx][j+overy-1][z_level]>-10) {
                    characterIndex=219;   
                }
                cr = 140;
                cg = 140;
                cb = 120;
            }

            // Calculate the source rectangle for the sprite sheet
            SDL_Rect srcRect = {
                (characterIndex % NUM_COLUMNS) * CHAR_WIDTH,
                (characterIndex / NUM_COLUMNS) * CHAR_HEIGHT,
                CHAR_WIDTH,
                CHAR_HEIGHT
            };

            // Calculate the destination rectangle for rendering
            SDL_Rect dstRect = { i * 16, j * 24, CHAR_WIDTH * 2, CHAR_HEIGHT * 2 }; // Scale up for visibility

            // Set color modulation for the texture (optional)
            SDL_SetTextureColorMod(spriteSheet, cr, cg, cb); // Example: render white pixels as green

            // Render the character from the sprite sheet
            SDL_RenderCopy(renderer, spriteSheet, &srcRect, &dstRect);
        }
    }
    
            
    //printing dwarves
    size_t cxCount;
    int *cxValues = getAllCxValues(&list, &cxCount);
    size_t czCount;
    size_t cyCount;
    int *cyValues = getAllCyValues(&list, &cyCount);
    int *czValues = getAllCzValues(&list, &czCount);
    // Optionally process cxValues here
    for (size_t i = 0; i < cxCount; i++) {
        if (czValues[i] == z_level) {
            if (cxValues[i]-overx > 0) {
                if (cxValues[i]-overx < scx) {
                    if (cyValues[i]-overy < scy) {
                        if (cyValues[i]-overy > 0) {
                            EntityPrint(renderer, spriteSheet, 16*cxValues[i]-(16*overx), 24*cyValues[i]-(24*overy), 1, 2,0,255,255);
                        }
                    }
                }
            }
        }
    }
    //dwarf idle movement
    int *freeValues = getAllFreeValues(&list, &czCount);
    for (size_t i = 0; i < czCount; i++) {
        if (t % 150 == getOneOffsetValue(&list, i) && paused == 0) {
            if (freeValues[i] == 1) {
                int mx = (rand() % 3) - 1;
                int my = (rand() % 3) - 1;
                if (array[cxValues[i]+mx-overx][cyValues[i]+my-overy][czValues[i]]> 1) {
                    UpdateDwarfCx(&list, i, cxValues[i]+mx);
                    UpdateDwarfCy(&list, i, cyValues[i]+my);
                }   
            }
        }
    }
    //trying to fix memory leaks
    free(cxValues);
    free(cyValues);
    free(czValues);
    free(freeValues);


    
	if (cursorx > scx-5 && overx < X_SIZE-30) {
		for (int i=0; i < 5; i++) {
			cursorx--;
			overx++;
		}
	} else if (overx == X_SIZE-30 && cursorx >scx-1) {
		cursorx--;
	} else if (cursorx < 5 && overx > 0) {
		for (int i=0; i < 5; i++) {
			cursorx++;
			overx--;
		}
	} else if (overx == 0 && cursorx <= -1) {
		cursorx++;
	}


    //border char 219
    for(int i=0; i <scx; i++) {
            int characterIndex = 219; // Change this index based on your logic
            int cr = 100;
            int cg = 100;
            int cb = 100;

            SDL_Rect srcRect = {
                (characterIndex % NUM_COLUMNS) * CHAR_WIDTH,
                (characterIndex / NUM_COLUMNS) * CHAR_HEIGHT,
                CHAR_WIDTH,
                CHAR_HEIGHT
            };

            
            SDL_Rect dstRect = { i * 16,490, CHAR_WIDTH * 2, CHAR_HEIGHT * 2 }; 

            // colorify it
            SDL_SetTextureColorMod(spriteSheet, cr, cg, cb); 

            // Render
            SDL_RenderCopy(renderer, spriteSheet, &srcRect, &dstRect);
    }
    for(int i = 0; i < 2*scy+2; i++) {
        SheetPrint(renderer, spriteSheet, 
                      scx*16, i*12, 219, 1, 100, 100, 100);
    }
    SheetPrint(renderer, spriteSheet, 
                      scx*16, (2*scy+2)*12-2, 219, 1, 100, 100, 100);
    
        
        
    
    ///////pause gui
    if(paused==1) {
    
    char test[10] = "Paused";
    PrintLine(renderer, spriteSheet, test, 0, 2, 0, 525, 0, 0, 255);//char-len-scale-startx-starty-r-g-b
    }    
    ///////bottom menu stuff
    char result[3]; // Enough to hold up to 999 z-levels

    splitInteger(z_level, result);
    for (int i = 0; result[i] != '\0'; i++) {
        Texter(renderer, spriteSheet, i*8, 590, result[i], 1,255,255,255); 
    }
    char result2[10]; // Enough to hold t for a while
    splitInteger(t, result2);
    for (int i = 0; result2[i] != '\0'; i++) {
        Texter(renderer, spriteSheet, i*8, 580, result2[i], 1,255,255,255); 
    }
    updateFPS(&fpsCounter, t);
    if (menu == 0) {
        menu0print(renderer, spriteSheet);
    } else if (menu == 1) {
    	menu1print(renderer, spriteSheet,designation);
    }
    if (cursor==1) {
        
        Texter(renderer, spriteSheet, 16*cursorx, 24*cursory,'x',2,255,255,255);
        numTile=array[cursorx-overx][cursory-overy][z_level];
        if (numTile > 0) {
            if (numTile == 1) {
                strcpy(strTile, "Air");
            } else if (numTile == 2) {
                strcpy(strTile, "Grass");
            } else if (numTile == 3) {
                strcpy(strTile, "UNKNOWN TILE");
            } else if (numTile == 4) {
                strcpy(strTile, "Dense Grass");
            } else if (numTile == 5) {
                strcpy(strTile, "Short Grass");
            } else if (numTile == 6) {
                strcpy(strTile, "Grassy Upward Slope");
            } else if (numTile == 7) {
                strcpy(strTile, "Grassy Downward Slope");
            }
            PrintLine(renderer, spriteSheet,strTile, 90, 2, 400, 570, 200, 200, 200);
        }
    }
    
    




    // does stuff
    SDL_RenderPresent(renderer);
    usleep(10000);
    fflush(stdout);


        

        
    }
    

    // if I quit.
    SDL_DestroyTexture(spriteSheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

