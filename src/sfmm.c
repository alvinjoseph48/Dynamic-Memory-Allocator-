/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "errno.h"



void setBlockInfo(sf_block_info *infoBlock, int allocatedBit, int prev_allocBit, int zeroes, int blockSize, int requestedSize){
    infoBlock->allocated = allocatedBit;
    infoBlock->prev_allocated= prev_allocBit;
    infoBlock->two_zeroes = zeroes;
    infoBlock->block_size = blockSize;
    infoBlock->requested_size = requestedSize;
}

int intializeHeap(){

        if(sf_mem_grow()== NULL){
            sf_errno = ENOMEM;
            return 0;
        }
        sf_prologue * prologue  = sf_mem_start();
        sf_epilogue * addressOfEpi = sf_mem_end() - sizeof(sf_epilogue);

        memset(prologue, 0,40 ); // padding and half the prologue header
        memset((sf_mem_start()+sizeof(sf_prologue)) , 0 ,4048);
        memset(addressOfEpi, 0 , 8);

        int freeSpace = (PAGE_SZ - sizeof(sf_prologue)-sizeof(sf_epilogue))>>4;

         //check this intialization
        setBlockInfo(&prologue->header.info,1,0,0,0,0);
        setBlockInfo(&prologue->footer.info,1,0,0,0,0);
        setBlockInfo(&addressOfEpi->footer.info,1,0,0,0,0);


        sf_free_list_node *firstSentienel = sf_add_free_list(freeSpace<<4, sf_free_list_head.next);

         sf_header *firstHeader =  sf_mem_start() + sizeof(sf_prologue);

       firstHeader->links.next=  &(firstSentienel->head);
       firstHeader->links.prev = &(firstSentienel->head);

        firstSentienel->head.links.next = firstHeader;
        firstSentienel->head.links.prev = firstHeader;
        sf_free_list_head.next->head.info.allocated = 1;
        sf_free_list_head.next->head.info.block_size = freeSpace;

        sf_free_list_head.next->head.payload = (uint64_t)(sf_mem_start()+sizeof(sf_prologue) );
        setBlockInfo(&firstHeader->info,0,1,0,freeSpace,0);
        sf_footer *firstFooter = sf_mem_end() - sizeof(sf_epilogue) - sizeof(sf_footer);
        setBlockInfo(&firstFooter->info,0,1,0,freeSpace,0);
        return 1;
}
        /* give payload back and update payload for both orignal
        update linked list size
        update first original head ( allocated space) */
void splitBlock(int requestedSize, int nodeSize, sf_header *needtoBeUpdatedHeader){
    int allocatedBlockSpace = nodeSize;
   // if(requestedSize>0)
    int newFreeBlockSpace = (allocatedBlockSpace- requestedSize);
    printf("\n new Size %d \n" , newFreeBlockSpace);

     // DO I  have to add the new header SIZE ino account and mutiple of 16 !!!!!!
    // looping  through list and add to linked list the new orginal block
       // printf("\nBLOCK SIZE IS %d node is is  %d REQUESTED SIZe %d\n", nodeSize , allocatedBlockSpace, newFreeBlockSpace) ;

     sf_free_list_node * current_node;
    for(current_node = sf_free_list_head.next; current_node != &sf_free_list_head; current_node = current_node->next){
       // printf("\nin 1 %lu , %p %p",current_node->size, current_node, current_node->next);
        if(current_node->size == newFreeBlockSpace){
            // instead of creating new size block add new blocks size to already header
            printf("\n3 \n");

            sf_header * newHeader = (sf_header*)((char*)needtoBeUpdatedHeader+ (requestedSize) );
            setBlockInfo(&newHeader->info,0,1,0,newFreeBlockSpace>>4,0);

            sf_footer* newFooter = (sf_footer*)((char*) newHeader +((newHeader->info.block_size <<4))- sizeof(sf_footer));

            setBlockInfo(&newFooter->info,0,1,0,newFreeBlockSpace>>4,0);
            newHeader->payload = (uint64_t)(newHeader + sizeof(sf_block_info));
            // updating linked list LIFO  inserting at top
            newHeader->links.next = current_node->head.links.next;
            newHeader->links.prev = &current_node->head;
            current_node->head.links.next->links.prev = newHeader; // old 1st sf_header
            current_node->head.links.next = newHeader;
            break;
            // Case where 4106 being inserted before 4048
    }else if((newFreeBlockSpace < current_node->size  &&  current_node->prev == &sf_free_list_head )  ){
            sf_free_list_node *sentienel  = sf_add_free_list(newFreeBlockSpace,current_node);
            printf("\n1 \n");
            sf_header * newHeader = (sf_header*)((char*)needtoBeUpdatedHeader+ (requestedSize) );

             printf("\n HIIIII newHeader %p \n" , newHeader);
            setBlockInfo(&newHeader->info,0,1,0,newFreeBlockSpace>>4,0);
            //sf_show_heap();
             printf("\n1 \n");

            sentienel->head.links.prev = newHeader;
          sentienel->head.links.next = newHeader;
          newHeader->links.next = &sentienel->head;
          newHeader->links.prev = &sentienel->head;
            sf_footer* newFooter = (sf_footer*)((char*) newHeader +((newHeader->info.block_size <<4))- sizeof(sf_footer));
          //sf_footer* newFooter = (sf_footer*)((char*)sf_mem_end() - sizeof(sf_epilogue) - sizeof(sf_footer));
            printf("\n  HIIIIIII newFooter %p \n" , newFooter);
          setBlockInfo(&newFooter->info,0,1,0,newFreeBlockSpace>>4,0);
            break ;
          // 32  |inserting 4106|  4048
    }else if( (newFreeBlockSpace > current_node->size  &&  newFreeBlockSpace <current_node->next->size )
    || (newFreeBlockSpace> current_node->size && current_node->next == &sf_free_list_head) ){
           printf("\n2 \n");

            sf_free_list_node *sentienel  = sf_add_free_list(newFreeBlockSpace,current_node->next);
            sf_header * newHeader = (sf_header*)((char*)needtoBeUpdatedHeader+ (requestedSize) );
            setBlockInfo(&newHeader->info,0,1,0,newFreeBlockSpace>>4,0);
            sf_footer* newFooter = (sf_footer*)((char*) newHeader +((newHeader->info.block_size <<4))- sizeof(sf_footer));
            setBlockInfo(&newFooter->info,0,1,0,newFreeBlockSpace>>4,0);

          //newHeader->payload = (needtoBeUpdatedHeader+allocatedBlockSpace);


          sentienel->head.links.next = newHeader;
          sentienel->head.links.prev = newHeader;
          newHeader->links.next = &sentienel->head;
          newHeader->links.prev = &sentienel->head;

          break;
    }
            //  update payload  and block size
    }

 }
void addToFree_ListNode(int size , sf_header *header){
    sf_free_list_node * current_node;
// new header of 32
    for(current_node = sf_free_list_head.next; current_node != &sf_free_list_head; current_node = current_node->next){
        if(current_node->size == size){
            printf("\n BROOOOO\n");
            current_node->head.links.next->links.prev = header;
            header->links.next = current_node->head.links.next;
            current_node->head.links.next = header;
            header->links.prev = &current_node->head;
            break;
        }else if(size < current_node->size  &&  current_node->prev == &sf_free_list_head){
                        printf("\n BROOOOO 3333\n");

            sf_free_list_node *sentienel  = sf_add_free_list(size,current_node);

            sentienel->head.links.prev = header;
            sentienel->head.links.next = header;
            header->links.next = &sentienel->head;
            header->links.prev = &sentienel->head;
            break;

        }else if ((size > current_node->size  &&  size <current_node->next->size ) ||
         (size> current_node->size && current_node->next == &sf_free_list_head)){
                        printf("\n BROOOOO 33\n");

            sf_free_list_node * addedNode = sf_add_free_list(size, current_node->next);
            addedNode->head.links.next = header;
            addedNode->head.links.prev = header;
            header->links.next = &addedNode->head;
            header->links.prev = &addedNode->head;
            break;
        }
    }
 }
void removeFree_ListNode( sf_header *header){
    sf_free_list_node * current_node;
// new header of 32
    int size = header->info.block_size<<4;
    for(current_node = sf_free_list_head.next; current_node != &sf_free_list_head; current_node = current_node->next){
        if(current_node->size == size){
            current_node->head.links.next = current_node->head.links.next->links.next;
            current_node->head.links.next->links.prev = &current_node->head;
        }
    }
 }

void * callMemGrow(int size){

    sf_footer* lastFooter = (sf_footer*)((char*) sf_mem_end()- sizeof(sf_footer) -sizeof(sf_epilogue));
    sf_header * lastHeader = (sf_header*)((char*) lastFooter- (lastFooter->info.block_size<<4)+sizeof(sf_block_info)) ;
   // int fullMemGrow= 0;
int bytesadded;
    if(lastHeader->info.allocated==1){
   // fullMemGrow =1 ;
    sf_epilogue * lastepilogue = (sf_epilogue*)((char*) lastFooter + sizeof(sf_footer) );
    sf_header * newHeader = (sf_header*)((char*)lastepilogue);

    //printf("\n lastepilogue%p\n",lastepilogue );

    int newNumOfBytes =  0;
    while(newNumOfBytes< size){
        if(sf_mem_grow()== NULL){

            sf_errno = ENOMEM;
            return NULL;
        }else
        newNumOfBytes += PAGE_SZ;
    }
    sf_footer* newFooter = (sf_footer*)((char*) sf_mem_end()-sizeof(sf_epilogue)- sizeof(sf_footer));
    setBlockInfo(&newFooter->info, 0,0,0,newNumOfBytes>>4,0);

    setBlockInfo(&newFooter->info, 0,0,0,newNumOfBytes>>4,0);
    setBlockInfo(&newHeader->info, 0,0,0,newNumOfBytes>>4,0);
    sf_epilogue* newEpilogue = (sf_epilogue*)((char*) sf_mem_end()-sizeof(sf_epilogue));
    setBlockInfo(&newEpilogue->footer.info, 1,0,0,0,0);
    addToFree_ListNode(newNumOfBytes, newHeader);
    }else{
        sf_header * lastHeader = (sf_header*)((char*) lastFooter- (lastFooter->info.block_size<<4)+sizeof(sf_block_info)) ;
        sf_epilogue* oldEpi = (sf_epilogue*)((char*) sf_mem_end()-sizeof(sf_epilogue));
        setBlockInfo(&oldEpi->footer.info, 0,0,0,0,0);

     bytesadded =  lastFooter->info.block_size<<4;
    while(bytesadded< size){
        if(sf_mem_grow()== NULL){
            sf_errno = ENOMEM;
            return NULL;
        }else
        bytesadded += PAGE_SZ;
    }
       // printf("\n  size %d lastHeader %p %p \n",bytesadded, lastHeader ,oldEpi);
        removeFree_ListNode(lastHeader);
        setBlockInfo(&lastHeader->info, 0,0,0,(bytesadded>>4),0);
        sf_epilogue* newEpilogue = (sf_epilogue*)((char*) sf_mem_end()-sizeof(sf_epilogue));
        sf_footer * newFooter = (sf_footer*)((char*) newEpilogue- sizeof(sf_footer)) ;
        setBlockInfo(&newEpilogue->footer.info, 1,0,0,0,0);
        setBlockInfo(&newFooter->info, 0,0,0,bytesadded>>4,0);

      //  printf("\n  size %d lastHeader%p new footer %p new epi %p \n",bytesadded, lastHeader , newFooter ,newEpilogue);
        addToFree_ListNode((bytesadded), lastHeader);

    }
    sf_free_list_node *current_node;
    for(current_node = sf_free_list_head.next; current_node != &sf_free_list_head; current_node = current_node->next){
        if(current_node->size == size&& current_node->head.links.next != &current_node->head){
            sf_header * returnHeader = current_node->head.links.next;
       //     printf("\n returnHeader %p \n", returnHeader );
            setBlockInfo(&returnHeader->info, 1,1,0,size>>4,size);
            sf_header * nextHeader = current_node->head.links.next; // 1 free blocks next
            current_node->head.links.next = nextHeader->links.next;
            nextHeader->links.prev = current_node->head.links.next->links.prev;;
             // update header to allocated
            return &(returnHeader->payload);
        }else if(current_node->size > size && current_node->head.links.next != &current_node->head){
//printf("\nINSIDE2  %lu %d\n", current_node->size ,size);
            sf_header * returnHeader = current_node->head.links.next;
        //   printf("\n Return HEader %p\n", returnHeader);
        //   printf("\n Block size %d\n", size>>4);
            if(current_node->size- size >=32 ){
                setBlockInfo(&returnHeader->info, 1,1,0,size>>4,size);
            }else{
                setBlockInfo(&returnHeader->info, 1,1,0,(current_node->size>>4),size);
            }

            sf_header * nextHeader = current_node->head.links.next; // 1 free blocks next

            current_node->head.links.next = nextHeader->links.next;
            nextHeader->links.prev = current_node->head.links.next->links.prev;
            if(current_node->size- size >=32 ){
            splitBlock(size, current_node->size , returnHeader);
            }
            return &(returnHeader->payload);
        }
    }

   // int oldBlockFreeBytes = (lastFooter->info.block_size);
   // sf_header lastHeader = lastFooter-
    return NULL;

}
void coalesceBlock(int ifMemGrowCalled, sf_header *blockTocheck){
// 4 cases both prev alloc =0 , both prev alloc = 1 , either or prev alloc is 1

}

void *sf_malloc(size_t size) {
    int actualRequestedSize = size;
    if(size <=0){
        return NULL;
    }
   if(sf_mem_start() ==sf_mem_end()){
       if(!intializeHeap()){
            return NULL;
    }
    }
    size = size+ sizeof(sf_block_info);
    if(size<32){
        size = 32;
    }
    int extraBytes  = size % 16 ;
    if(extraBytes!= 0){
        extraBytes = 16- extraBytes;
        size = size + extraBytes;
    }
    sf_free_list_node * current_node;

    for(current_node = sf_free_list_head.next; current_node != &sf_free_list_head; current_node = current_node->next){
        if(current_node->size == size&& current_node->head.links.next != &current_node->head){

            sf_header * returnHeader = current_node->head.links.next;
            setBlockInfo(&returnHeader->info, 1,1,0,size>>4,actualRequestedSize);

            sf_header * nextHeader = current_node->head.links.next; // 1 free blocks next
            current_node->head.links.next = nextHeader->links.next;
            nextHeader->links.prev = current_node->head.links.next->links.prev;;
             // update header to allocated
            return &(returnHeader->payload);
        }else if(current_node->size > size && current_node->head.links.next != &current_node->head){
            sf_header * returnHeader = current_node->head.links.next;
            if(current_node->size- size >=32 ){
            setBlockInfo(&returnHeader->info, 1,1,0,size>>4,actualRequestedSize);
            }else{
                setBlockInfo(&returnHeader->info, 1,1,0,(current_node->size>>4),actualRequestedSize);
            }
            sf_header * nextHeader = current_node->head.links.next; // 1 free blocks next
            current_node->head.links.next = nextHeader->links.next;
            nextHeader->links.prev = current_node->head.links.next->links.prev;
            if(current_node->size- size >=32 ){
                splitBlock(size, current_node->size , returnHeader);
            }
            return &(returnHeader->payload);
        }
    }

    // means there was no size that could be allocated must grow heap



    return callMemGrow(size);

// coalesce with previous do not check with foward

    return NULL;

}
void * removeFree_ExactListBlock(sf_header *header){
    sf_free_list_node * current_node;
    sf_header * currentHeader;
// new header of 32
    int size = header->info.block_size<<4;
    for(current_node = sf_free_list_head.next; current_node != &sf_free_list_head; current_node = current_node->next){
        if(current_node->size == size && current_node->head.links.next != &current_node->head){
            for(currentHeader = current_node->head.links.next; currentHeader != &current_node->head; currentHeader = currentHeader->links.next){
                if(currentHeader== header){
                    currentHeader->links.prev->links.next =  currentHeader->links.next;
                    currentHeader->links.next->links.prev = currentHeader->links.prev;
                    return  currentHeader;
                }
            }
        }
    }
    return NULL;
 }
 void * coalesceFreeBlocks(sf_header *middleBlock){
// 4 cases
    //1 case both alloc
    int blockSize = (middleBlock->info.block_size)<<4;
   // printf("\n blockSize %d \n", blockSize);
    sf_header * newHeader = middleBlock;
    sf_footer * middleBlocksFooter = (sf_footer *)((char*)middleBlock+ blockSize-sizeof(sf_footer));
    //sf_footer * newFooterMWHAHAHHAHA = middleBlocksFooter;
    // sf_epilogue * addressOfEpi = (sf_epilogue*)((char*)sf_mem_end() - sizeof(sf_epilogue));
    // sf_prologue * EndOfPro =     (sf_prologue*) ((char*) sf_mem_start()+sizeof(sf_prologue));
    sf_header * nextBlockHeader = (sf_header*)((char*)middleBlocksFooter+sizeof(sf_footer)) ;

   // printf("\n nextBlockHead %p  %d\n",nextBlockHeader , nextBlockHeader->info.allocated);
    int nextAllocBit = nextBlockHeader->info.allocated;
    int prevallocBit = middleBlock->info.prev_allocated;
//    printf("\n prev bit %d next alloc bit %d\n", nextAllocBit, prevallocBit);
    int newBytes = blockSize;
 //   printf("\n newBytes %d \n", newBytes);

    sf_footer * previousFooter;
    sf_header * previousHeader;
    if(prevallocBit == 1 && nextAllocBit ==1){
      printf("\n HIII\n");
       // printf("\n nextBlockHeader %p\n", nextBlockHeader);
        addToFree_ListNode((middleBlock->info.block_size<<4), middleBlock);
        setBlockInfo(&middleBlock->info,0,1,0,(middleBlock->info.block_size),0);
        setBlockInfo(&middleBlocksFooter->info,0,1,0,(middleBlock->info.block_size),0);
        setBlockInfo(&nextBlockHeader->info,1,0,0,(nextBlockHeader->info.block_size),0);
       return middleBlock;
    }
    if(prevallocBit == 0 && nextAllocBit ==0){
       printf("\n YOLOOOOOO \n");
        previousFooter = (sf_footer *)((char*)middleBlock - sizeof(sf_footer));
        previousHeader = (sf_header *)((char*)previousFooter -(previousFooter->info.block_size<<4 )+sizeof(sf_block_info));

        newBytes += (previousHeader->info.block_size<<4)+ (nextBlockHeader->info.block_size<<4) ;
        sf_footer *newFooter = (sf_footer*)((char*)previousHeader + newBytes - sizeof(sf_block_info));

        removeFree_ExactListBlock(previousHeader);
        removeFree_ExactListBlock(nextBlockHeader);
       // newHeader = previousHeader;
        setBlockInfo(&previousHeader->info,0,1,0,newBytes>>4,0);
        setBlockInfo(&newFooter->info,0,1,0,newBytes>>4,0);
        addToFree_ListNode((previousHeader->info.block_size<<4), previousHeader);

       return newHeader;
    }
    // previous free
   // printf("\n Header %p  %d\n",middleBlock , middleBlock->info.prev_allocated );
    if(prevallocBit==0 && nextAllocBit ==1){
      printf("\n HIIIIIIII \n");
        // previousFooter = (sf_footer *)((char*)middleBlock - sizeof(sf_footer));
        // previousHeader = (sf_header *)((char*)previousFooter - (previousFooter->info.block_size<<4 )+ sizeof(sf_block_info));
        previousFooter = (sf_footer*)((char*)middleBlock - sizeof(sf_footer));
        previousHeader = (sf_header*)((char*)previousFooter -(previousFooter->info.block_size<<4 ) + sizeof(sf_block_info));
       // printf("\n previous Foorter %p previous Header  %p\n",previousFooter , previousHeader);

        newBytes += (previousHeader->info.block_size<<4);
      //  printf("\n prev header Block size %d\n" ,(previousHeader->info.block_size<<4));
       // printf("\n new bytes %d\n",newBytes);
        newHeader= previousHeader;
        sf_footer *newFooter = (sf_footer*)((char*)newHeader + newBytes - sizeof(sf_block_info));
      //  printf("\n newFooter %p\n", newFooter);
        removeFree_ExactListBlock(previousHeader);
        setBlockInfo(&newHeader->info,0,1,0,newBytes>>4,0);
        setBlockInfo(&newFooter->info,0,1,0,newBytes>>4,0);
        addToFree_ListNode((newHeader->info.block_size<<4), newHeader);
        return newHeader;
    }

    if(prevallocBit==1 && nextAllocBit==0){
        printf("\n NOOOO\n");

        sf_footer *newFooter = (sf_footer *)((char*)nextBlockHeader + (nextBlockHeader->info.block_size<<4)-sizeof(sf_footer));
      //  printf("\n Middle Block Header %p\n", nextBlockHeader);
       // printf("\n nextBlockHeader %p\n", nextBlockHeader);
       // printf("\n Next Footer  %p \n", newFooter);
        newBytes += (nextBlockHeader->info.block_size<<4) ;
       // printf("\n newBytes  %d nextHeaderBytes %d \n", newBytes ,(nextBlockHeader->info.block_size<<4));
        //sf_footer *newFooter = (sf_footer*)((char*)previousHeader + newBytes - sizeof(sf_block_info));

        removeFree_ExactListBlock(middleBlock);
        removeFree_ExactListBlock(nextBlockHeader);
       // newHeader = previousHeader;
        setBlockInfo(&middleBlock->info,0,1,0,newBytes>>4,0);
        setBlockInfo(&newFooter->info,0,1,0,newBytes>>4,0);
        addToFree_ListNode(newBytes, middleBlock);

       return newHeader;
    }
    // now add to free list based
    return NULL;
}
void * verifyPointer(void *pp){
    sf_footer * previousFooter;
    sf_header * previousHeader;
    sf_header *givenHeader ;
    if(pp == NULL) abort();

    givenHeader =  (sf_header*)((char*)pp - sizeof(sf_block_info));
    sf_epilogue * addressOfEpi = (sf_epilogue*)(char*)sf_mem_end() - sizeof(sf_epilogue);
    if( (void*)givenHeader < (sf_mem_start()+sizeof(sf_prologue)) || ((void*)givenHeader) >= (void*)addressOfEpi ) abort();
    //printf("\n Given Header %p %d\n", givenHeader, givenHeader->info.allocated);
    if(givenHeader->info.allocated ==0)abort();
    //printf("\n block Size %d  \n", ((givenHeader->info.block_size<<4) %16));
    if(givenHeader->info.block_size<<4< 32 || ((givenHeader->info.block_size<<4) %16) != 0) abort();

    if((givenHeader->info.requested_size + sizeof(sf_block_info)) > givenHeader->info.block_size<<4) abort();
    if(givenHeader->info.prev_allocated ==0){
        previousFooter = (sf_footer *)((void*)givenHeader -sizeof(sf_footer));
        if(previousFooter->info.allocated ==1) abort();
        previousHeader = (sf_header *)((void*)previousFooter- previousFooter->info.block_size);
        if(previousHeader->info.allocated ==1) abort();
    }
    return givenHeader;
}
void * verifyPointerForRealloc(void *pp){
    sf_footer * previousFooter;
    sf_header * previousHeader;
    sf_header *givenHeader ;
    if(pp == NULL) sf_errno = EINVAL;

    givenHeader =  (sf_header*)((char*)pp - sizeof(sf_block_info));
   // printf("\ngiven Header %p \n", givenHeader);
    sf_epilogue * addressOfEpi = (sf_epilogue*)(char*)sf_mem_end() - sizeof(sf_epilogue);
    if( (void*)givenHeader < (sf_mem_start()+sizeof(sf_prologue)) || ((void*)givenHeader) >= (void*)addressOfEpi ) sf_errno = EINVAL;
    if(givenHeader->info.allocated ==0)sf_errno = EINVAL;
    if(givenHeader->info.block_size<<4 < 32 || ((givenHeader->info.block_size<<4) %16) != 0) sf_errno = EINVAL;
    if((givenHeader->info.requested_size + sizeof(sf_block_info)) > givenHeader->info.block_size<<4) sf_errno = EINVAL;
    if(givenHeader->info.prev_allocated ==0){
        previousFooter = (sf_footer *)((void*)givenHeader -sizeof(sf_footer));
        if(previousFooter->info.allocated ==1) sf_errno = EINVAL;
        previousHeader = (sf_header *)((void*)previousFooter- previousFooter->info.block_size);
        if(previousHeader->info.allocated ==1) sf_errno = EINVAL;
    }
    return givenHeader;
}
void sf_free(void *pp) {
    sf_header *givenHeader = verifyPointer(pp);
    coalesceFreeBlocks(givenHeader);
}
int getCorrectSize(int givenSize){
    int size = givenSize + sizeof(sf_block_info);
    if(size<32){
        size = 32;
    }
    int extraBytes  = size % 16 ;
    if(extraBytes!= 0){
        extraBytes = 16- extraBytes;
        size = size + extraBytes;
    }
    return size;
}
void *sf_realloc(void *pp, size_t rsize) {
     sf_header *givenHeader = verifyPointerForRealloc(pp);
    if(sf_errno ==EINVAL){
        return NULL;
     }
    if(rsize == 0){
        sf_free(givenHeader);
        return NULL;
     }


    size_t requestedSize = rsize;
    printf("\n before rsize %lu \n", rsize);

    rsize = getCorrectSize(rsize);
    printf("\n after rsize %lu \n", rsize);

    int oldSize = (givenHeader->info.block_size<<4);
    //printf("\n blockSize %d \n",oldSize );
    if(rsize == oldSize){
        return pp;
    }
    if(rsize>oldSize){
        void* ptr = sf_malloc(requestedSize);
        if(ptr==NULL){
            return NULL;
        }
        memcpy(ptr,pp,oldSize);
        sf_free(pp);
        return ptr;
    }
    if((oldSize-rsize ) < 32 ){

            givenHeader->info.requested_size = requestedSize;
            return pp;
    }
    if(rsize<oldSize){

        int mod2 = (oldSize- rsize) %16;
            if(mod2==0){
            // realloc to non Splinter

            givenHeader->info.requested_size = requestedSize;

           // removeFree_ExactListBlock(givenHeader);
            sf_show_heap();
            setBlockInfo(&givenHeader->info,1,givenHeader->info.prev_allocated,0,rsize>>4,requestedSize );
            sf_header *restOfBlock = (sf_header*)(char*) (givenHeader+rsize);

            addToFree_ListNode((oldSize-rsize), restOfBlock);
           coalesceFreeBlocks(restOfBlock);
           sf_show_heap();
            printf("\n block size: %d\n",givenHeader->info.block_size<<4 );
            return pp;
        }

    }
    return NULL;
}
