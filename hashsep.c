       #include "fatal.h"
       #include "hashsep.h"
       #include <stdlib.h>
       
       #define printf(...)
       #define MinTableSize (10)

        struct ListNode
        {
            ElementType Element;
            Position    Next;
        };

        typedef Position List;

        /* *TheList will be an array of lists, allocated later */
        /* The lists use headers (for simplicity), */
        /* though this wastes space */
        struct HashTbl
        {
            int TableSize;
            List *TheLists;
        };

        /* Return next prime; assume N >= 10 */
        static int
        NextPrime( int N )
        {
            int i;

            if( N % 2 == 0 )
                N++;
            for( ; ; N += 2 )
            {
                for( i = 3; i * i <= N; i += 2 )
                    if( N % i == 0 )
                        goto ContOuter;  /* Sorry about this! */
                return N;
              ContOuter: ;
            }
        }

        /* Hash function for ints */
        Index
        Hash( ElementType Key, int TableSize )
        {
            return Key % TableSize;
        }

/* START: fig5_8.txt */
        HashTable
        InitializeTable( int TableSize )
        {
            HashTable H;
            int i;

/* 1*/      if( TableSize < MinTableSize )
            {
/* 2*/          Error( "Table size too small" );
/* 3*/          return NULL;
            }

            /* Allocate table */
/* 4*/      H = malloc( sizeof( struct HashTbl ) );
/* 5*/      if( H == NULL )
/* 6*/          FatalError( "Out of space!!!" );

/* 7*/      H->TableSize = NextPrime( TableSize );

            /* Allocate array of lists */
/* 8*/      H->TheLists = malloc( sizeof( List ) * H->TableSize );
/* 9*/      if( H->TheLists == NULL )
/*10*/          FatalError( "Out of space!!!" );

            /* Allocate list headers */
/*11*/      for( i = 0; i < H->TableSize; i++ )
            {
/*12*/          H->TheLists[ i ] = malloc( sizeof( struct ListNode ) );
/*13*/          if( H->TheLists[ i ] == NULL )
/*14*/              FatalError( "Out of space!!!" );
                else
/*15*/              H->TheLists[ i ]->Next = NULL;
            }

/*16*/      return H;
        }
/* END */

        /* This is by time (i.e. sec ) elapsed report, use the Time Hash table, key is time */
        /* Table entry number changes every second monotonicly */
        void ElementAgeReport (ElementType Key, HashTable H)
        {
        	unsigned int counts[] = {0, 0, 0, 0, 0};
        	unsigned int TotalCounts = 0;
        	Position P;
           	List L;
        	Index i = Hash( Key, H->TableSize );

        	for (int j = i; j>=0; j--){
        	     L = H->TheLists[ j ];
        	     P = L;
        	     while (P->Next  )
        	     {
        	    	TotalCounts++;
        	        if ( i - j  < 5)
        	        	counts[0]++;
        	        if ( i - j >= 5 &&  i - j < 10)
        	        	counts[1]++;
        	        if ( i - j >= 10 &&  i - j < 100)
        	             counts[2]++;
        	        if ( i - j >= 100 &&  i - j < 1000)
        	             counts[3]++;
        	        if (  i - j >=  1000)
        	            counts[4]++;

        	        P = P->Next;
        	     }

        	 }

        	fprintf(stderr, "Current Allocations by age # = 8,123 allocations \n");
        	for (int i = 0; i< 5; i++)
        	{
        		if ( i == 0)
        			fprintf(stderr, "< 1 sec %u\n", counts[i]);

        		if ( i == 1)
        			fprintf(stderr, "< 10 sec %u\n", counts[i]);

        		if ( i == 2)
        		    fprintf(stderr, "< 100 sec %u\n", counts[i]);

        		if ( i == 3)
        		    fprintf(stderr, "< 1000 sec %u\n", counts[i]);

        		if ( i == 4)
        			fprintf(stderr, ">= 1000 sec %u\n", counts[i]);
        	}

        }

        /* */
        int
		ElementCount ( ElementType Key, HashTable H )
        {
        	unsigned int count = 0;
        	Position P;
        	List L;
        	Index i = Hash( Key, H->TableSize );

        	for (int j = i; j>=0; j--){

        		L = H->TheLists[ j ];
        		P = L;
        		while (P->Next  )
        		{
        			count++;
        			P = P->Next;
        		}

        	}

            return count;
        }
        void
       // Remove (ElementType Key, HashTable H )
        Remove ( ElementType Key, ElementType Value, HashTable H )
        {
             Position P;
             List L;

             /* Hash the key to get List in the indexed Tbl */
             L = H->TheLists[ Hash( Key, H->TableSize ) ];
             // Pos = Find( Key, H );

             if (L == NULL){
                    FatalError(" Remove() finds no elements to remove !!");
             }
             P = L;
             while (P->Next  ) {
            	 if (P->Next->Element == Value){
            		 List tmp = P->Next;
            		 P->Next = P->Next->Next;
            		 free(tmp);
            		 break;
            	 }
             }
         }

/* START: fig5_9.txt */
        Position
        Find( ElementType Key, HashTable H )
        {
            Position P;
            List L;

            /* Hash the key to get List in the indexed Tbl */
/* 1*/      L = H->TheLists[ Hash( Key, H->TableSize ) ];
/* 2*/      P = L->Next;
/* 3*/      while( P != NULL && P->Element != Key )
                                /* Probably need strcmp!! */
/* 4*/          P = P->Next;
/* 5*/      return P;
        }
/* END */

/* START: fig5_10.txt */
        void
        //Insert( ElementType Key, HashTable H )
        Insert( ElementType Key, ElementType Value, HashTable H )
        {
            Position Pos, NewCell;
            List L;

	    printf("...Insert enter \n");
/* 1*/      Pos = Find( Key, H );
/* 2*/      if( Pos == NULL )   /* Key is not found */
            {
/* 3*/          NewCell = malloc( sizeof( struct ListNode ) );
/* 4*/          if( NewCell == NULL )
/* 5*/              FatalError( "Out of space!!!" );
                else
                {
/* 6*/              L = H->TheLists[ Hash( Key, H->TableSize ) ];
/* 7*/              NewCell->Next = L->Next;
/* 8*/              NewCell->Element = Value;  /* Probably need strcpy! */
/* 9*/              L->Next = NewCell;
                }
            }
	    printf("...Insert exit\n");
        }
/* END */

        ElementType
        Retrieve( Position P )
        {
            return P->Element;
        }

        void
        ShowTable( HashTable H )
        {
            int i;

            for( i = 0; i < H->TableSize; i++)
            {
            	//
                Position P = H->TheLists[ i ];
                //Position Tmp;

                while( P != NULL )
                {
                    //Tmp = P->Next;
                    if (P->Element)
                     fprintf(stderr, "index =0x%.8X Value=0x%.8X \n", i, P->Element);
                    P = P->Next;
                }
            }

        }

        void
        DestroyTable( HashTable H )
        {
            int i;

            for( i = 0; i < H->TableSize; i++ )
            {
                Position P = H->TheLists[ i ];
                Position Tmp;

                while( P != NULL )
                {
                    Tmp = P->Next;
                    free( P );
                    P = Tmp;
                }
            }

            free( H->TheLists );
            free( H );
        }
