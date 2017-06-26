// File for playing the adventure game with room files

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>

// Hardcoded filenames that link up with those used in buildrooms
char FILE_NAME1[256] = "Room1";
char FILE_NAME2[256] = "Room2";
char FILE_NAME3[256] = "Room3";
char FILE_NAME4[256] = "Room4";
char FILE_NAME5[256] = "Room5";
char FILE_NAME6[256] = "Room6";
char FILE_NAME7[256] = "Room7";

char* FILE_NAMES[] = { FILE_NAME1, FILE_NAME2 , FILE_NAME3 , FILE_NAME4 , FILE_NAME5 , FILE_NAME6 , FILE_NAME7 };
// Determined number of room files
int NUMBER_OF_ROOM_FILES = 7;

// My personal onid cat with the directory title. This is a prefix for the pid later
char ROOM_DIRECTORY_PREFIX[128] = "pinedam.rooms.";
// Directory where rooms will be read from
char DIR_TO_READ[256];

// Declare mutex locks
pthread_mutex_t time_thread;

// Cleaner typedefs 
typedef struct Room Room;
typedef enum Room_Type RoomType;

void findRoomPath();

enum Room_Type
{
	START_ROOM = 0,
	MID_ROOM = 1,
	END_ROOM = 2
};

// Function acting as a map from type enum -> char*
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


// Useful struct with info about a sinuglar room, most importanatly the conenctions
struct Room
{
	int room_num;
	char name[256];
	int num_connections;
	RoomType type;
	char connection_names[6][256];
	int room_numbers[6];
};

// Mainly intended to be used in own thread
void* writeTime()
{
	pthread_mutex_lock(&time_thread); // begin the lock for the thread
	time_t time_plain;
	struct tm* time_info;

	time(&time_plain);
	time_info = localtime(&time_plain); // get the local time

	FILE* time_file;
	time_file = fopen("currentTime.txt", "w");
	if(time_file == NULL)
		pthread_mutex_unlock(&time_thread);
	fprintf(time_file, "%s", asctime(time_info));
	fflush(time_file);
	fclose(time_file);

	pthread_mutex_unlock(&time_thread); // unlock the process and allow main thread to cont
	return NULL;
}

// Simple file reader designed speficially for currentTime.txt
void  readTime()
{
	char time_string[256];

	FILE* time_file;
	time_file = fopen("currentTime.txt", "r");
	fgets(time_string, 256, time_file);
	fclose(time_file);
	printf("%s", time_string);

}

// File used to get rid of a leading whitespace char
// Neccessary for string comparisons
char* stripLeadingSpace(char* string)
{
	if (!string)
		return string;
	char *end;

	// Trim leading space
	while (isspace((unsigned char)*string)) string++;

	if (*string == 0)  // All spaces?
		return string;

	// Trim trailing space
	end = string + strlen(string) - 1;
	while (end > string && isspace((unsigned char)*end)) end--;

	// Write new null terminator
	*(end + 1) = 0;

	return string;
}

// Reads room files and creates an array of rooms
Room** readRoomsFromFiles(int num_rooms)
{
	// Init room array and individual rooms
	Room** rooms = malloc(sizeof(Room*) * num_rooms);
	int room_array_index;
	for (room_array_index = 0; room_array_index < num_rooms; room_array_index++)
		rooms[room_array_index] = (Room*)malloc(sizeof(Room));

	FILE* roomFile;
	char buffer[256];

	int file_index;
	for (file_index = 0; file_index < NUMBER_OF_ROOM_FILES; file_index++)
	{
		int connection_count = 0;
		rooms[file_index]->num_connections = 0;
		char file_path[128];
		sprintf(file_path, "%s/%s", DIR_TO_READ, FILE_NAMES[file_index]);

		roomFile = fopen(file_path, "r");
		//while loop for reading file
		while (fgets(buffer, 256, roomFile) != NULL) // read until fgets fails
		{

			//fgets(buffer, 256, roomFile);
			char* token = strtok(buffer, ":"); // All lines are delimited by ':', so tokenizing is ok here
			char* pos = NULL;
			if (strcmp(token, "ROOM NAME") == 0)
			{
				token = strtok(NULL, ":");
				token = stripLeadingSpace(token);
				strcpy(rooms[file_index]->name, token);
			}
			pos = strstr(token, "CONNECTION"); // check to see if a substring in the token is found. This way dont need to worry about connection number
			if (pos)
			{
				token = strtok(NULL, ":");
				token = stripLeadingSpace(token);
				strcpy(rooms[file_index]->connection_names[connection_count], token);
				rooms[file_index]->num_connections++;
				connection_count++;
			}

			if (strcmp(token, "ROOM TYPE") == 0)
			{
				token = strtok(NULL, ":");
				token = stripLeadingSpace(token);
				if (strcmp(token, "START_ROOM") == 0)
					rooms[file_index]->type = START_ROOM;
				if (strcmp(token, "MID_ROOM") == 0)
					rooms[file_index]->type = MID_ROOM;
				if (strcmp(token, "END_ROOM") == 0)
					rooms[file_index]->type = END_ROOM;
				break;
			}

		}

		fclose(roomFile);
	}
	return rooms;
}

// Returns the room index in a given list for a given name
// Mostly a convenience function
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

// determines the most recent 'room' dir to use
void findRoomPath()
{
	struct stat* s;
	struct stat statbuf;
	struct dirent* dp;
	DIR* directory;
	char pwd[128];
	getcwd(pwd, sizeof(pwd));
	directory = opendir(pwd);
	time_t recent_time;
	//char* most_recent_file = NULL;
	int file_count = 0;
	while ((dp = readdir(directory)) != NULL) // cycle through all of the items in the cwd
	{
		char* pos = NULL;
		pos = strstr(dp->d_name, ROOM_DIRECTORY_PREFIX); // does the current item match the onid prefix?
		if (pos)
		{
			stat(dp->d_name, &statbuf);
			if (file_count == 0)
				recent_time = statbuf.st_mtime; // initial time is used as the best time
			if (difftime(statbuf.st_mtime, recent_time) >= 0) //only care about the item's name if it's most recent
			{
				recent_time = statbuf.st_mtime;
				strcpy(DIR_TO_READ, dp->d_name);
			}
		}

	}

}

int main(int argc, char* argv[])
{
	setbuf(stdout, NULL);
	// Initialize rooms list
	findRoomPath();

	// As good a place as any to spin thread
	pthread_t t_thread;
	pthread_mutex_init(&time_thread, NULL);

	//After room files are found...
	Room** room_list;
	room_list = readRoomsFromFiles(7);

	RoomType type;
	int current_room = -1;
	int room_index;
	int sub_index;
	int num_steps = 0;
	char path_taken[50][256];
	// Find the starting room from the loaded rooms
	for (room_index = 0; room_index < 7; room_index++)
	{
		type = room_list[room_index]->type;

		switch (type)
		{
		case START_ROOM:
			current_room = room_index;
			break;
		default:
			break;
		}
		if (current_room >= 0)
			break;
	}
	// Found the starting room, current_room
	int game_state = 1;
	while (game_state)
	{
		// Start loop
		printf("\nCURRENT LOCATION: %s\n", room_list[current_room]->name);
		printf("POSSIBLE CONNECTIONS: ");
		for (room_index = 0; room_index < room_list[current_room]->num_connections; room_index++)
		{
			if (room_index != room_list[current_room]->num_connections - 1)
				printf("%s, ", room_list[current_room]->connection_names[room_index]);
			else
				printf("%s.\n", room_list[current_room]->connection_names[room_index]);
		}
		printf("WHERE TO? ");
		char current_player_selection[256];
		fgets(current_player_selection, 256, stdin);
		current_player_selection[strcspn(current_player_selection, "\n")] = 0;
		// Look through connections
		int selected_index = roomListNameMap(room_list, current_player_selection);
		if (strcmp(current_player_selection, "time") == 0)
		{
			//create second thread that gets and writes the time 
			pthread_create(&t_thread, NULL, writeTime, NULL);
			// join merdges the two threads together 
			pthread_join(&t_thread, NULL);
			//reads the time 
			//fflush(stdout);
			readTime();
			
			continue;
		}
		// Check if all other arguements have failed
		else if (selected_index < 0)
		{
			printf("\nHUH ? I DON'T UNDERSTAND THAT ROOM.TRY AGAIN.\n");
			continue;
		}
		//Keeping track of the users steps
		else
		{
			strcpy(path_taken[num_steps], current_player_selection);
			num_steps++;
			current_room = selected_index;
		}

		// Once the end room has been found 
		type = room_list[current_room]->type;
		if (type == END_ROOM)
		{
			game_state = 0;
			printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
			printf("\nYOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", num_steps);
			int step;
			for (step = 0; step < num_steps; step++)
			{
				printf("%s\n", path_taken[step]);
			}
		}

	}
	
	// destroy the lock 
	pthread_mutex_destroy(&time_thread);
	return 0;
}
