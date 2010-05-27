#include <string.h>
#include "owl.h"

#define BOTTOM_OFFSET 1

/* initialize the viewwin e.  'win' is an already initialzed curses
 * window that will be used by viewwin
 */
void owl_viewwin_init_text(owl_viewwin *v, owl_window *win, const char *text)
{
  owl_fmtext_init_null(&(v->fmtext));
  if (text) {
    owl_fmtext_append_normal(&(v->fmtext), text);
    if (text[0] != '\0' && text[strlen(text) - 1] != '\n') {
      owl_fmtext_append_normal(&(v->fmtext), "\n");
    }
    v->textlines=owl_fmtext_num_lines(&(v->fmtext));
  }
  v->topline=0;
  v->rightshift=0;
  v->onclose_hook = NULL;

  owl_viewwin_set_window(v, win);
}

void owl_viewwin_append_text(owl_viewwin *v, const char *text) {
    owl_fmtext_append_normal(&(v->fmtext), text);
    v->textlines=owl_fmtext_num_lines(&(v->fmtext));  
    owl_window_dirty(v->window);
}

/* initialize the viewwin e.  'win' is an already initialzed curses
 * window that will be used by viewwin
 */
void owl_viewwin_init_fmtext(owl_viewwin *v, owl_window *win, const owl_fmtext *fmtext)
{
  char *text;

  owl_fmtext_copy(&(v->fmtext), fmtext);
  text = owl_fmtext_print_plain(fmtext);
  if (text[0] != '\0' && text[strlen(text) - 1] != '\n') {
      owl_fmtext_append_normal(&(v->fmtext), "\n");
  }
  owl_free(text);
  v->textlines=owl_fmtext_num_lines(&(v->fmtext));
  v->topline=0;
  v->rightshift=0;

  owl_viewwin_set_window(v, win);
}

void owl_viewwin_set_window(owl_viewwin *v, owl_window *w)
{
  if (v->window) {
    owl_window_set_redraw_cb(v->window, 0, 0, 0);
    owl_window_set_resize_cb(v->window, 0, 0, 0);
  }
  v->window = w;
  if (w) {
    owl_window_get_position(w, &v->winlines, &v->wincols, 0, 0);
    owl_window_set_redraw_cb(w, owl_viewwin_redisplay, v, 0);
    owl_window_set_resize_cb(w, owl_viewwin_resize_hook, v, 0);
  }
}

void owl_viewwin_set_onclose_hook(owl_viewwin *v, void (*onclose_hook) (owl_viewwin *vwin, void *data), void *onclose_hook_data) {
  v->onclose_hook = onclose_hook;
  v->onclose_hook_data = onclose_hook_data;
}

void owl_viewwin_resize_hook(owl_window *w, void *user_data)
{
  owl_viewwin *v = user_data;
  owl_window_get_position(w, &v->winlines, &v->wincols, 0, 0);
}

/* regenerate text on the curses window. */
void owl_viewwin_redisplay(owl_window *w, WINDOW *curswin, void *user_data)
{
  owl_fmtext fm1, fm2;
  owl_viewwin *v = user_data;
  
  werase(curswin);
  wmove(curswin, 0, 0);

  owl_fmtext_init_null(&fm1);
  owl_fmtext_init_null(&fm2);
  
  owl_fmtext_truncate_lines(&(v->fmtext), v->topline, v->winlines-BOTTOM_OFFSET, &fm1);
  owl_fmtext_truncate_cols(&fm1, v->rightshift, v->wincols-1+v->rightshift, &fm2);

  owl_fmtext_curs_waddstr_without_search(&fm2, curswin);

  /* print the message at the bottom */
  wmove(curswin, v->winlines-1, 0);
  wattrset(curswin, A_REVERSE);
  if (v->textlines - v->topline > v->winlines-BOTTOM_OFFSET) {
    waddstr(curswin, "--More-- (Space to see more, 'q' to quit)");
  } else {
    waddstr(curswin, "--End-- (Press 'q' to quit)");
  }
  wattroff(curswin, A_REVERSE);

  owl_fmtext_cleanup(&fm1);
  owl_fmtext_cleanup(&fm2);
}

void owl_viewwin_pagedown(owl_viewwin *v)
{
  v->topline+=v->winlines - BOTTOM_OFFSET;
  if ( (v->topline+v->winlines-BOTTOM_OFFSET) > v->textlines) {
    v->topline = v->textlines - v->winlines + BOTTOM_OFFSET;
  }
  owl_window_dirty(v->window);
}

void owl_viewwin_linedown(owl_viewwin *v)
{
  v->topline++;
  if ( (v->topline+v->winlines-BOTTOM_OFFSET) > v->textlines) {
    v->topline = v->textlines - v->winlines + BOTTOM_OFFSET;
  }
  owl_window_dirty(v->window);
}

void owl_viewwin_pageup(owl_viewwin *v)
{
  v->topline-=v->winlines;
  if (v->topline<0) v->topline=0;
  owl_window_dirty(v->window);
}

void owl_viewwin_lineup(owl_viewwin *v)
{
  v->topline--;
  if (v->topline<0) v->topline=0;
  owl_window_dirty(v->window);
}

void owl_viewwin_right(owl_viewwin *v, int n)
{
  v->rightshift+=n;
  owl_window_dirty(v->window);
}

void owl_viewwin_left(owl_viewwin *v, int n)
{
  v->rightshift-=n;
  if (v->rightshift<0) v->rightshift=0;
  owl_window_dirty(v->window);
}

void owl_viewwin_top(owl_viewwin *v)
{
  v->topline=0;
  v->rightshift=0;
  owl_window_dirty(v->window);
}

void owl_viewwin_bottom(owl_viewwin *v)
{
  v->topline = v->textlines - v->winlines + BOTTOM_OFFSET;
  owl_window_dirty(v->window);
}

void owl_viewwin_cleanup(owl_viewwin *v)
{
  if (v->onclose_hook) {
    v->onclose_hook(v, v->onclose_hook_data);
    v->onclose_hook = NULL;
    v->onclose_hook_data = NULL;
  }
  owl_fmtext_cleanup(&(v->fmtext));
}
