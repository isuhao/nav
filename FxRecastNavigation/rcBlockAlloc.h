//------------------------------------------------------------------------------
// 文件名:		rcBlockAlloc.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月9日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#ifndef _RCBLOCKALLOC_H_
#define _RCBLOCKALLOC_H_
#include <stdlib.h>
#include <stddef.h>

template<class type, int blockSize>
class rcBlockAlloc {
public:
	rcBlockAlloc( void );
	~rcBlockAlloc( void );

	void					Shutdown( void );

	type *					Alloc( void );
	void					Free( type *element );

	int						GetTotalCount( void ) const { return total; }
	int						GetAllocCount( void ) const { return active; }
	int						GetFreeCount( void ) const { return total - active; }

private:
	typedef struct element_s {
		struct element_s *	next;
		type				t;
	} element_t;
	typedef struct block_s {
		element_t			elements[blockSize];
		struct block_s *	next;
	} block_t;

	block_t *				blocks;
	element_t *				freelst;
	int						total;
	int						active;
};

template<class type, int blockSize>
rcBlockAlloc<type,blockSize>::rcBlockAlloc( void ) {
	blocks = 0;
	freelst = 0;
	total = active = 0;
}

template<class type, int blockSize>
rcBlockAlloc<type,blockSize>::~rcBlockAlloc( void ) {
	Shutdown();
}

template<class type, int blockSize>
type *rcBlockAlloc<type,blockSize>::Alloc( void ) {
	if ( !freelst ) {
		block_t *block = (block_t*)malloc(sizeof(block_t));
		block->next = blocks;
		blocks = block;
		for ( int i = 0; i < blockSize; i++ ) {
			block->elements[i].next = freelst;
			freelst = &block->elements[i];
		}
		total += blockSize;
	}
	active++;
	element_t *element = freelst;
	freelst = freelst->next;
	element->next = 0;
	return &element->t;
}

template<class type, int blockSize>
void rcBlockAlloc<type,blockSize>::Free( type *t ) {
	element_t *element = (element_t *)( ((unsigned char*)t) - offsetof(element_t,t) );
	element->next = freelst;
	freelst = element;
	active--;
}

template<class type, int blockSize>
void rcBlockAlloc<type,blockSize>::Shutdown( void ) {
	while( blocks ) {
		block_t *block = blocks;
		blocks = blocks->next;
		free((void*)block);
	}
	blocks = 0;
	freelst = 0;
	total = active = 0;
}

#endif
