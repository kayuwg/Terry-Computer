#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include <stdarg.h>
#include<string.h>
#include<time.h>
#include<ctype.h>
#include<windows.h>
#include<direct.h> 
#include<assert.h>
#include<math.h>

//#define assert 
#define NDEBUG
#undef DEBUGPRINT										//If defined, enable output debug message 

#ifdef DEBUGPRINT
int DebugPrintf(const char *format, ...)				//Output debug message
{
    va_list argPtr;
    int count;

    va_start(argPtr, format); 
    fflush(stdout); 
    count = vfprintf(stderr, format, argPtr);
    va_end(argPtr);
}

int DebugSystem(const char * command)
{
	return system(command);
}
#else
int DebugPrintf(const char *format, ...)
{
	//Blank function: Do not outupt any messages
}

int DebugSystem(const char * command)
{
	
}
#endif 

#define MAXLEN 27										//Max length of most strings
#define MAXTRANS 50										//Maximum number of items in a transaction
#define PRTYPE 4										//Number of type of items
#define NODEDT -1										//Number representation for Node type
#define MERCHDT 0										//Number representation for Merch type
#define PERSONDT 1										//Number representation for Person type
#define TRANSDT 2										//Number representation for Transaction type
#define USRPSWDT 3										//Number representation for Userpswd type
/*********************************
Reference Data

Functions:
A: Used in A part
B: Not used in A part(help functions)

Dtype:
0:Merch , 1:Person , 2:Transaction, 3:UserPasw
Categories in struct Merchandise(int)
0:CPU , 1:RAM , 2:Display Card , 3:Hard disk
Levels in struct Person(int)
0:Manager, 1:Staff, -1:Administrator
Kinds in struct Transaction(int)
0:Sold out, 1: Goods in

Mask configuration:
					64			32			16			8			4			2			1
					int			int			int			char[]		char[]		int			char[]
Merchmask 			remain		cost		price		brand		model		category	prkey_M	

					short		int			int			char[]		char[]		int			char[]	
Peoplemask		 	ismale		sales		salary		firstname	surname		level		prkey_P

								char[]		long long 	int			time_t		int			char[]
Transmask 						prkey_P		amount		kind		date		type		prkey_T
*********************************/

/* Global variables */
const int BLANK = -16384;									//For function get_number to represent no number
int series = 0;												//For assign_serial_node(). Distribute serial for each node
int fseries = 0;											//ALso. Distribute fserial for search results
unsigned short Merchmask = 0x0;								//Masks for searching
unsigned short Personmask = 0x0;
unsigned short Transmask = 0x0;
short Merchsort = 0;										//Masks for sorting
short Personsort = 0;
short Transsort = 0;
bool Searchisza = 0;										//Ascending/Descending
char search_content[10][MAXLEN];
const char merchType[4][MAXLEN] = {"CPU", "RAM", "Display card", "Hard disk"};
const char personType[2][MAXLEN] = {"Manager", "Staff"};
const char transType[2][MAXLEN] = {"Sold out", "Goods in"};			
const char merchtitle[7][MAXLEN] = {"Key", "Category", "Product Name", "Brand Name", "Price"
						,"Cost", "Remain"};					//Field names
const char persontitle[7][MAXLEN] = {"Key", "Level", "Family Name", "Given Name", "Salary"
						,"Sales", "Gender"};
const char transtitle[6][MAXLEN] = {"Key", "Action", "Date&Time", "Kind", "Amount", "Person"};
const int merchindent[7] = {2, 12, 27, 42, 57, 65, 72};		//Used for alignment(x coordinate)
const int personindent[7] = {2, 12, 27, 42, 57, 65, 72};
const int transindent[6] = {2, 12, 24, 42, 53, 65};

/* File name array */
char global[5][MAXLEN] =									//The array storing file names
{
	"Product.txt",											//0:Product			dtype = MERCHDT
	"Profile.txt",											//1:Profile			dtype = PERSONDT
	"Transaction.txt",										//2:Transaction		dtype = TRANSDT
	"SEARCHFILE.txt",										//3:SEARCHFILE
	"Login.txt"												//4:Login
};

/* Merchandise(MERCHDT), Personnel(PERSONDT), and Transaction(TRANSDT) structures */
typedef struct Merch
{
	char prkey_M[MAXLEN];									//Primary key
	char model[MAXLEN];										//Product name
	char brand[MAXLEN];	
	short category;											//0:CPU , 1:RAM , 2:Display Card , 3:Hard disk
	int price;
	int cost;
	int remain;												//remaining number of this product
} Merch;

typedef struct Person
{
	char prkey_P[MAXLEN];
	char surname[MAXLEN];
	char firstname[MAXLEN];
	short level;											//0:Manager, 1:Staff, -1:Administrator
	int salary;
	int sales;
	short ismale;
} Person;

typedef struct Transaction
{
	char prkey_T[MAXLEN];
	short type;
	char prkey_P[MAXLEN];									//The primary key for the person handling the transaction
	short different;										//Number of kinds of different product
	char prkey_M[MAXTRANS][MAXLEN];							//Store the primary keys for each item purchased
	int price_then[MAXTRANS];
	int quantity[MAXTRANS];
	time_t ttime;
} Transaction;

typedef struct Userpswd
{
	char prkey_P[MAXLEN];
	char user[MAXLEN];
	char pswd[MAXLEN];
} Userpswd;

/* Node(NODEDT) structure */
typedef struct Node Node;
struct Node
{
	void* pDT;												//Store the data(e.g. a Person)
	Node* pleft;											//The node to the left
	Node* pright;
	int serial;												//Position in the tree(traversal); also store quantity in link list
	int fserial;											//Posistion in the tree when search is conducted
};

/* File functions*/
void initialize_files(void);										//A:Make sure all files are ready, and initialize account
FILE* open_file(int file, const char* mode);						//B:Open file of given data type with error check
void ensure_folder(const char* folderName);							//B:Ensure the folder exists
int file_number(int dtype);											//B:Given the data type, return which file to be open

/* Node functions */
int bntree(Node** ppRoot, bool issearch, int dtype);				//A:Build a binary tree for a data type from its file, and distribute serial number for each node
int linklist(Node** ppFirst, int dtype);							//B:Create a link list storing multiple records from file
Node* create_node(void* pData, int dtype);							//B:Create a Node with given data type
Node* add_node(Node* pNode, void* pData, int dtype);				//B:Finding the correct position in tree to create a node
void assign_serial_node(Node* pNode, bool issearch);				//B:Assign serial numbers to each node within current node list
Node* retrieve_node(Node* pNode, int sequence, bool issearch);		//A:Return node given certain serial/fserial number
void release_node(Node* pNode);										//A:Free the memory allocated to the tree

Node* create_link(Merch* pmerch, int qnty);							//A:Create a Node with merch type for a link list
void push_link(Node* pFirst, Merch* pmerch, int qnty);				//A:Add a Node on the rightest side of link list
int update_link(Node* pFirst, const char* prkey, int qnty);			//A:Update a Node by matching the primary key
int delete_link(Node** ppFirst, const char* prkey);					//A:delete a Node by matching the primary key
int find_qnty_link(Node* pFirst, const char* prkey);				//A:Return the quantity of the Node by matching the primary key
void release_link(Node* pFirst);									//A:Free the memory allocated to the link list

/* Miscellaneous functions */
Transaction produce_trans(short type, const char* person_key, Node* pList, int different);	//A:Make a transaction record out of a link list and¡¡write to file 
void reduce_merch(Node* pList, int different);						//A:Reduce the items' 'remain'
void update_person(Person* loginer, long long grand_total);			//A:Increase the 'sales' of the person

void* allocate_memory(int dtype);									//B:Allocate memory of given data type with error check
void get_string(char* result, const char * reference, bool accept_blank);	//B:Get inputted string and delete '\n'
int get_number(int lowerlimit, int upperlimit, int original, char* prompt);	//B:Get number, prevent wrong inputs
int numdig(int num);												//B:Find the number of digits of a number
char* upper_case(char* update, char* source);						//B:Turn string into upper case for comparison
void make_prkey(int dtype, void* pdata, FILE* pfile);				//B:Make unique primary key

/* Data/Record functions */ 
int get_all_data(int dtype, char* mode);							//A:Get multiple records from keyboard and save to file
int edit_data(int dtype, void* pdata, bool isdelete, void (*func)(void* pnew, void* pold, void* any), void* anything);
//Note that we pass a function pointer with all void* parameters	//A:Update or Delete the given record(Two modes)
int surf_data(int dtype, void* pdata,FILE* ptemp);					//A:Locate the position of specific record in file
void duplicate_data(int dtype, void* pData,int index,FILE* ptemp, char* filename, bool isdelete);
																	//B:Duplicate whole file to new file with updating or deleting
void search_data(int dtype, Node* pRoot);							//A:Conduct searching, distribute fserial and store results in file
bool check_field(int dtype, int field, Node* pnode, char upper_string[10][MAXLEN]);	//B:Substring check for a given data type
void get_data(int dtype, void* pnew, const void* pold, bool isedit);	//B:Get a record of given data type from stdin
int write_data(int dtype, void* pdata,FILE* ptemp);					//B:Write a record of given data type to file
void* read_data(int dtype, void* pdata, FILE* ptemp);				//B:Read a record of given data type from file
bool retrieve_data(int dtype, void* pdata, int field, char string[MAXLEN], int * pnum);

int compare_data(int dtype, void* pdata1, void* pdata2);			//B:Compare 2 records to give -1, 0, or -1 for sorting

/*Username and password functions */
int authenticate(const char* username, const char* password, Person* result);	//A:Compare username and password in file
int change_password(const char* prkey_P);							//A:Change the password of a person
void add_user(const char prkey_P[]); 								//B:Add a new user and password combination
void userpswd_get_verify(char * userpswd, bool ispswd);				//B:Get and verify the username/password 
bool userpswd_validation(const char* userpswd, bool ispswd);		//B:Validate the username and password


/* Display functions */
void list_data(int dtype, void* pdata);								//A:Display a record of given data type
void display_merch(Merch* pmerch);									//A:In order page
void list_bought(Merch* pmerch, int qnty);							//A:List a bought record
void list_field(int dtype);											//A:Display the field names of given data type
void apisetxy(int x,int y);											//A:Use gotoxy independent of conio.h since many versons of it may not include this function
void apicolor(int color);											//A:Change text color
void apigetxy(int coor[2]);											//B:Get the current xy position 

/*Report functions */
void print_records(int dtype, Node* pRoot, int dtcount, bool issearch);
void print_single(int dtype, void* pdata, FILE* pFile);
void print_receipt(Transaction* pTrans, const char* staff_name, char model[MAXTRANS][MAXLEN], char brand[MAXTRANS][MAXLEN], long long grand);
																	//A:Print receipt from a record and searched results
/*
int main(void)
{
	initialize_files();
	
	change_password("P00001");
	
	
	return 0;
	
}
*/
								/* File, basic I/O function definitions */

void initialize_files(void)
{//A:Make sure all files are ready, and initialize account

	FILE* ptemp = NULL;											//Temporary File*
	Person admin_Person = {"P00000", "Administrator", "Admin", -1, -1, -1, 0};	//Initial Adminisrator account
	Userpswd admin_Userpswd = {"P00000", "admin", "admin"};						//Initial Administrator username and password
	int testeof = 0;											//Test Character
	bool no_account = false;
	
	ensure_folder("Reports");									//Ensure the folder exists
	ensure_folder("Receipt");
											
	for(int i = 0; i < 5; i++)
	{
		ptemp = open_file(i, "a+");
		
		if(i == file_number(PERSONDT))							//When Person file is opened, do the following:
		{														//If currently no account, create an initial Administrator account
			testeof = fgetc(ptemp);								//Testing EOF by get the first character of file
			if(!feof(ptemp))
				ungetc(testeof,ptemp);							//If not EOF, put it back:some accounts exist
			else
			{
				no_account = true;
				write_data(PERSONDT, &admin_Person, ptemp);		//Else, insert an initial account
			}
		}
		if(i == file_number(USRPSWDT) && no_account)
			write_data(USRPSWDT, &admin_Userpswd, ptemp);		//Create username and password for initial account
		
		fclose(ptemp);
	}
	ptemp = NULL;
	DebugPrintf("Success:Open all files.\n");
	return;
}

FILE* open_file(int file, const char* mode)
{//B:Open file of given data type with error check
	FILE* ptemp = NULL;											//Temporary File*
	ptemp = fopen(global[file], mode);							//"a+": readable and writable
	if(!ptemp)
	{
		fprintf(stderr,"Open file %s failed.Terminate program",global[file]);
		exit(1);
	}
	return ptemp;
}

void ensure_folder(const char* folderName)
{//B:Ensure the folder exists
	FILE* ptemp = NULL;
	char folderPath[100] = "";
	char filePath[100] = "";
	sprintf(folderPath, "./%s", folderName);
	sprintf(filePath, "%s/TESTING", folderPath);
	
	ptemp = fopen(filePath, "w");
	if(!ptemp)
	{
		if(mkdir(folderPath) == EOF)
		{
			fprintf(stderr,"Open folder %s failed.Terminate program", folderName);
			exit(1);
		}
	}
	else
	{
		fclose(ptemp);
		 if(remove(filePath) == EOF)
		{
			fprintf(stderr,"Remove TESTING failed.Terminate program");
			exit(1);
		}
	}
	return;
}

int file_number(int dtype)
{//B:Given the data type, return which file to be open
	switch(dtype)
	{
		case MERCHDT:
		return 0;
		case PERSONDT:
		return 1;
		case TRANSDT:
		return 2;
		case USRPSWDT:
		return 4;
		default:
		DebugPrintf("No such data type to open file!");
	}
	return -1;
}

								/* Node function definitions */


int bntree(Node** ppRoot, bool issearch, int dtype)
{//A:Build a binary tree for a data type from its file, and distribute serial number for each node

	FILE* ptemp = NULL;											//Temporary File*
	void* tmpData = NULL;
	int count = 0;												//Count of Nodes embodied in the tree
	int testeof = 0;											//Testing(The first) character for EOF in opened files
	tmpData = allocate_memory(dtype);							//Allocate memory to tmpData according to type
	if(*ppRoot)
		release_node(*ppRoot);									//Release the memory, if any
	*ppRoot = NULL;	
		
	if(!issearch)												//Not in search mode
		ptemp = open_file(file_number(dtype) , "a+");							
	else
		ptemp = open_file(3 ,"r+");								//Open SEARCHFILE
	
	testeof = fgetc(ptemp);										//Testing EOF by get the first character of file
	if(!feof(ptemp))
		ungetc(testeof,ptemp);									//If not EOF, put it back
	
	while(!feof(ptemp))
	{
		assert(!feof(ptemp));									//File must be opened or terminate program	
		read_data(dtype, tmpData, ptemp);						//Read a record from file

		if(!*ppRoot)											//If *ppRoot is new,
		{
			*ppRoot = create_node(tmpData, dtype);				//then create a Node as root Node for binary tree
							
			count = 1;											//The first Node
			fflush(stdout);
		}
		else
		{
			add_node(*ppRoot, tmpData, dtype);			//If *ppRoot has contents(the second or more times), use add_node to find an
			count++;									//appropriate place to create Node									
		}
	}
	fclose(ptemp);
	if(!issearch)												//Distribute series if not searching or fseries if in searching mode
	{
		series = 0;												//Initialize series
		assign_serial_node(*ppRoot, false);
	}
	else
	{
		fseries = 0;
		assign_serial_node(*ppRoot, true);
	}
	free(tmpData);
	tmpData = NULL;
	return count;												//Number of placed Nodes
}

int linklist(Node** ppFirst, int dtype)
{//B:Create a link list storing multiple data from file
	FILE* pfile = open_file(file_number(dtype), "r+");			//Open file of certain data type
	void* tmpData = allocate_memory(dtype);
	int count = 0;												//Count of Nodes embodied in the tree
	int testeof = 0;											//Testing(The first) character for EOF in opened files
	*ppFirst = NULL;
	Node * pCurrent = NULL;										//Current node
	Node * pPrevious = NULL;									//Previous Node
	
	testeof = fgetc(pfile);										//Testing EOF by get the first character of file
	if(!feof(pfile))
		ungetc(testeof,pfile);									//If not EOF, put it back
	else
		return 0;												//count = 0
	
	while(!feof(pfile))
	{
		assert(!feof(pfile));									//File must be opened or terminate program	
		read_data(dtype, tmpData, pfile);						//Read a record from file

		pCurrent = create_node(tmpData, dtype);					//Current node
		if(!*ppFirst)											//If *ppFirst is NULL, it is the first time
		{
			*ppFirst = pCurrent;
			pPrevious = pCurrent;								//Store the current node for next loop
		}
		else													//Not the first time
		{
			pPrevious->pright = pCurrent;						//The previous node's next node is current node
			pPrevious = pCurrent;								//Store the current node for next loop
		}
		count ++;
	}
	fclose(pfile);
	
	free(tmpData);
	tmpData = NULL;
	return count;
}


Node* create_node(void* pData, int dtype)
{//B:Create a Node with given data type

	Node* pNode = NULL;	
	pNode = allocate_memory(NODEDT);								//On Heap: Allocate memory for a new Node
	
	pNode->pDT = allocate_memory(dtype);							//On Heap
	
	if(!pData)														//If no data, return NULL
		return NULL;
	switch(dtype)
	{
		case MERCHDT:	
		*((Merch*)pNode->pDT)= *((Merch*)pData);				//Assign data to the Node
		break;
		case PERSONDT:
		*((Person*)pNode->pDT) = *((Person*)pData);
		break;
		case TRANSDT:
		*((Transaction*)pNode->pDT) = *((Transaction*)pData);
		case USRPSWDT:
		*((Userpswd*)pNode->pDT) = *((Userpswd*)pData);
		break;
	}
		
	pNode->pleft = pNode->pright = NULL;
	return pNode;
}

Node* add_node(Node* pNode, void* pData, int dtype)
{//B:[Recursion]Finding the correct position in tree to create a node

	if(!pData)
		return NULL;												//For protection
	if(!pNode)
		return NULL;												//For protection
		
	int result = 0;													//result of comparing the given and existed record as 1, 0 and -1
	
	result = compare_data(dtype, pNode->pDT, pData);
		
	if(result == 0)													//Absolutely the same!
		return NULL;
	
	if(result == 1)													//The given record is prior to the existed, so put the given one in the left side
	{
		if(!pNode->pleft)											//If the Node has no pleft yet,
			return (pNode->pleft = create_node(pData, dtype)); 		//create the Node here
		else
			return  add_node(pNode->pleft,pData, dtype);			//[Recursion]If the Node has pleft already, apply add_node to analyse pleft
	}
	else if(result == -1)	//Vice versa
	{
		if(!pNode->pright)
			return (pNode->pright = create_node(pData, dtype));
		else
			return  add_node(pNode->pright,pData, dtype);
	}
}

void assign_serial_node(Node* pNode, bool issearch)
{//B:[Recursion]Assign serial numbers to each node within current node list

	if(!pNode)
		return;														//For protection
	if(pNode->pleft)
		assign_serial_node(pNode->pleft, issearch);					//[Recursion]If pleft exists, dig into pleft
	
	//Now this Node has no pleft.
	
	if(issearch)
	{
		pNode->fserial = fseries;									//Allocate fseries
		fseries++;
	}
	if(!issearch)
	{
		pNode->serial = series;
		series++;
	}

	if(pNode->pright)												//After the node itself is done, if pright exists, go to pright
		assign_serial_node(pNode->pright, issearch);
	return;															//Mission complete, go above
}

Node* retrieve_node(Node* pNode, int sequence, bool issearch)
{//A:[Recursion]Return node given certain serial/fserial number

	Node* found = NULL;											//Initialize Node* found
	if(!pNode)
		return NULL;											//For protection
	if(pNode->pleft)												
		if(found = retrieve_node(pNode->pleft, sequence, issearch))//[Recursion]Dig to the left. After recursion, "found" is assigned,
			return found;										//return it immediately to go up.
			
	//Now it is in deeper Nodes
	
	if(issearch && pNode->fserial == sequence)					//If the serial matches, return it.
		return pNode;
	if(!issearch && pNode->serial == sequence)
		return pNode;
	
	if(pNode->pright)											//Go to the right Node
		if(found = retrieve_node(pNode->pright, sequence, issearch))
			return found;
	return NULL;												//Nothing found, return NULL
}
	
void release_node(Node* pNode)
{//A:[Recursion]Free the memory allocated to the tree

	if(!pNode)
		return;
	if(pNode->pleft)
		release_node(pNode->pleft);
	if(pNode->pright)
		release_node(pNode->pright);
	
	free(pNode->pDT);
	pNode->pDT = NULL;
	free(pNode);
	return;
}

Node* create_link(Merch* pmerch, int quantity)
{	
	Node* pFirst = allocate_memory(NODEDT);							//On Heap: Allocate memory for a new Node
	
	pFirst->pDT = allocate_memory(MERCHDT);							//On Heap
	
	if(!pmerch)														//If no data, return NULL
		return NULL;
		
	*((Merch*)pFirst->pDT)= *pmerch;								//Assign data to the Node
	pFirst->serial = quantity;										//Use serial to store quantity
	pFirst->pleft = pFirst->pright = NULL;
	return pFirst;
}

void push_link(Node* pFirst, Merch* pmerch, int qnty)
{
	if(!pFirst)													//Safety
		return;
		
	Node* pNew = allocate_memory(NODEDT);
	pNew->pDT = allocate_memory(MERCHDT);
	*((Merch*)pNew->pDT) = *pmerch;								//Copy the content to pNew
	pNew->serial = qnty;										//Use serial to store quantity
	
	Node* pCurrent = pFirst;
	while(pCurrent->pright)										//To the most right node
	{
		pCurrent = pCurrent->pright;
	}
	pCurrent->pright = pNew;
	pNew->pleft = pCurrent;
	pNew->pright = NULL;
	return;
}

int update_link(Node* pFirst, const char* prkey, int qnty)
{
	if(!pFirst)
		return -1;
	Node* pCurrent = pFirst;
	while(pCurrent)
	{
		if(strcmp( ((Merch*)pCurrent->pDT)->prkey_M, prkey) == 0)	//Same key
		{
			pCurrent->serial = qnty;
			return 0;											//Success
		}
		pCurrent = pCurrent->pright;
	}
	return -1;													//Fail
}

int delete_link(Node** ppFirst, const char* prkey)
{
	if(!(*ppFirst))
		return -1;												//Fail
	Node* pCurrent = *ppFirst;
	while(pCurrent)
	{
		if(strcmp( ((Merch*)pCurrent->pDT)->prkey_M, prkey) == 0)	//Same key
		{
			if(!pCurrent->pleft)								//Delete the most left(first) node
				*ppFirst = pCurrent->pright;					//Change the pFirst
			if(pCurrent->pleft)
				pCurrent->pleft->pright = pCurrent->pright;			//Connect left node with right node
			if(pCurrent->pright)
				pCurrent->pright->pleft = pCurrent->pleft;
			free(pCurrent->pDT);
			pCurrent->pDT = NULL;
			free(pCurrent);
			pCurrent = NULL;
			return 0;
		}
		pCurrent = pCurrent->pright;
	}
	return -1;													//Fail
}

int find_qnty_link(Node* pFirst, const char* prkey)
{
	if(!pFirst)
		return 0;
	Node* pCurrent = pFirst;
	while(pCurrent)
	{
		if(strcmp( ((Merch*)pCurrent->pDT)->prkey_M, prkey) == 0)
		{
			return pCurrent->serial;								//Found and return quantity bought
		}
		pCurrent = pCurrent->pright;
	}
	return 0;														//Not found and return 0
}

void release_link(Node* pFirst)
{
	if(!pFirst)
		return;
	if(pFirst->pright)
		release_link(pFirst->pright);
	
	free(pFirst->pDT);
	pFirst->pDT = NULL;
	free(pFirst);
	return;
}

											/* Miscellaneous functions*/
Transaction produce_trans(short type, const char* person_key, Node* pList, int different)
{//A:Make a transaction record out of a link list and¡¡write to file 
	Transaction newTrans;
	Node* pCurrent = pList;
	FILE* pFile = open_file(file_number(TRANSDT), "a+");
	
	newTrans.type = type;											//Copy the data to the new transaction
	strcpy(newTrans.prkey_P, person_key);
	newTrans.different = different;
	newTrans.ttime = time(NULL);
	for(int i = 0; i < different; i++)								//Copy every items' data
	{
		newTrans.quantity[i] = pCurrent->serial;
		newTrans.price_then[i] = ((Merch*)pCurrent->pDT)->price;
		strcpy(newTrans.prkey_M[i], ((Merch*)pCurrent->pDT)->prkey_M);
		pCurrent = pCurrent->pright;
	}
	make_prkey(TRANSDT, &newTrans, pFile);
	DebugPrintf("PRKEY:%s", newTrans.prkey_T);
	DebugSystem("pause");
	
	write_data(TRANSDT, &newTrans, pFile);							//Write to file
	fclose(pFile);
	
	return newTrans;
}

void reduce_merch(Node* pList, int different)
{//A:Reduce the items' 'remain'
	Node* pCurrent = pList;
	void reduce(Merch* pnew, Merch* pold, const int* poffset)
	{
		*pnew = *pold;
		pnew->remain -= *poffset;
		return;
	}
	
	for(int i = 0; i < different; i++)
	{
		edit_data(MERCHDT, pCurrent->pDT, false, (void (*)(void*,void*,void*))&reduce, &(pCurrent->serial));	//High order function
		pCurrent = pCurrent->pright;
	}
	if(pCurrent)
	{
		DebugPrintf("There are more data! Something got wrong.");
		DebugSystem("pause");
	}
	return;
}
void update_person(Person* loginer, long long grand_total)
{//A:Increase the 'sales' of the person
	void increase_sale(Person* pnew, Person* pold, const int* poffset)
	{
		*pnew = *pold;
		pnew->sales += *poffset;
		return;
	}
	edit_data(PERSONDT, loginer, false, (void (*)(void*, void*, void*))&increase_sale, &grand_total);
	return;
}



void * allocate_memory(int dtype)
{//B:Allocate memory of given data type with error check
	void * pData = NULL;
	switch(dtype)												//According to data type, allocate memory
	{
		case NODEDT:
			pData = malloc(sizeof(Node));
			break;
		case MERCHDT:
			pData = malloc(sizeof(Merch));
			break;
		case PERSONDT:
			pData = malloc(sizeof(Person));
			break;
		case TRANSDT:
			pData = malloc(sizeof(Transaction));
			break;
		case USRPSWDT:
			pData = malloc(sizeof(Userpswd));
			break;
		default:
			perror("\nNo such dtype. Cannot allocate memory. Terminate Program.\n");
			exit(2);
	}
	if(!pData)
	{
		perror("\nAllocate memory to data failed. Terminate Program.\n");
		exit(2);
	}
	return pData;
}

void get_string(char* result, const char * reference, bool accept_blank)
{//B:Get inputted string and delete '\n'
	
	if(accept_blank)
		assert(reference);										//If edit, must have original string
	size_t length = 0;											//length of string
	char temp[MAXLEN] = "";
	bool valid = false;
	do
	{
		fflush(stdin);											//Clear stdin
		valid = false;
		fgets(temp,MAXLEN,stdin);
		length = strlen(temp);
		if(temp[length - 1] == '\n')							//The character before '\0' should be '\n' or string is too long
		{
			temp[length - 1] = '\0';							//Replace '\n' with '\0' to cut '\n'
			valid = true;
		}
		else
		{
			printf("Too long! Try again:\n");
			valid = false;
		}
		if(strcmp(temp, "") == 0 && accept_blank)				//Nothing entered and accept blank
		{
			strcpy(result, reference);
			return;
		}
		else if(strcmp(temp, "") == 0 && !accept_blank)			//Do not accept blank!
		{
			printf("Do not accept blank! Try again:\n");
			valid = false;
		}
	}while(!valid);
	
	strcpy(result, temp);
	return;
}

int get_number(int lower, int upper, int original, char* prompt)
{//B:Get number, prevent wrong inputs

	char temp[MAXLEN] = "";
	size_t length = 0;
	int number = BLANK;											//Initialization
	int scantest = EOF;											//Receive the return value of sscanf to detect error
	while(true)
	{
		puts(prompt);											//Output to screen the prompt message
		fflush(stdin);											//Reset stdin to clear previous input storing in it
		fgets(temp, MAXLEN, stdin);
		length = strlen(temp);

		if(temp[length - 1] == '\n')							//The character before '\0' should be '\n'
			temp[length - 1] = '\0';							//Replace '\n' with '\0' to cut '\n'
		
		if(temp[0] == 0 && original != BLANK)
			return original;									//No change
		
		fflush(stdin);
		scantest = sscanf(temp," %d",&number);
		if(scantest != EOF && number >= lower && number <= upper)//Not EOF to make sure input is in correct format: an int
			return number;
		else
			printf("Invalid input. Try again:\n");
	}
}

int numdig(int num)
{//B:Find the number of digits of a number

	//Assume num is positive
	int dig = 1;		//At least 1
	while(num >= 10)	//If there are two or more digits left,
	{
		num /= 10;		//Cut the last,
		dig ++;			//And do increment
	}
	return dig;
}

char* upper_case(char* update, char* source)
{//B:Turn string into upper case for comparison

	for(int i = 0; i < strlen(source) ; update[i] = toupper(source[i++]));	//Converse to uppercase
	update[strlen(source)] = '\0';
	return update;													//Note that update is an address that need to be freed after using
}

void make_prkey(int dtype, void * pdata, FILE* pfile) 
{//B:Make unique primary key
	int largekey = 0;												//Greatest primary key number found
	int found = 0;													//Temporarily store the number
	char buf[MAXLEN];
	void* tmpData = allocate_memory(dtype);
	
	rewind(pfile);													//Move to the beginning of file 
	while(read_data(dtype, tmpData, pfile))							//Get the first/next record
	{
		switch(dtype)
		{
			case MERCHDT:
			found = sscanf(((Merch*)tmpData)->prkey_M, "%*c%s", &buf);	//Get the number neglecting(*) the leading letter
			break;
			case PERSONDT:
			found = sscanf(((Person*)tmpData)->prkey_P, "%*c%s", &buf); 
			break;
			case TRANSDT:
			found = sscanf(((Transaction*)tmpData)->prkey_T, "%*c%s", &buf); 
			break;
		}
		found = atoi(buf);											//Change character to integer
		
		if(found > largekey)
			largekey = found;
	}
	switch(dtype)
	{
		case MERCHDT:
		found = sprintf(((Merch*)pdata)->prkey_M, "M%05d", largekey + 1);	//Distribute the new prkey, 
		break;																//which is larger than the original largest by 1
		case PERSONDT:														//So the first record should be X000001
		found = sprintf(((Person*)pdata)->prkey_P, "P%05d", largekey + 1); 
		break;
		case TRANSDT:
		found = sprintf(((Transaction*)pdata)->prkey_T, "T%05d", largekey + 1); 
		break;
	}
	fseek(pfile, 0, SEEK_END);									//Move to the end of file for next writing instruction
	return;
}

									/*General data function definitions */

int get_all_data(int dtype, char* mode)
{//A:Get multiple data from keyboard and save to file

	int answer = 0;												//Ask if continues
	int count = 0;												//Count of inputted units of record
	void * TempData = NULL;										//A temporary record
	FILE* ptemp = NULL;
	
	ptemp = open_file(file_number(dtype), mode);				//Open corresponding file
	TempData = allocate_memory(dtype);							//Allocate memory to a temporary record
	
	do
	{
		get_data(dtype, TempData, NULL, false);					//Get input from keyboard
		make_prkey(dtype, TempData, ptemp);						//Register Primary key to a new record
		
		if(dtype == PERSONDT)									//For some type of Person, an account is needed
			if(((Person*)TempData)->level == 0 || ((Person*)TempData)->level == 1)		//Manager and staff should have a username and password to log in
				add_user(((Person*)TempData)->prkey_P);			//Create the username and password connected to a primary key of people
			
		write_data(dtype, TempData, ptemp);						//Write to the file
		count ++;
		puts("Do you want to enter another record?(Y for yes; otherwise no):");
		scanf(" %c",&answer); 
	}while(tolower(answer) == 'y');
	
	free(TempData);
	TempData = NULL;
	fclose(ptemp);
	
	return count;
}

int edit_data(int dtype, void* pdata,bool isdelete, void (*func)(void* pnew, void* pold, void* any), void* anything)
{//A:Update or Delete the given records(Two modes)

	FILE* ptemp = NULL;	
	int index = -1 ;											//The order of specific record in file
	int lengtho = 0;											//Length to be printed of original record
	int lengthu = 0;											//Length to be printed of updated record
	bool isduplic = true;										//Use the more complicated method to update(open a new file)
	char filename[30] = "Coercefile.txt";						//Used in the more complicated method of updating as temporary txt
	void * tmpData = NULL;										//The updated record
	
	ptemp = open_file(file_number(dtype), "r+");				//For a datatype, open the corresponding file
							
	tmpData = allocate_memory(dtype);							//To store the newly entered record
	index = surf_data(dtype, pdata, ptemp);						//Get the order of the designated record in file
	if(!isdelete)												//If not in delete mode but to edit and replace
	{
		if(func)												//Function provided(This enables "replacing codes" inside a function)
			func(tmpData, pdata, anything);						//Together with void* parameters, we can do almost everything!!!!!
		else													//Function not provided
		{
			list_field(dtype);									//Display field names
			apicolor(14);										//Yellow text color
			list_data(dtype, pdata);							//Display the original(designated) record
			apicolor(15);										//White text color
			get_data(dtype, tmpData, pdata, true);				//Require the editing record from keyboard
		}
		
		
		if(dtype == USRPSWDT)
			if(!strcmp("Failed", ((Userpswd*)tmpData)-> prkey_P))	//Change password failed, end editing
				return -1;
		
		switch(dtype)
		{
			case MERCHDT:
			lengtho = 16 + strlen(((Merch*)pdata)->model) + numdig(((Merch*)pdata)->remain) 
			+ strlen(((Merch*)pdata)->brand) + numdig(((Merch*)pdata)->price) 
			+ numdig(((Merch*)pdata)->cost);					//Length to be printed of original record
			
			lengthu = 16 + strlen(((Merch*)tmpData)->model) + numdig(((Merch*)tmpData)->remain) 
			+ strlen(((Merch*)tmpData)->brand) + numdig(((Merch*)tmpData)->price)
			+ numdig(((Merch*)tmpData)->cost);					//Length to be printed of updated record
			
			break;
			
			case PERSONDT:
			lengtho = 17 + strlen(((Person*)pdata)->surname) + strlen(((Person*)pdata)->firstname) 
			+ numdig(((Person*)pdata)->salary) + numdig(((Person*)pdata)->sales);			//Length to be printed of original record
		
			lengthu = 17 + strlen(((Person*)tmpData)->surname) + strlen(((Person*)tmpData)->firstname) 
			+ numdig(((Person*)tmpData)->salary) + numdig(((Person*)tmpData)->sales);		//Length to be printed of updated record
			break;
			
			case TRANSDT:
			lengtho = 22 + numdig(((Transaction*)pdata)->ttime);
			for(int i = 0; i < ((Transaction*)pdata)->different; i++)
			{
				lengtho += 10;
				lengtho += numdig(((Transaction*)pdata)->price_then[i]);
				lengtho += numdig(((Transaction*)pdata)->quantity[i]);
			}
			
			lengthu = 22 + numdig(((Transaction*)tmpData)->ttime);
			for(int i = 0; i < ((Transaction*)tmpData)->different; i++)
			{
				lengthu += 10;
				lengthu += numdig(((Transaction*)tmpData)->price_then[i]);
				lengthu += numdig(((Transaction*)tmpData)->quantity[i]);
			}
			break;
			
			case USRPSWDT:
			lengtho = 11 + strlen(((Userpswd*)pdata)->user) + strlen(((Userpswd*)pdata)->pswd);	
			lengthu = 11 + strlen(((Userpswd*)tmpData)->user) + strlen(((Userpswd*)tmpData)->pswd);
			DebugPrintf("\nOriginal:%d\nUpdated:%d\n", lengtho, lengthu);
			DebugSystem("pause");
			break;
			
		}
		
		
		if(lengtho == lengthu)				//Except for username&password, if the two records are luckly of same length,
		{
			isduplic = false;									//use simple replacement when editing the file.									
			fseek(ptemp,-(long)lengtho,SEEK_CUR);				//Move cursor a record back to the record what we want to edit
			write_data(dtype, tmpData, ptemp);					//Replace the original record
			
			fflush(ptemp);
			fclose(ptemp);
			ptemp = NULL; 
		}
		else													//If not of same length,
		{
			isduplic = true;									//Conduct a more complicated replacement
			duplicate_data(dtype, tmpData,index,ptemp,filename,false);	
		}
	}
	else														//In delete mode,
	{
		isduplic = true;
		duplicate_data(dtype, NULL,index,ptemp,filename,true);	//set isdelete true
	}
	
	if(isduplic)												//If duplicate_data is used
	{
		fflush(ptemp);
		fclose(ptemp);
		ptemp = NULL;
		Sleep(100);
		if(remove(global[file_number(dtype)]) == -1)							//Delete the orignial file
		{
			puts("Wait...");
			Sleep(2000);
			if(remove(global[file_number(dtype)]) == -1)
			{
				perror("Cannot remove file. Terminate program.");
				exit(4);
			}
		}
		Sleep(100);												//Windows API function, to wait for 100ms		
		rename(filename, global[file_number(dtype)]);
	}

	free(tmpData);
	tmpData = NULL;
									//Change the new file created in duplicate Coercefile.txt to Product.txt
	return 0;
}


int surf_data(int dtype, void* pdata,FILE* ptemp)
{//A:Locate the position of specific record in file

	int index = 0;												//The order of specific record in file
	int result = -1;
	void * tmpData = NULL;										//Temporary record(read from file)
	
	tmpData = allocate_memory(dtype);

	while(read_data(dtype, tmpData,ptemp))						//Get the first/next record
	{
		result = compare_data(dtype, tmpData, pdata);
		if(result == 0)											//Determine whether the same
			break;
		index ++;
	}
	if(result)
		index = -1;												//Not found
	
	free(tmpData);
	tmpData = NULL;
	return index;
}

void duplicate_data(int dtype, void* pData,int index,FILE* ptemp,char* filename, bool isdelete)
{//B:Duplicate whole file to new file with updating or deleting

	rewind(ptemp);												//Move the cursor to the beginning of file
	FILE* pnewfile = fopen(filename,"w+");						//Create a new file with given filename
	void * tmpData = NULL;										//Storing temporary record read from file

	if(!pnewfile)
	{
		fprintf(stderr,"Open new file failed. Terminate program.");
		exit(1);
	}
	tmpData = allocate_memory(dtype);
	
	for(int i = 0; i < index; i++)
	write_data(dtype, read_data(dtype, tmpData,ptemp),pnewfile);			//Copy to new file the record that should be kept the same
	if(!isdelete)															//If not in delete mode,
		write_data(dtype, pData,pnewfile);									//write the updated record instead of copying the original
	read_data(dtype, tmpData,ptemp);										//Skip the designated record in original file
	while(read_data(dtype, tmpData,ptemp))									//Continue copying the remaining records that should be kept the same
		write_data(dtype, tmpData,pnewfile);
	
	free(tmpData);
	tmpData = NULL;
	fclose(pnewfile);
	return;
}

void search_data(int dtype, Node* pRoot)
{//A:Conduct searching using global mask, distribute fserial and store results in file

	char buf[MAXLEN] = "";										//Buffer char array
	char upper_string[10][MAXLEN] = {};							//Being converted to uppercase for comparing
	FILE* ptemp = NULL;
	int secount = 0;											//Current number of Nodes found; fserial to be distributed
	bool ismatch = true;										//Result of check
	
	unsigned char mask;
	switch(dtype)
	{
		case MERCHDT:
		mask = Merchmask;
		break;
		case PERSONDT:
		mask = Personmask;
		break;
		case TRANSDT:
		mask = Transmask;
		break;
	}
	
	for(int i = 0; i < 10 ; upper_case(upper_string[i], search_content[i++]));	//Converse strings to uppercase
	
	DebugPrintf("\nIn function 'searchdata': series = %d\n",series);	
	
	ptemp = open_file(3, "w+");													//Open the file storing search results

	for(int i = 0; i < series; i++)
	{
		ismatch = true;															//Search begins
		DebugPrintf("\nIn function 'searchdata': i = %d", i);
		for(int j = 0; j < 10; j++)	
		{
			if((int)pow(2, j) & mask)											//Decide whether the field needs to be checked
				if(check_field(dtype, j, retrieve_node(pRoot, i, false), upper_string) == false)	//If any one field does not match, go to next record
				{
					ismatch = false;
					DebugPrintf("\nFail!! i = %d; j = %d", i , j);
					break;
				}
		}
		if(ismatch)
		{
			DebugPrintf("\nIn function 'searchdata': i = %d, success", i);
			retrieve_node(pRoot, i, false)->fserial = secount++;			//Distribute a fserial to the found Node
			write_data(dtype, retrieve_node(pRoot,i, false)->pDT,ptemp);	//Write the found Node into file
		}
	}
	fclose(ptemp);
	ptemp = NULL;
	return;
}

bool check_field(int dtype, int field, Node* pnode, char upper_string[10][MAXLEN])
{//B:Substring check for a given data type
	
	char node_string[MAXLEN] = "";								//The string data of the record in file
	char search_string[MAXLEN] = "";							//The string data to be searched
	int node_num = 0;											//The int data of the record in file
	int search_num = 0;											//The int data of the record in file
	bool isstring = true;										//Is the data a string(char*) or a int?
	
	upper_case(search_string, search_content[field]);			//Get the search content
	isstring = retrieve_data(dtype, pnode->pDT, field, node_string, &node_num);		//The function will return a boolean value indicating whether it is string
	DebugPrintf("\nfunction 'check_field': mask = %d; field = %d; node_string = %s; search_string = %s, isstring = %s", Merchmask, field, node_string, search_string, isstring? "ture":"false");
	if(isstring)												//For strings
	{
		if(strstr(node_string, search_string))					//If string is a substring of uattr1(sucessfully found)
		{
			return true;
		}
	}
	else														//For numbers
	{
		search_num = atoi(search_string);
		if(node_num == search_num)								//If numbers are exactly the same
			return true;
	}
	return false;												//Not matched
}

int write_data(int dtype, void* pdata,FILE* ptemp)
{//B:Write a record of given data type to file

	switch(dtype)
	{
		case MERCHDT:
		fputs(((Merch*)pdata)->prkey_M,ptemp);
		fprintf(ptemp,"|%d|",((Merch*)pdata)->category);		//Use '|' to separate fields
		fputs(((Merch*)pdata)->model,ptemp);
		fprintf(ptemp,"|%d|",((Merch*)pdata)->remain);
		fputs(((Merch*)pdata)->brand,ptemp);
		fprintf(ptemp,"|%d|%d|\n",((Merch*)pdata)->price,((Merch*)pdata)->cost);
		break;
		
		case PERSONDT:
		fputs(((Person*)pdata)->prkey_P,ptemp);
		fprintf(ptemp,"|%d|",((Person*)pdata)->level);						//Use '|' to separate fields
		fputs(((Person*)pdata)->surname,ptemp);
		fprintf(ptemp,"|%d|",((Person*)pdata)->ismale? 0: 1);
		fputs(((Person*)pdata)->firstname,ptemp);
		fprintf(ptemp,"|%d|%d|\n",((Person*)pdata)->salary, ((Person*)pdata)->sales);
		return 0;
		break;
		
		case TRANSDT:
		fputs(((Transaction*)pdata)->prkey_T,ptemp);
		fprintf(ptemp,"|%d|",((Transaction*)pdata)->type);					//Use '|' to separate fields
		fputs(((Transaction*)pdata)->prkey_P,ptemp);
		fprintf(ptemp,"|%d|",((Transaction*)pdata)->different);	
		for(int i = 0; i < ((Transaction*)pdata)->different; i++)
		{
			fprintf(ptemp,"|");												//Use '||' to separate different products
			fputs(((Transaction*)pdata)->prkey_M[i], ptemp);
			fprintf(ptemp,"|%d|%d|",((Transaction*)pdata)->price_then[i], ((Transaction*)pdata)->quantity[i]);
		}
		fprintf(ptemp,"|%I64d|\n",((Transaction*)pdata)->ttime);
		break;
		
		
		case USRPSWDT:
		fputs(((Userpswd*)pdata)->prkey_P, ptemp);
		fprintf(ptemp, "|");
		fputs(((Userpswd*)pdata)->user, ptemp);
		fprintf(ptemp, "|");
		fputs(((Userpswd*)pdata)->pswd, ptemp);
		fprintf(ptemp, "|\n");
		break;
	}
	return 0;
}

void* read_data(int dtype, void* pdata,FILE* ptemp)
{//B:Read a record of given data type from file
	
	char buf[MAXLEN];
	int test = -1;												//Test for empty file
	fscanf(ptemp,"%c",&test);
	if(test == EOF)												//If empty, return
		return NULL;
		
	switch(dtype)
	{
		case MERCHDT:

		fscanf(ptemp,"%[^|]", buf);								//Get the remaining string of the first item
		sprintf(((Merch*)pdata)->prkey_M, "%c%s", test, buf);	//Combine the test character and the remaining string
		fscanf(ptemp,"|%d",&((Merch*)pdata)->category);
		fscanf(ptemp,"|%[^|]",((Merch*)pdata)->model);
		fscanf(ptemp,"|%d",&((Merch*)pdata)->remain);
		fscanf(ptemp,"|%[^|]",((Merch*)pdata)->brand);
		fscanf(ptemp,"|%d|%d|\n",&((Merch*)pdata)->price,&((Merch*)pdata)->cost);
		break;
		
		case PERSONDT:
		
		fscanf(ptemp,"%[^|]", buf);
		sprintf(((Person*)pdata)->prkey_P, "%c%s", test, buf);
		fscanf(ptemp,"|%d",&((Person*)pdata)->level);
		fscanf(ptemp,"|%[^|]",((Person*)pdata)->surname);
		fscanf(ptemp,"|%d",&((Person*)pdata)->ismale);
		fscanf(ptemp,"|%[^|]",((Person*)pdata)->firstname);
		fscanf(ptemp,"|%d|%d|\n",&((Person*)pdata)->salary,&((Person*)pdata)->sales);
		break;
		
		case TRANSDT:
		
		fscanf(ptemp,"%[^|]", buf);
		sprintf(((Transaction*)pdata)->prkey_T, "%c%s", test, buf);
		fscanf(ptemp,"|%d", &((Transaction*)pdata)->type);
		fscanf(ptemp,"|%[^|]",((Transaction*)pdata)->prkey_P);
		fscanf(ptemp,"|%d|", &((Transaction*)pdata)->different);
		
		assert(((Transaction*)pdata)->different);				//At least one kind of merchandize processed

		for(int i = 0; i < ((Transaction*)pdata)->different; i++)
		{
			fscanf(ptemp, "|%[^|]",((Transaction*)pdata)->prkey_M[i]);
			fscanf(ptemp,"|%d|%d|", &((Transaction*)pdata)->price_then[i], &((Transaction*)pdata)->quantity[i]);
		}
		fscanf(ptemp,"|%I64d|\n", &((Transaction*)pdata)->ttime);
		break;
		
		case USRPSWDT:
		
		fscanf(ptemp,"%[^|]", buf);
		sprintf(((Userpswd*)pdata)->prkey_P, "%c%s", test, buf);
		fscanf(ptemp,"|%[^|]",((Userpswd*)pdata)->user);
		fscanf(ptemp,"|%[^|]|\n",((Userpswd*)pdata)->pswd);
		break;
	}
	return pdata;
}

void get_data(int dtype, void* pnew, const void* pold, bool isedit)
{//B:Get a record of given data type from stdin
	assert(pnew);							//If NULL, terminate function:pnew must exist
	if(isedit)								//If accept no change(explained after), pold must exist
		assert(pold);							
	char buffer1[MAXLEN];
	char buffer2[MAXLEN];
	
	fflush(stdin);												//Clear stdin
	if(isedit && dtype != USRPSWDT)
		printf("Enter blank represent no change for this field.\n");
	
	switch(dtype)
	{
		case MERCHDT:																	//get a Merchanize 
		if(isedit)
			strcpy(((Merch*)pnew)->prkey_M, ((Merch*)pold)->prkey_M);						//If in update mode, copy the prkey to the new onw
		((Merch*)pnew)->category = get_number(0,3,isedit? ((Merch*)pold)->category: BLANK	//If is edit mode, accept blank for retain original. Prepare original data for this condition.
		,"Now enter the category\n(0:CPU , 1:RAM , 2:Display Card , 3:Hard disk):");		//otherwise give BLANK to represent cannot blank
		puts("Now enter the model:");
		get_string(((Merch*)pnew)->model,((Merch*)pold)->model, isedit);
		puts("Now enter the brand:");
		get_string(((Merch*)pnew)->brand,((Merch*)pold)->brand, isedit);
		fflush(stdin);
		((Merch*)pnew)->price = get_number(0,10000000,isedit? ((Merch*)pold)->price: BLANK,"Now enter the selling price:");
		((Merch*)pnew)->cost = get_number(0,10000000,isedit? ((Merch*)pold)->cost: BLANK,"Now enter the cost per piece:");
		((Merch*)pnew)->remain = get_number(0,10000000,isedit? ((Merch*)pold)->remain: BLANK,"Now enter number of remaining pieces:");
		break;
		
		case PERSONDT:																		//get a Person
		if(isedit)
			strcpy(((Person*)pnew)->prkey_P, ((Person*)pold)->prkey_P);						//If in update mode, copy the prkey to the new onw
		((Person*)pnew)->ismale = get_number(0,1,isedit? ((Person*)pold)->ismale: BLANK,"Now enter the gender(0:Female , 1:male)");
		((Person*)pnew)->level = get_number(0,1,isedit? ((Person*)pold)->level: BLANK, "Now enter the level\n(0:Manager , 1:Staff):");
		puts("Now enter his/her family name:");
		get_string(((Person*)pnew)->surname,((Person*)pold)->surname, isedit);
		puts("Now enter his/her given name:");
		get_string(((Person*)pnew)->firstname,((Person*)pold)->firstname, isedit);
		fflush(stdin);
		((Person*)pnew)->salary = get_number(0,10000000,isedit? ((Person*)pold)->salary: BLANK,"Now enter his/her salary:");
		((Person*)pnew)->sales = get_number(0,10000000,isedit? ((Person*)pold)->sales: BLANK,"Now enter his/her sales:");
		break;
		
		case USRPSWDT:																		//Two modes: Get username & password OR Change password
		if(isedit)																			//Change password mode
		{
			puts("Now enter the old password:");
			get_string(buffer1, "Null", false);												//false means Cannot accept blank for retaining original password
			if(!strcmp(buffer1, ((Userpswd*)pold)->pswd))									//If old password is correct, allow input a new one
			{
				userpswd_get_verify(((Userpswd*)pnew)->pswd, true);							//Get, validate, and verify the password
				
				strcpy(((Userpswd*)pnew)->prkey_P, ((Userpswd*)pold)->prkey_P);				//After successfully changed password, copy prkey and username
				strcpy(((Userpswd*)pnew)->user, ((Userpswd*)pold)->user);
			}
			else
			{
				printf("Old password is incorrect!");
				strcpy(((Userpswd*)pnew)->prkey_P, "Failed");								//Change the new one's prkey to Failed to notify failure(as flag)
			}
		}
		else																				//Get username & password mode
		{
			userpswd_get_verify(((Userpswd*)pnew)->user, false);							//Get and validate the username
			userpswd_get_verify(((Userpswd*)pnew)->pswd, true);								//Get, validate, and verify the password
		}
		
		break;
	}
	return;
}

bool retrieve_data(int dtype, void* pdata, int field, char string[MAXLEN], int* pnum)
{
	bool isstring = true;
	int total_amount = 0;																	//Transaction: quantity * price_then
	switch(dtype)
	{
		case MERCHDT:
		switch(field)
		{
			case 0:											//prkey_M
			isstring = true;
			upper_case(string, ((Merch*)pdata)->prkey_M);
			break;
			case 1:											//category
			isstring = false;
			*pnum = ((Merch*)pdata)->category;
			break;
			case 2:											//model
			isstring = true;
			upper_case(string, ((Merch*)pdata)->model);
			break;
			case 3:											//brand
			isstring = true;
			upper_case(string, ((Merch*)pdata)->brand);
			break;
			case 4:
			isstring = false;								//price
			*pnum = ((Merch*)pdata)->price;
			break;
			case 5:											//cost
			isstring = false;
			*pnum = ((Merch*)pdata)->cost;
			break;
			case 6:											//remain
			isstring = false;
			*pnum = ((Merch*)pdata)->remain;
			break;
			default:
			isstring = false;
			*pnum = BLANK;
			break;
		}
		break;
		
		case PERSONDT:
		switch(field)
		{
			case 0:											//prkey_P
			isstring = true;
			upper_case(string, ((Person*)pdata)->prkey_P);
			break;
			case 1:											//level
			isstring = false;
			*pnum = ((Person*)pdata)->level;
			break;
			case 2:											//surname
			isstring = true;
			upper_case(string, ((Person*)pdata)->surname);
			break;
			case 3:											//firstname
			isstring = true;
			upper_case(string, ((Person*)pdata)->firstname);
			break;
			case 4:
			isstring = false;								//salary
			*pnum = ((Person*)pdata)->salary;
			break;
			case 5:											//sales
			isstring = false;
			*pnum = ((Person*)pdata)->sales;
			break;
			case 6:											//ismale
			isstring = false;
			*pnum = ((Person*)pdata)->ismale;
			break;
			default:
			isstring = false;
			*pnum = BLANK;
			break;
		}
		break;
		
		case TRANSDT:
		assert(MAXTRANS > ((Transaction*)pdata)->different);
		switch(field)
		{
			case 0:											//prkey_T
			isstring = true;
			upper_case(string, ((Transaction*)pdata)->prkey_T);
			break;
			case 1:											//type
			isstring = false;
			*pnum = ((Transaction*)pdata)->type;
			break;
			case 2:											//ttime
			isstring = false;
			*pnum = ((Transaction*)pdata)->ttime;
			break;
			case 3:											//kind
			isstring = false;
			*pnum = ((Transaction*)pdata)->different;
			break;
			case 4:
			isstring = false;								//amount
			for(int i = 0; i < ((Transaction*)pdata)->different; i++)
				total_amount += ((Transaction*)pdata)->price_then[i] * ((Transaction*)pdata)->quantity[i];
			*pnum = total_amount;
			break;
			case 5:											//person
			isstring = true;
			upper_case(string, ((Transaction*)pdata)->prkey_P);
			break;
			default:
			isstring = false;
			*pnum = BLANK;
			break;
		}
		break;
		
		case USRPSWDT:
		switch(field)
		{
			case 0:											//prkey_P
			isstring = true;
			strcpy(string, ((Userpswd*)pdata)->prkey_P);
			break;
			case 1:											//Username
			isstring = true;
			strcpy(string, ((Userpswd*)pdata)->user);
			break;
			case 2:											//Password
			isstring = true;
			strcpy(string, ((Userpswd*)pdata)->pswd);
			break;
		}
	}
	
	return isstring;
}
													/* Compare functions */

int compare_data(int dtype, void* pdata1, void* pdata2)
{//Level 4:Compare 2 record

	/*Mask configuration:
					64			32			16			8			4			2			1
					int			int			int			char[]		char[]		int			char[]
Merchmask 			remain		cost		price		brand		model		category	prkey_M	

					short		int			int			char[]		char[]		int			char[]	
Peoplemask		 	ismale		sales		salary		firstname	surname		level		prkey_P

								char[]		long long 	int			time_t		int			char[]
Transmask 						prkey_P		amount		kind		date		type		prkey_T*/
	int answer = 0;
	bool isstring = true;
	int field = 0;
	int field_i = 0;													//Re-sorting field
	char string_1[MAXLEN] = "";
	char string_2[MAXLEN] = "";
	int num_1 = 0;
	int num_2 = 0;
	
	switch(dtype)														//Decide which field to sort
	{
		case MERCHDT:
		field = Merchsort;
		break;
		case PERSONDT:
		field = Personsort;
		break;
		case TRANSDT:
		field = Transsort;
		break;
	}
	
	do
	{
		isstring = retrieve_data(dtype, pdata1, field, string_1, &num_1);	//Retrieve data of given field
		retrieve_data(dtype, pdata2, field, string_2, &num_2);
	
		if(isstring)
			answer = strcmp(string_1, string_2);
		else
			answer = num_1 > num_2? 1 : (num_1 < num_2? -1 : 0);
		if(Searchisza)														//Reverse
			answer = -answer;
			
		field = field_i++;													//Set field from 0, and incre to re-sort
	}while(answer == 0 && field < 10);
	
	return answer;
}
											
												/* Username and password functions */
												
												
int authenticate(const char* username, const char* password, Person* result)
{//A:Compare username and password in file
	/*Return value:	-4: Username not found	-3: Wrong password	-2: User&pass correct but no such account
	Format of the result: level|prkey|surname|firstname	*/
	Node* pFirst = NULL;
	linklist(&pFirst, USRPSWDT);											//Create the username and password linklist
	Node* pCurrent = pFirst;												//Current node is First
	char prkey_found[MAXLEN] = "";
	bool found = false;
	
	if(!pFirst)																//pFirst is NULL, i.e. file is empty
	{
		release_node(pFirst);
		pFirst = NULL;
		return -4;															//Return: Username not found
	}
		
	while(pCurrent)
	{
		if(strcmp(((Userpswd*)(pCurrent->pDT))->user, username))			//Username not matched
			pCurrent = pCurrent->pright;									//Go to next node
		else																//Username matched
		{
			if(strcmp(((Userpswd*)(pCurrent->pDT))->pswd, password))		//Password not matched
			{
				release_node(pFirst);
				pFirst = NULL;
				return -3;													//Return: Wrong password
			}
			else															//Password also matched
			{
				strcpy(prkey_found, ((Userpswd*)(pCurrent->pDT))->prkey_P);	//Fetch the prkey of the person
				found = true;
				break;
			}
		}
	}
	release_node(pFirst);													//Free all the memory of the link list
	pFirst = NULL;
	
	if(found)																//Both username and password are correct
	{
		linklist(&pFirst, PERSONDT);										//Create link list of Person to get the levels of person
		pCurrent = pFirst;
		assert(pFirst);														//There should be data in Person file
		
		while(pCurrent)
		{
			if(strcmp(((Person*)(pCurrent->pDT))->prkey_P, prkey_found))	//Primary key not matched
				pCurrent = pCurrent->pright;								//Go to next node
			else															//Primary key matched
			{
				*result = *(Person*)(pCurrent->pDT);						//Copy the person										
				break;														//e.g. "0|P00002|Wong|Peter"
			}
		}
	}
	else																	//All usernames do not match
	{
		release_node(pFirst);				 								//Return: Username not found
		pFirst = NULL;	
		return -4;
	}		
	release_node(pFirst);
	pFirst = NULL;
	return result->level;													//If authenicates successfully, return level
}

int change_password(const char* prkey_P)
{
	Userpswd original;
	bool found = false;
	bool success = false;
	Node* pFirst = NULL;
	linklist(&pFirst, USRPSWDT);
	Node* pCurrent = pFirst;
	while(pCurrent)
	{
		if(strcmp(prkey_P, ((Userpswd*)pCurrent->pDT)->prkey_P) == 0)
		{
			original = *((Userpswd*)pCurrent->pDT);
			found = true;
			break;
		}
		pCurrent = pCurrent->pright;
	}
	if(found == false)
	{
		DebugPrintf("Cannot find this person. Something goes wrong.");
		return -1;
	}
	return edit_data(USRPSWDT, &original, false, NULL, NULL);									
}

void add_user(const char prkey_P[])
{//B:Add a new user and password combination
	Userpswd TempData;															//To store the updated record
	FILE* pfile = NULL;
	
	get_data(USRPSWDT, &TempData, NULL, false);									//Get username and password
	strcpy(TempData.prkey_P, prkey_P);											//Copy the corresponding primary key to the new record
	pfile = open_file(file_number(USRPSWDT), "a+");								//Open user&pswd file
	write_data(USRPSWDT, &TempData ,pfile);										//Write to the file
	fclose(pfile);
	pfile = NULL;
	return;
}

void userpswd_get_verify(char* userpswd, bool ispswd)
{//B:Get and verify the username/password
	char buffer1[MAXLEN];
	char buffer2[MAXLEN];
	
	if(ispswd)																		//get and verify password
	{
		puts("Now enter the new password:");
		puts("Minimum length is 6. Blanks and | are not acceptable.");
		apicolor(15);
		do
		{
			do
			{
				get_string(buffer1, "Null", false);
				if(!userpswd_validation(buffer1, true))
					puts("Not valid. Try again:");		
			}while(!userpswd_validation(buffer1, true));							//If not valid, re-create
			
			puts("Enter the password again to verify:");
			get_string(buffer2, "Null", false);	
			
			if(strcmp(buffer1, buffer2))											//Not the same
				puts("Not the same. Please re-create the password:");				//If not the same, loop again	
		}while(strcmp(buffer1, buffer2));			
	}
	else																			//username
	{
		puts("Create your new username(cannot be changed after):");														//Light green
		puts("Minimum length is 3. Blanks and \'|\' are not acceptable.");
		apicolor(15);
		do
		{
			get_string(buffer1, "Null", false);
			if(!userpswd_validation(buffer1, false))
				puts("Username is used or format is wrong. Try again:");	
		}while(!userpswd_validation(buffer1, false));								//If not valid, re-create
	}
	
	strcpy(userpswd, buffer1);
	return;
}

bool userpswd_validation(const char* userpswd, bool ispswd)
{//B:Validate the username and password
	bool isvalid = true;
	Node* pFirst = NULL;
	Node* pCurrent = NULL;
	
	if(strlen(userpswd) < 3)												//At least 3 characters
		return false;
	if(ispswd && strlen(userpswd) < 6)										//For password, at least 6 characters
		return false;	
	for(int i = 0; i < strlen(userpswd); i++)
	{
		if(!isprint(userpswd[i]) || isblank(userpswd[i]) || userpswd[i] == '|')	//Check every character: printable, not blank and not '|'
			return false;
	}
	
	if(!ispswd)																//Check username not duplicated
	{
		linklist(&pFirst, USRPSWDT);
		pCurrent = pFirst;
		
		while(pCurrent)
		{
			if(strcmp(((Userpswd*)(pCurrent->pDT))->user, userpswd))		//Username not the same
				pCurrent = pCurrent->pright;								//Go to next node
			else
			{																//Username duplicated
				isvalid = false;
				break;
			}
		}
		release_link(pFirst);
	}
	
	return isvalid;
}


													/* Display functions */

void list_data(int dtype, void* pdata)
{//A:Display a record of given data type
	int coor[2];													//Coordinate of current position
	static struct tm * ptime;										//Transaction:transaction time
	long long total_amount = 0;										//Transaction:sum of price * quantity
	if(!pdata)														//For safety
		return;
	apigetxy(coor);													//Get x(coor[0]) and y(coor[1]) coordinates
	puts("                                                                                ");
	switch(dtype)
	{
		case MERCHDT:
		apisetxy(merchindent[0], coor[1]);							//move to (alignment x-coordinate,current y-coordinate)
		puts(((Merch*)pdata)->prkey_M);								//print corresponding record
		apisetxy(coor[0], coor[1]);
		apisetxy(merchindent[1],coor[1]);
		switch(((Merch*)pdata)->category)
		{
			case 0: case 1: case 2: case 3:
				puts(merchType[((Merch*)pdata)->category]);
				break;
			default:
				DebugPrintf("I shouldn't get here!");				//Error message
		}
		apisetxy(merchindent[2],coor[1]);
		puts(((Merch*)pdata)->model);
		apisetxy(merchindent[3],coor[1]);
		printf(((Merch*)pdata)->brand);
		apisetxy(merchindent[4] + strlen(merchtitle[4]) - 6,coor[1]);
		printf("%6d", ((Merch*)pdata)->price);
		apisetxy(merchindent[5] + strlen(merchtitle[5]) - 6,coor[1]);
		printf("%6d", ((Merch*)pdata)->cost);
		apisetxy(merchindent[6] + strlen(merchtitle[6]) - 6,coor[1]);
		printf("%6d\n", ((Merch*)pdata)->remain);
		break;
		
		case PERSONDT:
		apisetxy(personindent[0], coor[1]);							//keep y-coordinate unchanged while aligning x-coordinate
		puts(((Person*)pdata)->prkey_P);							//print corresponding record
		apisetxy(personindent[1],coor[1]);
		switch(((Person*)pdata)->level)
		{
			case 0: case 1:
				puts(personType[((Person*)pdata)->level]);
				break;
			case -1:
				printf("Administrator");
				break;
			default:
				DebugPrintf("I shouldn't get here!");				//Error message
		}
		apisetxy(personindent[2],coor[1]);
		puts(((Person*)pdata)->surname);
		apisetxy(personindent[3],coor[1]);
		printf(((Person*)pdata)->firstname);
		apisetxy(personindent[4] + strlen(persontitle[4]) - 6,coor[1]);
		printf("%6d", ((Person*)pdata)->salary);
		apisetxy(personindent[5] + strlen(persontitle[5]) - 6,coor[1]);
		printf("%6d", ((Person*)pdata)->sales);
		apisetxy(personindent[6],coor[1]);
		printf("%s\n", ((Person*)pdata)->ismale? "Male": "Female");
		break;
		
		case TRANSDT:
		assert(((Transaction*)pdata)->different);					//At least one kind of merchandize processed
		apisetxy(transindent[0], coor[1]);							//keep y-coordinate unchanged while aligning x-coordinate
		puts(((Transaction*)pdata)->prkey_T);
		apisetxy(transindent[1],coor[1]);
		switch(((Transaction*)pdata)->type)
		{
			case 0: case 1:
				puts(transType[((Transaction*)pdata)->type]);
				break;
			default:
				DebugPrintf("I shouldn't get here!");				//Error message
		}
		apisetxy(transindent[2],coor[1]);
		ptime = localtime(&((Transaction*)pdata)->ttime);
		printf("%4d/%02d/%02d %02d:%02d", 1900 + ptime->tm_year, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min);
		apisetxy(transindent[3] + strlen(transtitle[3]) - 6,coor[1]);
		printf("%6d", ((Transaction*)pdata)->different);
		apisetxy(transindent[4] + strlen(transtitle[4]) - 7,coor[1]);
		for(int i = 0; i < ((Transaction*)pdata)->different; i++)
			total_amount += ((Transaction*)pdata)->price_then[i] * ((Transaction*)pdata)->quantity[i];
		printf("%7lld", total_amount);
		apisetxy(transindent[5],coor[1]);
		puts(((Transaction*)pdata)->prkey_P);
		break;
	}
	return;
}

void display_merch(Merch* pmerch)
{
	int coor[2];													//Coordinate of current position
	if(!pmerch)														//For safety
		return;
	apigetxy(coor);													//Get x(coor[0]) and y(coor[1]) coordinates
	for(int i = 0; i < 3; i++)
	{
		apisetxy(coor[0], coor[1] + i);
		puts("                     ");								//20 x ' '
	}
		
	apisetxy(coor[0], coor[1]);
	puts(pmerch->model);
	apisetxy(coor[0], coor[1] + 1);
	printf(pmerch->brand);
	apisetxy(coor[0], coor[1] + 2);
	printf("$%7d", pmerch->price);
	apisetxy(coor[0] + 10,coor[1] + 2);
	printf("Remain:%4d", pmerch->remain);
}

void list_bought(Merch* pmerch, int qnty)
{
	int coor[2];													//Coordinate of current position
	if(!pmerch)														//For safety
		return;
	apigetxy(coor);													//Get x(coor[0]) and y(coor[1]) coordinates
	puts("                       ");
	apisetxy(56, coor[1]);										//move to (alignment x-coordinate,current y-coordinate)
	puts(pmerch->model);											//print corresponding record
	apisetxy(65, coor[1]);
	printf("%4d", qnty);
	apisetxy(70, coor[1]);
	printf("%6d", pmerch->price * qnty);
}

void list_field(int dtype)
{//A:Display the field names of given data type
	int coor[2];													//Coordinate of current position						
	apigetxy(coor);													//Get x(coor[0]) and y(coor[1]) coordinates
	apicolor(15);
	switch(dtype)
	{
		case MERCHDT:
		for(int i = 0; i < 7; i++)									//Display field name
		{
			apisetxy(merchindent[i], coor[1]);						//Move to alignment position
			puts(merchtitle[i]);									//And print field name
		}
		break;
		
		case PERSONDT:
		for(int i = 0; i < 7; i++)									//Display field name
		{
			apisetxy(personindent[i], coor[1]);						//Move to alignment position
			puts(persontitle[i]);									//And print field name
		}
		break;
		
		case TRANSDT:
		for(int i = 0; i < 6; i++)
		{
			apisetxy(transindent[i], coor[1]);						//Move to alignment position
			puts(transtitle[i]);									//And print field name
		}
	}
	
	return;
}

void apisetxy(int x,int y)
{//A:Use gotoxy independent of conio.h since many versons of it may not include this function
    COORD pos;  
    pos.X=x;
    pos.Y=y;  
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pos);  
} 

void apicolor(int color) 
{//A:Change text color
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),color);  
}

void apigetxy(int coor[2])
{//B:Get the current xy position 
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = {0, 0}; 
    CONSOLE_SCREEN_BUFFER_INFO csbi;
 
    if (GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        coor[0] = csbi.dwCursorPosition.X;
		coor[1] = csbi.dwCursorPosition.Y;
    }
    return;
}

												/* Report functions */
												
void print_records(int dtype, Node* pRoot, int dtcount, bool issearch)
{
	char fileName[MAXLEN] = "";	
	char buffer[MAXLEN] = "";
	time_t raw_time = 0;
	struct tm * ptime;
	time(&raw_time);																//Get current time
	ptime = localtime(&raw_time);													//Change it into a struct form
	
	if(dtype == MERCHDT)
		strcpy(buffer, "Merch");
	else if(dtype == PERSONDT)
		strcpy(buffer, "People");
	else if(dtype == TRANSDT)
		strcpy(buffer, "Event");
	sprintf(fileName, "./Reports/%s%d%02d%02d%02d%02d.txt", buffer, 1900 + ptime->tm_year, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min);
	
	FILE* pFile = fopen(fileName, "w");
	if(!pFile)
	{
		fprintf(stderr,"Open file %s failed.Terminate program", fileName);
		exit(1);
	}
		
	
	switch(dtype)
	{
		case MERCHDT:
		for(int j = 0; j < merchindent[0]; j++)
			fputs(" ", pFile);
		fputs(merchtitle[0], pFile);
		for(int i = 1; i < 7; i++)									//Display field name
		{
			for(int j = 0; j < merchindent[i] - merchindent[i - 1] - strlen(merchtitle[i - 1]); j++)
				fputs(" ", pFile);
			fputs(merchtitle[i], pFile);
		}
		break;
		
		case PERSONDT:
		for(int j = 0; j < personindent[0]; j++)
			fputs(" ", pFile);
		fputs(persontitle[0], pFile);
		for(int i = 1; i < 7; i++)									//Display field name
		{
			for(int j = 0; j < personindent[i] - personindent[i - 1] - strlen(persontitle[i - 1]); j++)
				fputs(" ", pFile);
			fputs(persontitle[i], pFile);
		}
		break;
		
		case TRANSDT:
		for(int j = 0; j < transindent[0]; j++)
			fputs(" ", pFile);
		fputs(transtitle[0], pFile);
		for(int i = 1; i < 6; i++)									//Display field name
		{
			for(int j = 0; j < transindent[i] - transindent[i - 1] - strlen(transtitle[i - 1]); j++)
				fputs(" ", pFile);
			fputs(transtitle[i], pFile);
		}
		break;
	}
	fputs("\n\n", pFile);
	for(int i = 0; i < dtcount; i++)
		print_single(dtype, retrieve_node(pRoot, i, issearch)->pDT, pFile);
	
	fclose(pFile);
	return;
}

void print_single(int dtype, void* pdata, FILE* pFile)
{
	static struct tm * ptime;										//Transaction:transaction time
	long long total_amount = 0;										//Transaction:sum of price * quantity
	switch(dtype)
	{
		case MERCHDT:

		for(int j = 0; j < merchindent[0]; j++)
			fputs(" ", pFile);
		fputs(((Merch*)pdata)->prkey_M, pFile);						//print corresponding record
		for(int j = 0; j < merchindent[1] - merchindent[0] - strlen(((Merch*)pdata)->prkey_M); j++)
			fputs(" ", pFile);
		switch(((Merch*)pdata)->category)
		{
			case 0: case 1: case 2: case 3:
				fputs(merchType[((Merch*)pdata)->category], pFile);
				break;
			default:
				fputs("I shouldn't get here!", pFile);				//Error message
		}
		for(int j = 0; j < merchindent[2] - merchindent[1] - strlen(merchType[((Merch*)pdata)->category]); j++)
			fputs(" ", pFile);
		fputs(((Merch*)pdata)->model, pFile);
		for(int j = 0; j < merchindent[3] - merchindent[2] - strlen(((Merch*)pdata)->model); j++)
			fputs(" ", pFile);
		fputs(((Merch*)pdata)->brand, pFile);
		for(int j = 0; j < merchindent[4] - merchindent[3] - strlen(((Merch*)pdata)->brand) + strlen(merchtitle[4]) - 6; j++)
			fputs(" ", pFile);
		fprintf(pFile, "%6d", ((Merch*)pdata)->price);
		for(int j = 0; j < merchindent[5] - merchindent[4] - strlen(merchtitle[4]) + strlen(merchtitle[5]) - 6; j++)
			fputs(" ", pFile);
		fprintf(pFile, "%6d", ((Merch*)pdata)->cost);
		for(int j = 0; j < merchindent[6] - merchindent[5] - strlen(merchtitle[5]) + strlen(merchtitle[6]) - 6; j++)
			fputs(" ", pFile);
		fprintf(pFile, "%6d\n", ((Merch*)pdata)->remain);
		break;
		
		case PERSONDT:

		for(int j = 0; j < personindent[0]; j++)
			fputs(" ", pFile);
		fputs(((Person*)pdata)->prkey_P, pFile);								//print corresponding record
		for(int j = 0; j < personindent[1] - personindent[0] - strlen(((Person*)pdata)->prkey_P); j++)
			fputs(" ", pFile);
		switch(((Person*)pdata)->level)
		{
			case 0: case 1:
				fputs(personType[((Person*)pdata)->level], pFile);
				break;
			case -1:
				fputs("Administrator", pFile);
				break;
			default:
				fputs("I shouldn't get here!", pFile);				//Error message
		}
		if(((Person*)pdata)->level >= 0)
			for(int j = 0; j < personindent[2] - personindent[1] - strlen(personType[((Person*)pdata)->level]); j++)
				fputs(" ", pFile);
		else
			for(int j = 0; j < personindent[2] - personindent[1] - strlen("Administrator"); j++)
				fputs(" ", pFile);
		fputs(((Person*)pdata)->surname, pFile);
		for(int j = 0; j < personindent[3] - personindent[2] - strlen(((Person*)pdata)->surname); j++)
			fputs(" ", pFile);
		fputs(((Person*)pdata)->firstname, pFile);
		for(int j = 0; j < personindent[4] - personindent[3] - strlen(((Person*)pdata)->firstname) + strlen(persontitle[4]) - 6; j++)
			fputs(" ", pFile);
		fprintf(pFile, "%6d", ((Person*)pdata)->salary);
		for(int j = 0; j < personindent[5] - personindent[4] - strlen(persontitle[4]) + strlen(persontitle[5]) - 6; j++)
			fputs(" ", pFile);
		fprintf(pFile, "%6d", ((Person*)pdata)->sales);
		for(int j = 0; j < personindent[6] - personindent[5] - strlen(persontitle[5]); j++)
			fputs(" ", pFile);
		fprintf(pFile, "%s\n", ((Person*)pdata)->ismale? "Male": "Female");
		break;
		
		case TRANSDT:
		
		assert(((Transaction*)pdata)->different);					//At least one kind of merchandize processed
		for(int j = 0; j < transindent[0]; j++)
			fputs(" ", pFile);
		fputs(((Transaction*)pdata)->prkey_T, pFile);								//print corresponding record
		for(int j = 0; j < transindent[1] - transindent[0] - strlen(((Transaction*)pdata)->prkey_T); j++)
			fputs(" ", pFile);
		switch(((Transaction*)pdata)->type)
		{
			case 0: case 1:
				fputs(transType[((Transaction*)pdata)->type], pFile);
				break;
			default:
				fputs("I shouldn't get here!", pFile);				//Error message
		}
		for(int j = 0; j < transindent[2] - transindent[1] - strlen(transType[((Transaction*)pdata)->type]); j++)
			fputs(" ", pFile);
		ptime = localtime(&((Transaction*)pdata)->ttime);
		fprintf(pFile, "%4d/%02d/%02d %02d:%02d", 1900 + ptime->tm_year, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min);
		for(int j = 0; j < transindent[3] - transindent[2] - 16 + strlen(transtitle[3]) - 6; j++)	//strlen of customized time string
			fputs(" ", pFile);
		fprintf(pFile, "%6d", ((Transaction*)pdata)->different);
		for(int j = 0; j < transindent[4] - transindent[3] - strlen(transtitle[3]) + strlen(transtitle[4]) - 7; j++)
			fputs(" ", pFile);
		for(int i = 0; i < ((Transaction*)pdata)->different; i++)
			total_amount += ((Transaction*)pdata)->price_then[i] * ((Transaction*)pdata)->quantity[i];
		fprintf(pFile, "%7lld", total_amount);
		for(int j = 0; j < transindent[5] - transindent[4] - strlen(transtitle[4]); j++)
			fputs(" ", pFile);
		fprintf(pFile, "%s\n", ((Transaction*)pdata)->prkey_P);
		break;
	}
	return;
}
												
void print_receipt(Transaction* pTrans, const char* staff_name, char model[MAXTRANS][MAXLEN], char brand[MAXTRANS][MAXLEN], long long grand)
{
	char fileName[MAXLEN] = "";
	time_t raw_time = 0;
	struct tm * ptime;
	time(&raw_time);
	ptime = localtime(&raw_time);
	sprintf(fileName, "./Receipt/Receipt%d%02d%02d%02d%02d.txt", 1900 + ptime->tm_year, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min);
	
	FILE* pFile = fopen(fileName, "w");
	if(!pFile)
	{
		fprintf(stderr,"Open file %s failed.Terminate program", fileName);
		exit(1);
	}
	//General information
	for(int j = 0; j < 2; j++)
		fputs(" ", pFile);
	fprintf(pFile, "Transaction: %s", pTrans->prkey_T);
	for(int j = 0; j < 27 - 2 - 13 - strlen(pTrans->prkey_T); j++)
		fputs(" ", pFile);
	fputs("Action: ", pFile);
	switch(pTrans->type)
	{
		case 0: case 1:
			fputs(transType[pTrans->type], pFile);
			break;
		default:
			fputs("I shouldn't get here!", pFile);				//Error message
	}
	for(int j = 0; j < 49 - 27 - 8 - strlen(transType[pTrans->type]); j++)
		fputs(" ", pFile);
	fputs("Date&Time: ", pFile);
	ptime = localtime(&(pTrans->ttime));						//Reassign ptime
	fprintf(pFile, "%4d/%02d/%02d %02d:%02d\n", 1900 + ptime->tm_year, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min);
	
	for(int j = 0; j < 2; j++)
		fputs(" ", pFile);
	fprintf(pFile, "Staff Key: %s", pTrans->prkey_P);
	for(int j = 0; j < 27 - 2 - 11 - strlen(pTrans->prkey_P); j++)
		fputs(" ", pFile);
	fprintf(pFile, "Staff Name: %s", staff_name);
	for(int j = 0; j < 54 - 27 - 12 - strlen(staff_name); j++)
		fputs(" ", pFile);
	fprintf(pFile, "Grand Total: %9lld\n\n", grand);
	
	//Field Name
	for(int j = 0; j < 4; j++)
		fputs(" ", pFile);
	fputs("#", pFile);
	for(int j = 0; j < 6 - 4 - 1; j++)
		fputs(" ", pFile);
	fputs("PrdctKey", pFile);
	for(int j = 0; j < 16 - 6 - 8; j++)
		fputs(" ", pFile);
	fputs("Name", pFile);
	for(int j = 0; j < 35 - 16 - 4; j++)
		fputs(" ", pFile);
	fputs("Brand", pFile);
	for(int j = 0; j < 54 - 35 - 5; j++)
		fputs(" ", pFile);
	fputs("SoldPrice", pFile);
	for(int j = 0; j < 64 - 54 - 9; j++)
		fputs(" ", pFile);
	fputs("Qnty", pFile);
	for(int j = 0; j < 71 - 64 - 4; j++)
		fputs(" ", pFile);
	fputs("Total\n", pFile);
	
	//Print each item
	for(int i = 0; i < pTrans->different; i++)
	{
		for(int j = 0; j < 2; j++)
			fputs(" ", pFile);
		fprintf(pFile, "%03d", i);
		for(int j = 0; j < 6 - 2 - 3; j++)
			fputs(" ", pFile);
		fputs(pTrans->prkey_M[i], pFile);
		for(int j = 0; j < 16 - 6 - strlen(pTrans->prkey_M[i]); j++)
			fputs(" ", pFile);
		fputs(model[i], pFile);
		for(int j = 0; j < 35 - 16 - strlen(model[i]); j++)
			fputs(" ", pFile);
		fputs(brand[i], pFile);
		for(int j = 0; j < 54 - 35 - strlen(brand[i]); j++)
			fputs(" ", pFile);
		fprintf(pFile, "%9d", pTrans->price_then[i]);
		for(int j = 0; j < 64 - 54 - 9; j++)
			fputs(" ", pFile);
		fprintf(pFile, "%4d", pTrans->quantity[i]);
		for(int j = 0; j < 70 - 64 - 4; j++)
			fputs(" ", pFile);
		fprintf(pFile, "%6lld\n", (long long)pTrans->price_then[i] * pTrans->quantity[i]);
	}
	fclose(pFile);
	return;
}
