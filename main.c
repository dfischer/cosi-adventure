#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"

typedef struct room room_t;
typedef struct item item_t;
typedef struct itemList itemList_t;

struct item {
	char * name;
	char * description;
	char ** actions;
};

struct room {
	char * description;
	room_t * north, * south, * east, * west;
	itemList_t * items;
};

/* Although cumbersome, this will hopefully make dynamic allocation easier */
struct itemList {
	item_t ** itemArray;
	int capacity;
	int size;
};

typedef enum {
	NORTH,
	SOUTH,
	EAST,
	WEST
} compass;

room_t * room = NULL;
itemList_t * inventory = NULL;

/* Loads data into structs */
void init(){	
	/* This enum defines the sections of the datafile in order */
	enum {
		NONE,
		ROOM_DESC,
		ROOM_LINKS,
		OBJ_PROP,
		ROOM_OBJS
	} section = NONE;

	int x;
	int rv;
	int n, s, e, w;
	int rm, itm;
	FILE * f;
	room_t * rooms = malloc(sizeof(room_t) * 4);
	item_t * items = malloc(sizeof(item_t) * 3);

	char str[160]; /* This limits the length of room descriptions */
	str[159] = 0;

	/* probably should move this somewhere else */
	/* Also need a better way of doing this. Should not have the number of rooms hardcoded! */
	for (x = 0; x < 4; x++) {
		rooms[x].description = NULL;
		rooms[x].north = NULL;
		rooms[x].south = NULL;
		rooms[x].east  = NULL;
		rooms[x].west  = NULL;
		rooms[x].items = malloc(sizeof(itemList_t));

		rooms[x].items->itemArray = malloc(sizeof(item_t) * 3);
		rooms[x].items->itemArray[0] = NULL;
		rooms[x].items->itemArray[1] = NULL;
		rooms[x].items->itemArray[2] = NULL;
		rooms[x].items->capacity = 3;
		rooms[x].items->size = 0;
	}

	for (x=0; x<3; x++){
		items[x].name = NULL;
		items[x].description = NULL;
		items[x].actions = NULL;
	}

	f = fopen("data", "r");
	assert(f != NULL);

	/* KLUDGE - stupid assignment trick by Andrew */
	while ((rv = fscanf(f, "%d ", &x)) && rv != EOF) {
		if (-1 == x) {
			while ((rv = fgetc(f)) != '\n' && rv != EOF);
			section++;
		} else {
			switch (section) {
			case NONE:
				fprintf(stderr, "Error: Yous si fidodin somehtin srong.\n");
				break;

			case ROOM_DESC:
				assert(x < 4);
				rooms[x].description = getstring('\n', f);
				break;

			case ROOM_LINKS:
				assert(4 == fscanf(f, " %d %d %d %d\n", &n, &s, &e, &w));

				assert(x < 4);
				assert(n < 4);
				assert(s < 4);
				assert(e < 4);
				assert(w < 4);

				rooms[x].north = n != -1 ? rooms + n : NULL;
				rooms[x].south = s != -1 ? rooms + s : NULL;
				rooms[x].east  = e != -1 ? rooms + e : NULL;
				rooms[x].west  = w != -1 ? rooms + w : NULL;
				break;

			case OBJ_PROP:
				assert(x < 3);
				items[x].name = getstring('\n', f);
				fscanf(f, "%d ", &x);
				items[x].description = getstring('\n', f);
				fscanf(f, "%d ", &x);
				fgets(str, 159, f);
				break;

			case ROOM_OBJS:
				assert(2 == fscanf(f, " %d %d\n", &rm, &itm));
				rooms[rm].items->itemArray[rooms[rm].items->size] = &items[itm];
				(rooms[rm].items->size)++;
				break;

			default:
				fgets(str, 159, f);
				fprintf(stderr, "Error: default case reached?\n");
				break;
			}
		}
	}

	fclose(f);

	room = rooms;

	/* Initialize inventory */
	inventory = malloc(sizeof(itemList_t));
	inventory->itemArray = malloc(sizeof(item_t) * 3);
		for (x = 0; x < 3; x++) {
			inventory->itemArray[x] = NULL;
		}
	inventory->capacity = 3;
	inventory->size = 0;

	return;
}

/* Describes the situation */
int watsup(){
	/* The room description should be able to change even within the same room.
	   Semi-random events should also be printed. (e.g. "Chris falls out of his chair")
	   Additionally, it should list objects in the room. */
	int x;

	assert(room != NULL);
	puts(room->description);

	for (x = 0; x < (room->items->size); x++){
		puts(room->items->itemArray[x]->description);
	}
	return 0;
}

void go(compass c){
	room_t * n = NULL;

	switch (c) {
	case NORTH: n = room->north; break;
	case SOUTH: n = room->south; break;
	case EAST:  n = room->east;  break;
	case WEST:  n = room->west;  break;
	}

	if (n == NULL) {
		puts("You cannot go that way.");
	} else {
		room = n;
		watsup();
	}
}

void take(){
	if ((inventory->size < inventory->capacity)&&(room->items->size > 0)){
		inventory->itemArray[inventory->size] = room->items->itemArray[(room->items->size)-1];
		(inventory->size)++;
		room->items->itemArray[(room->items->size)-1] = NULL;
		(room->items->size)--;
	}else puts("Nothing here, or your inventory is full");
}

void drop(){
	if ((room->items->size < room->items->capacity)&&(inventory->size > 0)){
		room->items->itemArray[room->items->size] = inventory->itemArray[(inventory->size)-1];
		(room->items->size)++;
		inventory->itemArray[(inventory->size)-1] = NULL;
		(inventory->size)--;
	}else puts("Your inventory is empty, or the room is full");
}

void showinv(){
	int x;
	for (x = 0; x < (inventory->size); x++){
		if (inventory->itemArray[x] != NULL) puts(inventory->itemArray[x]->name);
	}
}

int main(){
	int dead=0,win=0,quit=0;
	char inp[80];
	inp[79] = 0;

	/* Load data, create world */
	init();
	watsup();

	/* loop runs watsup() and inner loop takes commands until one works */
	while(!dead && !win && !quit){
		printf("What do? ");
		scanf ("%79s",inp); /* Get commands */

		if (!strncmp(inp,"quit",4)) quit=1;
		if (!strncmp(inp,"north",5)) go(NORTH);
		if (!strncmp(inp,"south",5)) go(SOUTH);
		if (!strncmp(inp,"east",4)) go(EAST);
		if (!strncmp(inp,"west",4)) go(WEST);
		if (!strncmp(inp,"take",4)) take();
		if (!strncmp(inp,"drop",4)) drop();
		if (!strncmp(inp,"look",4)) watsup();
		if (!strncmp(inp,"inv",3)) showinv();

		/* This is where a parse function would be called, and it would call other functons accordingly.
		   To get started, let's implement "go <dir>, take <obj>, look, eat <inv item>. */
	}
	return 0;
}
