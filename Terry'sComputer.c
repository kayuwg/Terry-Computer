/*Testing result and instructions:
1. apicolor()
0 - black		1 - dark blue	2 - green
3 - bluegreen	4 - dark red	5 - dark purple
6 - yellowgreen	7 - pale grey	8 - grey
9 - blue		10- light green	11- light cyan
12- red			13- purple		14- yellow
15- white		T = 256
n/16 = background color; n%16 = font color

2. ANSCII Code reference
Backspace 8   Enter 13   
getch() == 224
Up 72   Down 80	  Left 75   Right 77   PageUp 73   PageDown 81

3. Mask configuration:
Since unsigned char, 1 Byte i.e., 8 bits are occupied.
					64			32			16			8			4			2			1
					int			int			int			char[]		char[]		int			char[]
Merchmask 			remain		cost		price		brand		model		category	prkey_M	

					short		int			int			char[]		char[]		int			char[]	
Peoplemask		 	ismale		sales		salary		firstname	surname		level		prkey_P

								char[]		long long 	int			time_t		int			char[]
Transmask 						prkey_P		amount		kind		date		type		prkey_T
***********************/

#include <conio.h>
#include "Foruse.c"


#undef DEBUGPRINT												//If defined, enable output debug message 


typedef struct Ctrlc											//Choice control in an interface
{
	int verc;													//Vertical n th choice chosen
	int horc;													//Horizontal n th choice chosen
	int page;
} Ctrlc;

typedef struct Ctrls											//Choice control in an interface
{
	int section;
	int previous;
	Ctrlc current[4];
} Ctrls;

/* Global variable */
Person current_loginer;											//The logged-in account
extern unsigned short Merchmask;								//Masks for searching
extern unsigned short Personmask;
extern unsigned short Transmask;
extern short Merchsort;
extern short Personsort;
extern short Transsort;
extern bool Searchisza;
const int ITEMAX = 18;											//Maximum 15 items in a page


/* Supporting function */
void initialize_search(void);									//Initialize all global variables about searching
void center_align(char pstr[MAXLEN], int length);				//Fill the string to the given length to achieve center alignment
int message_box(int type,int color, const char* format, ...);	//Message box
void select_quantity(Node** ppList, Merch* pmerch, int* pdiff);	//A message box that ask for the quantity of the selected item
void print_color(const char* str);								//Print color boxes

/* Interface function of some interfaces that can be reused */
void sort_detect(int dtype);
void sort_fresh(Ctrlc* sortc, int dtype);
void search_detect(int dtype);
void search_fresh(Ctrlc* searchc, int dtype, unsigned short mask); 

/* Interface function prototypes */
//For each interface, xxx_detect function is to detect real-time keyboard input, and xxx_fresh is to display once refreshed.
void startup_detect(void);
void startup_fresh(int startupc);

void login_detect(void);
void login_fresh(Ctrlc* loginc, char* username, int pswd);

void staff_detect(void);
void staff_fresh(int staffc);

void manager_detect(void);
void manager_fresh(int managerc);

int  database_detect(int dtype);
void database_fresh(int dtype, Ctrlc* handlec, Node* pRoot, int dtcount, bool isdown, bool issearch);

int  transaction_detect(void);
void transaction_fresh(Ctrlc* handlec, Node* pRoot, int dtcount, bool isdown, bool issearch);

void detail_detect(Transaction* pTrans);
void detail_fresh(Ctrlc* detailc, Transaction* pTrans, char* staff_name, char model[MAXTRANS][MAXLEN], char brand[MAXTRANS][MAXLEN], long long grand);

int order_detect(void);
void order_fresh(Ctrls* orderc, Node* pfRoot, int max_page[4], int dimension[4][2], int mercount, Node* pList, int different, long long grand_total);


int main(void)
{
	system("chcp 437");
	initialize_files();
	startup_detect();
	apicolor(15);
	system("cls");
	apisetxy(0,20);
	return 0;
}


/* Supporting function */

void initialize_search(void)
{
	Merchmask = 0x0;							//Masks for searching
	Personmask = 0x0;
	Transmask = 0x0;
	for(int i = 0; i < 10; i++)
		strcpy(search_content[i], ""); 
}

void center_align(char pstr[MAXLEN], int length)
{
	int now_length = strlen(pstr);
	char buf[MAXLEN] = "";
	while(now_length + 2 <= length)				//While the wanted length and the length now differs by 2 or more,
	{											//Add 1 blank at both sides
		strcpy(buf, " ");
		strcat(buf, pstr);
		strcat(buf, " ");
		strcpy(pstr, buf);
		now_length += 2;
	}
	if(now_length + 1 == length)				//If still differ by 1, add blank in front of the string
	{
		strcpy(buf, " ");
		strcat(buf, pstr);
		strcpy(pstr, buf);
	}
		
	return;
}

int message_box(int type, int color, const char* format, ...)
{
	int messagec = 0;
	int c1 = 0, c2 = 0;
	va_list arg_ptr;							//stdarg.h
	va_start(arg_ptr, format);					//various variables after the last named variable 'format'
	
	
	apicolor(16 * color + 15);					//Background color, white font
	apisetxy(17,6);
	for(int j = 0; j < 40; j++)
		printf(" ");
	apisetxy(17,7);
	printf("  Message");
	for(int j = 0; j < 31; j++)
		printf(" ");
	apicolor(112);								//Pale grey background, black font
	for(int i = 8; i <= 15; i++)
	{
		apisetxy(17,i);
		for(int j = 0; j < 40; j++)
			printf(" ");
	}
	apisetxy(19, 10);
	vprintf(format, arg_ptr);					//print using given format and variables
	va_end(arg_ptr);
	
	do
	{
		apisetxy(17,15);
		for(int j = 0; j < 40; j++)
			printf(" ");						//Cover the original "^"s
		switch(type)
		{
			case 0:								//One-choice type: OK button only
			apicolor(112 + color);
			apisetxy(35, 14);
			printf("OK");
			apisetxy(35,15);
			printf("^^");
			break;
			case 1:								//Two-choice type: No and Yes
			apicolor(messagec == 0? (112 + color): 112);	//Change color if chosen 
			apisetxy(29,14);
			printf("No");
			apicolor(messagec == 1? (112 + color): 112);
			apisetxy(42,14);
			printf("Yes");
			apicolor(112 + color);
			if(messagec == 0)					//If chosen, add "^^" at the bottom to notify
			{
				apisetxy(29, 15);
				printf("^^");
			}
			else
			{
				apisetxy(42, 15);
				printf("^^^"); 
			}
			break;
			case 2:								//Three-chioce type: Yes, No, and Cancel
			apicolor(messagec == 0? (112 + color):112);
			apisetxy(25,14);
			printf("Yes");
			apicolor(messagec == 1? (112 + color):112);
			apisetxy(35,14);
			printf("No");
			apicolor(messagec == 2? (112 + color):112);
			apisetxy(45,14);
			printf("Cancel");
			switch(messagec)
			{
				case 0:
				apisetxy(25,15);
				printf("^^^");
				break;
				case 1:
				apisetxy(35, 15);
				printf("^^");
				break;
				case 2:
				apisetxy(45, 15);
				printf("^^^^^^");
			}
			break;
			
		}
		
		c1 = getch();
		if(c1 == 224)							//Special character pressed
		{
			c2 = getch();
			switch(c2)
			{
				case 77://rigt pressed
				messagec++;
				break;
				case 75://left pressed
				messagec--;
				break;
				
			}
		}
		if(messagec == -1)						//Out of left boundary: choice go back from bottom
			messagec = type;
		if(messagec > type)						//Out of right boundary
			messagec %= (type + 1);
		if(c1 == 13)							//Enter pressed
		{
			apicolor(15);
			return messagec;
		}
	}while(true);
}

void select_quantity(Node** ppList, Merch* pmerch, int* pdiff)
{
	if(!pmerch)
		return;
	
	Ctrlc qntyc = 
	{
		.horc = 0,
		.verc = 0
	};
	int c1 = 0, c2 = 0;
	bool found = false;
	int quantity = find_qnty_link(*ppList, pmerch->prkey_M);	//Current bought quantity of product
	if(quantity == 0)											//Orignially no record
	{
		found = false;
		quantity = 1;
	}
	else														//Orignial record exists
		found = true;
	apicolor(16 * 1 + 15);										//Background color, white font
	apisetxy(17,6);
	for(int j = 0; j < 40; j++)
		printf(" ");
	apisetxy(17,7);
	printf("  Select quantity(¡û/¡ú)");
	for(int j = 0; j < 16; j++)
		printf(" ");
	apicolor(112);												//Pale grey background, black font
	for(int i = 8; i <= 15; i++)
	{
		apisetxy(17,i);
		for(int j = 0; j < 40; j++)
			printf(" ");
	}
	apisetxy(19, 9);
	printf("%s", pmerch->model);
	apisetxy(19, 10);
	printf("%s", pmerch->brand);
	apisetxy(19, 11);
	printf("Quantity:           Out of     %4d", pmerch->remain);
	do
	{
		apisetxy(30, 12);
		for(int j = 0; j < 4; j++)
			printf(" ");
		apisetxy(17,15);
		for(int j = 0; j < 40; j++)
			printf(" ");										//Cover the original "^"s
			
		apicolor(112);											//Display amount of price and current quantity
		apisetxy(39, 12);
		printf("Total: $%7d", quantity * pmerch->price);
		apicolor(qntyc.verc == 0? (112 + 1): 112);
		apisetxy(30, 11);
		printf("%4d", quantity);
		
		apicolor(qntyc.horc == 0? (112 + 1): 112);				//Change color if chosen 
		apisetxy(29,14);
		printf("OK");
		apicolor(qntyc.horc == 1? (112 + 1): 112);
		apisetxy(42,14);
		printf("Cancel");
		apicolor(112 + 1);
		if(qntyc.verc == 0)										//If chosen, add "^^" at the bottom to notify
		{
			for(int j = 0; j < numdig(quantity); j++)
			{
				apisetxy(33 - j, 12);
				printf("^");
			}
		}
		else if(qntyc.horc == 0 && qntyc.verc == 1)				
		{
			apisetxy(29, 15);
			printf("^^");
		}
		else if (qntyc.horc == 1 && qntyc.verc == 1)
		{
			apisetxy(42, 15);
			printf("^^^^^^"); 
		}
		apisetxy(19, 24);
		DebugPrintf("Select qnty: verc=%d, horc=%d", qntyc.verc, qntyc.horc);
		
		c1 = getch();
		if(c1 == 224)											//Special character pressed
		{
		c2 = getch();
			switch(c2)
			{
				case 80://Down
				qntyc.verc++;
				break;
				case 72://Up
				qntyc.verc--;
				break;
				case 77://rigt pressed
				if(qntyc.verc == 0)
					quantity++;
				else
					qntyc.horc++;
				break;
				case 75://left pressed
				if(qntyc.verc == 0)
					quantity--;
				else
					qntyc.horc--;
				break;
			}
		}
		if(qntyc.verc < 0)
			qntyc.verc = 1;
		if(qntyc.verc > 1)
			qntyc.verc %= 2;
		if(qntyc.horc < 0)										//Out of left boundary: choice go back from bottom
			qntyc.horc = 1;
		if(qntyc.horc > 1)										//Out of right boundary
			qntyc.horc %= 2;
		if(quantity < 0)
			quantity = 0;
		if(quantity > pmerch->remain)
			quantity = pmerch->remain;
		if(c1 == 13)											//Enter pressed
		{
			apicolor(15);
			if(qntyc.horc == 0)									//OK pressed
			{
				if(found)										//Original record exists, so just modify it
				{
					if(quantity == 0)
					{
						delete_link(ppList, pmerch->prkey_M);
						(*pdiff)--;								//Reduce number of kinds bought
						DebugPrintf("Delete.");
						DebugSystem("pause");
					}
					else
					{
						update_link(*ppList, pmerch->prkey_M, quantity);
						DebugPrintf("Update.");
						DebugSystem("pause");
					}
						
				}
				else											//Orignial record does not exist for that merch
				{
					if(quantity == 0)
						DebugSystem("pause");					//Do nothing
					else 
					{
						(*pdiff)++;								//Increase number of kinds bought
						if(*pdiff > MAXTRANS)
						{
							message_box(0, 6, "Maximum %d kinds of items!", MAXTRANS);
							(*pdiff)--;
							return;
						}
						if(!(*ppList))							//If originally no record list, then create one
						{
							*ppList = create_link(pmerch, quantity);
							DebugPrintf("Create");
							DebugSystem("pause");
						}
						else									//If originally list exists
						{
							push_link(*ppList, pmerch, quantity);		//Add record
							DebugPrintf("Push");
							DebugSystem("pause");
						}
					}
				}
			}
			else
				DebugPrintf("Cancel pressed");					//Cancel pressed, Do nothing
			return;												//As long as Enter is pressed, return
		}
	}while(true);
}



void print_color(const char* str)
{
	int coor[2];
	apigetxy(coor);												//Get current coordinate
	int initial_x = coor[0];									//Initial x
	for(int i = 0; i < strlen(str); i++)
		if(!isblank(str[i]))
		{
			apisetxy(coor[0] + i, coor[1]);
			printf(" ");
		}
	apisetxy(initial_x, coor[1] + 1);
}

/* Display function of some interfaces that can be reused */


void sort_detect(int dtype)
{
	
	int c1 = 0 , c2 = 0;
	static Ctrlc sortc =										//Record which choices are chosen
	{
		.horc = 0,												//The n th horizontal choice chosen
		.verc = 0												//The n th vertical choice chosen
	};
	
	int maxrow = 0;
	switch(dtype)
	{
		case MERCHDT:
		maxrow = 7;
		break;
		case PERSONDT:
		maxrow = 7;
		break;
		case TRANSDT:
		maxrow = 6;
		break;
	}
	
	if(sortc.horc > maxrow)										//Prevent out of range
		sortc.horc = 0;
	sort_fresh(&sortc, dtype);									//Tell fresh function which choices chosen and which database used
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					sortc.verc++;
					break;
				case 72://Up
					sortc.verc--;
					break;
				case 77://Right
					sortc.horc++;
					break;
				case 75://Left
					sortc.horc--;
					break;
			}
		}
		if(sortc.verc == -1)									//Out of choice boundary
			sortc.verc = maxrow;
		if(sortc.verc > maxrow)
			sortc.verc %= (maxrow + 1);
		if(sortc.horc == -1)
			sortc.horc = 1;
		if(sortc.horc >1)
			sortc.horc %= 2;
			
		
		if(c1 == 13)											//Enter pressed
		{
			if(sortc.verc == maxrow)							//6 - Quit without save
				return;
			
			switch(dtype)										//If not quit, choose to set mask of which database
			{
				case MERCHDT:
				Merchsort = sortc.verc;
				break;
				
				case PERSONDT:
				Personsort = sortc.verc;		
				break;
				
				case TRANSDT:
				Transsort = sortc.verc;
				break;
			}
			Searchisza = (bool)sortc.horc;						//Ascending or descending
			return;
		}
		sort_fresh(&sortc, dtype);								//Refresh screen for every change
	}
	
	return;
}

void sort_fresh(Ctrlc* sortc, int dtype)
{
	int maxrow = 0;
	static char vercontent[10][MAXLEN];									//vertical content
	static char horcontent[2][MAXLEN] = {"Ascending", "Descending"};	//horizontal content
	
	apicolor(0);
	system("cls");
	
	apicolor(16);														//Display the heading
	for(int i = 0; i <= 1; i++)
	{
		apisetxy(20, i);
		for(int j = 0; j < 39; j++)
		printf(" ");
	}
	apicolor(160);
	apisetxy(22, 1);
	for(int j = 0; j < 39; j++)
		printf(" ");
	apisetxy(40, 1);
	printf("Sort");
	
	switch(dtype)														//Determine the maximum row and content
	{
		case MERCHDT:
		maxrow = 7;
		for(int i = 0; i <= maxrow; i++)
			strcpy(vercontent[i], merchtitle[i]);
		break;
		case PERSONDT:
		maxrow = 7;
		for(int i = 0; i < maxrow; i++)
			strcpy(vercontent[i], persontitle[i]);
		break;
		case TRANSDT:
		maxrow = 6;
		for(int i = 0; i < maxrow; i++)
			strcpy(vercontent[i], transtitle[i]);
		break;
	}
	strcpy(vercontent[maxrow], "Cancel");
	
	for(int i = 0; i < maxrow + 1; i++)
		center_align(vercontent[i], 25);								//Center alignment
	for(int i = 0; i < 2; i++)
		center_align(horcontent[i], 13);
	
	
	for(int i = 0; i < 2; i++)											//Display horizontal buttons
	{
		apicolor(128);													//Grey sub-background
		if(i == sortc->horc)
			apicolor(96);												//Green-yellow - shadow of chosen button
			
		for(int j = 3; j <= 5; j++)
		{
			apisetxy(18 + 32 * i, j);
			for(int k = 0; k < 15; k++)	
			printf(" ");
		}

		if(i == sortc->horc)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(19 + 32 * i, 4);
		printf("%13s", horcontent[i]);
	}
	apicolor(15);
	
	for(int i = 0; i < maxrow * 2 + 3; i++)								//Display vertical buttons
	{																	//Chioces are different
		apicolor(128);													//Grey sub-background
		if(i == sortc->verc * 2 || i == sortc->verc * 2 + 1)
			apicolor(96);												//Green-yellow - shadow of chosen button
		apisetxy(25, 7 + i);
		for(int j = 0; j < 32; j++)	
			printf(" ");
			
		if(i == sortc->verc * 2 + 1)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(27, 7 + i);
		if(i % 2 == 1)
			printf("    %-25s   ", vercontent[i / 2]);
	}

	return;
}

void search_detect(int dtype)
{
	
	int c1 = 0 , c2 = 0;
	static Ctrlc searchc =						//Record which choices are chosen
	{
		.horc = 0,								//The n th horizontal choice chosen
		.verc = 0								//The n th vertical choice chosen
	};
	
	int maxrow = 0;								//Determine no. of rows and mask
	unsigned short mask = 0x0;					//Record which field(s) to search
	char original_search[10][MAXLEN];			//Store initial search content
	for(int i = 0; i < 10; i++)
		strcpy(original_search[i], search_content[i]);	//Update from global
	
	switch(dtype)								
	{
		case MERCHDT:
		maxrow = 7;
		mask = Merchmask;
		break;
		case PERSONDT:
		maxrow = 7;
		mask = Personmask;
		break;
		case TRANSDT:
		maxrow = 6;
		mask = Transmask;
		break;
	}

	
	search_fresh(&searchc, dtype, mask);		//Tell fresh function which choices chosen and which database used
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					searchc.verc++;
					break;
				case 72://Up
					searchc.verc--;
					break;
				case 77://Right
					searchc.horc++;
					break;
				case 75://Left
					searchc.horc--;
					break;
			}
		}
		if(searchc.verc == -1)											//Out of choice boundary
			searchc.verc = maxrow;
		if(searchc.verc > maxrow)
			searchc.verc %= (maxrow + 1);
		if(searchc.horc == -1)
			searchc.horc = 1;
		if(searchc.horc >1)
			searchc.horc %= 2;
		
		if(isprint(c1) && !isblank(c1) && c1 != '|')					//valid character as search_string
		{
			if(searchc.verc < maxrow && strlen(search_content[searchc.verc]) < MAXLEN - 1)		//Not button line, not exceeded length
			{
				sprintf(search_content[searchc.verc], "%s%c\0", search_content[searchc.verc], (char)c1);
			}
				
		}
		if(c1 == 8)														//Backspace pressed
		{
			if(searchc.verc < maxrow && strlen(search_content[searchc.verc]) >0)					//Not button line, not empty
				search_content[searchc.verc][strlen(search_content[searchc.verc]) - 1] = '\0'; 	//Reduce a character
		}
		if(c1 == 13)													//Enter pressed
		{
			if(searchc.verc == maxrow)									//Quit with or without save
			{
				if(searchc.horc == 0)									//DO NOT SAVE
				{
					for(int i = 0; i < 10; i++)							//Load back initial
						strcpy(search_content[i], original_search[i]);
					return;
				}
				else													//SAVE
				{
					switch(dtype)										//Update the maximum row, content, and mask
					{
						case MERCHDT:
						Merchmask = mask;
						break;
						case PERSONDT:
						Personmask = mask;
						break;
						case TRANSDT:
						Transmask = mask;
						break;
					}
					return;
				} 
			}
			else
			mask = mask ^ (int)pow(2, searchc.verc);					//Xor operation...Hard to explain
		}
		search_fresh(&searchc, dtype, mask);							//Refresh screen for every change
	}
	
	return;
}

void search_fresh(Ctrlc* searchc, int dtype, unsigned short mask)
{		
	int maxrow = 0;
	static char vercontent[10][MAXLEN];									//vertical content
	static char horcontent[2][MAXLEN] = {"Cancel", "Save"};				//horizontal content
	
	apicolor(0);
	system("cls");
	
	apicolor(16);														//Display the heading
	for(int i = 0; i <= 1; i++)
	{
		apisetxy(20, i);
		for(int j = 0; j < 39; j++)
		printf(" ");
	}
	apicolor(160);
	apisetxy(22, 1);
	for(int j = 0; j < 39; j++)
		printf(" ");
	apisetxy(39, 1);
	printf("Search");
	
	switch(dtype)														//Determine the maximum row, content, and mask
	{
		case MERCHDT:
		maxrow = 7;
		for(int i = 0; i < maxrow; i++)
			strcpy(vercontent[i], merchtitle[i]);
		break;
		case PERSONDT:
		maxrow = 7;
		for(int i = 0; i < maxrow; i++)
			strcpy(vercontent[i], persontitle[i]);
		break;
		case TRANSDT:
		maxrow = 6;
		for(int i = 0; i < maxrow; i++)
			strcpy(vercontent[i], transtitle[i]);
		break;
	}
	
	
	for(int i = 0; i < 2; i++)
		center_align(horcontent[i], 13);
	
	
	for(int i = 0; i < maxrow * 2 + 1; i++)								//Display vertical buttons
	{																	//Chioces are different
		apicolor(128);													//Grey sub-background
		if((i == searchc->verc * 2 || i == searchc->verc * 2 + 1) && searchc->verc  != maxrow)	//Chosen but not last row
			apicolor(96);												//Green-yellow - shadow of chosen button
		apisetxy(5, 4 + i);
		for(int j = 0; j < 70; j++)	
			printf(" ");
			
		if(i == searchc->verc * 2 + 1)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		if(mask & (int)pow(2, i / 2))									//Check if the bit is 1. If so, this field needs to be searched.
			apicolor(160);												//Green - search mode enabled
		
		if(i % 2 == 1)
		{
			apisetxy(7, 4 + i);
			printf("%15s", vercontent[i / 2]);
			for(int j = 0; j < 51; j++)	
				printf(" ");
			apisetxy(23, 4 + i);
			printf("%s", search_content[i / 2]);
		}
	}
	

	for(int i = 0; i < 2; i++)											//Display horizontal buttons
	{
		apicolor(128);													//Grey sub-background
		if(i == searchc->horc && searchc->verc == maxrow)
			apicolor(96);												//Green-yellow - shadow of chosen button
			
		for(int j = 20; j <= 22; j++)
		{
			apisetxy(18 + 32 * i, j);
			for(int k = 0; k < 15; k++)	
			printf(" ");
		}

		if(i == searchc->horc && searchc->verc == maxrow)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(19 + 32 * i, 21);
		printf("%13s", horcontent[i]);
	}
	apicolor(15);
	DebugPrintf("\n%d, vert=%d",mask, searchc->verc);
	return;
}


/* Detect and fresh function definition */

void startup_detect()
{
	int c1 = 0 , c2 = 0;
	static int startupc = 0;
	int about = 0;
	startup_fresh(startupc);
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					startupc++;
					break;
				case 72://Up
					startupc--;
					break;
				default:
					startupc = 0;
					break;
			}
		}
		if(startupc == -1)
			startupc = 2;
		if(startupc > 2)
			startupc %= 3;
		if(c1 == 13)
		{
			switch(startupc % 3)
			{
				case 0://Login
				login_detect();
				break;
				case 1://About
				do
				{
					about = message_box(2, 1, "Do you like this program?");
					if(about == 0)
						message_box(0, 2, "Good! This is made by Wong Ka Yu.");
					else if(about == 2)
						message_box(0, 4, "Sorry, you have no choice.");
				}while(about != 0);
				break;
					
				case 2://Leave
				message_box(0, 1, "(<©f¦Ø¡¤)¡î~Kira");
				return;
			}
		}
		startup_fresh(startupc);
	}
	return;
}

void startup_fresh(int startupc)
{	
	static char content[3][MAXLEN] = {"Login", "About", "Quit"};
	clock_t clock_temp = 0;											//Used in delay
	int mark = 0;													//How many mark shown(animation)
	int incre = 1;
	for(int i = 0; i < 3; i++)
		center_align(content[i], 20);
	apicolor(32);
	system("cls");
	
	for(int i = 0; i < 2; i++)
	{
		apicolor(i == 0? 16: 144);
		apisetxy(i == 0? 2: 2, i == 0? 1: 2);
		print_color(" --+--  +----  +----  +----  \\   /  /  +----");
		print_color("   |    |      |   |  |   |   \\ /      |    ");
		print_color("   |    +----  |+\\--  |+\\--    |       +---+");
		print_color("   |    |      |  \\   |  \\     |           |");
		print_color("   |    +----  |   \\  |   \\    |       +---+");
		apicolor(i == 0? 48: 176);
		print_color("");
		print_color("");
		print_color("                       +----  +---+  +   +  +--+  |   |  --+--  +----  +----");
		print_color("                       |      |   |  |\\ /|  |  |  |   |    |    |      |   |");
		print_color("                       |      |   |  | | |  |--+  |   |    |    +----  |+\\--");
		print_color("                       |      |   |  |   |  |     |   |    |    |      |  \\ ");
		print_color("                       +----  +---+  |   |  |     +---+    |    +----  |   \\");
	}
	
	
	for(int i = 0; i < 9; i++)											//Display all choices
	{																	//Chioces are different
		apicolor(128);													//Grey sub-background
		if(i == startupc * 3 || i == startupc * 3 + 1)
			apicolor(80);												//Green-yellow - shadow of chosen button
		apisetxy(20, 16 + i);
		for(int j = 0; j < 39; j++)	
			printf(" ");
			
		if(i == startupc * 3 + 1)
			apicolor(222);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(22, 16 + i);
		if(i % 3 == 1)
			printf("           %-20s        ", content[i / 3]);
	}
	
	apicolor(45);														//Arrow color
	for(; kbhit() == 0;)												//Animation effect
	{
		if(mark == -5)
			incre = 1;
		if(mark == 5)
			incre = -1;
		mark += incre;													//Oscilate
		clock_temp = clock();
		for(;clock() - clock_temp <= 0.1 * CLOCKS_PER_SEC;);			//1/4sec delay
		for(int i = 0; i < 4; i++)
		{
				apisetxy(18 - i, startupc * 3 +17);
				printf("%c", i < abs(mark)? '>': ' ');
				apisetxy(62 + i, startupc * 3 + 17);
				printf("%c", i < abs(mark)? '<': ' ');
		}
	}
	return;
}


void login_detect(void)
{
	static Ctrlc loginc =
	{
		.verc = 0,
		.horc = 1														//Default is Login choice									
	};
	loginc.horc = 1;
	int c1 = 0, c2 = 0;
	int user = 0;														//Real-time character entered in username row
	int pswd = 0;														//Real-time character entered in password row
	Person loginer;														//The identified person
	int authen_level = -4;												//Level of current account
	char username[MAXLEN] = "";
	char password[MAXLEN] = "";
	
	
	login_fresh(&loginc,username,pswd);
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					loginc.verc++;
					break;
				case 72://Up
					loginc.verc--;
					break;
				case 77://Right
					loginc.horc++;
					break;
				case 75://Left
					loginc.horc--;
					break;
				default:
					DebugPrintf("Other controlling char entered");
					break;
			}
		}
		if(loginc.verc == -1)
			loginc.verc = 1;
		if(loginc.verc > 1)
			loginc.verc %= 2;
		
		if(loginc.horc == -1)
			loginc.horc = 1;
		if(loginc.horc > 1)
			loginc.horc %= 2;
		
		if(isprint(c1) && !isblank(c1) && c1 != '|')					//valid character as username or password
		{
			if(loginc.verc % 2 == 0 && user < 29)						//Username row selected
				username[user++] = (char)c1;
			else if(loginc.verc % 2 == 1 && pswd < 29)					//Password row selected
				password[pswd++] = (char)c1;
		}
		if(c1 == 8)//Backspace pressed
		{
			if(loginc.verc % 2 == 0 && user > 0)						//Backspace username
				username[--user] = '\0';
			else if(loginc.verc % 2 == 1 && pswd > 0)					//Backspace password
				password[--pswd] = '\0';
		}
		if(c1 == 13)//Enter pressed
		{
			switch(loginc.horc % 2)
			{
				case 0://Leave
					return;
				case 1://Authenticate
					authen_level = authenticate(username,password, &loginer);		
					if(authen_level > -2)								//Authentication succeed
						current_loginer = loginer;
						
					switch(authen_level)
					{
						case -4:
						message_box(0, 6, "Username does not exist.");
						break;
						case -3:
						message_box(0, 6, "Password is wrong.", 0);
						break;
						case -1:
						message_box(0, 5, "Welcome administrator, %s!", loginer.firstname);
						break;
						case 0:
						message_box(0, 2, "Welcome manager, %s!", loginer.firstname);
						manager_detect();
						break;
						case 1:
						message_box(0, 9, "Welcome, %s!", loginer.firstname);
						staff_detect();
						break;
					}
			}
		}

		login_fresh(&loginc,username,pswd);								//For password, only tell the number of characters entered
	}
	return;
}

void login_fresh(Ctrlc* loginc, char* username, int pswd)
{	
	static char content[2][MAXLEN] = {"Leave", "Login"};
	for(int i = 0; i < 2; i++)
		center_align(content[i], 11);
	apicolor(0);
	system("cls");
	
	apicolor(16);														//Display the heading
	for(int i = 2; i <= 3; i++)
	{
		apisetxy(20, i);
		for(int j = 0; j < 39; j++)
		printf(" ");
	}
	apicolor(160);
	apisetxy(22, 3);
	for(int j = 0; j < 39; j++)
		printf(" ");
	apisetxy(36, 3);
	printf("Authentication");
	
	
	for(int i = 0; i < 6; i++)											//Display username and password rows
	{
		apicolor(128);													//Grey sub-background
		if(i == loginc->verc * 3 || i == loginc->verc * 3 + 1)
			apicolor(96);												//Green-yellow - shadow of chosen button
		apisetxy(20, 6 + i);
		for(int j = 0; j < 39; j++)	
			printf(" ");
			
		if(i == loginc->verc * 3 + 1)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(22, 6 + i);
		if(i == 1)														//Print username
		{
			for(int j = 0; j < 39; j++)
			printf(" ");
			apisetxy(22, 7);
			printf("Username:%-27s", username);
		}
			
		if(i == 4)														//Print password
		{
			for(int j = 0; j < 39; j++)
			printf(" ");
			apisetxy(22, 10);
			printf("Password:%-27");
			for(int j = 0; j < pswd; j++)
				printf("*");
		}
	}

	for(int i = 0; i < 2; i++)											//Display horizontal buttons
	{
		apicolor(128);													//Grey sub-background
		if(i == loginc->horc)
			apicolor(96);												//Green-yellow - shadow of chosen button
			
		for(int j = 14; j <= 16; j++)
		{
			apisetxy(17 + 30 * i, j);
			for(int j = 0; j < 15; j++)	
			printf(" ");
		}

		if(i == loginc->horc)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(19 + 30 * i, 15);
		printf("%11s", content[i]);
	}
	return;
}

void staff_detect(void)
{
	static int staffc = 0;
	int c1 = 0 , c2 = 0;
	staffc = 0;
	staff_fresh(staffc);
	
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					staffc++;
					break;
				case 72://Up
					staffc--;
					break;
				default:
					staffc = 0;
					break;
			}
		}
		if(staffc == -1)
			staffc = 2;
		if(staffc > 2)
			staffc %= 3;
		if(c1 == 13)
		{
			switch(staffc % 3)
			{
				case 0://Ordering
					while(order_detect());
					break;
				case 1://Change Password
					apicolor(15);
					system("cls");
					if(change_password(current_loginer.prkey_P) == -1)
						message_box(0, 6, "Password Changing failed.");
					else
						message_box(0, 2, "Password Changing succeeded.");
					break;
				case 2://Leave
					return;
				default:
					DebugPrintf("I should not get here");
					break;
			}
		}
		staff_fresh(staffc);
	}
	return;
}

void staff_fresh(int staffc)
{	
	static char content[3][MAXLEN] = {"Ordering", "Change password", "Leave"};
	clock_t clock_temp = 0;												//Used in delay
	int mark = 0;														//How many mark shown(animation)
	int incre = 1;
	for(int i = 0; i < 3; i++)
		center_align(content[i], 20);
	apicolor(32);
	system("cls");
	
	apisetxy(65, 0);
	printf("%s %s", current_loginer.firstname, current_loginer.surname);
	apicolor(16);														//Display the Staff heading
	for(int i = 2; i <= 3; i++)
	{
		apisetxy(20, i);
		for(int j = 0; j < 39; j++)
		printf(" ");
	}
	apicolor(160);
	apisetxy(22, 3);
	for(int j = 0; j < 39; j++)
		printf(" ");
	apisetxy(36, 3);
	printf("Staff Menu");
	
	
	for(int i = 0; i < 9; i++)											//Display all choices
	{																	//Chioces are different
		apicolor(128);													//Grey sub-background
		if(i == staffc * 3 || i == staffc * 3 + 1)
			apicolor(96);												//Green-yellow - shadow of chosen button
		apisetxy(20, 6 + i);
		for(int j = 0; j < 39; j++)	
			printf(" ");
			
		if(i == staffc * 3 + 1)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(22, 6 + i);
		if(i % 3 == 1)
			printf("           %-20s        ", content[i / 3]);
	}
	
	apicolor(46);
	for(; kbhit() == 0;)												//Animation effect
	{
		if(mark == -5)
			incre = 1;
		if(mark == 5)
			incre = -1;
		mark += incre;													//Oscilate
		clock_temp = clock();
		for(;clock() - clock_temp <= 0.1 * CLOCKS_PER_SEC;);			//1/4sec delay
		for(int i = 0; i < 4; i++)
		{
				apisetxy(18 - i, staffc * 3 + 7);
				printf("%c", i < abs(mark)? '>': ' ');
				apisetxy(62 + i, staffc * 3 + 7);
				printf("%c", i < abs(mark)? '<': ' ');
		}
	}
	return;
}

void manager_detect(void)
{
	static int managerc = 0;
	int c1 = 0 , c2 = 0;
	managerc = 0;
	manager_fresh(managerc);
	
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					managerc++;
					break;
				case 72://Up
					managerc--;
					break;
				default:
					managerc = 0;
					break;
			}
		}
		if(managerc == -1)
			managerc = 5;
		if(managerc >5)
			managerc %= 6;
		if(c1 == 13)
		{
			switch(managerc % 6)
			{
				case 0://Merchandise control
					while(database_detect(MERCHDT));					//If database_detect returns 1, Invoke it back again
					break;												//It is used to rebuild a binary tree when new data are introduced
				case 1://Profile control
					while(database_detect(PERSONDT));
					break;
				case 2://Transaction control
					while(transaction_detect());
					break;
				case 3://Ordering
					while(order_detect());
					break;
				case 4://Change Password
					apicolor(15);
					system("cls");
					if(change_password(current_loginer.prkey_P) == -1)
						message_box(0, 6, "Password Changing failed.");
					else
						message_box(0, 2, "Password Changing succeeded.");
					break;
				case 5://Leave
					return;
				default:
					DebugPrintf("I should not get here");
					break;
			}
		}
		manager_fresh(managerc);
	}
	return;
}

void manager_fresh(int managerc)
{	
	static char content[6][MAXLEN] = {"Manage Merchandize", "Manage Personnel", "Manage Transaction", """Ordering", "Change password", "Leave"};
	clock_t clock_temp = 0;												//Used in delay
	int mark = 0;														//How many mark shown(animation)
	int incre = 1;
	for(int i = 0; i < 6; i++)
		center_align(content[i], 20);
	apicolor(32);
	system("cls");
	
	apisetxy(65, 0);
	printf("%s %s", current_loginer.firstname, current_loginer.surname);
	apicolor(16);														//Display the Manager heading
	for(int i = 2; i <= 3; i++)
	{
		apisetxy(20, i);
		for(int j = 0; j < 39; j++)
		printf(" ");
	}
	apicolor(160);
	apisetxy(22, 3);
	for(int j = 0; j < 39; j++)
		printf(" ");
	apisetxy(36, 3);
	printf("Manager Menu");
	
	
	for(int i = 0; i < 18; i++)											//Display all choices
	{																	//Chioces are different
		apicolor(128);													//Grey sub-background
		if(i == managerc * 3 || i == managerc * 3 + 1)
			apicolor(96);												//Green-yellow - shadow of chosen button
		apisetxy(20, 6 + i);
		for(int j = 0; j < 39; j++)	
			printf(" ");
			
		if(i == managerc * 3 + 1)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(22, 6 + i);
		if(i % 3 == 1)
			printf("           %-20s        ", content[i / 3]);
	}
	
	apicolor(46);
	for(; kbhit() == 0;)												//Animation effect
	{
		if(mark == -5)
			incre = 1;
		if(mark == 5)
			incre = -1;
		mark += incre;													//Oscilate
		clock_temp = clock();
		for(;clock() - clock_temp <= 0.1 * CLOCKS_PER_SEC;);			//1/4sec delay
		for(int i = 0; i < 4; i++)
		{
				apisetxy(18 - i, managerc * 3 + 7);
				printf("%c", i < abs(mark)? '>': ' ');
				apisetxy(62 + i, managerc * 3 + 7);
				printf("%c", i < abs(mark)? '<': ' ');
		}
	}
	return;
}


int database_detect(int dtype)
{
	static Ctrlc handlec =
	{
		.verc = 0,
		.horc = 0,
		.page = 0
	};
	int c1 = 0, c2 = 0;
	int dtcount = 0;											//Number of merchandise
	unsigned short mask;										//Determine which field(s) to be searched 
	bool isdown = false;										//Is last control key 'Down'
	bool issearch = false;										//Is search mode open
	Node* pfRoot = NULL;										//Pointer storing root of searching binary tree
	Node* pRoot = NULL;											//Pointer stroing root of merchandise bianry tree
	dtcount = bntree(&pRoot, false, dtype);						//Use binary tree to sort merchs(defined in B file)
	int maxpage = dtcount?((dtcount - 1))/ ITEMAX: 0;			//Calculate the max number of page
	if(handlec.page > maxpage)									//Prevent overpage due to different no. of pages of Merch and Person
		handlec.page = maxpage;
	initialize_search();										//Clear the global variable about searching
	
	database_fresh(dtype, &handlec, pRoot, dtcount, isdown, issearch);
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					handlec.verc++;
					isdown = true;								//We need 'isdown' because we conduct automatic page++ only when
					break;										//the last control key is 'down' from the last item of previoius page
				case 72://Up
					handlec.verc--;
					isdown = false;
					break;
				case 77://Right
					handlec.horc++;
					break;
				case 75://Left
					handlec.horc--;
					break;
				case 73://PageUp
					if(handlec.page > 0)						//Ensure not the first page so page-- is valid
					{
						handlec.page--;
						isdown = false;							//Prevent auto page++, since when the first item of the page is selected
																//and the last key is 'Down', auto page++ will be conducted.
						handlec.verc = handlec.page * ITEMAX;	//Select the first item at the page
					}
					break;
				case 81://PageDown
					if(handlec.page < maxpage)
					{
						handlec.page++;
						isdown = true;							//Prevent auto page++ for the same reason
						handlec.verc = handlec.page * ITEMAX;
					}	
					break;
				default:
					DebugPrintf("I should not get here!");
					break;
			}
		}
		if(handlec.verc <= -1)
			handlec.verc = 0;
		if(handlec.verc >= dtcount)
			handlec.verc = dtcount - 1; 
		if(handlec.horc == -1)
			handlec.horc = 6;
		if(handlec.horc >6)
			handlec.horc %= 7;

		if(c1 == 13)//Enter pressed
		{
			switch(handlec.horc % 7)
			{
				case 0://Add
				if(message_box(1, 3, "Insert some records?") == 1)
				{
					system("cls");
					get_all_data(dtype, "a+");
				}
					
				release_node(pRoot);						//Swipe out original binary tree because there may be new item(B file defined)
				pRoot = NULL;
				release_node(pfRoot);						//Swipe out original search binary tree(B file defined)
				pfRoot = NULL;
				return 1;									//To request refreshing to build a new tree, because
				break;										//the tree-building function is located on the front part of this function
				
				case 1://Edit
				if(dtcount > 0)
				{
					database_fresh(dtype, &handlec, issearch?pfRoot:pRoot, dtcount, isdown, issearch);
					if(message_box(1, 3, "Update the selected record?") == 1)							//To verify whether it is a delibrate editing
					{
						system("cls");
						fflush(stdin);
						edit_data(dtype, retrieve_node(issearch?pfRoot:pRoot,handlec.verc, issearch)->pDT,false, NULL, NULL);
						//retrieve_node:return the node of the chosen item(defined in B file)
						//edit_data:an interface to 1.ask for changes of a given datum or 2.delete the given datum
					release_node(pRoot);
					pRoot = NULL;
					release_node(pfRoot);
					pfRoot = NULL;
					return 1;									//To request rebuilding binary tree
					}
				}			
				fflush(stdin);
				break;
				
				case 2://Delete
				if(dtcount > 0)
				{
					database_fresh(dtype, &handlec, issearch?pfRoot:pRoot, dtcount, isdown, issearch);
					if(message_box(1, 12, "Delete the selected record?") == 1)							//To verify whether it is a delibrate deletion
					{
						system("cls");
						fflush(stdin);
						edit_data(dtype, retrieve_node(issearch?pfRoot:pRoot,handlec.verc, issearch)->pDT,true, NULL, NULL);
					release_node(pRoot);
					pRoot = NULL;
					release_node(pfRoot);
					pfRoot = NULL;
					return 1;									//To request rebuilding binary tree
					}
				}			
				fflush(stdin);
				break;
					
				case 3://Sorting
				sort_detect(dtype);								//Invoke 'sort' interface
				release_node(pRoot);							//To swipe out original binary tree
				pRoot = NULL;
				release_node(pfRoot);
				pfRoot = NULL;
				return 1;										//To request rebuilding a new binary tree
				
				case 4://Report
				if(message_box(1, 1, "Print the pages?"))
				{
					print_records(dtype, issearch? pfRoot: pRoot, dtcount, issearch);
					message_box(0, 1, "Report printed.");
				}
				break;
					
				case 5://Searching			
				search_detect(dtype);
				switch(dtype)									//Determine which mask to check
				{
					case MERCHDT:
					mask = Merchmask;
					break;
					case PERSONDT:
					mask = Personmask;
					break;
				}
				issearch = mask? true: false;				//If no mask, not in search mode
				
				if(issearch)
				{
					search_data(dtype, pRoot);				//Conduct searching using global mask, distribute fserial and store results in file
					dtcount = bntree(&pfRoot, true, dtype);	//create new binary tree and update number of Merch under search mode
				}
				else
				{
					dtcount = bntree(&pRoot, false, dtype);
				}
				handlec.verc = 0;							//initialize item chosen
				maxpage = dtcount?((dtcount - 1))/ ITEMAX: 0;	//update max page
				handlec.page = 0;							//initialize item chosen
				isdown = false;								//Prevent auto page++
				break;
				
				case 6://Leave
				release_node(pRoot);						//To swipe out original binary tree
				pRoot = NULL;
				release_node(pfRoot);
				pfRoot = NULL;
				return 0;
			}
		}
		database_fresh(dtype, &handlec, issearch?pfRoot:pRoot, dtcount, isdown, issearch);
	}
	
	

	return 0;
}

void database_fresh(int dtype, Ctrlc* handlec,Node* pRoot, int dtcount, bool isdown, bool issearch)
{
	int maxpage = dtcount?((dtcount - 1))/ ITEMAX: 0;							//Calculate the max number of page
	apicolor(0);
	system("cls");
	
	//When the first item of the page is selected and the last key is 'Down', auto page+- will be conducted.
	if(handlec->verc % ITEMAX == 0 && isdown == true && handlec->page < maxpage)					//Auto page ++ 
	{
		DebugPrintf("auto page++ conducted.");
		handlec->page++;
	}
	if(handlec->verc % ITEMAX == (ITEMAX - 1) && isdown == false && handlec->page > 0)			//Auto page --
	{
		DebugPrintf("auto page-- conducted.");
		handlec->page--;
	}
	//Display the buttons
	char button[7][MAXLEN] = {"Add", "Edit", "Delete", "Sort", "Report", "Search", "Leave"};
	for(int i = 0; i < 7; i++)
		center_align(button[i], 9);										//Center align the titles in each button

	for(int i = 0; i < 7; i++)											//Display horizontal buttons
	{
		apicolor(128);													//Grey sub-background
		if(i == handlec->horc)
				apicolor(96);											//Green-yellow - shadow of chosen button
			
		for(int j = 0; j <= 2; j++)
		{
			apisetxy(1 + 11 * i, j);
			for(int j = 0; j < 11; j++)	
			printf(" ");
		}

		if(i == handlec->horc)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		if(issearch && i == 5)
			apicolor(160);												//Green - search button if in search mode
		apisetxy(2 + 11 * i, 1);
		printf("%9s", button[i]);
	}
	
	printf("\n\n");

	list_field(dtype);													//Print corresponding field name

	printf("\n");
	
	if(dtcount == 0)
	{
		apicolor(3);
		apisetxy(35,15);
		printf("No data exists.");
		return;
	}
	for(int i = ITEMAX * handlec->page; i < (handlec->page == maxpage?dtcount:ITEMAX * handlec->page + ITEMAX); i++)
	{//Show ITEMAX itmes per page. If it is the last page, no. of items shown may be less than ITEMAX
		apicolor(handlec->verc % dtcount == i?240:15);
		if(retrieve_node(pRoot, i, issearch))
			list_data(dtype, retrieve_node(pRoot, i, issearch)->pDT);
		else
		{
			DebugPrintf("list_data is NULL. Issearch = %s", issearch? "true":"false");			//It should not occur. If no such record for serial i, there are some problems.
			DebugSystem("pause");
		}
	}
	apicolor(15);
	apisetxy(0,24);
	printf("Page %d of %d\tverc = %d\tcount = %d\t", handlec->page + 1, maxpage + 1,handlec->verc,dtcount);
	
	return;
}

int transaction_detect(void)
{
	static Ctrlc handlec =
	{
		.verc = 0,
		.horc = 0,
		.page = 0
	};
	int c1 = 0, c2 = 0;
	int dtcount = 0;											//Number of transactions
	bool isdown = false;										//Is last control key 'Down'
	bool issearch = false;										//Is search mode open
	Node* pfRoot = NULL;										//Pointer storing root of searching binary tree
	Node* pRoot = NULL;											//Pointer stroing root of transaction bianry tree
	dtcount = bntree(&pRoot, false, TRANSDT);					//Use binary tree to sort transaction
	int maxpage = dtcount?((dtcount - 1))/ ITEMAX: 0;			//Calculate the max number of page
	initialize_search();										//Clear the global variable about searching
	
	
	transaction_fresh(&handlec, pRoot, dtcount, isdown, issearch);
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					handlec.verc++;
					isdown = true;								//We need 'isdown' because we conduct automatic page++ only when
					break;										//the last control key is 'down' from the last item of previoius page
				case 72://Up
					handlec.verc--;
					isdown = false;
					break;
				case 77://Right
					handlec.horc++;
					break;
				case 75://Left
					handlec.horc--;
					break;
				case 73://PageUp
					if(handlec.page > 0)						//Ensure not the first page so page-- is valid
					{
						handlec.page--;
						isdown = false;							//Prevent auto page++, since when the first item of the page is selected
																//and the last key is 'Down', auto page++ will be conducted.
						handlec.verc = handlec.page * ITEMAX;	//Select the first item at the page
					}
					break;
				case 81://PageDown
					if(handlec.page < maxpage)
					{
						handlec.page++;
						isdown = true;							//Prevent auto page++ for the same reason
						handlec.verc = handlec.page * ITEMAX;
					}	
					break;
				default:
					DebugPrintf("I should not get here!");
					break;
			}
		}
		if(handlec.verc <= -1)
			handlec.verc = 0;
		if(handlec.verc >= dtcount)
			handlec.verc = dtcount - 1; 
		if(handlec.horc == -1)
			handlec.horc = 5;
		if(handlec.horc >5)
			handlec.horc %= 6;

		if(c1 == 13)//Enter pressed
		{
			switch(handlec.horc % 6)
			{
				case 0://View Detail
				detail_detect(retrieve_node(issearch?pfRoot:pRoot,handlec.verc, issearch)->pDT);
				break;
				
				case 1://Delete
				if(dtcount > 0)
				{
					transaction_fresh(&handlec, issearch?pfRoot:pRoot, dtcount, isdown, issearch);
					if(message_box(1, 12, "Delete the selected record?") == 1)							//To verify whether it is a delibrate deletion
					{
						system("cls");
						fflush(stdin);
						edit_data(TRANSDT, retrieve_node(issearch?pfRoot:pRoot,handlec.verc, issearch)->pDT,true, NULL, NULL);
					release_node(pRoot);
					pRoot = NULL;
					release_node(pfRoot);
					pfRoot = NULL;
					return 1;								//To request rebuilding binary tree
					}
				}			
				fflush(stdin);
				break;
					
				case 2://Sorting
				sort_detect(TRANSDT);						//Invoke 'sort' interface
				release_node(pRoot);						//To swipe out original binary tree
				pRoot = NULL;
				release_node(pfRoot);
				pfRoot = NULL;
				return 1;									//To request rebuilding a new binary tree
				
				case 3://Report
				if(message_box(1, 1, "Print the pages?"))
				{
					print_records(TRANSDT, issearch? pfRoot: pRoot, dtcount, issearch);
					message_box(0, 1, "Report printed.");
				}
					
				break;
					
				case 4://Searching
				search_detect(TRANSDT);
				issearch = Transmask? true: false;			//If no mask, not in search mode	
				if(issearch)
				{
					search_data(TRANSDT, pRoot);	//Conduct searching using global mask, distribute fserial and store results in file
					dtcount = bntree(&pfRoot, true, TRANSDT);//create new binary tree and update number of Merch under search mode
				}
				else
				{
					dtcount = bntree(&pRoot, false, TRANSDT);
				}
				
				handlec.verc = 0;							//initialize item chosen
				maxpage = dtcount?((dtcount - 1))/ ITEMAX: 0;	//update max page
				handlec.page = 0;							//initialize item chosen
				isdown = false;								//Prevent auto page++
				break;

				case 5://Leave
				release_node(pRoot);						//To swipe out original binary tree
				pRoot = NULL;
				release_node(pfRoot);
				pfRoot = NULL;
				return 0;
			}
		}
		transaction_fresh(&handlec, issearch?pfRoot:pRoot, dtcount, isdown, issearch);
	}

	return 0;
}

void transaction_fresh(Ctrlc* handlec,Node* pRoot, int dtcount, bool isdown, bool issearch)
{
	int maxpage = dtcount?((dtcount - 1))/ ITEMAX: 0;							//Calculate the max number of page
	apicolor(0);
	system("cls");
	
	//When the first item of the page is selected and the last key is 'Down', auto page+- will be conducted.
	if(handlec->verc % ITEMAX == 0 && isdown == true && handlec->page < maxpage)					//Auto page ++ 
	{
		DebugPrintf("auto page++ conducted.");
		handlec->page++;
	}
	if(handlec->verc % ITEMAX == (ITEMAX - 1) && isdown == false && handlec->page > 0)			//Auto page --
	{
		DebugPrintf("auto page-- conducted.");
		handlec->page--;
	}
	//Display the buttons
	char button[6][MAXLEN] = {"Detail", "Delete", "Sort", "Report", "Search", "Leave"};
	for(int i = 0; i < 6; i++)
		center_align(button[i], 9);										//Center align the titles in each button

	for(int i = 0; i < 6; i++)											//Display horizontal buttons
	{
		apicolor(128);													//Grey sub-background
		if(i == handlec->horc)
				apicolor(96);											//Green-yellow - shadow of chosen button
			
		for(int j = 0; j <= 2; j++)
		{
			apisetxy(9 + 11 * i, j);
			for(int j = 0; j < 11; j++)	
			printf(" ");
		}

		if(i == handlec->horc)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		if(issearch && i == 4)
			apicolor(160);												//Green - search button if in search mode
		apisetxy(10 + 11 * i, 1);
		printf("%9s", button[i]);
	}
	
	printf("\n\n");

	list_field(TRANSDT);												//Print corresponding field name
	
	printf("\n");
	
	if(dtcount == 0)
	{
		apicolor(3);
		apisetxy(35,15);
		printf("No data exists.");
		return;
	}
	for(int i = ITEMAX * handlec->page; i < (handlec->page == maxpage?dtcount:ITEMAX * handlec->page + ITEMAX); i++)
	{//Show ITEMAX itmes per page. If it is the last page, no. of items shown may be less than ITEMAX
		apicolor(handlec->verc % dtcount == i?240:15);
		if(retrieve_node(pRoot, i, issearch))
			list_data(TRANSDT, retrieve_node(pRoot, i, issearch)->pDT);
		else
		{
			DebugPrintf("list_data is NULL");			//It should not occur. If no such merchandize for serial i, there are some problems.
			DebugSystem("pause");
		}
	}
	apicolor(15);
	apisetxy(0,24);
	printf("Page %d of %d\tverc = %d\tcount = %d\t", handlec->page + 1, maxpage + 1,handlec->verc,dtcount);
	
	return;
}

void detail_detect(Transaction* pTrans)
{
	static Ctrlc detailc =
	{
		.page = 0
	};
	int c1 = 0, c2 = 0;
	int maxpage = pTrans->different?((pTrans->different - 1))/ ITEMAX: 0;	//Calculate the max number of page
	long long grand_total = 0;
	for(int i = 0; i < pTrans->different; i++)
		grand_total += pTrans->price_then[i] * pTrans->quantity[i];
	//Search for staff name using prkey_P
	Person person_found;
	char staff_name[MAXLEN];
	Node* pRoot = NULL;											//Pointer storing root of person binary tree
	Node* pfRoot = NULL;										//Pointer storing root after search
	bntree(&pRoot, false, PERSONDT);							//Use binary tree to sort people
	Personmask = 0x1;											//Search prkey_P only
	strcpy(search_content[0], pTrans->prkey_P);	
	search_data(PERSONDT, pRoot);		//Conduct search using Personmask, distribute fserial and store results in file
	int found = bntree(&pfRoot, true, PERSONDT);	//create new binary tree and update number of Merch under search mode
	if(found)
	{
		person_found = *(Person*)(retrieve_node(pfRoot, 0, true)->pDT);
		sprintf(staff_name, "%s %s", person_found.firstname, person_found.surname);
	}
	else
		strcpy(staff_name, "Not Found");
	found = 0;
	release_node(pRoot);
	pRoot = NULL;
	release_node(pfRoot);
	pfRoot = NULL;
	
	//Search for merchandise information
	Merch merch_found;
	char search_model[MAXTRANS][MAXLEN];
	char search_brand[MAXTRANS][MAXLEN];
	pRoot = NULL;											//Pointer storing root of person binary tree
	pfRoot = NULL;											//Pointer storing root after search
	bntree(&pRoot, false, MERCHDT);							//Use binary tree to sort merchandises
	Merchmask = 0x1;										//Search prkey_M only
	for(int i = 0; i < pTrans->different; i++)
	{
		strcpy(search_content[0], pTrans->prkey_M[i]);	
		search_data(MERCHDT, pRoot);		//Conduct search using Merchmask, distribute fserial and store results in file
		found = bntree(&pfRoot, true, PERSONDT);	//create new binary tree and update number of Merch under search mode
		if(found)
		{
			merch_found = *(Merch*)(retrieve_node(pfRoot, 0, true)->pDT);
			strcpy(search_model[i], merch_found.model);
			strcpy(search_brand[i], merch_found.brand);
		}
		else
		{
			strcpy(search_model[i], "-LOST-");
			strcpy(search_brand[i], "-LOST-");
		}
	}
	release_node(pRoot);
	pRoot = NULL;
	release_node(pfRoot);
	pfRoot = NULL;
	
	detail_fresh(&detailc, pTrans, staff_name, search_model, search_brand, grand_total);
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 77://Right
					detailc.horc++;
					break;
				case 75://Left
					detailc.horc--;
					break;
				case 72://Up
				case 73://PageUp
					if(detailc.page > 0)						//Ensure not the first page so page-- is valid
					{
						detailc.page--;
					}
					break;
				case 80://Down
				case 81://PageDown
					if(detailc.page < maxpage)
					{
						detailc.page++;
					}	
					break;
				default:
					DebugPrintf("I should not get here!");
					break;
			}
		}
		if(detailc.horc == -1)
			detailc.horc = 1;
		if(detailc.horc > 1)
			detailc.horc %= 2;

		if(c1 == 13)//Enter pressed
		{
			if(detailc.horc == 0)
				return;														//Leave
			else if(detailc.horc == 1)
			{
				print_receipt(pTrans, staff_name, search_model, search_brand, grand_total);
				message_box(0, 1, "Receipt printed.");
			}
		}
		detail_fresh(&detailc, pTrans, staff_name, search_model, search_brand, grand_total);
	}
	
	return; 
}

void detail_fresh(Ctrlc* detailc, Transaction* pTrans, char* staff_name, char model[MAXTRANS][MAXLEN], char brand[MAXTRANS][MAXLEN], long long grand)
{
	int maxpage = pTrans->different?((pTrans->different - 1))/ ITEMAX: 0;	//Calculate the max number of page
	apicolor(0);
	system("cls");
	apicolor(16);														//Display the heading
	for(int i = 0; i <= 1; i++)
	{
		apisetxy(10, i);
		for(int j = 0; j < 25; j++)
		printf(" ");
	}
	apicolor(160);
	apisetxy(12, 1);
	for(int j = 0; j < 25; j++)
		printf(" ");
	apisetxy(22, 1);
	printf("Detail");

	//Display the buttons
	char button[2][MAXLEN] = {"Return", "Print"};
	for(int i = 0; i < 2; i++)
		center_align(button[i], 9);										//Center align the titles in each button

	for(int i = 0; i < 2; i++)											//Display horizontal buttons
	{


		if(i == detailc->horc)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(45 + 15 * i, 1);
		printf("%9s", button[i]);
	}
	
	
	if(pTrans->different == 0)
	{
		apicolor(3);
		apisetxy(32,15);
		DebugPrintf("No merchandise exchange. Something wrong.");
		return;
	}
	
	apicolor(228);														//Grey background, black font
	for(int i = 2; i <= 3; i++)
	{
		apisetxy(2,i);
		for(int j = 2; j <= 75; j++)
			printf(" ");
	}
	//Display general information
	apisetxy(2, 2);
	printf("Transaction: %s", pTrans->prkey_T);
	apisetxy(27,2);
	printf("Action: ");
	if(pTrans->type == 0)
		printf("Sold out");
	else if(pTrans->type == 1)
		printf("Goods in");
	apisetxy(49, 2);
	printf("Date&Time: ");
	struct tm* ptime = localtime(&(pTrans->ttime));
	printf("%4d/%02d/%02d %02d:%02d", 1900 + ptime->tm_year, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min);
	
	apisetxy(2, 3);
	printf("Staff Key: %s", pTrans->prkey_P);
	apisetxy(27, 3);
	printf("Staff Name: %s", staff_name);
	apisetxy(54, 3);
	printf("Grand Total: %9lld", grand);
		
	apicolor(15);
	apisetxy(4, 4);
	printf("#");
	apisetxy(6, 4);
	printf("PrdctKey");
	apisetxy(16, 4);
	printf("Name");
	apisetxy(35, 4);
	printf("Brand");
	apisetxy(54, 4);
	printf("SoldPrice");
	apisetxy(64, 4);
	printf("Qnty");
	apisetxy(71, 4);
	printf("Total");
	
	for(int i = ITEMAX * detailc->page; i < (detailc->page == maxpage?pTrans->different:ITEMAX * detailc->page + ITEMAX); i++)
	{//Show ITEMAX merchandises per page. If it is the last page, no. of items shown may be less than ITEMAX
		apisetxy(2, 5 + i % ITEMAX);
		printf("%03d", i);
		apisetxy(6, 5 + i % ITEMAX);
		printf("%s", pTrans->prkey_M[i]);
		apisetxy(16, 5 + i % ITEMAX);
		printf("%s", model[i]);
		apisetxy(35, 5 + i % ITEMAX);
		printf("%s", brand[i]);
		apisetxy(54, 5 + i % ITEMAX);
		printf("%9d", pTrans->price_then[i]);
		apisetxy(64, 5 + i % ITEMAX);
		printf("%4d", pTrans->quantity[i]);
		apisetxy(70, 5 + i % ITEMAX);
		printf("%6lld", (long long)pTrans->price_then[i] * pTrans->quantity[i]);
		
	}
	apicolor(15);
	apisetxy(0, 24);
	printf("Page %d of %d\tcount = %d\t", detailc->page + 1, maxpage + 1 ,pTrans->different);
	
}

int order_detect(void)
{
	static Ctrls orderc;
	static Node* pList;											//A link list storing bought items and respective quantities
	static int different;										//Number of bought items
	long long grand_total = 0;
	int c1 = 0, c2 = 0;
	Node* pRoot = NULL;											//Pointer storing root of binary tree
	Node* pfRoot = NULL;										//Pointer storing root of searching binary tree
	Node* pCurrent = NULL;										//Temporarily store a Node*
	Transaction finished;										//Store the finished transaction
	int max_page[4] = {0, 0};									//Max page for each section
	int dimension[4][2] = 										//Maximum index for each section
	{
		{1, 6}, {3, 1}, {2, 5}, {3, 17}							//In the section, there are x * y choices
	};
	
	orderc.previous = 2;										//Initialize previous
		
	initialize_search();										//Clear the global variable about searching
	bntree(&pRoot,false, MERCHDT);
	Merchmask = 2;												//Search category
	sprintf(search_content[1], "%d", orderc.current[0].verc);
	search_data(MERCHDT, pRoot);
	int mercount = bntree(&pfRoot, true, MERCHDT);				//Create binary tree
	max_page[2] = mercount? (mercount - 1) / (dimension[2][0] * dimension[2][1]): 0;	//Initialize max_page
	max_page[3] = different? (different - 1)/ dimension[3][1]: 0;
	
	order_fresh(&orderc, pfRoot, max_page, dimension, mercount, pList, different, grand_total);
	while(true)
	{
		c1 = getch();
		if(c1 == 224)
		{
			c2 = getch();
			switch(c2)
			{
				case 80://Down
					orderc.current[orderc.section].verc++;
					break;										//the last control key is 'down' from the last item of previoius page
				case 72://Up
					orderc.current[orderc.section].verc--;
					break;
				case 77://Right
					orderc.current[orderc.section].horc++;
					break;
				case 75://Left
					orderc.current[orderc.section].horc--;
					break;
				case 73://PageUp
					if(orderc.current[orderc.section].page > 0)						//Ensure not the first page so that page-- is valid
					{
						orderc.current[orderc.section].page--;
						orderc.current[orderc.section].verc = orderc.current[orderc.section].page * dimension[orderc.section][1];	//Select the first item at the page
					}
					break;
				case 81://PageDown
					if(orderc.current[orderc.section].page < max_page[orderc.section])
					{
						orderc.current[orderc.section].page++;
						orderc.current[orderc.section].verc = orderc.current[orderc.section].page * dimension[orderc.section][1];
					}	
					break;
				default:
					DebugPrintf("I should not get here!");
					break;
			}
		}
		//Go to another section when border is crossed
		if(orderc.current[orderc.section].horc < 0)				//Outside left boundary of a section
		{
			orderc.current[orderc.section].horc = 0;

			switch(orderc.section)
			{
				case 1: case 2:
				orderc.previous = orderc.section;
				orderc.section = 0;
				break;
				case 3:											//Back to previous section
				if(orderc.previous == 2 && mercount == 0)		//If No record in section 2, go back to section 1
					orderc.previous = 1;						
				orderc.section = orderc.previous;
				orderc.previous = 3;
				break;
			}
		}
		if(orderc.current[orderc.section].verc < 0 )			//Outside upper boundary
		{
			orderc.current[orderc.section].verc = 0;
			if(orderc.section == 2)
			{
				orderc.section = 1;
				orderc.previous = 2;
			}
		}
		if(orderc.section == 2 && orderc.current[2].verc % dimension[2][1] == dimension[2][1] - 1 && c1 == 224 && c2 == 72)	
		{//Special adjustment for ection 2 upper boundary
			orderc.current[2].verc++;
			orderc.section = 1;
			orderc.previous = 2;
		}
		if(orderc.current[orderc.section].horc > dimension[orderc.section][0] - 1)	//Outside right boundary
		{
			orderc.current[orderc.section].horc = dimension[orderc.section][0] - 1;
			switch(orderc.section)
			{
				case 0:
				orderc.section = orderc.previous;
				orderc.previous = 0;
				break;
				case 1: case 2:
				if(different > 0)
				{
					orderc.previous = orderc.section;
					orderc.section = 3;
				}
				break;
			}
		}
		if(orderc.current[orderc.section].verc > dimension[orderc.section][1] - 1)	//Outside lower boundary
		{
			switch(orderc.section)
			{
				case 1:
				if(mercount > 0)											//If section 2 has record
				{
					orderc.current[1].verc = dimension[1][1] - 1;
					orderc.previous = orderc.section;
					orderc.section = 2;
				}
				break;
				case 2://Prevent outside boundary when pressing 'Down' in Section 2
				if(orderc.section == 2 && c1 == 224 && c2 == 80 && orderc.current[2].verc % dimension[2][1] == 0)
					orderc.current[2].verc--;
				break;
			}
		}													
		
		
		//Auto page ++ (Except Section 2)
		if(orderc.section != 2 && c1 == 224 && c2 == 80 					//'Down' Pressed
		&& orderc.current[orderc.section].verc % dimension[orderc.section][1] == 0 
		&& orderc.current[orderc.section].page < max_page[orderc.section])
		{
			DebugPrintf("auto page++ conducted.");
			orderc.current[orderc.section].page++;
		}
		//Auto page -- (Except Section 2)
		if(orderc.section != 2 && c1 == 224 && c2 == 72 					//'Up' pressed
		&& orderc.current[orderc.section].verc % dimension[orderc.section][1] == dimension[orderc.section][1] - 1
		&& orderc.current[orderc.section].page > 0)
		{
			DebugPrintf("auto page-- conducted.");
			orderc.current[orderc.section].page--;
		}
		//Set the verc limit of verc and horc(for sections that have multiple pages)
		if(orderc.section == 0)
		{
			if(orderc.current[0].verc > PRTYPE - 1)
				orderc.current[0].verc = PRTYPE - 1;
		}
		if(orderc.section == 2)
		{
			if(orderc.current[2].verc * dimension[2][0] + orderc.current[2].horc + 1 > mercount)
				orderc.current[2].verc--;
		}
		if(orderc.section == 3)
		{
			if(orderc.current[3].verc > different - 1)
				orderc.current[3].verc--;
		}
		
		//Section 0: Refilter by merch category
		if(orderc.section == 0)
		{
			sprintf(search_content[1], "%d", orderc.current[0].verc);
			search_data(MERCHDT, pRoot);	//Conduct searching using global mask, distribute fserial and store results in file
			mercount = bntree(&pfRoot, true, MERCHDT);	//create new binary tree and update number of Merch under search mode
			orderc.current[2].verc = 0;					//initialize item chosen
			orderc.current[2].horc = 0;
			max_page[2] = mercount?((mercount - 1))/ (dimension[2][0] * dimension[2][1]): 0;	//update max page
			orderc.current[2].page = 0;					//initialize item chosen
		}
		
		if(c1 == 13)//Enter pressed
		{
			switch(orderc.section)
			{
				case 1:											//Upper panel
				switch(orderc.current[1].horc % 3)
				{
					case 0://Leave
					release_node(pRoot);
					pRoot = NULL;
					release_node(pfRoot);
					pRoot = NULL;
					return 0;
					
					case 1://Sort
					sort_detect(MERCHDT);
					release_node(pRoot);
					pRoot = NULL;
					release_node(pfRoot);
					pfRoot = NULL;
					return 1;
						
					case 2://Search
					
					search_detect(MERCHDT);
					sprintf(search_content[1], "%d", orderc.current[0].verc);	//Override categorize search
					Merchmask |= 2;								//Enable categorize filter
					search_data(MERCHDT, pRoot);	//Conduct searching using global mask, distribute fserial and store results in file
					mercount = bntree(&pfRoot, true, MERCHDT);//create new binary tree and update number of Merch under search mode
					orderc.current[2].verc = 0;					//initialize item chosen
					orderc.current[2].horc = 0;
					max_page[2] = mercount?((mercount - 1))/ (dimension[2][0] * dimension[2][1]): 0;	//update max page
					orderc.current[2].page = 0;					//initialize item chosen
					break;
				}
				break;
				
				case 2:											//Item panel
				select_quantity(&pList, retrieve_node(pfRoot, orderc.current[2].verc * dimension[2][0] 
					+ orderc.current[2].horc, true)->pDT, &different);
				break;
				
				case 3:											//Shopping cart
				switch(orderc.current[3].horc % 3)
				{
					case 0://Edit
					if(different == 0)							//No item
						break;
					pCurrent = pList;
					for(int i = 0; i < orderc.current[3].verc; i++)
						pCurrent = pCurrent->pright;
					select_quantity(&pList, pCurrent->pDT, &different);
					break;
					
					case 1://Clear
					if(message_box(1, 6, "Clear the %d item%s in cart?", different, different == 1? "":"s"))
					{
						release_link(pList);
						pList = NULL;
						different = 0;
						orderc.current[3].verc = -1;
					}
					break;
						
					case 2://Proceed
					if(different < 1)
						message_box(0, 1, "Nothing in the cart!");
					else if(message_box(1, 1, "Confirm the purchase?"))
					{
						finished = produce_trans(0, current_loginer.prkey_P, pList, different);
						detail_detect(&finished);							//Display the detail page and choose to print receipt
						reduce_merch(pList, different);						//Reduce the number of merchandises
						update_person(&current_loginer, grand_total);		//Increase the sales of the person
						release_link(pList);								//Clear the cart
						pList = NULL;
						different = 0;
						orderc.current[3].verc = -1;
						release_node(pRoot);
						pRoot = NULL;
						release_node(pfRoot);
						pfRoot = NULL;
						return 1;											//Quit and rebuild binary tree
					}
					
					break;
				}
				break;
			}
		}
		//Some immediate update
		if(mercount == 0)													//Adjust for empty situation
		{
			orderc.current[2].verc = 0;
			orderc.current[2].horc = 0;
		}
		max_page[3] = different? (different - 1)/ dimension[3][1]: 0;		//Update max_page for shopping cart
		if(orderc.current[3].page > max_page[3])
			orderc.current[3].page = max_page[3];
			
		grand_total = 0;
		pCurrent = pList;
		while(pCurrent)														//Calculate grand total
		{
			grand_total += pCurrent->serial * ((Merch*)pCurrent->pDT)->price;
			pCurrent = pCurrent->pright;
		}
		order_fresh(&orderc, pfRoot, max_page, dimension, mercount, pList, different, grand_total);
	}
	
}

void order_fresh(Ctrls* orderc, Node* pfRoot, int max_page[4], int dimension[4][2], int mercount, Node* pList, int different, long long grand_total)
{
	apicolor(15);
	system("cls");
	
	//Display the heading
	apicolor(16);
	for(int i = 0; i <= 1; i++)
	{
		apisetxy(1, i);
		for(int j = 0; j < 14; j++)
		printf(" ");
	}
	apicolor(160);
	apisetxy(3, 1);
	for(int j = 0; j < 14; j++)
		printf(" ");
	apisetxy(8, 1);
	printf("Order");
	apicolor(15);
	apisetxy(3, 2);
	printf("%s %s", current_loginer.firstname, current_loginer.surname);	//Display the Login staff name
	
	
	//Print Section 0: Left panel, which has buttons of  all merchandize types for filtering
	char left_buttons[PRTYPE][3][MAXLEN] = {};							//Store each line for each button(max 3 lines)
	char button_line[PRTYPE] = {0};										//Record the number of lines of the button
	for(int i = 0; i < PRTYPE; i++)
	{
		int line = 0;
		char* pstr = NULL;
		char origin[MAXLEN];											//Copy of merchType
		strcpy(origin, merchType[i]);
		char slice[MAXLEN];												//Slice of separated line
		
		pstr = strtok(origin, " ");										//Separate original string by ' '
		strcpy(slice, pstr);
		center_align(slice, 9);
		strcpy(left_buttons[i][0], slice);								//Store the aligned line
		if(strlen(pstr) == strlen(merchType[i]))
		{
			continue;
			button_line[0] = 0;
		}
		pstr = strtok(NULL, " ");										//Separate the second time
		while(pstr)
		{
			strcpy(slice, pstr);
			center_align(slice, 9);
			strcpy(left_buttons[i][++line], slice);						//Concat slice to button with '\n'
			pstr = strtok(NULL, " ");
		}
		button_line[i] = line;											//Record the number of lines of the button
	}
	for(int i = 0; i < (orderc->current[0].page == max_page[0]? (PRTYPE) % dimension[0][1]: dimension[0][1] - 1); i++)	//Display buttons
	{
		if(i == orderc->current[0].verc && orderc->section == 0)
			apicolor(228);												//Yellow - chosen button
		else if (i == orderc->current[0].verc && orderc->section != 0)
			apicolor(96);												//Green-yellow - on this type but not controlling the button
		else
			apicolor(112);												//Light grey - not chosen button
		for(int j = 0; j < button_line[i] + 1; j++)
		{
			apisetxy(1 , 4 + 4 * (i % dimension[0][1]) + j);
			puts(left_buttons[i][j]);
		}
	}
	//Print Section 1: Upper panel
	char up_buttons[3][MAXLEN] = {"Leave", "Sort", "Search"};
	for(int i = 0; i < 3; i++)
		center_align(up_buttons[i], 9);										//Center align the titles in each button

	for(int i = 0; i < 3; i++)											//Display horizontal buttons
	{
		apicolor(128);													//Grey sub-background
		if(i == orderc->current[1].horc)
			apicolor(96);												//Green-yellow - shadow of chosen button
			
		for(int j = 0; j <= 2; j++)
		{
			apisetxy(21 + 11 * i, j);
			for(int j = 0; j < 11; j++)	
			printf(" ");
		}

		if(i == orderc->current[1].horc && orderc->section == 1)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		if(i == 2 && Merchmask ^ 2)										//Xor
			apicolor(160);												//Green - search button if in search mode
		apisetxy(22 + 11 * i, 1);
		printf("%9s", up_buttons[i]);
	}
	
	//Print Section 2: Item panel
	if(mercount == 0)
	{
		apicolor(3);
		apisetxy(24,10);
		printf("No item exists");
	}
	
	apisetxy(20,10);
	for(int i = orderc->current[2].page * dimension[2][0] * dimension[2][1]; i < (orderc->current[2].page < max_page[2]? 
	(orderc->current[2].page + 1) * dimension[2][0] * dimension[2][1]: mercount) ; i++)
	{
		if(i == orderc->current[2].verc * dimension[2][0] + orderc->current[2].horc && orderc->section == 2)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(11 + 22 * (i % dimension[2][0]), 4 + 4 * ((i / dimension[2][0]) % dimension[2][1]));
		display_merch(retrieve_node(pfRoot, i, true)->pDT);
	}
	
	apicolor(15);
	apisetxy(15, 24);
	printf("Page %2d of %2d; Item %2d of %2d", orderc->current[2].page + 1, max_page[2] + 1,
		orderc->current[2].verc * dimension[2][0] + orderc->current[2].horc + 1, mercount);
	apisetxy(15,25);
	DebugPrintf("section = %d, verc=%d, horc=%d", orderc->section, orderc->current[2].verc, orderc->current[2].horc);
	
	apicolor(48);
	for(int i = 0; i <= 24; i++)
	{
		apisetxy(54, i);
		printf(" ");
	}
	
	//Print Section 3: Cart panel
	Node* pCurrent = pList;												//Used for traversal
	apicolor(16);
	for(int i = 0; i <= 1; i++)
	{
		apisetxy(59, i);
		for(int j = 0; j < 12; j++)
		printf(" ");
	}
	apicolor(160);
	apisetxy(61, 1);
	for(int j = 0; j < 12; j++)
		printf(" ");
	apisetxy(65, 1);
	printf("Cart");
	
	apicolor(15);														//Field names
	apisetxy(56, 2);
	printf("Item");
	apisetxy(66, 2);
	printf("Qty");
	apisetxy(71, 2);
	printf("Total");
	
	if(different == 0)
	{
		apicolor(3);
		apisetxy(59,10);
		printf("Nothing bought");
	}
	
	for(int i = 0; i < orderc->current[3].page * dimension[3][1]; i++)	//Item List
	{
		pCurrent = pCurrent->pright;
		
	}
	for(int i = 0; i < (orderc->current[3].page == max_page[3]? (different - 1) % dimension[3][1] + 1: dimension[3][1]); i++)
	{
		apicolor(i == orderc->current[3].verc % dimension[3][1] && orderc->section == 3?228:15);
		apisetxy(56, 4 + i);
		if(pCurrent)
			list_bought(pCurrent->pDT, pCurrent->serial);
		else
		{
			DebugPrintf("List_bought NULL\n");
			DebugSystem("pause");
		}
		pCurrent = pCurrent->pright;
	}
	
	char cart_buttons[3][MAXLEN] = {"Edit", "Clear", "Proceed"};		//Three buttons
	for(int i = 0; i < 3; i++)
		center_align(cart_buttons[i], 7);								//Center align the titles in each button

	for(int i = 0; i < 3; i++)											//Display horizontal buttons
	{
		if(i == orderc->current[3].horc && orderc->section == 3)
			apicolor(228);												//Yellow - chosen button
		else
			apicolor(112);												//Light grey - not chosen button
		apisetxy(55 + 8 * i, 23);
		printf("%7s", cart_buttons[i]);
	}
	
	apicolor(15);														//Grand total and page display
	apisetxy(56, 22);
	printf("Grand Total:%8d", grand_total);
	apisetxy(55,24);
	printf("Page %d of %d;Item %d of %d", orderc->current[3].page + 1, max_page[3] + 1, orderc->current[3].verc + 1, different);
	
	return;
	
}
