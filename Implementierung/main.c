#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "svg.c"

// Funktion Deklarationen
void print_help();

int mooreA(int n);

int mooreC(int n);

int main(int argc, char **argv){

    FILE *out_file = fopen("test.svg", "w");
    if (out_file == NULL)
    {
        printf("failed to create file");
    }

    int nr_coords = 4;
    uint64_t x[] = {0, 0, 1, 1};
    uint64_t y[] = {0, 1, 1, 0};

    save_as_svg(x, y, nr_coords, 10, 10, out_file);

    fclose(out_file);

    // schaut ob -help aufgerufen wurde
    if (strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-h") == 0){
      print_help();
      return 0;
    }
    // schaut dass die richtige Anzahl an Argumenten uebergeben wurde
    if(argc < 3){
      printf("Ungenügend Argumente\n");
      print_help();
      return 1;
    } else if (argc > 3){
      printf("Zu viele Argumente\n");
      print_help();
      return 1;
    }
    int grad = atoi(argv[2]); // Konversion des ubergebenen Arguments in ein int
    int option;
    int impflags = 0;
    // checkt properties des grades
    if(grad < 0){
      printf("Grad kann nicht negativ sein\n");
    }
    // check die Flags die uebergeben wurden
    while((option = getopt(argc, argv, "ca")) != -1){
      switch (option){
        case 'c':
          if (impflags > 0) {
            print_help();
          }else{
            impflags++;
            printf("Sie haben die C-Implementation gewaehlt, mit Grad %d\n", grad);
            mooreA(grad);
          }
          break;
        case 'a':
          if(impflags > 0){
            print_help();
          }else{
            impflags++;
            printf("Sie haben die Assembler-Implementation gewaehlt, mit Grad %d\n", grad);
            mooreC(grad);
          }
          break;
        default:
          print_help();
          break;
        }
      }
    return 0;
}

void print_help(){
  printf("================Help================\n");
  printf("-help oder -h : Printed diese Nachricht.\n");
  printf("Benutzung : moore -c <Implementationsart> -n <grad> | moore -a <Implementationsart> -n <grad>\n");
  printf("====================================\n");
  exit(2);
}

// TODO: Assembler anbindung/Implementation
int mooreA(int n){

  return 0;
}

//TODO: C Implementation
int mooreC(int n){

  return 0;
}