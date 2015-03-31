/*
 * Copyright 1990, 1991, 1992, Brown University, Providence, RI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef lint
static char Version[] =
   "$Id: td_editor.c,v 1.3 1993/06/03 14:16:32 rch Exp $";
#endif /* lint */

#include "td/defs.h"
#include <fcntl.h>
#include <errno.h>
#include "parse/parse.h"
#include "parse/scan.h"

#ifndef MOTIF_WIDGETS 

fnw_type fnw_type_editor = {
   WIDGET_TYPE_EDITOR,  /* type */
   STD_ARG_EDITOR,	/* name arg */
   td_cant_create,	/* create */
   NULL,		/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};

#else 

/* forward referenced */

static METsym 	*td_editor_create();
static void	td_editor_destroy();
static void	GoAway();
static void     xtd_cancel_quit();

/* Fnord widget definition for editors */

fnw_type fnw_type_editor = {
   WIDGET_TYPE_EDITOR,  /* type */
   STD_ARG_EDITOR,	/* name arg */
   td_editor_create,	/* create */
   td_editor_destroy,	/* destroy */
   NULL,		/* secure */
   NULL,		/* step */
   NULL,		/* clear */
   NULL,		/* update */
   NULL,		/* post update */
   td_cant_request,	/* request */
};


/* Translations for read-only editors */

static char *dumb_translations =
   "<Key>space:		go-away()	\n\
    <Key>Return:	go-away()	\n\
    <Key>q:		go-away()	\n\
    <Key>Q:		go-away()";


/* Accelerators for the various buttons in the widget */

static char *run_accel_trans =
   "#override \n\
    Ctrl<Key>c, Ctrl<Key>c:		ArmAndActivate()";

static char *load_accel_trans =
   "#override \n\
    Ctrl<Key>x, Ctrl<Key>f:		ArmAndActivate()";

static char *save_accel_trans =
   "#override \n\
    Ctrl<Key>x, Ctrl<Key>s:		ArmAndActivate()";

static char *quit_accel_trans =
   "#override \n\
    Ctrl<Key>x, Ctrl<Key>c:		ArmAndActivate()";

static XtActionsRec extra_text_actions[] = {
{"go-away", GoAway},
};

static XtTranslations load_accel = None;
static XtTranslations save_accel = None;
static XtTranslations quit_accel = None;
static XtTranslations run_accel = None;
static XtTranslations dumb_trans = None;


/*
 * this is the routine used to get the next characters from the
 * input text when the text in the editor window is being processed
 * but there is no other source of input in the program.  In that
 * case this procedure is set to be "scan_read", using the scan
 * supply read function and yyparse is called.
 */

/* ARGSUSED */
int td_text_read(fd, buf, count)
   int fd;
   char *buf;
   int count;
{
   int length = strlen(widget__top->info.top.text);
   ME(td_text_read);

   if (length == 0)
      return 0;

   else if (length < count)
   {
      (void) strncpy(buf, widget__top->info.top.text, length);
      *widget__top->info.top.text = 0;
      return length;
   }
   else
   {
      (void) strncpy(buf, widget__top->info.top.text,  count);
      bcopy(widget__top->info.top.text + count, 
	    widget__top->info.top.text, length - count + 1);
      return count;
   }
}

/*
 *	Adds the text of the widget to the input buffer stored by
 *	widget__top, if there is no input waiting on the file
 *	descriptor that is associated with the widget.  That text
 *	will be read in and parsed later if the file descriptor 
 *	associated with widget__top exists.  If there is none,
 *	after setting the input, we have to call yyparse, with
 *	a bogus scan file.
 */

static void execute_text(fnorse)
   char *fnorse;

{
   static Sym name = (Sym) NULL;
   char *temp;

   if ((Sym) NULL == name)
      name = sym_from_string ("text_widget");

   temp = widget__top->info.top.text;
   widget__top->info.top.text = fnorse;
   (void) fnord_parse (name, -1, td_text_read,
		       PARSE_NO_FLAGS, PARSE_NO_FLAGS);
   widget__top->info.top.text = temp;
}

/*
 * the activate callback for the run button, this function gets the
 * text out of the text widget and executes it, using the execute-text
 * procedure.
 */

/* ARGSUSED */
static void xtd_run_fnorse(widget, our_data, motif_data)
   Widget widget;
   caddr_t our_data;
   caddr_t motif_data;
{
   fnord_widget *editor = CAST(fnord_widget *, our_data);
   char *temp, *edit_text;
   char name[120];
   ME(xtd_run_fnorse);

   edit_text = XmTextGetString(editor->xw.w);
   parse_clear();
   temp = widget__top->info.top.text;
   widget__top->info.top.text = edit_text;
   (void) sprintf (name, "text widget (%s)", editor->name);
   (void) fnord_parse (sym_from_string(name), -1, td_text_read,
		       PARSE_NO_FLAGS, PARSE_NO_FLAGS);
   widget__top->info.top.text = temp;
   XtFree(edit_text);
}

/*
 * saves the text in the editor widget to the file whose name
 * is stored in the editor.
 */

static void xtd_save_text(fw)
   fnord_widget *fw;

{
   char *text;
   FILE *fp;
   ME(xtd_save_text);

   text = XmTextGetString(fw->xw.w);
   if ((fp = fopen(fw->info.editor.filename, "w")) == NULL)
   {
      if (fw->info.editor.filename != NULL)
      {
	 XtFree(fw->info.editor.filename);
	 fw->info.editor.filename = NULL;
      }
      perror("Unable to save file");
      return;
   }
   (void) fprintf( fp, "%s", text);
   fclose(fp);
   XtFree(text);
}

/*
 * this callback gets the filename from the prompt and 
 * sets the editor's filename to it.  Then it calls 
 * the save_text function.
 */

/* ARGSUSED */
static void xtd_do_save(widget, our_data, motif_data)
   Widget widget;
   caddr_t our_data;
   caddr_t motif_data;
{
   fnord_widget *editor = CAST(fnord_widget *, our_data);
   Arg arg;
   XmString temp;
   ME(xtd_do_save);

   if (editor->info.editor.filename != NULL)
      XtFree(editor->info.editor.filename);
   XtSetArg(arg, XmNtextString, &temp);
   XtGetValues(widget, &arg, 1);
   if (!XmStringGetLtoR(temp, XmSTRING_DEFAULT_CHARSET,
			&editor->info.editor.filename))
   {
      fprintf(stderr, "Motif problem");
   }
   else
   {
      xtd_save_text(editor);
   }

   XtUnmanageChild(widget);
   XmStringFree(temp);
   return;

}
 
/*
 * this function actually loads a file into a widget.  If the
 * text_dest field of the editor is EDIT then the text is
 * put into the editor widget; otherwise it is put into the
 * dest_text widget.  It returns true if it can load from
 * the file.
 */

static int xtd_load_file(editor, filename)
   fnord_widget *editor;
   char *filename;
{
   char *text;
   int length;
   FILE *fnorse;
   ME(xtd_load_file);


   fnorse = fopen(filename, "r");
   if (fnorse == NULL)
   {
      perror("Unable to open file");
      return FAILURE;
   }
   fseek(fnorse, (long) 0, 2); /* Go to end of file */
   length = ftell(fnorse); /* Where are we? */
   fseek(fnorse, (long) 0, 0); /* Go back to start */
   ALLOCNM(text, char, length + 1);
   length = read(fileno(fnorse), text, length);
   if (length < 0)
   {
      perror("Read error");
      FREE( text, (length + 1) * sizeof(char));
      return FAILURE;
   }
   fclose(fnorse);
   text[length] = 0;
   editor->info.editor.filename = filename;
   XmTextSetString(editor->xw.w, text);
   FREE(text, (length+1) * sizeof(char));
   return SUCCESS;
}

/*
 * this is the callback to load a file called from the file selection
 * widget.  It receives the editor widget as our data.  It sets the
 * filename to the string passed, if the open is successful, and copies 
 * the entire file into the value of the text widget.
 */

/* ARGSUSED */
static void xtd_do_load(widget, our_data, their_data)
   Widget widget;
   caddr_t our_data;
   caddr_t their_data;
{
   fnord_widget *editor = CAST(fnord_widget *, our_data);
   XmFileSelectionBoxCallbackStruct *fsbcs =
      CAST(XmFileSelectionBoxCallbackStruct *, their_data);
   char *filename;
   ME(xtd_do_load);

   if (!XmStringGetLtoR( fsbcs->value, XmSTRING_DEFAULT_CHARSET, &filename))
   {
      fprintf(stderr, "Motif error!");
      return;
   }
   
   if (!xtd_load_file(editor, filename))
   {
      XtFree(filename);
      return;
   }

   XtUnmanageChild(editor->info.editor.dialog);

}


/*
 * this callback runs the quit button in the editor widget.
 * It does just what you'd think it would.
 */

/* ARGSUSED */
static void xtd_quit_button(widget, our_data, motif_data)
   Widget widget;
   caddr_t our_data;
   caddr_t motif_data;
{
   extern void clean_up();
   ME(xtd_quit_button);
   
   clean_up();
   (void) exit(0);
}

/*
 * this callback runs the save button in the editor widget.  It
 * creates/pops up a prompt widget to get the filename if none
 * is associated with the editor widget
 */

/* ARGSUSED */
static void xtd_save_button(widget, our_data, motif_data)
   Widget widget;
   caddr_t our_data;
   caddr_t motif_data;
{
   fnord_widget *editor = CAST(fnord_widget *, our_data);
   Arg arg[3];
   static XmString sa;
   static XmString na;
   ME(xtd_save_button);

   if (editor->info.editor.filename != NULL)
      xtd_save_text(editor);
   else if (editor->info.editor.prompt != None)
   {
      XtSetArg(arg[0], XmNselectionLabelString, sa);
      XtSetArg(arg[1], XmNselectionLabelString, na);
      XtSetValues(editor->info.editor.prompt, arg, 2);
      XtManageChild(editor->info.editor.prompt);
   }
   else
   {
      XtSetArg(arg[0], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
      sa = XmStringCreateLtoR("Save as:", XmSTRING_DEFAULT_CHARSET);
      na = XmStringCreateLtoR("Untitled.fnorse", XmSTRING_DEFAULT_CHARSET);
      XtSetArg(arg[1], XmNselectionLabelString, sa);
      XtSetArg(arg[2], XmNselectionLabelString, na);
      editor->info.editor.prompt = 
	 XmCreatePromptDialog(editor->xw.top, "save-as", arg, 3);
      XtOverrideTranslations(
	XmSelectionBoxGetChild(
	     editor->info.editor.prompt, XmDIALOG_TEXT ), td_text_translations);
      XtAddCallback(editor->info.editor.prompt, XmNokCallback, 
		    xtd_do_save, (caddr_t) editor);
      XtAddCallback(editor->info.editor.prompt, XmNcancelCallback,
		    xtd_cancel_quit, (caddr_t) editor);
      XtManageChild(editor->info.editor.prompt);
   }
}

/*
 * this callback runs the load button in the editor widget.  It 
 * creates (?) a file selection box dialog and has it go.
 */

/* ARGSUSED */
static void xtd_load_button(widget, our_data, motif_data)
   Widget widget;
   caddr_t our_data;
   caddr_t motif_data;
{
   fnord_widget *editor = CAST(fnord_widget *, our_data);
   Arg arg[3];
   ME(xtd_load_button);

   if (editor->info.editor.dialog != None)
      XtManageChild(editor->info.editor.dialog);
   else
   {
      XtSetArg(arg[0], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
      XtSetArg(arg[1], XmNpattern, XmStringCreateSimple("*.fnorse"));
      editor->info.editor.dialog =
	 XmCreateFileSelectionDialog(editor->xw.top,
				     "fnord file select",
				     arg, 2);
      XtAddCallback(editor->info.editor.dialog, XmNokCallback,
		    xtd_do_load, (char *) editor);
      XtAddCallback(editor->info.editor.dialog, XmNcancelCallback,
		    xtd_cancel_quit, (char *) editor);
      XtOverrideTranslations(
	XmFileSelectionBoxGetChild(
	     editor->info.editor.dialog, XmDIALOG_TEXT ), td_text_translations);
      XtOverrideTranslations(
	XmFileSelectionBoxGetChild(
	     editor->info.editor.dialog, XmDIALOG_FILTER_TEXT ),
			     td_text_translations);
      XtManageChild(editor->info.editor.dialog);
   }
}

/*
 * this callback removes the file selection dialog box when
 * the cancel button is hit.
 */

/* ARGSUSED */
static void xtd_cancel_quit(widget, our_data, their_data)
   Widget widget;
   caddr_t our_data;
   caddr_t their_data;
{
   fnord_widget *editor = CAST(fnord_widget *, our_data);
   ME(xtd_cancel_quit);

   if (XtIsManaged(editor->info.editor.dialog))
      XtUnmanageChild(editor->info.editor.dialog);
   else
      XtUnmanageChild(editor->info.editor.prompt);
}

/*
 * Causes the widget to disappear.
 */

/* ARGSUSED */
static void GoAway(w, event, parms, count)
   Widget w;
   XEvent *event;
   String *parms;
   Cardinal count;
{
   ME(GoAway);

   XtUnmapWidget(XtParent(XtParent(w)));
}

/*
 * builds the children of the form widget in the editor.
 * This is a separate procedure to please the compiler which
 * is not happy.
 */

static void xtd_text_form_kids(editor)
   fnord_widget *editor;
{
   Arg arg[10];
   int count = 0;
   ME(xtd_text_form_kids);

   count = 0;
   XtSetArg(arg[count], XmNtopAttachment, XmATTACH_FORM);count++;
   XtSetArg(arg[count], XmNtopOffset, 15); count++;
   XtSetArg(arg[count], XmNleftAttachment, XmATTACH_POSITION); count++;
   XtSetArg(arg[count], XmNrightAttachment, XmATTACH_POSITION); count++;
   XtSetArg(arg[count], XmNlabelString,
	    XmStringCreateLtoR("Load", XmSTRING_DEFAULT_CHARSET));
   count++;
   XtSetArg(arg[count], XmNleftPosition, 1); count++;
   XtSetArg(arg[count], XmNrightPosition, 5); count++;
   XtSetArg(arg[count], XmNaccelerators, load_accel); count++;
   editor->info.editor.load = XmCreatePushButton(editor->info.editor.form,
						 editor->name, arg, count);
   XtManageChild(editor->info.editor.load);
   XtAddCallback(editor->info.editor.load, XmNactivateCallback,
		 xtd_load_button, (char *) editor);
   
   count = 4;
   XtSetArg(arg[count], XmNlabelString, 
	    XmStringCreateLtoR("Save", XmSTRING_DEFAULT_CHARSET));
   count++;
   XtSetArg(arg[count], XmNleftPosition, 6); count++;
   XtSetArg(arg[count], XmNrightPosition, 10); count++;
   XtSetArg(arg[count], XmNaccelerators, save_accel); count++;
   editor->info.editor.save = XmCreatePushButton(editor->info.editor.form,
						 editor->name, arg, count);
   XtManageChild(editor->info.editor.save);
   XtAddCallback(editor->info.editor.save, XmNactivateCallback,
		 xtd_save_button, (char *) editor);
   
   count = 4;
   XtSetArg(arg[count], XmNlabelString,
	    XmStringCreateLtoR("Run", XmSTRING_DEFAULT_CHARSET));
   count++;
   XtSetArg(arg[count], XmNleftPosition, 11); count++;
   XtSetArg(arg[count], XmNrightPosition, 15); count++;
   XtSetArg(arg[count], XmNaccelerators,  run_accel); count++;
   editor->info.editor.exec = XmCreatePushButton(editor->info.editor.form,
						 editor->name, arg, count);
   XtManageChild(editor->info.editor.exec);
   XtAddCallback(editor->info.editor.exec, XmNactivateCallback,
		 xtd_run_fnorse, (char *) editor);
   
   count = 4;
   XtSetArg(arg[count], XmNlabelString,
	    XmStringCreateLtoR("Quit", XmSTRING_DEFAULT_CHARSET));
   count++;
   XtSetArg(arg[count], XmNleftPosition, 26); count++;
   XtSetArg(arg[count], XmNrightPosition, 30); count++;
   XtSetArg(arg[count], XmNaccelerators,  quit_accel); count++;
   editor->info.editor.quit = XmCreatePushButton(editor->info.editor.form,
						 editor->name, arg, count);
   XtManageChild(editor->info.editor.quit);
   XtAddCallback(editor->info.editor.quit, XmNactivateCallback,
		 xtd_quit_button, (char *) editor);

   count = 0;
   XtSetArg(arg[count], XmNtopAttachment, XmATTACH_WIDGET); count++;
   XtSetArg(arg[count], XmNtopWidget, editor->info.editor.load); count++;
   XtSetArg(arg[count], XmNtopOffset, 15); count++;
   XtSetArg(arg[count], XmNleftAttachment, XmATTACH_POSITION); count++;
   XtSetArg(arg[count], XmNleftPosition,1);count++;
   XtSetArg(arg[count], XmNrightAttachment, XmATTACH_POSITION); count++;
   XtSetArg(arg[count], XmNrightPosition, 30); count++;
   XtSetArg(arg[count], XmNbottomAttachment, XmATTACH_FORM); count++;
   XtSetArg(arg[count], XmNbottomOffset, 15); count++;
   XtSetArg(arg[count], XmNeditMode, XmMULTI_LINE_EDIT); count++;
   editor->xw.w = XmCreateScrolledText(editor->info.editor.form,
				       editor->name, &arg[count-1], 1);
   XtSetValues(XtParent(editor->xw.w), arg, count-1);
   XtOverrideTranslations(editor->xw.w, td_text_translations);
   XtInstallAccelerators(editor->xw.w, editor->info.editor.load);
   XtInstallAccelerators(editor->xw.w, editor->info.editor.save);
   XtInstallAccelerators(editor->xw.w, editor->info.editor.exec);
   XtInstallAccelerators(editor->xw.w, editor->info.editor.quit);
   XtManageChild(editor->xw.w);
}

static void
xtd__text_init()
{
   ME(xtd__text_init);

   if (dumb_trans == None)
   {
      XtAppAddActions(widget__top->info.top.app,
		      extra_text_actions, XtNumber(extra_text_actions));
      load_accel = XtParseAcceleratorTable(load_accel_trans);
      save_accel = XtParseAcceleratorTable(save_accel_trans);
      quit_accel = XtParseAcceleratorTable(quit_accel_trans);
      run_accel  = XtParseAcceleratorTable(run_accel_trans);
      dumb_trans = XtParseTranslationTable(dumb_translations);
   }
}

/*
 * builds an editor widget.  This is one of the grosser widget
 * creation functions for fnord, but there is little that can be
 * done about it.  The text editor is a complex object, with 
 * lots of our own special touches.  Oh, well.
 */
/* ARGSUSED */
static METsym *td_editor_create(parent, name, alist, sym)
   fnord_widget *parent;
   char *name;
   METset *alist;
   METsym *sym;
{
   Arg arg[4];
   int count = 0;
   fnord_widget *editor;
   widget_list *list;
   ME(td_editor_create);

   xtd__text_init();

   ALLOCM(editor, fnord_widget);
   editor->type = WIDGET_TYPE_EDITOR;
   strcpy(editor->name, name);
   editor->kids = NULL;
   editor->dft_kid_type = WIDGET_TYPE_NONE;

   /* Never destroy an editor if you don't know what you're doing */
   editor->state = FNW_STATE_START | FNW_STATE_FIXED;

   editor->xw.w = editor->xw.top = None;
   editor->info.editor.dialog = editor->info.editor.prompt = None;
   editor->info.editor.disp = editor->info.editor.disp_edit = None;
   editor->info.editor.filename = NULL;

   td_link_in(&parent, name, WIDGET_TYPE_EDITOR, sym);

   if (parent == NULL)
   {
      FREE(editor, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }

   XtSetArg(arg[count], XmNwidth, 400);count++;
   XtSetArg(arg[count], XmNheight, 400);count++;
   editor->xw.top = XtAppCreateShell("fnord", "Fnord",
				     applicationShellWidgetClass,
				     XtDisplay(parent->xw.w), arg, 2);

   XtSetArg(arg[count], XmNfractionBase, 31); count++;
   XtSetArg(arg[count], XmNverticalSpacing, 25); count++;
   editor->info.editor.form = 
      XmCreateForm(editor->xw.top, editor->name, arg, count);
   XtManageChild(editor->info.editor.form);

   xtd_text_form_kids(editor);

   ALLOCM(list, widget_list);
   list->w = editor;
   list->next = parent->kids;
   parent->kids = list;
   XtRealizeWidget(editor->xw.top);
   return METsym_new_control((char *) editor, sym->origin);

}

static void td_editor_destroy(editor)
   fnord_widget		*editor;
{
   ME(td_editor_destroy);
   
   XtDestroyWidget(editor->xw.w);
   XtDestroyWidget(editor->xw.top);
   XtFree(editor->info.editor.filename);
}

#endif /* MOTIF_WIDGETS */

