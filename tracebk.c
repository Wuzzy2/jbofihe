/***************************************
  $Header$

  Provide traceback capability when parse errors are encountered.
  ***************************************/

/* COPYRIGHT */

#include <stdio.h>
#include <string.h>
#include "functions.h"
#include "trctabs.c"
#include "trcftabs.c"

typedef struct {
  int state;
  int rule;
} Reduction;

static Reduction reductions[1024];
static int n_reductions=0;

/* command line options in main.c */
extern int show_backtrace;
extern int require_elidables;

static void
display_rule(int ruleno, int indent, int focus)
{
  int i, j;
  char **toknames = require_elidables ? full_toknames : norm_toknames;
  unsigned short *ruleindex = require_elidables ? full_ruleindex : norm_ruleindex;
  unsigned short *rulelhs = require_elidables ? full_rulelhs : norm_rulelhs;
  unsigned short *rulerhs = require_elidables ? full_rulerhs : norm_rulerhs;


  for (i=0; i<indent; i++) fputc(' ', stdout);
  fprintf(stderr, "Rule %d : %s <- ", ruleno, toknames[rulelhs[ruleno]]);
  for (i=0, j=ruleindex[ruleno]; j<ruleindex[ruleno+1]; i++, j++) {
    int rhs = rulerhs[j];
    if (i>0) fprintf(stderr, " ");
    if (strncmp(toknames[rhs], "PRIVATE_", 8)) {
      fprintf(stderr, "%s", toknames[rulerhs[j]]);
    }
    if (i+1 == focus) {
      fprintf(stderr, " .");
    }
  }
  fprintf(stderr, "\n");
}

void
report_trace_reduce(int stateno, int ruleno)
{
  reductions[n_reductions].rule = ruleno;
  reductions[n_reductions].state = stateno;
  n_reductions++;
}

void
report_trace_shift(int yychar, int yychar1)
{
  /* After a successful shift, reduction list clears */
  n_reductions = 0;
}

void
report_trace_error(short *yyss, short *yyssp)
{
  int i;
  short *s;
  unsigned short *stateindex = require_elidables ? full_stateindex : norm_stateindex;
  unsigned short *shiftrule = require_elidables ? full_shiftrule : norm_shiftrule;
  unsigned char *focus = require_elidables ? full_focus : norm_focus;
  unsigned short *shift_in_state = require_elidables ? full_shift_in_state : norm_shift_in_state;
  unsigned short *shift_in_state_index = require_elidables ? full_shift_in_state_index : norm_shift_in_state_index;
  char **toknames = require_elidables ? full_toknames : norm_toknames;

  /* This stuff is done here now instead of in yyerror.  It means the token
     dump comes before the backtrace rather than after it, which makes more
     sense. */
  fprintf(stderr, "--------------------\n");
  fprintf(stderr, "SYNTAX ERROR IN TEXT\n");
  fprintf(stderr, "--------------------\n");
  print_last_toks();
  fprintf(stderr, "--------------------\n");

  /* If not doing backtrace, exit now. */
  if (!show_backtrace) return;

  fprintf(stderr,
         "==============================================\n"
         "Rules reduced since misparsed token was read :\n"
         "==============================================\n");
  if (n_reductions == 0) {
    fprintf(stderr,"  NONE\n");
  } else {
    for (i=0; i<n_reductions; i++) {
      int ruleno = reductions[i].rule;
      int stateno = reductions[i].state;
      int r, r1, r2;
      int is_first = 1;
      display_rule(ruleno, 1, 0);
      r1 = shift_in_state_index[stateno], r2 = shift_in_state_index[stateno+1];
      if (r2 > r1) {
        for (r=r1; r<r2; r++) {
          int tokidx = shift_in_state[r];
          if (strncmp("PRIVATE_", toknames[tokidx], 8)) {
            if (is_first) {
              fprintf(stderr,"   (Next word class could be :");
            }
            fprintf(stderr," %s", toknames[tokidx]);
            is_first = 0;
          }
        }
        if (!is_first) fprintf(stderr,")\n");
      }
    }
  }

  fprintf(stderr,
         "==========================\n"
         "Jammed parser state (%d) :\n"
         "==========================\n", *yyssp);
  {
    int r1, r2;
    int r;
    int is_first;
    r1 = stateindex[*yyssp], r2 = stateindex[*yyssp+1];
    for (r=r1; r<r2; r++) {
      display_rule(shiftrule[r], 1, focus[r]);
    }
  
    /* Print out which tokens would have been possible */
    fprintf(stderr,"\nNext word class could be : ");
    r1 = shift_in_state_index[*yyssp], r2 = shift_in_state_index[*yyssp+1];
    is_first = 1;
    for (r=r1; r<r2; r++) {
      int tokidx = shift_in_state[r];
      if (strncmp("PRIVATE_", toknames[tokidx], 8)) {
        fprintf(stderr,"%s%s", is_first ? "" : " ", toknames[tokidx]);
        is_first = 0;
      }
    }
    fprintf(stderr,"\n\n");
  }

  fprintf(stderr,"=========================================================\n"
         "Pending parser states (innermost first, outermost last) :\n"
         "=========================================================\n");
  for (s=yyssp-1; s>=yyss; s--) {
    int r1, r2;
    int r;
    r1 = stateindex[*s], r2 = stateindex[*s+1];
    fprintf(stderr, "State %d\n", *s);
    for (r=r1; r<r2; r++) {
      display_rule(shiftrule[r], 1, focus[r]);
    }
    fprintf(stderr,"---------------------------\n");
  }
}
