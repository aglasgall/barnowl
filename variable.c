#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "owl.h"

int owl_variable_set_default(owl_variable *v, const GValue *newval);

#define OWLVAR_BOOL(name,default,summary,description) \
  { g_strdup(name), OWL_VARIABLE_BOOL, NULL, default, "on,off", g_strdup(summary), g_strdup(description), {0}, {0}, \
        NULL, NULL, NULL, NULL, NULL, NULL }

#define OWLVAR_BOOL_FULL(name,default,summary,description,validate,set,get) \
  { g_strdup(name), OWL_VARIABLE_BOOL, NULL, default, "on,off", g_strdup(summary), g_strdup(description), {0}, {0}, \
        validate, set, NULL, get, NULL, NULL }

#define OWLVAR_INT(name,default,summary,description) \
  { g_strdup(name), OWL_VARIABLE_INT, NULL, default, "<int>", g_strdup(summary), g_strdup(description), {0}, {0}, \
        NULL, NULL, NULL, NULL, NULL, NULL }

#define OWLVAR_INT_FULL(name,default,summary,description,validset,validate,set,get) \
  { g_strdup(name), OWL_VARIABLE_INT, NULL, default, validset, g_strdup(summary), g_strdup(description), {0}, {0}, \
        validate, set, NULL, get, NULL, NULL }

#define OWLVAR_PATH(name,default,summary,description) \
  { g_strdup(name), OWL_VARIABLE_STRING, g_strdup(default), 0, "<path>", g_strdup(summary), g_strdup(description),  {0}, {0}, \
        NULL, NULL, NULL, NULL, NULL, NULL }

#define OWLVAR_STRING(name,default,summary,description) \
  { g_strdup(name), OWL_VARIABLE_STRING, g_strdup(default), 0, "<string>", g_strdup(summary), g_strdup(description), {0}, {0}, \
        NULL, NULL, NULL, NULL, NULL, NULL }

#define OWLVAR_STRING_FULL(name,default,validset,summary,description,validate,set,get) \
  { g_strdup(name), OWL_VARIABLE_STRING, g_strdup(default), 0, validset, g_strdup(summary), g_strdup(description), {0}, {0}, \
        validate, set, NULL, get, NULL, NULL }
/* enums are really integers, but where validset is a comma-separated
 * list of strings which can be specified.  The tokens, starting at 0,
 * correspond to the values that may be specified. */
#define OWLVAR_ENUM(name,default,summary,description,validset) \
  { g_strdup(name), OWL_VARIABLE_INT, NULL, default, validset, g_strdup(summary), g_strdup(description), {0}, {0}, \
        owl_variable_enum_validate, \
        NULL, owl_variable_enum_set_fromstring, \
        NULL, owl_variable_enum_get_tostring, \
        NULL }

#define OWLVAR_ENUM_FULL(name,default,summary,description,validset,validate, set, get) \
  { g_strdup(name), OWL_VARIABLE_INT, NULL, default, validset, g_strdup(summary), g_strdup(description), {0}, {0}, \
        validate, \
        set, owl_variable_enum_set_fromstring, \
        get, owl_variable_enum_get_tostring, \
        NULL }

int owl_variable_add_defaults(owl_vardict *vd)
{
  owl_variable variables_to_init[] = {

  OWLVAR_STRING( "personalbell" /* %OwlVarStub */, "off",
		 "ring the terminal bell when personal messages are received",
		 "Can be set to 'on', 'off', or the name of a filter which\n"
		 "messages need to match in order to ring the bell"),

  OWLVAR_BOOL( "bell" /* %OwlVarStub */, 1,
	       "enable / disable the terminal bell", "" ),

  OWLVAR_BOOL_FULL( "debug" /* %OwlVarStub */, OWL_DEBUG,
		    "whether debugging is enabled",
		    "If set to 'on', debugging messages are logged to the\n"
		    "file specified by the debugfile variable.\n",
		    NULL, owl_variable_debug_set, NULL),

  OWLVAR_BOOL( "startuplogin" /* %OwlVarStub */, 1,
	       "send a login message when owl starts", "" ),

  OWLVAR_BOOL( "shutdownlogout" /* %OwlVarStub */, 1,
	       "send a logout message when owl exits", "" ),

  OWLVAR_BOOL( "rxping" /* %OwlVarStub */, 0,
	       "display received pings", "" ),

  OWLVAR_BOOL( "txping" /* %OwlVarStub */, 1,
	       "send pings", "" ),

  OWLVAR_BOOL( "sepbar_disable" /* %OwlVarStub */, 0,
	       "disable printing information in the separator bar", "" ),

  OWLVAR_BOOL( "smartstrip" /* %OwlVarStub */, 1,
	       "strip kerberos instance for reply", ""),

  OWLVAR_BOOL( "newlinestrip" /* %OwlVarStub */, 1,
	       "strip leading and trailing newlines", ""),

  OWLVAR_BOOL( "displayoutgoing" /* %OwlVarStub */, 1,
	       "display outgoing messages", "" ),

  OWLVAR_BOOL( "loginsubs" /* %OwlVarStub */, 1,
	       "load logins from .anyone on startup", "" ),

  OWLVAR_BOOL( "logging" /* %OwlVarStub */, 0,
	       "turn personal logging on or off", 
	       "If this is set to on, personal messages are\n"
	       "logged in the directory specified\n"
	       "by the 'logpath' variable.  The filename in that\n"
	       "directory is derived from the sender of the message.\n" ),

  OWLVAR_BOOL( "classlogging" /* %OwlVarStub */, 0,
	       "turn class logging on or off",
	       "If this is set to on, class messages are\n"
	       "logged in the directory specified\n"
	       "by the 'classlogpath' variable.\n" 
	       "The filename in that directory is derived from\n"
	       "the name of the class to which the message was sent.\n" ),

  OWLVAR_ENUM( "loggingdirection" /* %OwlVarStub */, OWL_LOGGING_DIRECTION_BOTH,
	       "specifies which kind of messages should be logged",
	       "Can be one of 'both', 'in', or 'out'.  If 'in' is\n"
	       "selected, only incoming messages are logged, if 'out'\n"
	       "is selected only outgoing messages are logged.  If 'both'\n"
	       "is selected both incoming and outgoing messages are\n"
	       "logged.",
	       "both,in,out"),

  OWLVAR_BOOL( "colorztext" /* %OwlVarStub */, 1,
	       "allow @color() in zephyrs to change color",
	       "Note that only messages received after this variable\n"
	       "is set will be affected." ),

  OWLVAR_BOOL( "fancylines" /* %OwlVarStub */, 1,
	       "Use 'nice' line drawing on the terminal.",
	       "If turned off, dashes, pipes and pluses will be used\n"
	       "to draw lines on the screen.  Useful when the terminal\n"
	       "is causing problems" ),

  OWLVAR_BOOL( "zcrypt" /* %OwlVarStub */, 1,
	       "Do automatic zcrypt processing",
	       "" ),

  OWLVAR_BOOL_FULL( "pseudologins" /* %OwlVarStub */, 0,
		    "Enable zephyr pseudo logins",
		    "When this is enabled, Owl will periodically check the zephyr\n"
		    "location of users in your .anyone file.  If a user is present\n"
		    "but sent no login message, or a user is not present that sent no\n"
		    "logout message, a pseudo login or logout message will be created\n",
		    NULL, owl_variable_pseudologins_set, NULL),

  OWLVAR_BOOL( "ignorelogins" /* %OwlVarStub */, 0,
	       "Enable printing of login notifications",
	       "When this is enabled, Owl will print login and logout notifications\n"
	       "for AIM, zephyr, or other protocols.  If disabled Owl will not print\n"
	       "login or logout notifications.\n"),

  OWLVAR_STRING( "logfilter" /* %OwlVarStub */, "",
		 "name of a filter controlling which messages to log",

		 "If non empty, any messages matching the given filter will be logged.\n"
		 "This is a completely separate mechanism from the other logging\n"
		 "variables like logging, classlogging, loglogins, loggingdirection,\n"
		 "etc.  If you want this variable to control all logging, make sure\n"
		 "all other logging variables are in their default state.\n"),

  OWLVAR_BOOL( "loglogins" /* %OwlVarStub */, 0,
	       "Enable logging of login notifications",
	       "When this is enabled, Owl will login login and logout notifications\n"
	       "for AIM, zephyr, or other protocols.  If disabled Owl will not print\n"
	       "login or logout notifications.\n"),

  OWLVAR_ENUM_FULL( "disable-ctrl-d" /* %OwlVarStub:lockout_ctrld */, 1,
		    "don't send zephyrs on C-d",
		    "If set to 'off', C-d won't send a zephyr from the edit\n"
		    "window.  If set to 'on', C-d will always send a zephyr\n"
		    "being composed in the edit window.  If set to 'middle',\n"
		    "C-d will only ever send a zephyr if the cursor is at\n"
		    "the end of the message being composed.\n\n"
		    "Note that this works by changing the C-d keybinding\n"
		    "in the editmulti keymap.\n",
		    "off,middle,on",
		    NULL, owl_variable_disable_ctrl_d_set, NULL),

  OWLVAR_PATH( "logpath" /* %OwlVarStub */, "~/zlog/people",
	       "path for logging personal zephyrs", 
	       "Specifies a directory which must exist.\n"
	       "Files will be created in the directory for each sender.\n"),

  OWLVAR_PATH( "classlogpath" /* %OwlVarStub:classlogpath */, "~/zlog/class",
	       "path for logging class zephyrs",
	       "Specifies a directory which must exist.\n"
	       "Files will be created in the directory for each class.\n"),

  OWLVAR_PATH( "debug_file" /* %OwlVarStub */, OWL_DEBUG_FILE,
	       "path for logging debug messages when debugging is enabled",
	       "This file will be logged to if 'debug' is set to 'on'.\n"
               "BarnOwl will append a dot and the current process's pid to the filename."),
  
  OWLVAR_PATH( "zsigproc" /* %OwlVarStub:zsigproc */, NULL,
	       "name of a program to run that will generate zsigs",
	       "This program should produce a zsig on stdout when run.\n"
	       "Note that it is important that this program not block.\n\n"
               "See the documentation for 'zsig' for more information about\n"
               "how the outgoing zsig is chosen."
               ),

  OWLVAR_PATH( "newmsgproc" /* %OwlVarStub:newmsgproc */, NULL,
	       "name of a program to run when new messages are present",
	       "The named program will be run when owl receives new\n"
	       "messages.  It will not be run again until the first\n"
	       "instance exits"),

  OWLVAR_STRING( "zsender" /* %OwlVarStub */, "",
             "zephyr sender name",
         "Allows you to customize the outgoing username in\n"
         "zephyrs.  If this is unset, it will use your Kerberos\n"
         "principal. Note that customizing the sender name will\n"
         "cause your zephyrs to be sent unauthenticated."),

  OWLVAR_STRING( "zsigfunc" /* %OwlVarStub */, "BarnOwl::default_zephyr_signature()",
		 "zsig perl function",
		 "Called every time you start a zephyrgram without an\n"
		 "explicit zsig.  The default setting implements the policy\n"
		 "described in the documentation for the 'zsig' variable.\n"
		 "See also BarnOwl::random_zephyr_signature().\n"),

  OWLVAR_STRING( "zsig" /* %OwlVarStub */, "",
	         "zephyr signature",
		 "The zsig to get on outgoing messages. If this variable is\n"
		 "unset, 'zsigproc' will be run to generate a zsig. If that is\n"
		 "also unset, the 'zwrite-signature' zephyr variable will be\n"
		 "used instead.\n"),

  OWLVAR_STRING( "appendtosepbar" /* %OwlVarStub */, "",
	         "string to append to the end of the sepbar",
		 "The sepbar is the bar separating the top and bottom\n"
		 "of the owl screen.  Any string specified here will\n"
		 "be displayed on the right of the sepbar\n"),

  OWLVAR_BOOL( "zaway" /* %OwlVarStub */, 0,
	       "turn zaway on or off", "" ),

  OWLVAR_STRING( "zaway_msg" /* %OwlVarStub */, 
		 OWL_DEFAULT_ZAWAYMSG,
	         "zaway msg for responding to zephyrs when away", "" ),

  OWLVAR_STRING( "zaway_msg_default" /* %OwlVarStub */, 
		 OWL_DEFAULT_ZAWAYMSG,
	         "default zaway message", "" ),

  OWLVAR_BOOL_FULL( "aaway" /* %OwlVarStub */, 0,
		    "Set AIM away status",
		    "",
		    NULL, owl_variable_aaway_set, NULL),

  OWLVAR_STRING( "aaway_msg" /* %OwlVarStub */, 
		 OWL_DEFAULT_AAWAYMSG,
	         "AIM away msg for responding when away", "" ),

  OWLVAR_STRING( "aaway_msg_default" /* %OwlVarStub */, 
		 OWL_DEFAULT_AAWAYMSG,
	         "default AIM away message", "" ),

  OWLVAR_STRING( "view_home" /* %OwlVarStub */, "all",
	         "home view to switch to after 'X' and 'V'", 
		 "SEE ALSO: view, filter\n" ),

  OWLVAR_STRING( "alert_filter" /* %OwlVarStub */, "none",
		 "filter on which to trigger alert actions",
		 "" ),

  OWLVAR_STRING( "alert_action" /* %OwlVarStub */, "nop",
		 "owl command to execute for alert actions",
		 "" ),

  OWLVAR_STRING_FULL( "tty" /* %OwlVarStub */, "", "<string>", "tty name for zephyr location", "",
		      NULL, owl_variable_tty_set, NULL),

  OWLVAR_STRING( "default_style" /* %OwlVarStub */, "default",
		 "name of the default formatting style",
		 "This sets the default message formatting style.\n"
		 "Styles may be created with the 'style' command.\n"
		 "Some built-in styles include:\n"
		 "   default  - the default owl formatting\n"
		 "   oneline  - one line per-message\n"
		 "   perl     - legacy perl interface\n"
		 "\nSEE ALSO: style, show styles, view -s <style>\n"
		 ),


  OWLVAR_INT(    "edit:maxfillcols" /* %OwlVarStub:edit_maxfillcols */, 70,
	         "maximum number of columns for M-q (edit:fill-paragraph) to fill text to",
                 "This specifies the maximum number of columns for M-q to fill text\n"
                 "to.  If set to 0, M-q will wrap to the width of the window, and\n"
                 "values less than 0 disable M-q entirely.\n"),

  OWLVAR_INT(    "edit:maxwrapcols" /* %OwlVarStub:edit_maxwrapcols */, 70,
	         "maximum number of columns for line-wrapping",
                 "This specifies the maximum number of columns for\n"
                 "auto-line-wrapping.  If set to 0, text will be wrapped at the\n"
                 "window width. Values less than 0 disable automatic wrapping.\n"
                 "\n"
                 "As a courtesy to recipients, it is recommended that outgoing\n"
                 "Zephyr messages be no wider than 70 columns.\n"),

  OWLVAR_INT( "aim_ignorelogin_timer" /* %OwlVarStub */, 15,
	      "number of seconds after AIM login to ignore login messages",
	      "This specifies the number of seconds to wait after an\n"
	      "AIM login before allowing the receipt of AIM login notifications.\n"
	      "By default this is set to 15.  If you would like to view login\n"
	      "notifications of buddies as soon as you login, set it to 0 instead."),

	      
  OWLVAR_INT_FULL( "typewinsize" /* %OwlVarStub:typwin_lines */, 
		   OWL_TYPWIN_SIZE,
		  "number of lines in the typing window", 
		   "This specifies the height of the window at the\n"
		   "bottom of the screen where commands are entered\n"
		   "and where messages are composed.\n",
		   "int > 0",
		   owl_variable_int_validate_gt0,
		   owl_variable_typewinsize_set,
		   NULL /* use default for get */
		   ),

  OWLVAR_INT( "typewindelta" /* %OwlVarStub */, 0,
		  "number of lines to add to the typing window when in use",
		   "On small screens you may want the typing window to\n"
		   "auto-hide when not entering a command or message.\n"
		   "This variable is the number of lines to add to the\n"
           "typing window when it is in use; you can then set\n"
           "typewinsize to 1.\n\n"
           "This works a lot better with a non-default scrollmode;\n"
           "try :set scrollmode pagedcenter.\n"),

  OWLVAR_ENUM( "scrollmode" /* %OwlVarStub */, OWL_SCROLLMODE_NORMAL,
	       "how to scroll up and down",
	       "This controls how the screen is scrolled as the\n"
	       "cursor moves between messages being displayed.\n"
	       "The following modes are supported:\n\n"
	       "   normal      - This is the owl default.  Scrolling happens\n"
	       "                 when it needs to, and an attempt is made to\n"
	       "                 keep the current message roughly near\n"
	       "                 the middle of the screen.\n"
	       "   top         - The current message will always be the\n"
	       "                 the top message displayed.\n"
	       "   neartop     - The current message will be one down\n"
	       "                 from the top message displayed,\n"
	       "                 where possible.\n"
	       "   center      - An attempt is made to keep the current\n"
	       "                 message near the center of the screen.\n"
	       "   paged       - The top message displayed only changes\n"
	       "                 when user moves the cursor to the top\n"
	       "                 or bottom of the screen.  When it moves,\n"
	       "                 the screen will be paged up or down and\n"
	       "                 the cursor will be near the top or\n"
	       "                 the bottom.\n"
	       "   pagedcenter - The top message displayed only changes\n"
	       "                 when user moves the cursor to the top\n"
	       "                 or bottom of the screen.  When it moves,\n"
	       "                 the screen will be paged up or down and\n"
	       "                 the cursor will be near the center.\n",
	       "normal,top,neartop,center,paged,pagedcenter" ),

  OWLVAR_BOOL( "narrow-related" /* %OwlVarStub:narrow_related */, 1,
               "Make smartnarrow use broader filters",
               "Causes smartfiler to narrow to messages \"related\" to \n"
               "the current message, as well as ones to the same place.\n\n"
               "for Zephyr, this controls whether to narrow to e.g. class-help or\n"
               "class-help.d alone, or to related-class-help, which includes\n"
               "help, unhelp, help.d, etc.\n\nDefault is true (include unclasses, etc.).\n" ),

  OWLVAR_BOOL( "_followlast" /* %OwlVarStub */, 0,
	       "enable automatic following of the last zephyr",
	       "If the cursor is at the last message, it will\n"
	       "continue to follow the last message if this is set.\n"
	       "Note that this is currently risky as you might accidentally\n"
	       "delete a message right as it came in.\n" ),

  OWLVAR_STRING_FULL( "default_exposure" /* %OwlVarStub */, "",
                      "none,opstaff,realm-visible,realm-announced,net-visible,net-announced",
                      "controls the persistent value for exposure",
                      "The default exposure level corresponds to the Zephyr exposure value\n"
                      "in ~/.zephyr.vars.  Defaults to realm-visible if there is no value in\n"
                      "~/.zephyr.vars.\n"
                      "See the description of exposure for the values this can be.",
                      NULL, owl_variable_default_exposure_set, owl_variable_default_exposure_get ),

  OWLVAR_STRING_FULL( "exposure" /* %OwlVarStub */, "",
                      "none,opstaff,realm-visible,realm-announced,net-visible,net-announced",
                      "controls who can zlocate you",
                      "The exposure level, defaulting to the value of default_exposure,\n"
                      "can be one of the following (from least exposure to widest exposure,\n"
                      "as listed in zctl(1)):\n"
                      "\n"
                      "   none            - This completely disables Zephyr for the user. \n"
                      "                     The user is not registered with Zephyr.  No user\n"
                      "                     location information is retained by Zephyr.  No\n"
                      "                     login or logout announcements will be sent.  No\n"
                      "                     subscriptions will be entered for the user, and\n"
                      "                     no notices will be displayed by zwgc(1).\n"
                      "   opstaff         - The user is registered with Zephyr.  No login or\n"
                      "                     logout announcements will be sent, and location\n"
                      "                     information will only be visible to Operations\n"
                      "                     staff.  Default subscriptions and any additional\n"
                      "                     personal subscriptions will be entered for the\n"
                      "                     user.\n"
                      "   realm-visible   - The user is registered with Zephyr.  User\n"
                      "                     location information is retained by Zephyr and\n"
                      "                     made available only to users within the user’s\n"
                      "                     Kerberos realm.  No login or logout\n"
                      "                     announcements will be sent.  This is the system\n"
                      "                     default.  Default subscriptions and any\n"
                      "                     additional personal subscriptions will be\n"
                      "                     entered for the user.\n"
                      "   realm-announced - The user is registered with Zephyr.  User\n"
                      "                     location information is retained by Zephyr and\n"
                      "                     made available only to users authenticated\n"
                      "                     within the user’s Kerberos realm.  Login and\n"
                      "                     logout announcements will be sent, but only to\n"
                      "                     users within the user’s Kerberos realm who have\n"
                      "                     explicitly requested such via subscriptions. \n"
                      "                     Default subscriptions and any additional\n"
                      "                     personal subscriptions will be entered for the\n"
                      "                     user.\n"
                      "   net-visible     - The user is registered with Zephyr.  User\n"
                      "                     location information is retained by Zephyr and\n"
                      "                     made available to any authenticated user who\n"
                      "                     requests such.  Login and logout announcements\n"
                      "                     will be sent only to users within the user’s\n"
                      "                     Kerberos realm who have explicitly requested\n"
                      "                     such via subscriptions.  Default subscriptions\n"
                      "                     and any additional personal subscriptions will\n"
                      "                     be entered for the user.\n"
                      "   net-announced   - The user is registered with Zephyr.  User\n"
                      "                     location information is retained by Zephyr and\n"
                      "                     made available to any authenticated user who\n"
                      "                     requests such.  Login and logout announcements\n"
                      "                     will be sent to any user has requested such. \n"
                      "                     Default subscriptions and any additional\n"
                      "                     personal subscriptions will be entered for the\n"
                      "                     user.\n",
                      NULL, owl_variable_exposure_set, NULL /* use default for get */ ),

  /* This MUST be last... */
  { NULL, 0, NULL, 0, NULL, NULL, NULL, {0}, {0}, NULL,
    NULL, NULL, NULL, NULL, NULL }

  };

  int ret = owl_variable_dict_add_from_list(vd, variables_to_init);
  owl_variable *var;
  for (var = variables_to_init; var->name != NULL; var++)
    owl_variable_cleanup(var);
  return ret;
}

/**************************************************************************/
/*********************** SPECIFIC TO VARIABLES ****************************/
/**************************************************************************/


/* commonly useful */

int owl_variable_int_validate_gt0(const owl_variable *v, const GValue *newval)
{
  if (newval == NULL) return(0);
  else if (g_value_get_int(newval) < 1) return(0);
  else return (1);
}

int owl_variable_int_validate_positive(const owl_variable *v, const GValue *newval)
{
  if (newval == NULL) return(0);
  else if (g_value_get_int(newval) < 0) return(0);
  else return (1);
}

/* typewinsize */
int owl_variable_typewinsize_set(owl_variable *v, const GValue *newval)
{
  int rv;
  rv = owl_variable_set_default(v, newval);
  if (0 == rv) owl_mainpanel_layout_contents(&g.mainpanel);
  return(rv);
}

/* debug (cache value in g->debug) */
int owl_variable_debug_set(owl_variable *v, const GValue *newval)
{
  int val;
  if (newval) {
    val = g_value_get_boolean(newval);
    if (val == 1 || val == 0) {
      g.debug = val;
    }
  }
  return owl_variable_set_default(v, newval);
}

/* When 'aaway' is changed, need to notify the AIM server */
int owl_variable_aaway_set(owl_variable *v, const GValue *newval)
{
  int val;
  if (newval) {
    val = g_value_get_boolean(newval);
    if (val == 1) {
      owl_aim_set_awaymsg(owl_global_get_aaway_msg(&g));
    } else if (val == 0) {
      owl_aim_set_awaymsg("");
    }
  }
  return owl_variable_set_default(v, newval);
}

int owl_variable_pseudologins_set(owl_variable *v, const GValue *newval)
{
  static guint timer = 0;
  int val;
  if (newval) {
    val = g_value_get_boolean(newval);
    if (val == 1) {
      owl_function_zephyr_buddy_check(0);
      if (timer == 0) {
        timer = g_timeout_add_seconds(180, owl_zephyr_buddycheck_timer, NULL);
      }
    } else {
      if (timer != 0) {
        g_source_remove(timer);
        timer = 0;
      }
    }
  }
  return owl_variable_set_default(v, newval);
}

/* note that changing the value of this will clobber 
 * any user setting of this */
int owl_variable_disable_ctrl_d_set(owl_variable *v, const GValue *newval)
{
  int val;
  if (newval && !owl_context_is_startup(owl_global_get_context(&g))) {
    val = g_value_get_int(newval);
    if (val == 2) {
      owl_function_command_norv("bindkey editmulti C-d command edit:delete-next-char");
    } else if (val == 1) {
      owl_function_command_norv("bindkey editmulti C-d command edit:done-or-delete");
    } else {
      owl_function_command_norv("bindkey editmulti C-d command edit:done");
    }
  }  
  return owl_variable_set_default(v, newval);  
}

int owl_variable_tty_set(owl_variable *v, const GValue *newval)
{
  owl_zephyr_set_locationinfo(g_get_host_name(), 
                              g_value_get_string(newval));
  return(owl_variable_set_default(v, newval));
}

int owl_variable_default_exposure_set(owl_variable *v, const GValue *newval)
{
  return owl_zephyr_set_default_exposure(g_value_get_string(newval));
}

CALLER_OWN const GValue *owl_variable_default_exposure_get(const owl_variable *v)
{
  GValue *ret = g_new0(GValue, 1);
  g_value_init(ret, G_TYPE_STRING);
  g_value_set_string(ret, owl_zephyr_get_default_exposure());
  return ret;
}

int owl_variable_exposure_set(owl_variable *v, const GValue *newval)
{
  const char *str = NULL;
  int ret;
  GValue tmp = {0};
  str = g_value_get_string(newval);
  ret = owl_zephyr_set_exposure(str);
  if (ret != 0)
    return ret;
  g_value_init(&tmp, G_TYPE_STRING);
  g_value_set_string(&tmp, owl_zephyr_normalize_exposure(str));
  ret = owl_variable_set_default(v, &tmp);
  g_value_unset(&tmp);
  return ret;
}

/**************************************************************************/
/****************************** GENERAL ***********************************/
/**************************************************************************/

int owl_variable_dict_setup(owl_vardict *vd) {
  owl_dict_create(vd);
  return owl_variable_add_defaults(vd);
}

int owl_variable_dict_add_from_list(owl_vardict *vd, owl_variable *variables_to_init)
{
  owl_variable *var, *cur;
  for (var = variables_to_init; var->name != NULL; var++) {
    cur = g_new(owl_variable, 1);
    *cur = *var;
    /* strdup all the strings so we can delete them consistently. */
    cur->name = g_strdup(var->name);
    cur->summary = g_strdup(var->summary);
    cur->description = g_strdup(var->description);
    GValue value = {0};
    switch (cur->type) {
      /*    case OWL_VARIABLE_OTHER:
      cur->set_fn(cur, cur->pval_default);
      break;*/
    case OWL_VARIABLE_STRING:
      if (!cur->validate_fn) {
        cur->validate_fn = owl_variable_string_validate_default;
      }
      if (!cur->set_fromstring_fn) {
        cur->set_fromstring_fn = owl_variable_string_set_fromstring_default;
      }
      if (!cur->get_tostring_fn) {
        cur->get_tostring_fn = owl_variable_string_get_tostring_default;
      }
      g_value_init(&value,G_TYPE_STRING);
      g_value_set_string(&value, var->pval_default);
      break;
    case OWL_VARIABLE_BOOL:
      if (!cur->validate_fn) {
        cur->validate_fn = owl_variable_bool_validate_default;
      }
      if (!cur->set_fromstring_fn) {
        cur->set_fromstring_fn = owl_variable_bool_set_fromstring_default;
      }
      if (!cur->get_tostring_fn) {
        cur->get_tostring_fn = owl_variable_bool_get_tostring_default;      
      }      
      g_value_init(&value,G_TYPE_BOOLEAN);
      g_value_set_boolean(&value, !!(cur->ival_default));
      break;
    case OWL_VARIABLE_INT:
      if (!cur->validate_fn) {
        cur->validate_fn = owl_variable_int_validate_default;
      }
      if (!cur->set_fromstring_fn) {
        cur->set_fromstring_fn = owl_variable_int_set_fromstring_default;
      }
      if (!cur->get_tostring_fn) {
        cur->get_tostring_fn = owl_variable_int_get_tostring_default;      
      }
      g_value_init(&value,G_TYPE_INT);
      g_value_set_int(&value, cur->ival_default);
      break;
    default:
      fprintf(stderr, "owl_variable_setup: invalid variable type\n");
      return(-2);
    }
    if (!cur->set_fn) {
      cur->set_fn = owl_variable_set_default;
    }
    if (!cur->get_fn) {
      cur->get_fn = owl_variable_get_default;
    }
    if (!cur->delete_fn) {
      cur->delete_fn = owl_variable_delete_default;
    }
    
    g_value_init(&(cur->gval_default), G_VALUE_TYPE(&value));
    g_value_init(&(cur->val), G_VALUE_TYPE(&value));
    g_value_copy(&value, &(cur->gval_default));
    cur->set_fn(cur, &(cur->gval_default));
    g_value_unset(&value);
    owl_dict_insert_element(vd, cur->name, cur, NULL);
  }
  return 0;
}


void owl_variable_dict_add_variable(owl_vardict * vardict,
                                    owl_variable * var) {
  owl_dict_insert_element(vardict, var->name, var, (void (*)(void *))owl_variable_delete);
}

CALLER_OWN owl_variable *owl_variable_newvar(const char *name, const char *summary, const char *description)
{
  owl_variable * var = g_new0(owl_variable, 1);
  var->name = g_strdup(name);
  var->summary = g_strdup(summary);
  var->description = g_strdup(description);
  return var;
}

void owl_variable_update(owl_variable *var, const char *summary, const char *desc) {
  g_free(var->summary);
  var->summary = g_strdup(summary);
  g_free(var->description);
  var->description = g_strdup(desc);
}

/* does *not* set tostring, fromstring, or validate functions */
/* returns the new variable if one was created, NULL otherwise */
owl_variable *owl_variable_dict_newvar(owl_vardict *vd, const char *name,
                              const char *summ, const char *desc,
                              const GValue *initval, const int owl_type) {
  owl_variable *old = owl_variable_get_var(vd, name, owl_type);
  if(old) {
    owl_variable_update(old, summ, desc);
    g_value_copy(initval, &(old->gval_default));
    return NULL;
  } else {
    owl_variable * var = owl_variable_newvar(name, summ, desc);
    var->type = owl_type;
    memset(&(var->gval_default), 0, sizeof(GValue));
    g_value_init(&(var->gval_default), G_VALUE_TYPE(initval));
    g_value_copy(initval, &(var->gval_default));
    memset(&(var->val), 0, sizeof(GValue));
    g_value_init(&(var->val), G_VALUE_TYPE(initval));
    var->set_fn = owl_variable_set_default;
    var->get_fn = owl_variable_get_default;
    var->delete_fn = owl_variable_delete_default;
    var->set_fn(var, initval);
    owl_variable_dict_add_variable(vd, var);
    return var;
  }
}

void owl_variable_dict_newvar_string(owl_vardict * vd, const char *name, const char *summ, const char * desc, const char * initval) {
  GValue temp = {0};
  owl_variable *var;
  g_value_init(&temp,G_TYPE_STRING);
  g_value_set_string(&temp,initval);
  var = owl_variable_dict_newvar(vd, name, summ, desc, &temp, 
                                 OWL_VARIABLE_STRING);
  if(var) {
    var->set_fromstring_fn = owl_variable_string_set_fromstring_default;
    var->get_tostring_fn = owl_variable_string_get_tostring_default;
  }
  g_value_unset(&temp);
}

void owl_variable_dict_newvar_int(owl_vardict * vd, const char *name, const char *summ, const char * desc, int initval) {
  GValue temp = {0};
  owl_variable *var;
  g_value_init(&temp,G_TYPE_INT);
  g_value_set_int(&temp,initval);
  var = owl_variable_dict_newvar(vd, name, summ, desc, &temp, 
                                 OWL_VARIABLE_INT);
  if(var) {
    var->validate_fn = owl_variable_int_validate_default;
    var->set_fromstring_fn = owl_variable_int_set_fromstring_default;
    var->get_tostring_fn = owl_variable_int_get_tostring_default;
  }
  g_value_unset(&temp);
}

void owl_variable_dict_newvar_bool(owl_vardict * vd, const char *name, const char *summ, const char * desc, int initval) {
  GValue temp = {0};
  owl_variable *var;
  gboolean defval = !!initval;
  g_value_init(&temp,G_TYPE_BOOLEAN);
  g_value_set_boolean(&temp, defval);
  var = owl_variable_dict_newvar(vd, name, summ, desc, &temp, 
                                 OWL_VARIABLE_INT);

  if(var) {
    var->validate_fn = owl_variable_bool_validate_default;
    var->set_fromstring_fn = owl_variable_bool_set_fromstring_default;
    var->get_tostring_fn = owl_variable_bool_get_tostring_default;
  }
  g_value_unset(&temp);
}

void owl_variable_dict_cleanup(owl_vardict *d)
{
  owl_dict_cleanup(d, (void (*)(void *))owl_variable_delete);
}

CALLER_OWN GPtrArray *owl_variable_dict_get_names(const owl_vardict *d) {
  return owl_dict_get_keys(d);
}

void owl_variable_cleanup(owl_variable *v)
{
  if (v->delete_fn) v->delete_fn(v);
  g_free(v->name);
  g_free(v->summary);
  g_free(v->description);
  if(G_IS_VALUE(&v->gval_default)) {
    g_value_unset(&(v->gval_default));
  }
}

void owl_variable_delete(owl_variable *v)
{
  owl_variable_cleanup(v);
  g_free(v);
}


const char *owl_variable_get_description(const owl_variable *v) {
  return v->description;
}

const char *owl_variable_get_summary(const owl_variable *v) {
  return v->summary;
}

const char *owl_variable_get_validsettings(const owl_variable *v) {
  if (v->validsettings) {
    return v->validsettings;
  } else {
    return "";
  }
}

/* functions for getting and setting variable values */

/* returns 0 on success, prints a status msg if msg is true */
int owl_variable_set_fromstring(owl_vardict *d, const char *name, const char *value, int msg, int requirebool) {
  owl_variable *v;
  char *tostring;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL) {
    if (msg) owl_function_error("Unknown variable %s", name);
    return -1;
  }
  if (!v->set_fromstring_fn) {
    if (msg) owl_function_error("Variable %s is read-only", name);
    return -1;   
  }
  if (requirebool && v->type!=OWL_VARIABLE_BOOL) {
    if (msg) owl_function_error("Variable %s is not a boolean", name);
    return -1;   
  }
  if (0 != v->set_fromstring_fn(v, value)) {
    if (msg) owl_function_error("Unable to set %s (must be %s)", name, 
				  owl_variable_get_validsettings(v));
    return -1;
  }
  if (msg && v->get_tostring_fn) {
    tostring = v->get_tostring_fn(v, v->get_fn(v));
    owl_function_makemsg("%s = '%s'", name, tostring);
    g_free(tostring);
  }    
  return 0;
}
 
static int owl_variable_set_gvalue(owl_vardict *d, const char *name, 
                                   const GValue *value) {
  owl_variable *v = NULL;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->set_fn) return(-1);
  if(!G_TYPE_CHECK_VALUE_TYPE(&(v->val), G_VALUE_TYPE(value))) {
    return -1;
  }
  return v->set_fn(v,value);

}

int owl_variable_set_string(owl_vardict *d, const char *name, const char *newval) {
  GValue temp = {0};
  int res = 0;
  g_value_init(&temp, G_TYPE_STRING);
  g_value_set_string(&temp, newval);
  res = owl_variable_set_gvalue(d, name, &temp);
  g_value_unset(&temp);
  return res;
}
 
int owl_variable_set_int(owl_vardict *d, const char *name, int newval) {
  GValue temp = {0};
  int res = 0;
  g_value_init(&temp, G_TYPE_INT);
  g_value_set_int(&temp, newval);
  res = owl_variable_set_gvalue(d, name, &temp);
  g_value_unset(&temp);
  return res;
}
 
int owl_variable_set_bool_on(owl_vardict *d, const char *name) {
  return owl_variable_set_int(d,name,1);
}

int owl_variable_set_bool_off(owl_vardict *d, const char *name) {
  return owl_variable_set_int(d,name,0);
}

CALLER_OWN char *owl_variable_get_tostring(const owl_vardict *d, const char *name)
{
  owl_variable *v;
  if (!name) return NULL;
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->get_tostring_fn) return NULL;
  return v->get_tostring_fn(v, v->get_fn(v));
}

CALLER_OWN char *owl_variable_get_default_tostring(const owl_vardict *d, const char *name)
{
  owl_variable *v;
  if (!name) return NULL;
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->get_tostring_fn) return NULL;
  return v->get_tostring_fn(v, &(v->gval_default));
}

owl_variable *owl_variable_get_var(const owl_vardict *d, const char *name, int require_type) {
  owl_variable *v;
  if (!name) return(NULL);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->get_fn || v->type != require_type) return(NULL);
  return v;
}

/* returns a reference */
const GValue *owl_variable_get(const owl_vardict *d, const char *name, int require_type) {
  owl_variable *v = owl_variable_get_var(d, name, require_type);
  if(v == NULL) return NULL;
  return v->get_fn(v);
}

/* returns a reference */
const char *owl_variable_get_string(const owl_vardict *d, const char *name) {
  const GValue *val = NULL;
  val = owl_variable_get(d,name, OWL_VARIABLE_STRING);
  if(!val) {
    return NULL;
  } else {
    return g_value_get_string(val);
  }
}

/* returns a reference */
/*const void *owl_variable_get_other(const owl_vardict *d, const char *name) {
  return owl_variable_get(d,name, OWL_VARIABLE_OTHER);
  }*/

int owl_variable_get_int(const owl_vardict *d, const char *name) {
  const GValue *val = NULL;
  val = owl_variable_get(d,name, OWL_VARIABLE_INT);
  if(!val) {
    return -1;
  } else {
    return g_value_get_int(val);
  }
}

int owl_variable_get_bool(const owl_vardict *d, const char *name) {
  const GValue *val = NULL;
  val = owl_variable_get(d,name, OWL_VARIABLE_BOOL);
  if(!val) {
    return -1;
  } else {
    return g_value_get_boolean(val);
  }
}

void owl_variable_describe(const owl_vardict *d, const char *name, owl_fmtext *fm) {
  char *default_buf;
  owl_variable *v;

  if (!name
      || (v = owl_dict_find_element(d, name)) == NULL 
      || !v->get_fn) {
    owl_fmtext_appendf_normal(fm, "     No such variable '%s'\n", name);
    return;
  }
  default_buf = v->get_tostring_fn(v, &(v->gval_default));
  owl_fmtext_appendf_normal(fm, OWL_TABSTR "%-20s - %s (default: '%s')\n",
			    v->name,
			    owl_variable_get_summary(v), default_buf);
  g_free(default_buf);
}

void owl_variable_get_help(const owl_vardict *d, const char *name, owl_fmtext *fm) {
  char *tostring;
  owl_variable *v;

  if (!name
      || (v = owl_dict_find_element(d, name)) == NULL 
      || !v->get_fn) {
    owl_fmtext_append_normal(fm, "No such variable...\n");
    return;
  }

  owl_fmtext_append_bold(fm, "OWL VARIABLE\n\n");
  owl_fmtext_append_normal(fm, OWL_TABSTR);
  owl_fmtext_append_normal(fm, name);
  owl_fmtext_append_normal(fm, " - ");
  owl_fmtext_append_normal(fm, v->summary);
  owl_fmtext_append_normal(fm, "\n\n");

  owl_fmtext_append_normal(fm, "Current:        ");
  tostring = owl_variable_get_tostring(d, name);
  owl_fmtext_append_normal(fm, tostring);
  g_free(tostring);
  owl_fmtext_append_normal(fm, "\n\n");
  
  tostring = v->get_tostring_fn(v, &(v->gval_default));
  owl_fmtext_append_normal(fm, "Default:        ");
  owl_fmtext_append_normal(fm, tostring);
  owl_fmtext_append_normal(fm, "\n\n");

  owl_fmtext_append_normal(fm, "Valid Settings: ");
  owl_fmtext_append_normal(fm, owl_variable_get_validsettings(v));
  owl_fmtext_append_normal(fm, "\n\n");

  if (v->description && *v->description) {
    owl_fmtext_append_normal(fm, "Description:\n");
    owl_fmtext_append_normal(fm, owl_variable_get_description(v));
    owl_fmtext_append_normal(fm, "\n\n");
  }
  g_free(tostring);
}




/**************************************************************************/
/*********************** GENERAL TYPE-SPECIFIC ****************************/
/**************************************************************************/

/* default common functions */

const GValue *owl_variable_get_default(const owl_variable *v) {
  return &(v->val);
}

int owl_variable_set_default(owl_variable *v,
                             const GValue *newval) 
{
  if (v->validate_fn) {
    if (!v->validate_fn(v, newval)) return(-1);
  }
  g_value_copy(newval, &(v->val));
  return 0;
}

void owl_variable_delete_default(owl_variable *v)
{
  if(G_IS_VALUE(&v->val)) {
    g_value_unset(&(v->val));
  }
}

/* default functions for booleans */

int owl_variable_bool_validate_default(const owl_variable *v, const GValue *newval) {
  if (newval == NULL) return(0);
  gboolean val = g_value_get_boolean(newval);
  if (val == 1 || val ==0) {
    return(1);
  } else {
    return (0);
  }
}

int owl_variable_bool_set_fromstring_default(owl_variable *v, const char *newval) {
  int i;
  int res = -1;
  GValue val = {0};
  g_value_init(&val, G_TYPE_BOOLEAN);
  if (!strcmp(newval, "on")) {
    i=1;
  } else if (!strcmp(newval, "off")) {
    i=0;
  } else {
    return(-1);
  }
  g_value_set_boolean(&val, i);
  res = v->set_fn(v, &val);
  g_value_unset(&val);
  return res;
}

CALLER_OWN char *owl_variable_bool_get_tostring_default(const owl_variable *v, const GValue *val)
{

  if (val == NULL || !G_IS_VALUE(val)) {
    return g_strdup("<null>");
  } else if (g_value_get_boolean(val) == 0) {
    return g_strdup("off");
  } else if (g_value_get_boolean(val) == 1) {
    return g_strdup("on");
  } else {
    return g_strdup("<invalid>");
  }
}

/* default functions for integers */

int owl_variable_int_validate_default(const owl_variable *v, const GValue *newval) {
  if (newval == NULL) return(0);
  else return (1);
}

int owl_variable_int_set_fromstring_default(owl_variable *v, const char *newval) {
  int i;
  int res = -1;
  char *ep;
  GValue temp = {0};
  g_value_init(&temp,G_TYPE_INT);
  i = strtol(newval, &ep, 10);
  if (*ep || ep==newval) return(-1);
  g_value_set_int(&temp, i);
  res = (v->set_fn(v, &temp));
  g_value_unset(&temp);
  return res;
}

CALLER_OWN char *owl_variable_int_get_tostring_default(const owl_variable *v, const GValue *val)
{
  if (val == NULL) {
    return g_strdup("<null>");
  } else {
    return g_strdup_printf("%d", g_value_get_int(val));
  } 
}

/* default functions for enums (a variant of integers) */

int owl_variable_enum_validate(const owl_variable *v, const GValue *newval) {  
  char **enums;
  int nenums, val;
  if (newval == NULL) return(0);
  enums = g_strsplit_set(v->validsettings, ",", 0);
  nenums = g_strv_length(enums);
  g_strfreev(enums);
  val = g_value_get_int(newval);
  if (val < 0 || val >= nenums) {
    return(0);
  }
  return(1);
}

int owl_variable_enum_set_fromstring(owl_variable *v, const char *newval) {
  char **enums;
  int i, val=-1;
  int res = -1;
  GValue temp = {0};
  g_value_init(&temp, G_TYPE_INT);
  if (newval == NULL) return(-1);
  enums = g_strsplit_set(v->validsettings, ",", 0);
  for (i = 0; enums[i] != NULL; i++) {
    if (0==strcmp(newval, enums[i])) {
      val = i;
    }
  }
  g_strfreev(enums);
  if (val == -1) return(-1);
  g_value_set_int(&temp, i);
  res = (v->set_fn(v, &temp));
  g_value_unset(&temp);
  return res;
}

CALLER_OWN char *owl_variable_enum_get_tostring(const owl_variable *v, const GValue *val)
{
  char **enums;
  int nenums, i;
  char *tostring;

  if (val == NULL) {
    return g_strdup("<null>");
  }
  enums = g_strsplit_set(v->validsettings, ",", 0);
  nenums = g_strv_length(enums);
  i = *(const int*)val;
  if (i<0 || i>=nenums) {
    g_strfreev(enums);
    return g_strdup_printf("<invalid:%d>", i);
  }
  tostring = g_strdup(enums[i]);
  g_strfreev(enums);
  return tostring;
}

/* default functions for stringeans */

int owl_variable_string_validate_default(const struct _owl_variable *v, const GValue *newval) {
  if (newval == NULL) return(0);
  else return (1);
}

int owl_variable_string_set_fromstring_default(owl_variable *v, const char *newval) {
  GValue temp = {0};
  int res;
  g_value_init(&temp, G_TYPE_STRING);
  g_value_set_string(&temp, newval);
  res = (v->set_fn(v, &temp));
  g_value_unset(&temp);
  return res;
}

CALLER_OWN char *owl_variable_string_get_tostring_default(const owl_variable *v, const GValue *val)
{
  if (val == NULL) {
    return g_strdup("<null>");
  } else {
    return g_value_dup_string(val);
  }
}

