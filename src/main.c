#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

      // void *x = sf_malloc(sizeof(double) * 8);
      // void *y = sf_malloc( sizeof(int));
      // void *a = sf_malloc(y, sizeof(int) * 8);
      // void *b = sf_malloc( a, sizeof(char));


  void *x = sf_malloc(sizeof(int) * 8);
        sf_show_heap();

  void *y = sf_realloc(x, sizeof(char));

    //  sf_free(a);
      printf("\n   %p %p\n",x ,y);

      sf_mem_fini();

    return EXIT_SUCCESS;

}
