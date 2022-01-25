// Custmised Dynamic File System.

//----------------------------------------------------


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<io.h>


#define MAXINODE 50

#define READ 1
#define WRITE 2

#define REGUALAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

typedef struct  superblock
{
	int TotalInodes;
	int FreeInode;
	
}SUPERBLOCK,*PSUPERBLOCK;


typedef struct inode
{
	char FileName[50];
	int InodeNumber;
	int FileSize;
	int FileActualSize;
	int FileType;
	char *Buffer;
	int LinkCount;
	int RefernceCount;;
	int permission; // 1 23
	struct inode *next;

}INODE,*PINODE,**PPINODE;


typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;
	int mode; //  1 2 3

	PINODE ptrinode;

}FILETABLE,*PFILETABLE;


typedef struct ufdt
{
   PFILETABLE ptrfiletable;

}UFDT;

UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;

PINODE head = NULL;


void man(char *name)
{
	if(name == NULL)
	{
		return;
	}

	if(strcmp(name,"create") == 0)
	{
		printf("\nDescription : Used to  create new regualar file\n");
		printf("\nUsage : create File_name,No_Of_Bytes_To_Read\n");
	}
	else if(strcmp(name,"write") == 0)
	{
		printf("\nDescription : Used tp write into regular file\n");
		printf("\nUsage : write File_name\nAfter this enter the data that we want to write\n");

	}
	else if(strcmp(name,"ls") == 0)
	{
		printf("\nDescription : Used to list all information of file\n");
		printf("\nUsage : ls\n");
	}
	else if(strcmp(name,"stat") == 0)
	{
		printf("\nDescription : Used to display information of file\n");
		printf("\nUsage : stat File_name\n");
	}
	else if(strcmp(name,"fstat") == 0)
	{
		printf("\nDescription : Used to display information of file\n");
		printf("\nUsage : stat File_Descriptor\n");
	}
	else if(strcmp(name,"truncate") == 0)
	{
		printf("\nDescription : Used to remove data from file\n");
		printf("\nUsage : truncate File_name\n");
	}
	else if(strcmp(name,"open") == 0)
	{
		printf("\nDescription : Used to open existing file\n");
		printf("\nUsage : open File_name mode\n");
	}
	else if(strcmp(name,"close") == 0)
	{
		printf("\nDescription : Used to close opened file\n");
		printf("\nUsage : close File_name\n");
	}
	else if(strcmp(name,"closeall") == 0)
	{
		printf("\nDescription : Used to close all opened file\n");
		printf("\nUsage : closeall\n");
	}
	else if(strcmp(name,"lseek") == 0)
	{
		printf("\nDescription : Used to change file offset\n");
		printf("\nUsage : lseek File_name ChangeInOffset StartPoint\n");
	}
	else if(strcmp(name,"rm") == 0)
	{
		printf("\nDescription : Used to delete the file\n");
		printf("\nUsage : rm File_name\n");
	}
	else
	{
		printf("ERROR : No manual entry available.\n");
	}

}

void DisplayHelp()
{
	printf("\nls : TO List out all files\n");
	printf("\nclear : TO clear console\n");
	printf("\nopen : TO open the file\n");
	printf("\nclose : TO close the file\n");
	printf("\ncloseall : TO close all opened files\n");
	printf("\nread : TO Read the contents from file\n");
	printf("\nwrite : TO write contents into file\n");
	printf("\nexit : TO Terminate file system\n");
	printf("\nstat : TO display information of file using name\n");
	printf("\nfstat : TO display information of file using file descriptor\n");
	printf("\ntruncate : TO remove all data from file\n");
	printf("\nrm : To delete the file\n");
}

int GetFDFromName(char *name)
{
	int i = 0;

	while(i < 50)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
			{
				break;
				i++;
			}
			
		}

		if(i == 50)
		{
			return -1;

		}
		else
		{
			return i;
		}
	}
}

PINODE Get_Inode(char *name)
{
	PINODE temp = head;
	int i =0;

	if(name == NULL)
	{
		return NULL;
	}

	while(temp != NULL)
	{
		if(strcmp(name,temp->FileName)==0)
		{
			break;
			temp = temp -> next;
		}
	}
	return temp;
}

void CreatDILB()
{
	int i = 1;
	PINODE newn = NULL;
	PINODE temp = head;

	while(i <= MAXINODE)
	{
		newn =(PINODE)malloc(sizeof(INODE));

		newn->LinkCount = 0;
		newn->RefernceCount = 0;
		newn->FileType = 0;
		newn->FileSize = 0;

		newn->Buffer = NULL;
		newn->next = NULL;

		newn->InodeNumber = i;

		if(temp == NULL)
		{
			head = newn;
			temp = head;
		}
		else
		{
			temp->next = newn;
			temp = temp->next;
		}

		i++;
	}
	printf("\nDILB created successfully\n");
}


void InitialiseSuperBlock()
{
	int i = 0;

	while(i < MAXINODE)
	{
		UFDTArr[i].ptrfiletable = NULL;
		i++;
	}

	SUPERBLOCKobj.TotalInodes = MAXINODE;
	SUPERBLOCKobj.FreeInode = MAXINODE;
}

int CreateFile(char *name,int permission)
{
	int i = 0;

	PINODE temp = head;

	if((name == NULL) || (permission == 0) ||(permission > 3))
	{
		return -1;
	}

	if(SUPERBLOCKobj.FreeInode == 0)
	{
		return -2;
		
	}

	(SUPERBLOCKobj.FreeInode)--;

	if(Get_Inode(name) != NULL)
	{
		return -3;
	}

	while(temp != NULL)
	{
		if(temp->FileName == 0)
		{
			break;
			temp = temp -> next;
		}
	}

	while(i < 50)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
		{
			break;
			i++;
		}
	}

	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = permission;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;

	UFDTArr[i].ptrfiletable->ptrinode = temp;

	strncpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);

	UFDTArr[i].ptrfiletable->FileType = REGUALAR;
	UFDTArr[i].ptrfiletable->RefernceCount = 1;
	UFDTArr[i].ptrfiletable->LinkCount = 1;
	UFDTArr[i].ptrfiletable->FileSize = MAXINODE;
	UFDTArr[i].ptrfiletable->FileActualSize = 0;
	UFDTArr[i].ptrfiletable->permission = permission;
	UFDTArr[i].ptrfiletable->Buffer = (char*)malloc(MAXFILESIZE);

	return i;

}


//rm_File("Demo.txt")

int rm_File(char * name)
{
	int fd = 0;

	fd = GetFDFromName(name);

	if(fd == -1)
	{
		return -1;
		
	}

	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

	if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
	{
		UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;

		// free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);

		free(UFDTArr[fd].ptrfiletable);
	}

	UFDTArr[fd].ptrfiletable = NULL;
	(SUPERBLOCKobj.FreeInode)++;
}

int ReadFile(int fd,char *arr,int isize)
{
	int read_size = 0;

	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}
	
	if(UFDTArr[fd].ptrfiletable->ptrinode->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
	{
		return -2;
	}
	
	if(UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->permission != READ + WRITE)
	{
		return -2;
	}

	if(UFDTArr[fd].ptrfiletable->ptrinode->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
	{
		return -3;
	}

	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGUALAR)
	{
		return -4;

		
	}

	read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) -(UFDTArr[fd].ptrfiletable->readoffset);

	if(read_size < isize)
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),read_size);

		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;

	}
	else
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),isize);

		(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + isize;
	}

	return isize;
}   


int WriteFile(int fd,char *arr,int isize)
{

  if(((UFDTArr[fd].ptrfiletable->mode)!= WRITE) && ((UFDTArr[fd].ptrfiletable->mode)!= READ + WRITE))
  {
  	return -1;
  }

  if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) != WRITE) &&((UFDTArr[fd].ptrfiletable->ptrinode->permission)!= READ + WRITE))
  {
  	return -1;
  }

  if((UFDTArr[fd].ptrfiletable->writeoffset)== MAXFILESIZE)
  {
  	return -2;
  }

  if((UFDTArr[fd].ptrfiletable->FileType)!= REGUALAR)
  {
  	return -3;
    
  }


    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);

    (UFDTArr[fd].ptrfiletable->ptrinode->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset)+isize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ isize;


  return isize;

}


int OpenFile(char *name,int mode)
{
	int i = 0;
	PINODE temp = NULL;

	if(name == NULL || mode <= 0)
	{
		return -1;
		
	}

	temp = Get_Inode(name);

	if(temp == NULL)
	{
		return -2;
	}

	if(temp -> permission < mode)
	{
		return -3;
	}

	while(i < 50)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
		{
			break;
			i++;
		}
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
	if(UFDTArr[i].ptrfiletable == NULL)
	{
		return -1;
		
	}

	    UFDTArr[i].ptrfiletable->count = 1;
		UFDTArr[i].ptrfiletable->mode = mode;

	if(mode == READ + WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	else if(mode == READ)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
	}
	else if(mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}

	UFDTArr[i].ptrfiletable->ptrinode = temp;
	(UFDTArr[i].ptrfiletable->ptrinode->RefernceCount)++;

	return i;
}

void CloseFileByName(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	(UFDTArr[fd].ptrfiletable->ptrinode->RefernceCount)--;
}

int CloseFileByName(char *name)
{

	int i = 0;

	i = GetFDFromName(name);

	if(i == 1)
	{
		return -1;

	}

	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;

	(UFDTArr[i].ptrfiletable->RefernceCount)--;

	return 0;
}

void CloseAllFile()
{
	int i = 0;

	while(i < 50)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			UFDTArr[i].ptrfiletable -> readoffset = 0;
			UFDTArr[i].ptrfiletable -> writeoffset = 0;
            (UFDTArr[i].ptrfiletable->RefernceCount)--;
            break;

		}
		
		i++;

	} 

}

int LseekFile(int fd,int size,int from)
{
	if((fd > 0) || (from  > 2))
	{
		return -1;
	}

	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}

	if((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ+WRITE))
	{
		if(from == CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable -> readoffset)+ size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
			{
				return -1;
			}

			if(((UFDTArr[fd].ptrfiletable->readoffset)+size)< 0)
			{
				return -1;
			}

			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset)+ size;

		}

			else if(from == START)
			{
				if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
				{
					return -1;
				}
				if(size > 0)
				{
					return -1;
				}

				(UFDTArr[fd].ptrfiletable->readoffset) == size;
			}
			else if(from == END)
			{
				if((UFDTArr[fd].ptrfiletable->FileActualSize) + size > MAXFILESIZE)
				{
					return -1;
				}

				if(((UFDTArr[fd].ptrfiletable->readoffset)+ size) < 0)
				{
					return -1;
				}
				(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size;
			}
		}
 
		else if(UFDTArr[fd].ptrfiletable -> mode == WRITE)
		{
			if(from == CURRENT)
			{
				if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
				{
					return -1;
				}

				if(((UFDTArr[fd].ptrfiletable->writeoffset)+ size) < 0)
				{
					return -1;
				}

				if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)>(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                 {
                 	(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)= (UFDTArr[fd].ptrfiletable->ptrinode->writeoffset)+ size;

                 	(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset)+size;

                 }
             }

                 else if(from == START)
                 {
                 	if(size > MAXFILESIZE)
                 	{
                 		return -1;
                 	}

                 	if(size < 0)
                 	{
                 		return -1;
                 	}

                 	if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                 	{
                 		(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
                 		(UFDTArr[fd].ptrfiletable -> writeoffset) = size;
                 	}
                 }

               	else if(from == END)
                 	{

                 		if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ size > MAXFILESIZE)
                 		{
                 			return -1;
                 		}

                 		if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)< 0)
                 		{
                 			return -1;
                 		}

                        (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+ size;	

                 	}
                 }

			}
		


void ls_file()
{

}


