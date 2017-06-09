/****************************************************************************
 *                                                                          *
 * File    : main.c                                                         *
 *                                                                          *
 * Purpose : Console mode (command line) program.                           *
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/****************************************************************************
 *                                                                          *
 * Function: main                                                           *
 *                                                                          *
 * Purpose : Main entry point.                                              *
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/
#define CARD_MASK 0x3f
#define MAST_MASK 0xc0
#define MAST_P 0x00
#define MAST_K 0x40
#define MAST_B 0x80
#define MAST_C 0xc0
#define MAX_CARDS 52
#define MAX_CARDS_M 13
#define MAX_MOVES 65536
#define INVALID_MOVE -1

/////////////////////////////////////////////////////////////////////
typedef struct
{
	int Moves[MAX_MOVES];// 0..f|0..f. 
	int Size;
} TMoves;

int AddMove(TMoves*mp,int Move)
{
	if (mp)
	if (mp->Size<MAX_MOVES)
	{
		mp->Moves[mp->Size]=Move;
		mp->Size++;
		return 1;
	};
	return 0;
};

void InitMoves(TMoves*mp)
{
	if (mp) mp->Size=0;
};

int LastMove(TMoves*mp)
{
	if (mp)
	if (mp->Size>0)
		return mp->Moves[mp->Size-1];
	return INVALID_MOVE;
};

/////////////////////////////////////////////////////////////////////
typedef struct
{
	int Count;
	int Cards[MAX_CARDS];
} TCardList;

int GetMast(int Crd)
{
	return ((Crd>>6)&3);
};

int GenerateCard(int MCrd)
{
	int mast=(rand()>>(rand()&7))&3;
	return (mast*MAST_K)|(rand()%MCrd);
};

void AddCardToList(TCardList*cl,int card)
{
	if (cl)
	{
		cl->Cards[cl->Count]=card;
		cl->Count++;
	};
};

int GetLastCard(TCardList*cl)
{
	if (cl)
	if (cl->Count>=0)
		return cl->Cards[cl->Count-1];
	return -1;
};

int CardInList(TCardList*cl,int card)
{
	if (cl)
	for (int i=0;i<cl->Count;i++)
		if (cl->Cards[i]==card)
			return 1;
	return 0;
};

typedef struct
{
int OUT[4];
int FC[4];
TCardList POLE[8];
} TGameState;

int CardInPole(TGameState*gs,int card)
{
	if (gs)
	for (int i=0;i<8;i++)
		if (CardInList(gs->POLE+i,card))
			return 1;
	return 0;
};

void Sdat(TGameState*gs)
{
	if (gs==NULL)
		return;
	//init
	gs->FC[0]=gs->FC[1]=gs->FC[2]=gs->FC[3]=-1;
	gs->OUT[0]=gs->OUT[1]=gs->OUT[2]=gs->OUT[3]=-1;
	for (int i=0;i<8;i++) gs->POLE[i].Count=0;
	//sdacha
	for (int i=0;i<MAX_CARDS;i++)
	{
		int NewCard=GenerateCard(MAX_CARDS_M);
		int cnt=0;
		while(CardInPole(gs,NewCard))
		{
			cnt++;
			NewCard=GenerateCard(MAX_CARDS_M);
			if (cnt>10000) break;
		};
		if (cnt>10000)
		for (int m=0;m<4;m++)
		{
		for (int j=0;j<MAX_CARDS_M;j++)
			if (CardInPole(gs,j+MAST_K*m)==0)
						{NewCard=j+MAST_K*m;break;};
		if (CardInPole(gs,NewCard)==0)
					break;
		};
		AddCardToList(gs->POLE+(i&7),NewCard);
	};
};


TGameState MGS;
TMoves MV;

int ConcatCard(int c_to,int c_from)
{
	int b0=c_to&MAST_MASK;
	if (b0<MAST_B) b0=0; else b0=1;
	int b1=c_from&MAST_MASK;
	if (b1<MAST_B) b1=0; else b1=1;
	if (b1^b0)
	if ((c_to&CARD_MASK)==((c_from&CARD_MASK)+1))
		return 1;
	return 0;
};

void DoMove(TGameState * gs,int m)
{
	int From=m&0xff;
	int To=m>>8;
	if (From<8)
	{
		if (To<8)
		{
			//� ���� �� ����
			gs->POLE[From].Count--;
			gs->POLE[To].Cards[gs->POLE[To].Count]=gs->POLE[From].Cards[gs->POLE[From].Count];
			gs->POLE[To].Count++;
		}
		else
		if (To<12)
		{
			//� ���� �� ��. ��.
			gs->POLE[From].Count--;
			gs->FC[To-8]=gs->POLE[From].Cards[gs->POLE[From].Count];
		}
		else
		{
			//� ���� �� �����
			gs->POLE[From].Count--;
			gs->OUT[To-12]=gs->POLE[From].Cards[gs->POLE[From].Count];
		};
	}
	else
	if (From<12)
	{
		//From>=8..12 - � ��. ��.
		if (To<8)
		{
			//� ��.��. �� ����
			AddCardToList(gs->POLE+To,gs->FC[From-8]);
			gs->FC[From-8]=-1;
		}
		else
		{
			//To>=12 & To<16
			//� ��.��. �� �����
			gs->OUT[To-12]=gs->FC[From-8];
			gs->FC[From-8]=-1;
		};
	}
	else
	{
		//From>=12 - � ������
		if (To<8)
		{
			//� ������ �� ����
			AddCardToList(gs->POLE+To,gs->OUT[From-12]);
		}
		else
		if (To<12)
		{
			//� ������ �� ��. ��.
			gs->FC[To-8]=gs->OUT[From-12];
		};
		gs->OUT[From-12]=((gs->OUT[From-12]&CARD_MASK)-1)|(gs->OUT[From-12]&MAST_MASK);
	};
};

void UndoMove(TGameState * gs,TMoves * m)
{
	if (gs&&m)
	if (m->Size)
	{
		int swm=m->Moves[m->Size-1];
		swm=(swm>>8)|((swm&0xff)<<8);
		DoMove(gs,swm);
		m->Size--;
	};
};

int MoveValid(TGameState * gs,int m)
{
	if (gs==NULL)
		return 0;
	int From=m&0xff;
	int To=m>>8;
	if (From<8)
	{
		if (To<8)
		{
			//� ���� �� ����
			if (gs->POLE[From].Count>0)
			{
				if (gs->POLE[To].Count==0)
					return 1;
				return ConcatCard(GetLastCard(gs->POLE+To),GetLastCard(gs->POLE+From));
			};
		}
		else
		if (To<12)
		{
			//� ���� �� ��. ��.
			if (gs->POLE[From].Count>0 && gs->FC[To-8]<0)
				return 1;
		}
		else
		{
			//� ���� �� �����
			if (gs->POLE[From].Count>0)
			{
			int LC=GetLastCard(gs->POLE+From);
				if ((GetMast(LC)+12)==To)
				if ((LC&CARD_MASK)==((gs->OUT[To-12]+1)&CARD_MASK))
						return 1;
			};
		};
	}
	else
	if (From<12)//�� ��. ��
	{
		if (To<8)
		{
			//�� ����
			if (gs->FC[From-8]>=0)
			{
				if (gs->POLE[To].Count==0)
					return 1;
				return ConcatCard(GetLastCard(gs->POLE+To),gs->FC[From-8]);
			};
		}
		else
		if (To<12)
		{
				return 0;
		}
		else
		{
			//� ��.�� �� �����
			if (gs->FC[From-8]>=0)
			{
			int LC=gs->FC[From-8];
				if ((GetMast(LC)+12)==To)
				if ((LC&CARD_MASK)==((gs->OUT[To-12]+1)&CARD_MASK))
						return 1;
			};
		};
	};
	return 0;
};
int EndGame(TGameState * gs)
{
	for (int i=0;i<4;i++)
		if ((gs->OUT[i]&CARD_MASK)!=12)
			return 0;
	return 1;
};

int MoveOut()
{
	int NewMoveO;
	int From,To;
	for (From=0;From<12;From++)
	{
		for (To=12;To<16;To++)
		{
			NewMoveO=From|(To<<8);
			if (MoveValid(&MGS,NewMoveO))
				if (AddMove(&MV,NewMoveO))
				{
					printf("\n%X",NewMoveO);
					DoMove(&MGS,NewMoveO);
					return 1;
				};
		};
	};
	return 0;
};

void printCard(int crd)
{
	if (crd<0)
	{
		printf("    ");
		return;
	};
	char s[20];
	s[0]=0x20;
	s[1]=6-GetMast(crd);
	switch(GetMast(crd))
	{
		case 0:
		case 1:
			_textcolor(15);
		break;
		case 2:
		case 3:
			_textcolor(12);
		break;
	};
	switch (crd&0xf)
	{
		case 0:
			s[2]='A';break;
		case 10:
			s[2]='J';break;
		case 11:
			s[2]='D';break;
		case 12:
			s[2]='K';break;
		default:
			s[2]='1'+(crd&0xf);
			if ((crd&0xf)==9)
				{s[2]='1';s[3]='0';};
				break;
	};
	if ((crd&0xf)!=9) s[3]=0x20;
	s[4]=0;
	printf("%s",s);
	_textcolor(7);
};


int main(int argc, char *argv[])
{
	srand( (unsigned)time( NULL ) );
	Sdat(&MGS);
	InitMoves(&MV);
	char s[20];
	int F,T;
do
{
	printf("\n");
	for (int i=0;i<8;i++) printf(" %2d ",i+8);
	printf("\n");
	for (int i=0;i<4;i++) printCard(MGS.FC[i]);
	for (int i=0;i<4;i++) printCard(MGS.OUT[i]);
	printf("\n");
	int cntout=0;
	for (int i=0;i<(MAX_CARDS*8);i++)
	{
		int j=i>>3;
		if ((i&7)==0) printf("\n");
		if (MGS.POLE[i&7].Count>j)
			{
			printCard(MGS.POLE[i&7].Cards[j]);
			cntout=0;
			}
		else
			{printCard(-1);cntout++;};
		if (cntout>8)
			break;
	};
	printf("\n");
	for (int i=0;i<8;i++) printf(" %2d ",i);
	printf("\n");
	//
	printf("\n\nEG=%d",EndGame(&MGS));
	printf("\n\ntype: 0...15-move,n-new game,u-undo move,q-exit\nFrom=");
	gets(s);
	if (s[0]=='n'||s[0]=='N')
	{
		Sdat(&MGS);
		InitMoves(&MV);
		continue;
	};
	if (s[0]=='u'||s[0]=='U')
	{
		UndoMove(&MGS,&MV);
		continue;
	};
	if (s[0]=='q'||s[0]=='Q')
		return 0;
	//
	F=atoi(s);
	printf("To=");
	gets(s);
	T=atoi(s);
	int NewMove=F|(T<<8);
	if (MoveValid(&MGS,NewMove))
	{
		printf("Done!\n\n");
		DoMove(&MGS,NewMove);
		AddMove(&MV,NewMove);
	}
	else
		printf("InvalidMove!\n\n");
	//
	while(MoveOut());
}
while(F>=0&&T>=0);
	return 0;
}