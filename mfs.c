#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <stdint.h>
#include<time.h>
#include <stdio.h>
#include <sys/stat.h>
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

#define NUM_BLOCKS 4226
#define BLOCK_SIZE 8192
#define NUM_FILES 128
#define NUM_INODES 128
#define MAX_BLOCKS_PER_FILE 1250

uint8_t data_blocks[NUM_BLOCKS][BLOCK_SIZE];
uint8_t *used_blocks;

struct directory_entry
{
	char *name;
	uint8_t valid;
	uint32_t inode_idx;
};	

struct inode
{
	time_t date;
	uint8_t valid;
	uint32_t blocks[MAX_BLOCKS_PER_FILE];
	int size;
};	

struct directory_entry *directory_ptr;
struct inode *inode_array_ptr;
int *inode_map;

//Initialize directory, inodes, data_blocks and used_blocks

/*
init data_blocks()
{
	int i;
	for(i = 0; i <
}
*/
void init_directory()
{
	int i;
	for(i = 0; i < NUM_FILES; i++)
	{
		//memset(directory_ptr[i].name, 0, 255);
		directory_ptr[i].name = NULL;
		directory_ptr[i].inode_idx = -1;
		directory_ptr[i].valid = 0;
	}		
}

void init_inodes_array()
{
	int i;
	for(i = 0; i < NUM_FILES; i++)
	{
		inode_array_ptr[i].date = 0;
		inode_array_ptr[i].valid = 0;
		inode_array_ptr[i].size = 0;
		
		int j;
		for(j = 0; j < MAX_BLOCKS_PER_FILE; j++)
		{
			inode_array_ptr[i].blocks[j] = -1;
		}	
	}
}

void init_used_blocks()
{
	int i;
	for(i = 0; i < NUM_BLOCKS; i++)
	{
		used_blocks[i] = 0;
	}	
}

void init_inode_map()
{
	int i;
	for(i = 0; i < NUM_FILES; i++)
	{
		inode_map[i] = 0;
	}	
}	

int findFreeDirectoryEntry()
{
	int i;
	int retval = -1;
	for (i = 0; i < NUM_FILES; i++)
	{
		if (directory_ptr[i].valid == 0)
		{
			//retval = i;
			return i;
		}
	}
	return retval;	
}	

int findFreeInode()
{
	int i;
	int retval = -1;
	for(i = 0; i < NUM_FILES; i++)
	{
		if(inode_array_ptr[i].valid == 0)
		{
			//retval = i;
			return i;
		}	
	}
	return retval;
}	

int findFreeBlock()
{
	int retval = -1;
	int i = 0;
	
	for(i = 131; i < NUM_BLOCKS; i++)
	{
		if(used_blocks[i] == 0)
		{
			return i;
			//return retval;
		}
	}
	return retval;
}
	
int df()
{
	int count = 0;
	int i = 0;
	for(i = 131; i < NUM_BLOCKS; i++)
	{
		if(used_blocks[i] == 0)
		{
			count++;
		}	
	}
	int free = count * BLOCK_SIZE;
	return free;
}

int findFreeInodeBlockEntry(int inode_index)
{
	int i;
	int retval = -1;
	for(i = 0; i < MAX_BLOCKS_PER_FILE; i++)
	{
		if(inode_array_ptr[inode_index].blocks[i] == -1)
		{
			retval = i;
			return retval;
		}
	}
	return retval;
}

int nameSearchDir(char *filename)
{
    int i;
    int retval = -1;
    for (i = 0; i < NUM_FILES; i++) 
	{
        if (strcmp(filename, directory_ptr[i].name) == 0) 
		{
            retval = i;
        }
    }
    return retval;	
}	

int put(char *filename)
{
	struct stat buf;
	int status;
	status = stat(filename, &buf);
	if(status == -1)
	{
		printf("Error: File not found.\n");
		return -1;
	}
	
	if(buf.st_size > df())
	{
		printf("Error: Not enough room in the file system \n");
		return -1;
	}
	
	int dir_idx = findFreeDirectoryEntry();
	printf("dir idx: %d\n", dir_idx);
	
	if(dir_idx == -1)
	{
		printf("Error: Not enough room in the file system \n");
		return -1;
	}

	directory_ptr[dir_idx].valid = 1;
	directory_ptr[dir_idx].name = (char *)malloc(strlen(filename));
	strncpy(directory_ptr[dir_idx].name, filename, strlen(filename));
	printf("Filename put: %s\n", directory_ptr[dir_idx].name);
	
	int inode_idx = findFreeInode();
	printf("inode idx: %d\n", inode_idx);
	if(inode_idx == -1)
	{
		printf("Error: No free inodes\n");
		return -1;
	}	
	
	directory_ptr[dir_idx].inode_idx = inode_idx;
	
	inode_array_ptr[inode_idx].size = buf.st_size;
	inode_array_ptr[inode_idx].date = time(NULL);
	inode_array_ptr[inode_idx].valid = 1;
	
	FILE *ifp = fopen(filename, "r");
	printf("Reading %d bytes from %s\n", (int) buf . st_size, filename );
	
	int copy_size = buf.st_size;
	printf("copysize: %d\n", copy_size);
	int offset = 0;
	int block_pos = 0; //to keep track of blocks in inode
	int block_index;

	
	while(copy_size > 0) //>= BLOCK_SIZE)
	{
		block_index = findFreeBlock();
		
		if(block_index == -1)
		{
			printf("Error: Can't find free block\n");
			//Cleanup a bunch of directory and inode stuff
			return -1;
		}
		
		used_blocks[block_index] = 1;
		//int inode_block_entry = findFreeInodeBlockEntry(inode_idx);
		//if(inode_block_entry == -1)
		//{
		//	printf("Error: Can't find free block\n");
		//	//Cleanup a bunch of directory and inode stuff
			//return;
		//}	
		//inode_array_ptr[inode_idx].blocks[inode_block_entry] = block_index;
		
		fseek(ifp, offset, SEEK_SET);
		int bytes = fread(data_blocks[block_index], BLOCK_SIZE, 1, ifp);
		if( bytes == 0 && !feof( ifp ) )
		{
			printf("An error occured reading from the input file.\n");
			return -1;
		}
		clearerr(ifp);
		
		copy_size -= BLOCK_SIZE;
		offset += BLOCK_SIZE;
		
		inode_array_ptr[inode_idx].blocks[block_pos] = block_index;
        block_pos++;
	}
	/*
	if(copy_size > 0)
	{
		//handle remainder
		int block_index = findFreeBlock();
		
		if(block_index == -1)
		{
			printf("Error: Can't find free block\n");
			//Cleanup a bunch of directory and inode stuff
			return;
		}
		used_blocks[block_index] = 1;
		int inode_block_entry = findFreeInodeBlockEntry(inode_idx);
		if(inode_block_entry == -1)
		{
			printf("Error: Can't find free block\n");
			//Cleanup a bunch of directory and inode stuff
			return;
		}	
		inode_array_ptr[inode_idx].blocks[inode_block_entry] = block_index;		
		fseek(ifp, offset, SEEK_SET);
		int bytes = fread(data_blocks[block_index], copy_size, 1, ifp);
		
		if( bytes == 0 && !feof( ifp ) )
		{
			printf("An error occured reading from the input file.\n");
			return;
		}
		clearerr(ifp);
		
		copy_size -= BLOCK_SIZE;
		offset += BLOCK_SIZE;
		
		inode_array_ptr[inode_idx].blocks[block_pos] = block_index;
        block_pos++;		
	}
	*/
	fclose(ifp);
	return 0;
}

/*
void get(char *filename, char *newfilename)
{
    FILE *ofp;
    ofp = fopen(argv[2], "w");

    if( ofp == NULL )
    {
      printf("Could not open output file: %s\n", argv[2] );
      perror("Opening output file returned");
      return -1;
    }

	struct stat buf;
	int status = stat(filename, &buf);
	if(status == -1)
	{
		printf("Error: File not found.\n");
		return;
	}

    // Initialize our offsets and pointers just we did above when reading from the file.
    int block_index = nameSearchDir(filename);
    int copy_size   = buf.st_size;
    int offset      = 0;
	int block_pos = 0;

    //printf("Writing %d bytes to %s\n", (int) buf . st_size, argv[2] );


    while( copy_size > 0 )
    { 

      int num_bytes;

      if( copy_size < BLOCK_SIZE )
      {
        num_bytes = copy_size;
      }
      else 
      {
        num_bytes = BLOCK_SIZE;
      }

      fwrite(data_blocks[block_index], num_bytes, 1, ofp ); 

      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;

		inode_array_ptr[inode_idx].blocks[block_pos] = block_index;
		block_pos++;	      

      fseek( ofp, offset, SEEK_SET );
    }

    // Close the output file, we're done. 
    fclose( ofp );
}
*/
void del(char *filename)
{
	int dir_idx = nameSearchDir(filename);
	int inode_idx = directory_ptr[dir_idx].inode_idx;
	
	int i;
	for(i = 0; i < sizeof(inode_array_ptr[inode_idx].blocks); i++)
	{
		used_blocks[i] = 0;
		//set block index to -1??
	}
	
	inode_array_ptr[inode_idx].valid = 0;
	inode_array_ptr[inode_idx].size = 0;
	
	directory_ptr[dir_idx].valid = 0;
	directory_ptr[dir_idx].inode_idx = -1;
	directory_ptr[dir_idx].name = NULL;
}	

void list()
{
	int i;
	int count = 0;
	for(i = 0; i < NUM_FILES; i++)
	{
		if(directory_ptr[i].valid == 1)
		{
			int inode_index = directory_ptr[i].inode_idx;
            printf("%d %s %s\n", inode_array_ptr[inode_index].size, ctime(&(inode_array_ptr[inode_index].date)), directory_ptr[i].name);
			count++;
		}	
	}
	
	if(count == 0)
	{
		printf("\nNo files found.\n");
		return;
	}
printf("Count: %d\n",count);	
}	

int main()
{
	directory_ptr = (struct directory_entry*)&data_blocks[0];
	init_directory();
	
	inode_map = (int*)&data_blocks[1];
	init_inode_map();
	
	used_blocks = (uint8_t*)&data_blocks[2];
	init_used_blocks();
	
	inode_array_ptr = (struct inode*)&data_blocks[3];
	init_inodes_array();
	


  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         

    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

	if(token[0] == NULL)
	{
		printf("Cannot process command, please try again\n");
	}
	else if(strcmp(token[0], "put") == 0)
	{
		put(token[1]);
	}
	else if(strcmp(token[0], "list") == 0)
	{
		list();
	}	
	else if(strcmp(token[0], "quit") == 0)
	{
		exit(0);
	}
    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality
	/*
    int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      printf("token[%d] = %s", token_index, token[token_index] );  
    }
	*/
	printf("\n Disk free: %d bytes\n", df());
    free( working_root );

  }
  return 0;
}


