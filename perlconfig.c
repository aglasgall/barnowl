/* Copyright (c) 2002,2003,2004,2009 James M. Kretchmar
 *
 * This file is part of Owl.
 *
 * Owl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Owl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Owl.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ---------------------------------------------------------------
 * 
 * As of Owl version 2.1.12 there are patches contributed by
 * developers of the branched BarnOwl project, Copyright (c)
 * 2006-2009 The BarnOwl Developers. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "owl.h"
#include <perl.h>
#include "XSUB.h"

static const char fileIdent[] = "$Id$";

extern char *owl_perlwrap_codebuff;

extern XS(boot_owl);

static void owl_perl_xs_init(pTHX)
{
  char *file = __FILE__;
  dXSUB_SYS;
  {
    newXS("owl::bootstrap", boot_owl, file);
  }
}

SV *owl_perlconfig_message2hashref(owl_message *m)  /*noproto*/
{
  HV *h;
  SV *hr;
  char *ptr, *blessas;
  int i, j;

  if (!m) return &PL_sv_undef;
  h = newHV();

#define MSG2H(h,field) hv_store(h, #field, strlen(#field), \
			      newSVpv(owl_message_get_##field(m),0), 0)

  if (owl_message_is_type_zephyr(m)
      && owl_message_is_direction_in(m)) {
    /* Handle zephyr-specific fields... */
    AV *av_zfields;

    av_zfields = newAV();
    j=owl_zephyr_get_num_fields(owl_message_get_notice(m));
    for (i=0; i<j; i++) {
      ptr=owl_zephyr_get_field(owl_message_get_notice(m), i+1);
      av_push(av_zfields, newSVpvn(ptr, strlen(ptr)));
      owl_free(ptr);
    }
    hv_store(h, "fields", strlen("fields"), newRV_noinc((SV*)av_zfields), 0);

    hv_store(h, "auth", strlen("auth"), 
	     newSVpv(owl_zephyr_get_authstr(owl_message_get_notice(m)),0),0);
  }

  MSG2H(h, type);
  MSG2H(h, direction);
  MSG2H(h, class);
  MSG2H(h, instance);
  MSG2H(h, sender);
  MSG2H(h, realm);
  MSG2H(h, recipient);
  MSG2H(h, opcode);
  MSG2H(h, hostname);
  MSG2H(h, body);
  MSG2H(h, login);
  MSG2H(h, zsig);
  MSG2H(h, zwriteline);
  if (owl_message_get_header(m)) {
    MSG2H(h, header); 
  }
  hv_store(h, "time", strlen("time"), newSVpv(owl_message_get_timestr(m),0),0);
  hv_store(h, "id", strlen("id"), newSViv(owl_message_get_id(m)),0);
  hv_store(h, "deleted", strlen("deleted"), newSViv(owl_message_is_delete(m)),0);
  hv_store(h, "private", strlen("private"), newSViv(owl_message_is_private(m)),0);

  if (owl_message_is_type_zephyr(m))       blessas = "owl::Message::Zephyr";
  else if (owl_message_is_type_aim(m))     blessas = "owl::Message::AIM";
  else if (owl_message_is_type_admin(m))   blessas = "owl::Message::Admin";
  else if (owl_message_is_type_generic(m)) blessas = "owl::Message::Generic";
  else                                     blessas = "owl::Message";

  hr = sv_2mortal(newRV_noinc((SV*)h));
  return sv_bless(hr, gv_stashpv(blessas,0));
}


SV *owl_perlconfig_curmessage2hashref(void) /*noproto*/
{
  int curmsg;
  owl_view *v;
  v=owl_global_get_current_view(&g);
  if (owl_view_get_size(v) < 1) {
    return &PL_sv_undef;
  }
  curmsg=owl_global_get_curmsg(&g);
  return owl_perlconfig_message2hashref(owl_view_get_element(v, curmsg));
}


/* Calls in a scalar context, passing it a hash reference.
   If return value is non-null, caller must free. */
char *owl_perlconfig_call_with_message(char *subname, owl_message *m)
{
  dSP ;
  int count, len;
  SV *msgref, *srv;
  char *out, *preout;
  
  ENTER ;
  SAVETMPS;
  
  PUSHMARK(SP) ;
  msgref = owl_perlconfig_message2hashref(m);
  XPUSHs(msgref);
  PUTBACK ;
  
  count = call_pv(subname, G_SCALAR|G_EVAL|G_KEEPERR);
  
  SPAGAIN ;

  if (SvTRUE(ERRSV)) {
    STRLEN n_a;
    owl_function_error("Perl Error: '%s'", SvPV(ERRSV, n_a));
    /* and clear the error */
    sv_setsv (ERRSV, &PL_sv_undef);
  }

  if (count != 1) {
    fprintf(stderr, "bad perl!  no biscuit!  returned wrong count!\n");
    abort();
  }

  srv = POPs;

  if (srv) {
    preout=SvPV(srv, len);
    out = owl_malloc(strlen(preout)+1);
    strncpy(out, preout, len);
    out[len] = '\0';
  } else {
    out = NULL;
  }
  
  PUTBACK ;
  FREETMPS ;
  LEAVE ;

  return out;
}

char *owl_perlconfig_readconfig(char *file)
{
  int ret, fd;
  PerlInterpreter *p;
  char filename[1024];
  char *embedding[5];
  char *err;
  struct stat statbuff;

  if (file==NULL) {
    sprintf(filename, "%s/%s", getenv("HOME"), ".owlconf");
  } else {
    strcpy(filename, file);
  }
  embedding[0]="";
  embedding[1]=filename;
  embedding[2]=0;

  /* create and initialize interpreter */
  p=perl_alloc();
  owl_global_set_perlinterp(&g, (void*)p);
  perl_construct(p);

  owl_global_set_no_have_config(&g);

  /* Before we let perl have at it, we'll do our own checks on the the
   *  file to see if it's present, readnable etc.
   */

  /* Not present, start without it */
  ret=stat(filename, &statbuff);
  if (ret) {
    return(NULL);
  }

  /* present, but stat thinks it's unreadable */
  if (! (statbuff.st_mode & S_IREAD)) {
    return(owl_sprintf("%s present but not readable", filename));
  }

  /* can we open it? */
  fd=open(filename, O_RDONLY);
  if (fd==-1) {
    return(owl_sprintf("could not open %s for reading", filename));
  }
  close(fd);

  ret=perl_parse(p, owl_perl_xs_init, 2, embedding, NULL);
  if (ret || SvTRUE(ERRSV)) {
    STRLEN n_a;
    err=owl_strdup(SvPV(ERRSV, n_a));
    sv_setsv(ERRSV, &PL_sv_undef);     /* and clear the error */
    return(err);
  }

  ret=perl_run(p);
  if (ret || SvTRUE(ERRSV)) {
    STRLEN n_a;
    err=owl_strdup(SvPV(ERRSV, n_a));
    sv_setsv(ERRSV, &PL_sv_undef);     /* and clear the error */
    return(err);
  }

  owl_global_set_have_config(&g);

  /* create legacy variables */
  perl_get_sv("owl::id", TRUE);
  perl_get_sv("owl::class", TRUE);
  perl_get_sv("owl::instance", TRUE);
  perl_get_sv("owl::recipient", TRUE);
  perl_get_sv("owl::sender", TRUE);
  perl_get_sv("owl::realm", TRUE);
  perl_get_sv("owl::opcode", TRUE);
  perl_get_sv("owl::zsig", TRUE);
  perl_get_sv("owl::msg", TRUE);
  perl_get_sv("owl::time", TRUE);
  perl_get_sv("owl::host", TRUE);
  perl_get_av("owl::fields", TRUE);
  
  perl_eval_pv(owl_perlwrap_codebuff, FALSE);

  if (SvTRUE(ERRSV)) {
    STRLEN n_a;
    err=owl_strdup(SvPV(ERRSV, n_a));
    sv_setsv (ERRSV, &PL_sv_undef);     /* and clear the error */
    return(err);
  }

  /* check if we have the formatting function */
  if (owl_perlconfig_is_function("owl::format_msg")) {
    owl_global_set_config_format(&g, 1);
  }

  return(NULL);
}

/* returns whether or not a function exists */
int owl_perlconfig_is_function(char *fn) {
  if (perl_get_cv(fn, FALSE)) return(1);
  else return(0);
}

/* returns 0 on success */
int owl_perlconfig_get_hashkeys(char *hashname, owl_list *l)
{
  HV *hv;
  HE *he;
  char *key;
  I32 i;

  if (owl_list_create(l)) return(-1);
  hv = get_hv(hashname, FALSE);
  if (!hv) return(-1);
  i = hv_iterinit(hv);
  while ((he = hv_iternext(hv))) {
    key = hv_iterkey(he, &i);
    if (key) {
      owl_list_append_element(l, owl_strdup(key));
    }
  }
  return(0);
}

/* caller is responsible for freeing returned string */
char *owl_perlconfig_execute(char *line)
{
  STRLEN len;
  SV *response;
  char *out, *preout;

  if (!owl_global_have_config(&g)) return NULL;

  /* execute the subroutine */
  response = perl_eval_pv(line, FALSE);

  if (SvTRUE(ERRSV)) {
    STRLEN n_a;
    owl_function_error("Perl Error: '%s'", SvPV(ERRSV, n_a));
    sv_setsv (ERRSV, &PL_sv_undef);     /* and clear the error */
  }

  preout=SvPV(response, len);
  /* leave enough space in case we have to add a newline */
  out = owl_malloc(strlen(preout)+2);
  strncpy(out, preout, len);
  out[len] = '\0';
  if (!strlen(out) || out[strlen(out)-1]!='\n') {
    strcat(out, "\n");
  }

  return(out);
}

char *owl_perlconfig_getmsg(owl_message *m, int mode, char *subname)
{ 
  /* if mode==1 we are doing message formatting.  The returned
   * formatted message needs to be freed by the caller.
   *
   * if mode==0 we are just doing the message-has-been-received
   * thing.
   */
  if (!owl_global_have_config(&g)) return(NULL);
  
  /* run the procedure corresponding to the mode */
  if (mode==1) {
    char *ret = NULL;
    ret = owl_perlconfig_call_with_message(subname?subname
					   :"owl::_format_msg_legacy_wrap", m);
    if (!ret) {
      ret = owl_sprintf("@b([Perl Message Formatting Failed!])\n");
    } 
    return ret;
  } else {
    char *ptr = NULL;
    if (owl_perlconfig_is_function("owl::receive_msg")) {
      ptr = owl_perlconfig_call_with_message(subname?subname
				       :"owl::_receive_msg_legacy_wrap", m);
    }
    if (ptr) owl_free(ptr);
    return(NULL);
  }
}
