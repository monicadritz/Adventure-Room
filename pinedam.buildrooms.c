// File to build rooms for adventure game

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_ROOMS 6;

//declaring the rooms of the game 
char ROOM_NAME1[256] = "Astral Queen";
char ROOM_NAME2[256] = "Galactica";
char ROOM_NAME3[256] = "Aurora";
char ROOM_NAME4[256] = "Carina";
char ROOM_NAME5[256] = "Celestra";
char ROOM_NAME6[256] = "Chrion";
char ROOM_NAME7[256] = "Colonial One";
char ROOM_NAME8[256] = "Cybele";
char ROOM_NAME9[256] = "Cloud 9";
char ROOM_NAME10[256] = "Gemini";

char* ROOM_NAMES[] = {ROOM_NAME1, ROOM_NAME2 , ROOM_NAME3 , ROOM_NAME4 , ROOM_NAME5 , ROOM_NAME6 , ROOM_NAME7 , ROOM_NAME8 , ROOM_NAME9 , ROOM_NAME10};
// setting up the file names 
char FILE_NAME1[256] = "Room1";
char FILE_NAME2[256] = "Room2";
char FILE_NAME3[256] = "Room3";
char FILE_NAME4[256] = "Room4";
char FILE_NAME5[256] = "Room5";
char FILE_NAME6[256] = "Room6";
char FILE_NAME7[256] = "Room7";

char* FILE_NAMES[] = { FILE_NAME1, FILE_NAME2 , FILE_NAME3 , FILE_NAME4 , FILE_NAME5 , FILE_NAME6 , FILE_NAME7 };

int NUMBER_POSSIBLE_ROOMS = 10;

//assigning my onid 
char* onid = "pinedam";

typedef struct Room Room;
typedef enum Room_Type RoomType;

void initConenctions(Room* rooms[], int num_rooms, int* room_num_list);
void addRoomConnection(Room* room, char* connection);
int roomListNameMap(Room* rooms[], char* name);
void shuffle(int *array, size_t n);


//setting up the room type definition
enum Room_Type 
{
	START_ROOM = 0, 
	MID_ROOM = 1, 
	END_ROOM = 2
};
 
//
char* roomTypeString(RoomType type)
{
	switch (type)	
	{
	case START_ROOM:
		return "START_ROOM";
		break;
	case MID_ROOM:
		return "MID_ROOM";
		break;
	case END_ROOM:
		return "END_ROOM";
		break;
	default:
		return "INVALID ROOM TYPE";
		break;
	}
	return "ERROR";
}

struct Room
{

	int room_num;
	char* name;
	int num_connections;
	RoomType type;
	//Room* connection_list[6];
	char connection_names[6][256];
	int room_numbers[6];

};


int compare(Room* one, Room* two)
{
	if (one->room_num == two->room_num)
		return 1;
	else
		return 0;
}
// shuffleing the room path
void shuffle(int *array, size_t n)
{
	if (n > 1)
	{
		size_t i;
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			int t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

// creating random int to use  to create a random set of rooms
int* randomInts(int num_ints)
{
	int* int_list = (int*)malloc(sizeof(int)*num_ints);
	int i;
	for (i = 0; i < num_ints; ++i)
		int_list[i] = i;
	shuffle(int_list, num_ints);

	return int_list;
}
// assignign the random ints to the rooms
Room** initRooms(int num_rooms)
{
	// Size array of rooms
	Room** rooms = malloc(sizeof(Room*) * num_rooms);
	int room_array_index;
	for(room_array_index = 0; room_array_index < num_rooms; room_array_index++)
		rooms[room_array_index] = (Room*)malloc(sizeof(Room));

	// Init rooms types, number, and number of conenctions
	// Start by creating a random list of ints
	int* random_ints = NULL;
	int* random_ints2 = NULL; //= (int*)malloc(sizeof(int)*NUMBER_POSSIBLE_ROOMS);
	random_ints = randomInts(num_rooms);
	random_ints2 = randomInts(NUMBER_POSSIBLE_ROOMS);

	int i;
	for (i = 0; i < num_rooms; ++i)
	{
		if (i == 0)
			rooms[random_ints[i]]->type = START_ROOM;
		else if(i == num_rooms - 1)
			rooms[random_ints[i]]->type = END_ROOM;
		else
			rooms[random_ints[i]]->type = MID_ROOM;

		// Initialize the number of conenctions in each room
		//initNumConnections(&rooms[random_ints[i]]);
		// Assign room id/room number (for comparisons) (basically pointless now, i think)
		rooms[i]->room_num = i;
		// Init number of connections to 0
		rooms[i]->num_connections = 0;
	}

	for (i = 0; i < num_rooms; ++i)
		rooms[i]->name = ROOM_NAMES[random_ints2[i]];
	

	// Last thing is to init connections
	initConenctions(rooms, num_rooms, random_ints);

	free(random_ints);
	free(random_ints2);

	return rooms;
}

// Init for a single room
void initNumConnections(Room* room)
{
	room->num_connections = rand() % 6 + 3;
}

// Init for all rooms
void initConenctions(Room* rooms[], int num_rooms, int* room_num_list)
{
	int i;
	for (i = 0; i < num_rooms; i++)
	{
		// Roll a random number of conenctions, 3 < x < 6
		int min_connections = rand() % 4 + 3;

		// Check previous rooms
		// Current room is i, rooms[i].room_num
		int connectionsComplete = 0;
		int conCount = rooms[i]->num_connections;

		// Check all previous conections
		int j;
		int* randomIndexArray = NULL;
		randomIndexArray = randomInts(num_rooms); // order in which rooms are added
		// Check if min connections has been met
		while (conCount < min_connections)
		{
			int index;
			for (index = 0; index < num_rooms; index++)
			{
				int room_int = randomIndexArray[index]; // pull random room (room_int) to possibly get added
				if (room_int == i) // i is room getting connections
					continue;
				// check to make sure connection is not already made
				if (!findRoomConnection(rooms[i], rooms[room_int]->name))
				{
					addRoomConnection(rooms[i], rooms[room_int]->name);
					// goes both ways
					addRoomConnection(rooms[room_int], rooms[i]->name);
					conCount++;
				}
				if (conCount >= min_connections)
					break;
			}
			conCount++;
		}
		free(randomIndexArray);
	}

	
}

// For finding if a room_number exists in the list of connections in room
int findRoomConnection(Room* room, char* room_name)
{
	int i;
	for (i = 0; i < room->num_connections; i++)
	{
		if (strcmp(room->connection_names[i], room_name) == 0)
			return 1;
	}
	return 0;
}
//create room connections
void addRoomConnection(Room* room, char* connection)
{
	strcpy(room->connection_names[room->num_connections], connection);
	//room->room_numbers[room->num_connections] = connection->room_num;
	room->num_connections++;
}
//prints rooms to a file  
void printRoomsOneFile(Room* rooms[], int num_rooms)
{
	FILE* roomFile;
	roomFile = fopen(onid, "w");

	if (roomFile != NULL)
	{
		int i;
		for (i = 0; i < num_rooms; i++)
		{
			fprintf(roomFile, "ROOM NAME: %s\n", rooms[i]->name);
			int j;
			for (j = 0; j < rooms[i]->num_connections; j++)
			{
				fprintf(roomFile, "CONNECTION %i: %s\n", j, rooms[i]->connection_names[j]);
			}
			fprintf(roomFile, "ROOM TYPE: %s\n", roomTypeString(rooms[i]->type));
			fprintf(roomFile, "\n\n");
		}
		fclose(roomFile);
	}
}

// Prints out individual rooms files
void printRooms(Room* rooms[], int num_rooms)
{
	int pid = getpid();
	char file_name[128];
	char directory_name[128];

	sprintf(directory_name, "%s.rooms.%i", onid, pid);

	mkdir(directory_name, 0777);

	FILE* roomFile;
	int i;
	for (i = 0; i < num_rooms; i++)
	{
		sprintf(file_name, "%s/%s", directory_name, FILE_NAMES[i]);
		roomFile = fopen(file_name, "w");
		if (roomFile != NULL)
		{
			fprintf(roomFile, "ROOM NAME: %s\n", rooms[i]->name);
			int j;
			for (j = 0; j < rooms[i]->num_connections; j++)
			{
				fprintf(roomFile, "CONNECTION %i: %s\n", j, rooms[i]->connection_names[j]);
			}
			fprintf(roomFile, "ROOM TYPE: %s\n", roomTypeString(rooms[i]->type));
			//fprintf(roomFile, "\n\n");
		}
		fclose(roomFile);

	}
}

int roomListNameMap(Room* rooms[], char* name)
{
	int room_index;
	for (room_index = 0; room_index < 7; room_index++)
	{
		if (strcmp(rooms[room_index]->name, name) == 0)
			return room_index;
	}
	return -1; // name not found
}

char* nameRoomListMap(Room* rooms[], int index)
{
	return rooms[index]->name;
}

int main(int argc, char* argv[])
{
	// Hard code number of rooms
	int num_rooms = 7;

	// Init all the rooms 
	Room** room_list;
	room_list = initRooms(num_rooms);
	printRooms(room_list, num_rooms);
	return 0;
}
