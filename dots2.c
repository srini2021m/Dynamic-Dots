#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "remote_gfx.h"


// A structure that represents a dot.  Don't change this!
typedef struct
{
  uint32_t color;
  int x, y;
  int radius;
} Dot;

// Our dots
Dot ** dots = NULL;
int num_dots = 0;

// Using find_dot_at(), we keep track of which dot the mouse is currently
// hovering over here.  It's NULL if the mouse isn't currently over a dot.
Dot * hovered_dot = NULL;

// If we're moving a dot, this is the one we're moving!
Dot * move_dot = NULL;

// Find the topmost dot underneath (x,y)
Dot * find_dot_at (int x, int y)
{
  for (int i = num_dots - 1; i >= 0; --i)
  {
    Dot * b = dots[i];
    // First do fast checks to see if the mouse is outside the rectangle that
    // encloses the dot.
    if (x < b->x - b->radius) continue;
    if (x > b->x + b->radius) continue;
    if (y < b->y - b->radius) continue;
    if (y > b->y + b->radius) continue;

    // Now check for real.
    if (sqrt(pow(x - b->x, 2) + pow(y - b->y, 2)) <= b->radius) return b;
  }

  return NULL; // No dot is at (x,y).
}


// Draws a dot
void draw_dot (Dot * dot)
{
  // Outer edge is white if it's the current dot, otherwise it's a tweaked
  // version of the normal color.
  int ring_color = (dot == hovered_dot) ? 0xffFFff : dot->color | 0x3f3f3f;

  int r = dot->radius;

  // Draw the ellipse full size for the ring around the edge
  gfx_draw_ellipse(dot->x - r, dot->y - r, 2*r, 2*r, ring_color);

  // Now draw it smaller on top
  gfx_draw_ellipse(dot->x-r+3, dot->y-r+3, 2*r-6, 2*r-6, dot->color);
}


// Draw all dots
void draw_dots ()
{
  // Clear to deep purple
  gfx_clear(0x300a1a);

  gfx_draw_text(11,11, 0x000000, "Welcome to the world of dots!");
  gfx_draw_text(10,10, 0xffFFff, "Welcome to the world of dots!");

  // Draw each dot...
  for (int i = 0; i < num_dots; ++i)
  {
    draw_dot(dots[i]);

    // Print the number in the middle (draw twice at slightly different
    // positions, first with black for a shadow, then with white on top).
    char buf[32];
    sprintf(buf, "Dot %i", i);
    gfx_draw_text_centered(dots[i]->x + 1, dots[i]->y + 1, 0x000000, buf);
    gfx_draw_text_centered(dots[i]->x - 0, dots[i]->y - 0, 0xffffff, buf);
  }
}


// Create a Dot centered on the mouse position.
Dot * new_dot ()
{
  Dot * dot = malloc(sizeof(*dot));
  if (!dot) exit(1); // Out of memory?
  dot->color = gfx_get_random_color();
  dot->radius = 30 + rand() % 70;
  dot->x = gfx_get_mouse_x();
  dot->y = gfx_get_mouse_y();
  return dot;
}


// Delete a specific dot
void delete_dot(Dot* dot) {
  if (!dot) return; // Special case -- do nothing!

  // Find the index of the dot to be deleted (hovered dot) in the array
  int dot_index = -1;
  for (int i = 0; i < num_dots; ++i) {
    if (dots[i] == dot) {
      dot_index = i;
      break;
    }
  }

  // Free memory for the dot to be deleted
  free(dot);

  // renumber the dots ie. shift the dots above the deleted dot down in the array
  for (int i = dot_index; i < num_dots - 1; ++i) {
    dots[i] = dots[i + 1];
  }

  // Decrease the number of dots and resize dots array
  --num_dots;
  dots = realloc(dots, num_dots * sizeof(Dot*));

  draw_dots();
}


// This should delete the highest-numbered dot
void delete_last_dot ()
{
  if (num_dots == 0) return; // Nothing to delete!

  // Just change the number of dots so that we ignore the last one.
  -- num_dots;

  // free() memory for the last dot aka the dot to be deleted
  free(dots[num_dots]);

  //reallocate memory for reduced number of dots
  dots = realloc(dots, (num_dots)*sizeof(Dot*));

  // If we've removed the last dot, quit.
  if (num_dots <= 0)
  {
    printf("No more dots!\n");
    gfx_quit();
  }
}


// Create a new dot just above the given dot
void add_dot_on_top_of(Dot *dot) {

  //mouse is clicked on background
  if (!dot) {
    dots = realloc(dots, (num_dots + 1) * sizeof(Dot*)); // rellocate memory for the new dot

    // Shift dots down in the array
    for (int i = num_dots; i > 0; --i) {
      dots[i] = dots[i - 1];
    }

    dots[0] = new_dot(); // add new dot at beginning
    ++num_dots;
    draw_dots();

  } else {
    int dot_index = -1;
    for (int i = 0; i < num_dots; ++i){
      if (dots[i] == dot){
        dot_index = i;
        break;
      }
    }

    dots = realloc(dots, (num_dots + 1) * sizeof(Dot*)); // reallocate memory for the new dot
  
    // Shift dots above the hovered dot up in the array
    for (int i = num_dots; i > dot_index; --i) {
      dots[i] = dots[i - 1];
    }
    
    dots[dot_index + 1] = new_dot(); // Create a new dot and insert it above the hovered dot
    ++num_dots;
    draw_dots();
  }
  
}

//function that free(s) memory allocated for dots by malloc() / realloc()
void free_dots(){  
  for (int i = 0; i < num_dots; ++i) {
        free(dots[i]);
    }
  free(dots);
}

// Add a new dot on top of all the other dots
void add_dot_at_end ()
{
  //reallocate memory for only the number of dots needed
  dots = realloc(dots, (num_dots+1)*sizeof(Dot*));
  dots[num_dots] = new_dot();
  ++ num_dots;
}


// Called when a key is pressed; we do various things
void on_keypress (int key, int mod)
{
  move_dot = NULL; // Make sure we're not in move mode

  if      (key == 'q') gfx_quit();
  else if (key == '\x1b') gfx_quit(); // (That's ESC)
  else if (key == 'x') delete_dot(hovered_dot);
  else if (key == 'd') delete_last_dot();
  else if (key == ' ') add_dot_at_end();
  else if (key == 'a') add_dot_on_top_of(hovered_dot);
}

// Called when a mouse button is pressed
void on_mouse_down (int x, int y, int buttons)
{
  if (move_dot)
    move_dot = NULL; // Exit move mode
  else
    move_dot = hovered_dot; // Enter move mode (maybe)
}

// Called when a mouse button is released
void on_mouse_up (int x, int y, int buttons)
{
  move_dot = NULL; // Exit move mode
}

// Called when the mouse moves
void on_mouse_move (int x, int y, int buttons)
{
  if (move_dot) // If we're moving a dot, then move it!
  {
    move_dot->x = x;
    move_dot->y = y;

    draw_dots();
  }

  hovered_dot = find_dot_at(x, y);
}


int main (int argc, char * argv[])
{
  // Initialize dot drawing library
  gfx_init("Dot World", 1);

  // This really starts the program.  The dot library will call the
  // on_keypress() and on_mouse_xxxx() functions above when keys are pressed
  // or the mouse does something.  It will eventually return if gfx_quit()
  // is called or if the viewer disconnects.
  gfx_run();
  free_dots();
  printf("Bye!\n");

  return 0;
}