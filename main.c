/***************************************
  $Header$

  Top level file.
  ***************************************/

/* COPYRIGHT */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "nodes.h"
#include "functions.h"
#include "version.h"

/* The node into which the parser inserts the top node of the parse
   tree */
TreeNode *top;

extern int yydebug;

extern int yyparse(void);
extern int full_yyparse(void);

extern DriverVector latex_driver, latex_block_driver;
extern DriverVector textout_driver, text_block_driver;
extern DriverVector html_driver;

/* Flag set by yyerror - i.e. there were syntax errors that were cleaned up so
   didn't get reported by yyparse */
static int had_syntax_error;

/* Flag set in lexical analyser + morphology block, if there are invalid words
   in the input stream.  This causes the run to be aborted before the parse
   phase. */
int had_bad_tokens;

/* Optional parameters to be read by backends */
int opt_output_width;
int insert_elidables;

/* Flag to allow cultural rafsi within lujvo (and as part of slinku'i test) */
int allow_cultural_rafsi;

/* Flag to use parser that requires all elidable separators and terminators to
   be present */
int require_elidables;

/* Flag to show which elidables could safely have been elided but weren't,
   and whether a verbose report is required */
int show_elisions;
int show_elisions_verbose;

/* Flag to display backtrace when a syntax error arises */
int show_backtrace;

/* Flag to show dictionary defects */
int show_dictionary_defects;

/* Error routine invoked from the parser */

void yyerror(char *s) {
  /* Printing out the token dump is now done in tracebk.c */
  had_syntax_error = 1;
}

/* Handle signals */

void
handle_signal(int x)
{
  /* Also providing a mail address to report bugs would be great,
     because GitHub requires registration to fill in issues. */
  fprintf(stderr,"\n"
                 "------------------------------------------------------------------\n"
                 "An unexpected error has occurred whilst running jbofihe.\n\n"
                 "Please submit a report on https://github.com/lojban/jbofihe/issues\n"
                 "with at least these pieces of information:\n"
                 "  Version of jbofihe (jbofihe -v)\n"
                 "  Command line options\n"
                 "  The input text\n"
                 "------------------------------------------------------------------\n");
  exit(1);
}

/* Print out usage information */

static void
show_usage(void)
{
  fprintf(stderr, "jbofihe version %s\n", version_string);
  fprintf(stderr, "Copyright 1998-2001 Richard P. Curnow <rc@rc0.org.uk>\n\n"
                  "-k           Show token lists before and after preprocessing\n"
                  "-t           Show edited parse tree\n"
                  "-tf          Show full parse tree\n"
                  "\n"
                  "These options produce formatted Lojban + partial English translation\n"
                  "-l           Produce LaTeX output\n"
                  "-x           Produce plain text output\n"
                  "-H           Produce HTML output\n"
                  "-b           Produce 'blocked' output (LaTeX & plain text only)\n"
                  "-w <width>   Set output width (plain text)\n"
                  "\n"
                  "These options relate to handling elidable separators/terminators\n"
                  "-ie          Display elided separators and terminators\n"
                  "-re          Require elidable separators and terminators to be present\n"
                  "-se          Show elidable separators/terminators that could be omitted\n"
                  "-sev         Ditto, plus verbose detail\n"
                  "\n"
                  "These options relate to grammar options\n"
                  "-cr          Allow cultural rafsi in lujvo (Reference Grammar section 4.16)\n"
                  "\n"
                  "These options relate to help & debug\n"
                  "-v           Show version number and exit\n"
                  "-h           Show this help information\n"
                  "-?           Show this help information\n"
                  "-m           Show memory usage statistics\n"
                  "-d           Show debug information\n"
                  "-dd          Report missing dictionary entries to stderr\n"
                  "-bt          Show parser state backtrace when a syntax error occurs\n"
                  );
}

/* Main routine */

int
main (int argc, char **argv)
{
  char *filename = NULL;
  FILE *in = NULL;
  
  int result;
  int debug;
  int token_lists;
  int show_tree;
  int full_tree;
  int gloss;
  int block;
  int latex;
  int textout;
  int htmlout;
  int show_memory;

  debug = 0;
  token_lists = 0;
  show_tree = 0;
  full_tree = 0;
  gloss = 0;
  latex = 0;
  textout = 0;
  htmlout = 0;
  block = 0;
  show_memory = 0;
  opt_output_width = 79;
  insert_elidables = 0;
  require_elidables = 0;
  show_elisions = 0;
  show_elisions_verbose = 0;
  allow_cultural_rafsi = 0;
  show_backtrace = 0;

  show_dictionary_defects = 0;

  had_bad_tokens = 0;

  while (++argv, --argc) {
    if (!strcmp(*argv, "-d")) {
      debug = 1;
    } else if (!strcmp(*argv, "-v")) {
      fprintf(stderr, "jbofihe version %s\n", version_string);
      exit(0);
    } else if (!strcmp(*argv, "-k")) {
      token_lists = 1;
    } else if (!strcmp(*argv, "-g")) {
      gloss = 1;
    } else if (!strcmp(*argv, "-t")) {
      show_tree = 1;
    } else if (!strcmp(*argv, "-tf")) {
      show_tree = 1;
      full_tree = 1;
    } else if (!strcmp(*argv, "-l")) {
      latex = 1;
    } else if (!strcmp(*argv, "-x")) {
      textout = 1;
    } else if (!strcmp(*argv, "-H")) {
      htmlout = 1;
    } else if (!strcmp(*argv, "-b")) {
      block = 1;
    } else if (!strcmp(*argv, "-m")) {
      show_memory = 1;
    } else if (!strcmp(*argv, "-ie")) {
      insert_elidables = 1;
    } else if (!strcmp(*argv, "-re")) {
      require_elidables = 1;
    } else if (!strcmp(*argv, "-se")) {
      show_elisions = 1;
    } else if (!strcmp(*argv, "-sev")) {
      show_elisions = 1;
      show_elisions_verbose = 1;
    } else if (!strcmp(*argv, "-cr")) {
      allow_cultural_rafsi = 1;
    } else if (!strcmp(*argv, "-bt")) {
      show_backtrace = 1;
    } else if (!strcmp(*argv, "-dd")) {
      show_dictionary_defects = 1;
    } else if (!strncmp(*argv, "-w", 2)) {
      if (strlen(*argv) > 2) {
        opt_output_width = atoi(*argv + 2);
      } else {
        /* Get next argument */
        --argc, ++argv;
        opt_output_width = atoi(*argv);
      }
    } else if (!strcmp(*argv, "--help")) {
      show_usage();
      exit(0);
    } else if (!strcmp(*argv, "-h")) {
      show_usage();
      exit(0);
    } else if (!strcmp(*argv, "-?")) {
      show_usage();
      exit(0);
    } else if (!strncmp(*argv, "-", 1)) {
      fprintf(stderr, "Unrecognized command line option %s\n", *argv);
      exit(1);
    } else {
      filename = *argv;
    }
  }

  /* Check flag compatibility */
  if (require_elidables && show_elisions) {
    fprintf(stderr, "-re and -se switches are not compatible, -se ignored\n");
    show_elisions = 0;
  }

#if !defined(EXPOSE_SIGNALS)
  signal(SIGABRT, handle_signal);
#if !defined(DJGPP)
  /* This signal doesn't seem to be defined on DJGPP */
  signal(SIGBUS, handle_signal);
#endif
  signal(SIGSEGV, handle_signal);
#endif

  lex2_initialise();

  if (filename) {
    in = fopen(filename, "r");
    if (!in) {
      fprintf(stderr, "Could not open %s for input\n", filename);
      exit(1);
    }
  }

  parse_file(in ? in : stdin);

  if (in) fclose(in);

  if (had_bad_tokens) {
    return 3;
  }

  if (token_lists) {
    printf("\nToken list before preprocessing\n\n");
    show_tokens();
  }

  preprocess_tokens();

  /* Check again, lex2 can detect bad constructs as well. */
  if (had_bad_tokens) {
    return 3;
  }

  if (token_lists) {
    printf("------------------------------\n"
           "Token list after preprocessing\n\n");
    show_tokens();
    printf("------------------------------\n\n");
  }

#if 1
  yydebug = debug;
  had_syntax_error = 0;

  if (require_elidables) {
    result = full_yyparse();
  } else {
    result = yyparse();
  }

  if (result == 0 && !had_syntax_error) {
    expand_bahe_ui(top);

    error_scan(top);

    if (show_tree) {
      if (!full_tree) {
        compress_singletons(top);
      }

      print_parse_tree(top);

    } else if (latex || textout || htmlout) { 
      
      terms_processing(top);
      do_conversions(top);
      tense_processing(top);
      connectives_processing(top);
      relative_clause_processing(top);

      add_bracketing_tags(top);

      if (latex) {
        do_output(top, block ? &latex_block_driver : &latex_driver);
      } else if (textout) {
        do_output(top, block ? &text_block_driver: &textout_driver);
      } else if (htmlout) {
        do_output(top, &html_driver);
      }

    } else {

      compress_singletons(top);
      print_bracketed_text(top, gloss);

    }

  }
#endif

  if (show_memory) {
    print_memory_statistics();
  }
  
  return result ? 1 : had_syntax_error ? 2 : 0;
}
