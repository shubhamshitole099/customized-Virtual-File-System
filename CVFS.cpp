// Custmised Dynamic File System.

//----------------------------------------------------


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
// #include<io.h>
#include <iostream>
using namespace std;


#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024

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
	char FileName[MAXINODE];
	int InodeNumber;
	int FileSize;
	int FileActualSize;
	int FileType;
	char *Buffer;
	int LinkCount;
	int RefernceCount;;
	int permission; // 1 2 3
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

UFDT UFDTArr[MAXINODE];
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
		cout<<"\nDescription : Used to  create new regualar file"<<"\n";
		cout<<"\nUsage : create File_name,Permission"<<"\n";
	}
	else if(strcmp(name,"read") == 0)
	{
		cout<<"\nDescription : Used to read data from regular file"<<"\n";
		cout<<"\nUsage : read File_name,No_OF_Bytes_TO_Read"<<"\n";
	}
	else if(strcmp(name,"write") == 0)
	{
		cout<<"\nDescription : Used tp write into regular file"<<"\n";
		cout<<"\nUsage : write File_name"<<"\n";
		cout<<"\nAfter this enter the data that we want to write"<<"\n";

	}
	else if(strcmp(name,"ls") == 0)
	{
		cout<<"\nDescription : Used to list all information of file"<<"\n";
		cout<<"\nUsage : ls"<<"\n";
	}
	else if(strcmp(name,"stat") == 0)
	{
		cout<<"\nDescription : Used to display information of file"<<"\n";
		cout<<"\nUsage : stat File_name"<<"\n";
	}
	else if(strcmp(name,"fstat") == 0)
	{
		cout<<"\nDescription : Used to display information of file"<<"\n";
		cout<<"\nUsage : stat File_Descriptor"<<"\n";
	}
	else if(strcmp(name,"truncate") == 0)
	{
		cout<<"\nDescription : Used to remove data from file"<<"\n";
		cout<<"\nUsage : truncate File_name"<<"\n";
	}
	else if(strcmp(name,"open") == 0)
	{
		cout<<"\nDescription : Used to open existing file"<<"\n";
		cout<<"\nUsage : open File_name mode"<<"\n";
	}
	else if(strcmp(name,"close") == 0)
	{
		cout<<"\nDescription : Used to close opened file"<<"\n";
		cout<<"\nUsage : close File_name"<<"\n";
	}
	else if(strcmp(name,"closeall") == 0)
	{
		cout<<"\nDescription : Used to close all opened file"<<"\n";
		cout<<"\nUsage : closeall"<<"\n";
	}
	else if(strcmp(name,"lseek") == 0)
	{
		cout<<"\nDescription : Used to change file offset"<<"\n";
		cout<<"\nUsage : lseek File_name ChangeInOffset StartPoint"<<"\n";
	}
	else if(strcmp(name,"rm") == 0)
	{
		cout<<"\nDescription : Used to delete the file"<<"\n";
		cout<<"\nUsage : rm File_name"<<"\n";
	}
	else
	{
		cout<<"ERROR : No manual entry available."<<"\n";
	}

}

void DisplayHelp()
{
	cout<<"\nls : TO List out all files"<<"\n";
	cout<<"\nclear : TO clear console"<<"\n";
	cout<<"\nopen : TO open the file"<<"\n";
	cout<<"\nclose : TO close the file"<<"\n";
	cout<<"\ncloseall : TO close all opened files"<<"\n";
	cout<<"\nread : TO Read the contents from file"<<"\n";
	cout<<"\nwrite : TO write contents into file"<<"\n";
	cout<<"\nexit : TO Terminate file system"<<"\n";
	cout<<"\nstat : TO display information of file using name"<<"\n";
	cout<<"\nfstat : TO display information of file using file descriptor"<<"\n";
	cout<<"\ntruncate : TO remove all data from file"<<"\n";
	cout<<"\nrm : To delete the file"<<"\n";
}

int GetFDFromName(char *name)
{
	int i = 0;

	while(i < MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
			{
				break;
			}

			i++;
		}
	}

		if(i == MAXINODE)
		{
			return -1;

		}
		else
		{
			return i;
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
		}
		temp = temp -> next; 
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
	cout<<"\nDILB created successfully"<<"\n";
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
		(SUPERBLOCKobj.FreeInode)++;
		return -3;
	}

	while(temp != NULL)
	{
		if(temp->FileType == 0)
		{
			break;
		}
		temp = temp -> next;
	}

	while(i < MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
		{
			break;
			
		}

		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = permission;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;

	UFDTArr[i].ptrfiletable->ptrinode = temp;

	strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);

	UFDTArr[i].ptrfiletable->ptrinode->FileType = REGUALAR;
	UFDTArr[i].ptrfiletable->ptrinode->RefernceCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
	UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
	UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char*)malloc(MAXFILESIZE);

	return i;

}


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

  return 0;
}

int ReadFile(int fd,char *arr,int isize)
{
	int read_size = 0;

	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}
	
	if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
	{
		return -2;
	}
	
	if(UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ + WRITE)
	{
		return -2;
	}

	if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
	{
		return -3;
	}

	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGUALAR)
	{
		return -4;

		
	}

	read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

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

  if((UFDTArr[fd].ptrfiletable->ptrinode->FileType)!= REGUALAR)
  {
  	return -3;
    
  }


    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);

    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset)+isize;

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
			
		}
		i++;
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

void ClosedFileByName(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	(UFDTArr[fd].ptrfiletable->ptrinode->RefernceCount)--;
}

int CloseFileByName(char *name)
{

	int i = 0;

	i = GetFDFromName(name);

	if(i == -


		1)
	{
		return -1;

	}

	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;

	(UFDTArr[i].ptrfiletable->ptrinode->RefernceCount)--;

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
      (UFDTArr[i].ptrfiletable->ptrinode->RefernceCount)--;
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
		if(size < 0)
		{
			return -1;
		}

		(UFDTArr[fd].ptrfiletable->readoffset) == size;
	}
	else if(from == END)
	{
		if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
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
           (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)= (UFDTArr[fd].ptrfiletable->writeoffset)+ size;

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

  return 0;
}
		


void ls_file()
{

  int i = 0;
  PINODE temp = head;

  if(SUPERBLOCKobj.FreeInode == MAXINODE)
  {
  	cout<<"\nERROR : there are no files"<<"\n";
  	return;
  }

  cout<<"\n----------------------------------------------------\n";
    
     while(temp != NULL)
     {
     	if(temp -> FileType != 0)
     	{
     		cout<<temp->FileName<<"\t"<<temp->InodeNumber<<"\t"<<temp->FileActualSize<<"\t"<<temp->LinkCount<<"\n";

     	}

     	temp = temp -> next;
     }

  cout<<"\n----------------------------------------------------\n";
    
}


int fstat_file(int fd)
{
	PINODE temp = head;
	int i = 0;

	if(fd < 0)
	{
		return -1;
	}

	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -2;
	}

	temp = UFDTArr[fd].ptrfiletable->ptrinode;

	cout<<"\n-------------------Statical Information About File ------------------------------\n";

       cout<<"\nFile name : "<<temp->FileName;
       cout<<"\nInode Number :"<<temp->InodeNumber;
       cout<<"\nFile size : "<<temp->FileSize;
       cout<<"\nActual file Size : "<<temp->FileActualSize;
       cout<<"\nLink Count : "<<temp->LinkCount;
       cout<<"\nReference Count : "<<temp->RefernceCount;


       if(temp -> permission == 1)
       {
       	cout<<"\nFile Permission : Read only"<<"\n";

       }
       else if(temp->permission == 2)
       {
       	cout<<"\nFile Permission : Write"<<"\n";
       }
       else if(temp -> permission == 3)
       {
         cout<<"\nFile Permission : Read & Write"<<"\n";
       }

	cout<<"\n---------------------------------------------------------------------------------\n";
     
     return 0;
 
}


int stat_file(char *name)
{
	PINODE temp = head;
	int i = 0;

	if(name == NULL)
	{
		return -1;
	}

	  while(temp != NULL)
	  {
	  	if(strcmp(name,temp->FileName)== 0)
	  	{
	  		break;
	  	}
	  	temp = temp->next;
	  }

	  if(temp == NULL)
	  {
	  	return -2;
	  }


	cout<<"\n-------------------Statical Information About File ------------------------------\n";

       cout<<"\nFile name : "<<temp->FileName;
       cout<<"\nInode Number : "<<temp->InodeNumber;
       cout<<"\nFile size : "<<temp->FileSize;
       cout<<"\nActual file Size : "<<temp->FileActualSize;
       cout<<"\nLink Count : "<<temp->LinkCount;
       cout<<"\nReference Count : "<<temp->RefernceCount;


       if(temp -> permission == 1)
       {
       	cout<<"\nFile Permission : Read only\n";

       }
       else if(temp->permission == 2)
       {
       	cout<<"\nFile Permission : Write\n";
       }
       else if(temp -> permission == 3)
       {
         cout<<"\nFile Permission : Read & Write\n";
       }

	cout<<"\n---------------------------------------------------------------------------------\n";
     
     return 0;
 

}

int truncate_File(char *name)
{
	int fd = GetFDFromName(name);

	if(fd == -1)
	{
		return -1;
	}


	memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize =0;
	return 0;
}



int main()
{
	char *ptr = NULL;
	int ret = 0,fd = 0,count = 0;

	char command[4][80],str[80],arr[1024];

	InitialiseSuperBlock();
	CreatDILB();

	while(1)
	{
		fflush(stdin);
		strcpy(str,"");

		cout<<"\nMarvellous VFS : > ";

		fgets(str,80,stdin);  //scanf("%[^'\n']s",str);

		count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);

	 if(count == 1)
	 {

     if(strcmp(command[0],"ls") == 0)
     {
     	ls_file();
     }
     else if(strcmp(command[0],"closeall") == 0)
     {
     	CloseAllFile();
     	cout<<"\nAll File closed Successfully"<<"\n";
     	continue;
     }
     else if(strcmp(command[0],"clear") == 0)
     {
     	system("clear");
     	continue;
     }
     else if(strcmp(command[0],"help") == 0)
     {
     	DisplayHelp();
     	continue;
     }
     else if(strcmp(command[0],"exit") == 0)
     {
     	cout<<"\nTerminating the marvellous virtual file system"<<"\n";
     	break;
     }
     else
     {
     	cout<<"\nERROR : Command Not Fount !!!"<<"\n";
     	continue;
     }

	 }

	 else if(count == 2)
	 {
      if(strcmp(command[0],"stat") == 0)
      {
      	ret = stat_file(command[1]);

      	if(ret == -1)
      	{
      		cout<<"\nERROR : Incorrect Parameter"<<"\n";
      	}

      	if(ret == -2)
      	{
      		cout<<"\nERROR : There is no such file\n";
      		continue;
      	}
         

      }

      else if(strcmp(command[0],"fstat") == 0)
      {
         ret = fstat_file(atoi(command[1]));

         if(ret == -1)
         {
         	cout<<"\nERROR : Incorrect Parameter"<<"\n";
         }

         if(ret == -2)
         {
         	cout<<"\nThere is no such file"<<"\n";
         	continue;
         }

      }

      else if(strcmp(command[0],"close") == 0)
      {
      	ret = CloseFileByName(command[1]);

      	if(ret == -1)
      	{
      		cout<<"\nThere is no such file"<<"\n";
      		continue;
      	}

      }

      else if(strcmp(command[0],"rm") == 0)
      {
      	 ret = rm_File(command[1]);

      	 if(ret == -1)
      	 {
      	 	cout<<"\nThere is no such file"<<"\n";
      	 	continue;
      	 }

      }

      else if(strcmp(command[0],"man") == 0)
      {
      	man(command[1]);
      }

      else if(strcmp(command[0],"write") == 0)
      {
      	fd = GetFDFromName(command[1]);

      	if(fd == -1)
      	{
      		cout<<"\nERROR : Incorrect Parameter"<<"\n";
      		continue;
      	}

      	cout<<"\nEnter The Data :\n";
      	scanf("%[^'\n']s",arr);
      	

         ret = strlen(arr);

         if(ret == 0)
         {
         	cout<<"\nERROR : Incorrect Parameter"<<"\n";
         	continue;
         }

         ret = WriteFile(fd,arr,ret);

         if(ret == -1)
         {
         	cout<<"\nERROR : Permission Denied"<<"\n";
         }

         if(ret == -2)
         {
         	cout<<"\nERROR : There is no sufficient memory to write"<<"\n";
         }

         if(ret == -3)
         {
         	cout<<"\nERROR : It is not regular file"<<"\n";
         }
      
      }

      else if(strcmp(command[0],"truncate") == 0)
      {


         ret = truncate_File(command[1]);

         if(ret == -1)
         {
         	cout<<"\nERROR : Incorrect Parameter"<<"\n";
         }

      }

      else
      {
      	cout<<"\nERROR : Command Not Found !!!"<<"\n";
      }


	 }

	 else if(count == 3)
	 {

      if(strcmp(command[0],"create") == 0)
      {

        ret = CreateFile(command[1],atoi(command[2]));

        if(ret >= 0)
        {
        	cout<<"\nFile Successfully Created with File Descriptor : "<<ret<<"\n";
        }

        if(ret == -1)
        {
        	cout<<"\nERROR : Incorrect Parameter"<<"\n";
        }

        if(ret == -2)
        {
        	cout<<"\nERROR : There is no inodes"<<"\n";
        }

        if(ret == -3)
        {
        	cout<<"\nERROR : File already exits"<<"\n";
        }

        if(ret == -4)
        {
        	cout<<"\nERROR : Memory allocation failure"<<"\n";
        	continue;
        }

      }


      else if(strcmp(command[0],"open") == 0)
      {
        
        ret = OpenFile(command[1],atoi(command[2]));

        if(ret >= 0)
        {
        	cout<<"\nFile is successfully opened with file descriptor : "<<ret<<"\n";
        }

        if(ret == -1)
        {
        	cout<<"\nERROR : Incorrect Parameter"<<"\n";
        }

        if(ret == -2)
        {
        	cout<<"\nERROR : File not present"<<"\n";
        }

        if(ret == -3)
        {
        	cout<<"\nERROR : Permission Denied"<<"\n";
        	continue;
        }

      }

      else if(strcmp(command[0],"read") == 0)
      {

      	fd = GetFDFromName(command[1]);

      	if(fd == -1)
      	{
      		cout<<"\nERROR : Incorrect Parameter"<<"\n";
      		continue;
      	}

      	 ptr = (char *)malloc(sizeof(atoi(command[2]))+1);

      	 if(ptr == NULL)
      	 {
      	 	cout<<"\nERROR : Memory allocation failure"<<"\n";
      	 	continue;
      	 }
       

         ret = ReadFile(fd,ptr,atoi(command[2]));

         if(ret == -1)
         {
         	cout<<"\nERROR : File not exits"<<"\n";
         }
    
         if(ret == -2)
         {
         	cout<<"\nERROR : Permission Denied"<<"\n";
         }

         if(ret == -3)
         {
         	cout<<"\nERROR : Reached at end of file"<<"\n";
         }

         if(ret == -4)
         {
         	cout<<"\nERROR : It is not regular file"<<"\n";
         }

         if(ret == 0)
         {
         	cout<<"\nERROR : File empty"<<"\n";
         }

         if(ret > 0)
         {
         	write(2,ptr,ret);
         }
         continue;

      }

      else
      {
      	cout<<"\nERROR : Command Not Found !!!"<<"\n";
      	continue;
      }


	 }

	 else if(count == 4)
	 {

      if(strcmp(command[0],"lseek") == 0)
      {

         fd = GetFDFromName(command[1]);

         if(fd == -1)
         {
         	cout<<"\nERROR : Incorrect Parameter"<<"\n";
         	continue;
         }

         ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));

         if(ret == -1)
         {
         	cout<<"\nERROR : Unable to perform lseek"<<"\n";
         }

      }

      else
      {
      	cout<<"\nERROR : Command Not Found !!!"<<"\n";
      	continue;
      }

	 }
	 
	 else
	 {
     cout<<"\nERROR : Command Not Found !!!"<<"\n";
     continue;

	 }

	}

  cout<<"\n";
	return 0;

}




