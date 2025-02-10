/*
 * Copyright 2021-2024 Morse Micro
 *
 * This file is licensed under terms that can be found in the LICENSE.md file in the root
 * directory of the Morse Micro IoT SDK software package.
 */

#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#include "FreeRTOS.h"
#include "task.h"

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE         ( ( size_t ) 8 )

/* MSB of the xBlockSize member of an BlockLink_t structure is used to track
 * the allocation status of a block.  When MSB of the xBlockSize member of
 * an BlockLink_t structure is set then the block belongs to the application.
 * When the bit is free the block is still part of the free heap space. */
#define heapBLOCK_ALLOCATED_BITMASK    ( ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 ) )
#define heapBLOCK_SIZE_IS_VALID( xBlockSize )    ( ( ( xBlockSize ) & heapBLOCK_ALLOCATED_BITMASK ) == 0 )
#define heapBLOCK_IS_ALLOCATED( pxBlock )        ( ( ( pxBlock->xBlockSize ) & heapBLOCK_ALLOCATED_BITMASK ) != 0 )
#define heapALLOCATE_BLOCK( pxBlock )            ( ( pxBlock->xBlockSize ) |= heapBLOCK_ALLOCATED_BITMASK )
#define heapFREE_BLOCK( pxBlock )                ( ( pxBlock->xBlockSize ) &= ~heapBLOCK_ALLOCATED_BITMASK )

/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK * pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize;                     /*<< The size of the free block. */
} BlockLink_t;

/* The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned. */
static const size_t xHeapStructSize = ( sizeof( BlockLink_t ) + ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & ~( ( size_t ) portBYTE_ALIGNMENT_MASK );

void *pvPortRealloc( void *pv, size_t xWantedSize)
{
	size_t xMoveSize;
	size_t xSourceSize;
	BlockLink_t *pxLink;
	void *pvReturn = NULL;
	uint8_t *puc = ( uint8_t * ) pv;

	if (xWantedSize > 0)
	{
		if (pv != NULL)
		{
			/* The memory being freed will have an BlockLink_t structure immediately before it. */
			puc -= xHeapStructSize;

			/* This casting is to keep the compiler from issuing warnings. */
			pxLink = ( void * ) puc;

			configASSERT( heapBLOCK_IS_ALLOCATED(pxLink) );
			configASSERT( pxLink->pxNextFreeBlock == NULL );

			/* Check allocate block */
			if (heapBLOCK_IS_ALLOCATED(pxLink))
			{
				if (pxLink->pxNextFreeBlock == NULL)
				{
					/* invert the heapBLOCK_ALLOCATED_BITMASK in block size and minus the linked list structure size */
					xSourceSize = (pxLink->xBlockSize & ~heapBLOCK_ALLOCATED_BITMASK) - xHeapStructSize;

					pvReturn = pvPortMalloc(xWantedSize);

					/* Check creation */
					if (pvReturn != NULL)
					{
						/* choose smaller size as move size */
						if (xSourceSize < xWantedSize)
						{
							xMoveSize = xSourceSize;
						}
						else
						{
							xMoveSize = xWantedSize;
						}

						memcpy(pvReturn, pv, xMoveSize);
						vPortFree(pv);
					}
				}

			}
		}
		else
		{
			pvReturn = pvPortMalloc(xWantedSize);
		}
	}
	else
	{
		/* if size <= 0 passed in */
		pvReturn = NULL;
	}
	/* Exit with memory block */
	return pvReturn;
}
