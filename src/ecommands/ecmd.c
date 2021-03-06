/*
 * Copyright (c) 2010, 2011, 2012 Ryan Flannery <ryan.flannery@gmail.com>
 * Copyright (c) 2013 Tiago Cunha <tcunha@gmx.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ecmd.h"

static int
ecmd_parse(const struct ecmd *ecmd, int argc, char ***argv)
{
   /* reset getopt(3) variables */
   optind = 0;
   optreset = 1;

   /* parse command line and if valid, skip parsed arguments */
   if (ecmd->parse != NULL) {
      /* parse error */
      if (ecmd->parse(argc, *argv) == -1)
         return -1;
      if (ecmd->check != NULL && ecmd->check() == -1)
         return -1;
      argc -= optind;
      *argv += optind;
   } else {
      /* no parse function; skip only its name */
      argc--;
      (*argv)++;
   }
 
   /* invalid number of arguments */
   if (argc < ecmd->args_lower)
      return -1;
   if (ecmd->args_upper >= 0 && argc > ecmd->args_upper)
      return -1;

   /* return updated (no name) number of arguments */
   return argc;
}

int
ecmd_exec(const char *ecmd, int argc, char **argv)
{
   /* set of e-commands */
   static const struct ecmd *ecmdtab[] = {
      &ecmd_add,
      &ecmd_addurl,
      &ecmd_check,
      &ecmd_flush,
      &ecmd_help,
      &ecmd_init,
      &ecmd_rmfile,
      &ecmd_tag,
      &ecmd_update,
   };
   static const int ecmdtab_size = sizeof ecmdtab / sizeof ecmdtab[0];
   int              i;

   /* search for e-command (first by name and then by alias) */
   for (i = 0; i < ecmdtab_size; i++) {
      if (strcmp(ecmd, ecmdtab[i]->name) == 0)
         break;
      if (ecmdtab[i]->alias != NULL && strcmp(ecmd, ecmdtab[i]->alias) == 0)
         break;
   }
   /* not found; bail out */
   if (i == ecmdtab_size) {
      warnx("Unknown e-command '%s'.  See 'vitunes -e help' for list.", ecmd);
      return -1;
   }

   /* parse e-command arguments */
   if ((argc = ecmd_parse(ecmdtab[i], argc, &argv)) == -1) {
      fprintf(stderr, "usage: %s -e %s %s\n", progname, ecmdtab[i]->name,
          ecmdtab[i]->usage != NULL ? ecmdtab[i]->usage : "");
      return 1;
   }

   /* finally execute it */
   ecmdtab[i]->exec(argc, argv);
   return 0;
}
