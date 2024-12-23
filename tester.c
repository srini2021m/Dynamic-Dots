#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
//#include "util.h"


static int relative_addrs = 0;
static int verbose = 1;
static void * relative_start;
static char * initial_brk;

static void print (char * s) { fprintf(stderr, "%s", s); }
static void nl () { fprintf(stderr, "\n"); }
static void printhex32 (int x) { fprintf(stderr, "%08x", x); }

char last_text[101];

void gfx_init (char * title, int quit_on_disconnect) {}
void gfx_draw_rect (int x, int y, int w, int h, uint32_t color) {}
void gfx_draw_ellipse (int x, int y, int w, int h, uint32_t color) {}
void gfx_quit () {}
void gfx_draw_text (int x, int y, uint32_t color, const char * str)
{
  strncpy(last_text, str, 100);
  if (verbose) printf("text: %s\n", str);
}
void gfx_draw_text_centered (int x, int y, uint32_t color, const char * str)
{
  strncpy(last_text, str, 100);
  if (verbose) printf("center: %s\n", str);
}
void gfx_clear (uint32_t color) {}

static int color;
uint32_t gfx_get_random_color ()
{
  //++color;
  //return 0xbef000 | (color & 0xff);
  return color++;
}

static int mousex, mousey, mouseb;
int gfx_get_mouse_x () { return mousex; }
int gfx_get_mouse_y () { return mousey; }
int gfx_get_mouse_buttons () { return mouseb; }



static void do_sbrk (char ** args)
{
  sbrk(atoi(args[0]));
}



static void do_rel (char ** args)
{
  // Turns printing of relative addresses on
  // rel <0 for absolute addresses, 1 for relative addresses>
  relative_addrs = atoi(args[0]) != 0;
  if (relative_addrs && relative_start == NULL) relative_start = sbrk(0);
}

static void do_srand (char ** args)
{
  int a = atoi(args[0]);
  if (a == -1) srand(time(NULL));
  else srand(a);
}

static void do_v (char ** args)
{
  // Turns on and off verbose mode
  verbose = atoi(args[0]) != 0;
}


typedef struct
{
  uint32_t color;
  int x, y;
  int radius;
} Dot;
extern void on_keypress (int key, int mod);
extern void on_mouse_move (int x, int y, int buttons);
extern void on_mouse_down (int x, int y, int buttons);
extern void on_mouse_up (int x, int y, int buttons);
extern Dot ** dots;
Dot * dotsarr = (Dot *)&dots;
extern int num_dots;
extern Dot * hovered_dot;
extern Dot * move_dot;
extern Dot * find_dot_at (int x, int y);

static Dot ** dotsptr = (Dot**)&dots;

static Dot ** getdots ()
{
#ifdef DOTS2
  return dots;
#endif
  return dotsptr;
  /*
  int offset = ((char*)&num_dots) - ((char*)(dots));
  if (offset == sizeof(void*)) return dots;
  if (offset == 10*sizeof(void*)) return dotsptr;
  fprintf(stderr, "There's something wrong with your definition of 'dots'.\n");
  exit(1);
  */
}

#define DOTS getdots()

#define MAYBE(var,val) if (val && strlen(val) && atoi(val) != -1) var = atoi(val)

static void do_dotssize (char ** args)
{
#ifdef DOTS2
  fprintf(stderr, "dots size: %li\n", malloc_usable_size(dots));
#else
  fprintf(stderr, "dotssize command is nonsense for first dots program.\n");
#endif
}

static void do_sethover (char ** args)
{
  int w = atoi(args[0]);
  hovered_dot = DOTS[w];
}

static void do_setmove (char ** args)
{
  int w = atoi(args[0]);
  move_dot = DOTS[w];
}

static void do_keyx (char ** args)
{
  int count = atoi(args[0]);
  int mod = atoi(args[2]);
  char * k = args[1];
  if (0 == strcmp(k, "sp") || 0 == strcmp(k, "space")) k = " ";

  for (int i = 0; i < count; ++i)
  {
    on_keypress(k[0], mod);
    hovered_dot = find_dot_at(mousex, mousey);
  }
}

static void do_key (char ** args)
{
  char * na[] = {"1", args[0], "0"};
  do_keyx(na);
}

static void do_mousemove (char ** args)
{
  MAYBE(mousex, args[0]);
  MAYBE(mousey, args[1]);
  MAYBE(mouseb, args[2]);
  on_mouse_move(mousex, mousey, mouseb);
}

static void do_mousedown (char ** args)
{
  MAYBE(mouseb, args[0]);
  on_mouse_down(mousex, mousey, mouseb);
}

static void do_mouseup (char ** args)
{
  MAYBE(mouseb, args[0]);
  on_mouse_down(mousex, mousey, mouseb);
}


static void dumpaddr (void * a)
{
  intptr_t aa = (intptr_t)a;
  if (relative_addrs) aa -= (intptr_t)relative_start;
  printhex32(aa);
}

static void do_showbrk (char ** args)
{
  print("brk: ");
  dumpaddr(sbrk(0));
  nl();
}

static void do_heapused (char ** args)
{
  struct mallinfo2 mi = mallinfo2();
  printf("heap used: %zu\n", mi.uordblks);
}

static void do_heaptrim (char ** args)
{
  malloc_trim(0);
}

static void do_brksize (char ** args)
{
  fprintf(stderr, "brksize: %li\n", ((char*)sbrk(0)) - initial_brk);
}


static void do_mark (char ** args)
{
  print("----\n");
}

static void do_showdots (char ** args)
{
  if (verbose) print("-- dots --\n");
  for (int i = 0; i < num_dots; ++i)
  {
    Dot * d = DOTS[i];
    //fprintf(stderr, "Dot %-3i %-3i %-3i %-3i %06x %zu\n",
    fprintf(stderr, "Dot %-3i %-3i %-3i %-3i %-3i %zu\n",
            i, d->x, d->y, d->radius, d->color, malloc_usable_size(d));
  }
  if (verbose) print("----------\n");
}

static void do_numdots (char ** args)
{
  fprintf(stderr, "num_dots: %i\n", num_dots);
}




#define CMD(name, count)                                                   \
  if (0 == strcmp(#name, argv[i])) {                                       \
    if (remaining < count) {                                               \
      print("Bad number of arguments for '"); print(#name); print("'.\n"); \
      return 1;                                                            \
    }                                                                      \
    do_ ## name(argv + i + 1);                                             \
    i += count;                                                            \
    continue;                                                              \
  }


static int not_main (int argc, char * argv[])
{
  int remaining = argc - 1;
  for (int i = 1; i < argc; ++i, --remaining)
  {
    if (0 == strcmp(argv[i], "--")) continue;
    CMD(showbrk, 0);
    CMD(brksize, 0);
    CMD(sbrk, 1);
    CMD(heapused, 0);
    CMD(heaptrim, 0);
    CMD(rel, 1);
    CMD(showdots, 0);
    CMD(numdots, 0);
    CMD(sethover, 1);
    CMD(setmove, 1);
    CMD(dotssize, 0);
    CMD(mark, 0);
    CMD(key, 1);
    CMD(srand, 1);
    CMD(keyx, 3);
    CMD(mousemove, 3);
    CMD(mousedown, 1);
    CMD(mouseup, 1);
    CMD(v, 1);
    print("Command not found: ");print(argv[i]);nl();
    exit(1);
  }

  return 0;
}

static char ** margv;
static int margc;
static int mret = -1;
void gfx_run ()
{
  srand(1);
  initial_brk = sbrk(0);
  mret = not_main(margc, margv);
}

extern int __real_main (int argc, char * argv[]);

int __wrap_main (int argc, char * argv[])
{
  margc = argc;
  margv = argv;
  char arg[20] = "./dots";
  char * nargv[] = {arg, NULL};

  if (argv[0][strlen(argv[0])-1] == '2') strcat(arg, "2");
  int r = __real_main(1, nargv);
  //printf("Testing complete.\n");
  if (r) return r;
  return mret;
}
