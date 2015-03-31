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

#ifdef X_TWO_D

#include "td/defs.h"
#include <fcntl.h>
#include <errno.h>
#include "parse/parse.h"
#include "parse/scan.h"

#ifdef MOTIF_WIDGETS
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <Xm/SelectioB.h>
#include <Xm/DialogS.h>
#include <Xm/ToggleBG.h>
#include <Xm/RowColumn.h>

extern void clean_up();

#ifndef lint
static char Version[] =
   "$Id: td_text.c,v 1.16 1992/06/22 17:10:43 mas Exp $";
#endif /* lint */

static void EndOfLine();
static void BeginningOfLine();
static void KillLine();
static void GoAway();
static void xtd_run_fnorse();
static void xtd_do_load();

static void xtd_load_button();
static void xtd_save_button();
static void xtd_quit_button();
static void xtd_view_button();
static void xtd_explain_button();

static void xtd_cancel_quit();

extern fnord_widget *widget__top;

static char *new_translations =
   "Meta<Key>Right:	backward-word() 	\n\
    Meta<Key>Left:	forward-word()		\n\
    Meta<Key>b:		backward-word()		\n\
    Meta<Key>f:		forward-word()		\n\
    Ctrl<Key>b:		backward-character()	\n\
    Ctrl<Key>f:		forward-character()	\n\
    Ctrl<Key>p:		previous-line()		\n\
    Ctrl<Key>n:		next-line()		\n\
    Ctrl<Key>d:		delete-next-character()	\n\
    Ctrl<Key>e:		my-end-of-line(c)	\n\
    Ctrl<Key>a:		my-beginning-of-line(c)	\n\
    Ctrl<Key>k:		kill-rest-of-line(c)	\n\
    Ctrl<Key>y:		copy-to()		\n\
    Meta<Key>e:		my-end-of-line(m)	\n\
    Meta<Key>a:		my-beginning-of-line(m)	\n\
    Meta<Key>k:		kill-rest-of-line(m)	\n\
    Meta<Key>d:		delete-next-word()	\n\
    Meta<Key>BackSpace:	delete-previous-word()	\n\
    Meta<Key>Delete:	delete-previous-word()	\n\
    <Key>Delete:	delete-previous-character()";

static char *dumb_translations =
   "<Key>space:		go-away()	\n\
    <Key>Return:	go-away()	\n\
    <Key>q:		go-away()	\n\
    <Key>Q:		go-away()";

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
{"my-end-of-line", EndOfLine },
{"my-beginning-of-line", BeginningOfLine },
{"kill-rest-of-line", KillLine },
{"go-away", GoAway},
};

static XtTranslations customizer = None;
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
static int edit_text_read(fd, buf, count)
   int fd;
   char *buf;
   int count;
{
   int length = strlen(widget__top->info.top.text);
   ME(edit_text_read);

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
   (void) fnord_parse (name, -1, edit_text_read,
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

   edit_text = XmTextGetString(editor->w);
   parse_clear();
   temp = widget__top->info.top.text;
   widget__top->info.top.text = edit_text;
   (void) sprintf (name, "text widget (%s)", editor->name);
   (void) fnord_parse (sym_from_string(name), -1, edit_text_read,
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

   text = XmTextGetString(fw->w);
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
   XmTextSetString(editor->w, text);
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
	 XmCreatePromptDialog(editor->top, "save-as", arg, 3);
      XtOverrideTranslations(
	XmSelectionBoxGetChild(
	     editor->info.editor.prompt, XmDIALOG_TEXT ), customizer);
      XtAddCallback(editor->info.editor.prompt, XmNokCallback, 
		    xtd_do_save, editor);
      XtAddCallback(editor->info.editor.prompt, XmNcancelCallback,
		    xtd_cancel_quit, editor);
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
	 XmCreateFileSelectionDialog(editor->top,
				     "fnord file select",
				     arg, 2);
      XtAddCallback(editor->info.editor.dialog, XmNokCallback,
		    xtd_do_load, editor);
      XtAddCallback(editor->info.editor.dialog, XmNcancelCallback,
		    xtd_cancel_quit, editor);
      XtOverrideTranslations(
	XmFileSelectionBoxGetChild(
	     editor->info.editor.dialog, XmDIALOG_TEXT ), customizer);
      XtOverrideTranslations(
	XmFileSelectionBoxGetChild(
	     editor->info.editor.dialog, XmDIALOG_FILTER_TEXT ),
			     customizer);
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
 * moves the cursor in the text widget to the end of the current 
 * line or fnorse statement.
 */

/* ARGSUSED */
static void EndOfLine(w, event, parms, count)
   Widget w;
   XEvent *event;
   String *parms;
   Cardinal count;
{
   XmTextPosition pos;
   char *text;
   char comp_char;
   Arg arg[2];
   ME(EndOfLine);

   comp_char = (count == 0) ? '\n' : (parms[0][0] == 'c') ? '\n' : ';' ;
   XtSetArg(arg[0], XmNcursorPosition, &pos);
   XtGetValues(w, arg, 1);
   text = XmTextGetString(w);

   while((text[pos] != 0) && ( text[pos] != comp_char ))
      pos++;

   if (comp_char != '\n')
      pos++;

   XtSetArg(arg[0], XmNcursorPosition, pos);
   XtSetValues(w, arg, 1);
   XtFree(text);

}

/*
 * moves the cursor in the text widget to the beginning of the current
 * line or fnorse statement.
 */

/* ARGSUSED */
static void BeginningOfLine(w, event, parms, count)
   Widget w;
   XEvent *event;
   String *parms;
   Cardinal count;
{
   XmTextPosition pos;
   char *text;
   char comp_char;
   Arg arg[2];
   ME(BeginningOfLine);

   comp_char = (count == 0) ? '\n' : (parms[0][0] == 'c') ? '\n' : ';' ;
   XtSetArg(arg[0], XmNcursorPosition, &pos);
   XtGetValues(w, arg, 1);
   text = XmTextGetString(w);

   while((pos > -1) && ( text[pos] != comp_char ))
      pos--;
   
   pos++;
   while((text[pos] == '\n')  || (text[pos] == ' '))
      pos++;

   XtSetArg(arg[0], XmNcursorPosition, pos);
   XtSetValues(w, arg, 1);
   XtFree(text);

}

/*
 * deletes the rest of the current line or fnorse statement.
 */

/* ARGSUSED */
static void KillLine(w, event, parms, count)
   Widget w;
   XEvent *event;
   String *parms;
   Cardinal count;
{
   XmTextPosition pos;
   XmTextPosition end;
   char *text;
   char comp_char;
   Arg arg[2];
   ME(KillLine);

   comp_char = (count == 0) ? '\n' : (parms[0][0] == 'c') ? '\n' : ';' ;

   XtSetArg(arg[0], XmNcursorPosition, &pos);
   XtGetValues(w, arg, 1);
   text = XmTextGetString(w);

   end = pos;
   if (text[pos] == comp_char)
      end++;
   else
      while((text[end] != 0) && ( text[end] != comp_char ))
	 end++;

   XmTextSetSelection(w, pos, end, event->xkey.time);
   bcopy(text + end, text + pos, strlen(text + end) + 1);
   XmTextSetString(w, text);
   XtFree(text);

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
		 xtd_load_button, editor);
   
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
		 xtd_save_button, editor);
   
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
		 xtd_run_fnorse, editor);
   
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
		 xtd_quit_button, editor);

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
   editor->w = XmCreateScrolledText(editor->info.editor.form,
				    editor->name, &arg[count-1], 1);
   XtSetValues(XtParent(editor->w), arg, count-1);
   XtOverrideTranslations(editor->w, customizer);
   XtInstallAccelerators(editor->w, editor->info.editor.load);
   XtInstallAccelerators(editor->w, editor->info.editor.save);
   XtInstallAccelerators(editor->w, editor->info.editor.exec);
   XtInstallAccelerators(editor->w, editor->info.editor.quit);
   XtManageChild(editor->w);
}

static void
xtd__text_init()
{
   ME(xtd__text_init);

   if (customizer == None)
   {
      XtAppAddActions(widget__top->info.top.app,
		      extra_text_actions, XtNumber(extra_text_actions));
      customizer = XtParseTranslationTable(new_translations);
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
METsym *xtd_new_editor(parent, name, alist)
   fnord_widget *parent;
   char *name;
   METset *alist;
{
   Arg arg[4];
   int count = 0;
   fnord_widget *editor;
   widget_list *list;
   ME(xtd_new_editor);

   xtd__text_init();

   ALLOCM(editor, fnord_widget);
   editor->type = WIDGET_TYPE_EDITOR;
   strcpy(editor->name, name);
   editor->w = editor->top = None;
   editor->kids = NULL;
   editor->dft_kid_type = WIDGET_TYPE_NONE;
   editor->info.editor.dialog = editor->info.editor.prompt = None;
   editor->info.editor.disp = editor->info.editor.disp_edit = None;
   editor->info.editor.filename = NULL;

   editor->fixed = YES;		/* Never destroy an editor if you
				   don't know what you're doing */

   xtd_link_in(&parent, name, WIDGET_TYPE_EDITOR);

   if (parent == NULL)
   {
      FREE(editor, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }

   XtSetArg(arg[count], XmNwidth, 400);count++;
   XtSetArg(arg[count], XmNheight, 400);count++;
   editor->top = XtAppCreateShell(NULL, "Fnord",
				  applicationShellWidgetClass,
				  XtDisplay(parent->w), arg, 2);

   XtSetArg(arg[count], XmNfractionBase, 31); count++;
   XtSetArg(arg[count], XmNverticalSpacing, 25); count++;
   editor->info.editor.form = 
      XmCreateForm(editor->top, editor->name, arg, count);
   XtManageChild(editor->info.editor.form);

   xtd_text_form_kids(editor);

   ALLOCM(list, widget_list);
   list->w = editor;
   list->next = parent->kids;
   parent->kids = list;
   XtRealizeWidget(editor->top);
   return METsym_new_control((char *) editor, (FILOCloc *) NULL);

}



METsym *
xtd_new_printer(parent, name, alist)
   fnord_widget *parent;
   char *name;
   METset *alist;
{
   fnord_widget *printer;
   widget_list *list;
   Arg		arg[4];
   int		i;
   ME(xtd_new_printer);

   xtd__text_init();

   ALLOCM(printer, fnord_widget);
   printer->type = WIDGET_TYPE_PRINTER;
   (void) strcpy(printer->name, name);
   printer->w = printer->top = None;
   printer->kids = NULL;
   printer->dft_kid_type = WIDGET_TYPE_NONE;
   printer->fixed = NO;
   printer->info.printer.set = MET_NULL_SET;
   printer->info.printer.append = NO;

   xtd_link_in(&parent, name, WIDGET_TYPE_PRINTER);

   if (parent == NULL)
   {
      FREE(printer, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }

   printer->top = XmCreateFrame(parent->w, printer->name, NULL, 0);

   i = 0;
   XtSetArg(arg[i], XmNeditMode, XmMULTI_LINE_EDIT); i++;
   printer->w = XmCreateScrolledText(printer->top,
				     printer->name, arg, i);
   XtOverrideTranslations(printer->w, customizer);

   xtd_winkid_geom(parent, printer, alist);
   XtManageChild( printer->top );
   XtManageChild( printer->w );

   printer->info.printer.set = xtd_make_set_recalc(alist, STD_ARG_SET);
   (void) xtd_lookup_boolean(alist, std_args[STD_ARG_APPEND],
			     &printer->info.printer.append,
			     NO, (Sym) NULL, NO);

      

   ALLOCM(list, widget_list);
   list->w = printer;
   list->next = parent->kids;
   parent->kids = list;

   xtd_printer_update(printer);

   return METsym_new_control((char *) printer, (FILOCloc *) NULL);
}

/* ARGSUSED */
static void xtd_typein_changed( w, our_info, call_data )
     Widget w;			/* Typein bar changed.		*/
     caddr_t our_info;		/* Pointer to Typein structure	*/
     caddr_t call_data;		/* What generated the callback	*/

{
   fnord_widget *typein = CAST(fnord_widget *, our_info);
   extern METset *MET_set_cast_to_type();
   char *s, *temp;
   char name[120];
   int len;
   METset *result, *cast;
   FOboolean ok;
   ME(xtd_typein_changed);

   s = XmTextGetString(w);
   /* Eek */
   len = strlen(s);
   s = XtRealloc(s, len + 2);
   s[len++] = ';';
   s[len] = 0;
   temp = widget__top->info.top.text;
   widget__top->info.top.text = s;
   sprintf(name, "typein widget (%s)", typein->name);
   (void) fnord_parse (sym_from_string(name), -1, edit_text_read, 
		       PARSE_NO_FLAGS, PARSE_ALL_FLAGS);
   widget__top->info.top.text = temp;
   XtFree(s);
   result = parse_last_expr();
   if ((METset *) NULL == result)
      return;

   cast = xtd_make_set_cast (result, typein->info.typein.type, &ok);
   MET_SET_FREE (result);

   if (NO == ok)
   {
      (void) fprintf (stderr, "Contents of widget are of wrong type.\n");
      MET_SET_FREE (cast);
      return;
   }

   MET_SET_FREE_NULL (typein->info.typein.val);
   typein->info.typein.val = cast;
   typein->changed = YES;
   td_update(NO);
}

void xtd_create_typein_widget(parent, typein, init)
   Widget parent;
   fnord_widget *typein;
   char *init;

{
   Arg arg[10];
   int i = 0;
   ME(xtd_new_typein_widget);

   xtd__text_init();
   typein->top = XmCreateFrame(parent, typein->name, NULL, 0);

   i = 0;
   XtSetArg(arg[i], XmNfractionBase, 2); i++;
   typein->w = XmCreateForm(typein->top, typein->name, arg, i);

   i = 0;
   XtSetArg(arg[i], XmNeditMode, XmSINGLE_LINE_EDIT); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftPosition, 1); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;

   typein->info.typein.editor = XmCreateText(typein->w, typein->name,
					     arg, i);

   XmTextSetString(typein->info.typein.editor, init);
   XtOverrideTranslations(typein->info.typein.editor, customizer);

   XtAddCallback(typein->info.typein.editor, XmNactivateCallback,
		 xtd_typein_changed, typein);


   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNrightPosition, 1); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate(typein->name, XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   typein->info.typein.label = XmCreateLabel(typein->w, typein->name,
					     arg, i);
}

void
xtd_printer_clear(fw)
   fnord_widget	*fw;
{
   ME(xtd_printer_clear);

   if (MET_NULL_SET != fw->info.printer.set)
      METset_clear_rep(fw->info.printer.set);
}

void
xtd_printer_update(fw)
   fnord_widget	*fw;
{
   char		*text, *new;
   int		len;
   Boolean	prepend_name = NO;
   ME(xtd_printer_update);

   if (MET_NULL_SET != fw->info.printer.set) {

      if (YES == fw->info.printer.append) {
	 text = XmTextGetString(fw->w);
	 if (0 == strlen(text))
	    prepend_name = YES;
      } else {
	 text = "";
	 prepend_name = YES;
      }
      len = strlen(text);
      ALLOCNM(new, char, len + 4000);
      if (YES == prepend_name) {
	 (void) strcpy(new, fw->name);
	 (void) strcat(new, "\n");
      } else
	 new[0] = '\0';

      (void) strcat(new, text);

      if (!prepend_name && fw->info.printer.append)
	 (void) strcat(new, "\n");
      
      time_begin();
      if (0 == METset_have_rep(fw->info.printer.set, NULL)) {
	 time_end(FNULL, "set");
	 (void) strcat(new, "error");
      } else {
	 time_end(FNULL, "set");
	 METset_print_pretty(FNULL, fw->info.printer.set, new, 0);
      }
      XmTextSetString(fw->w, new);
      FREEN(new, char, len + 4000);
      if (YES == fw->info.printer.append)
	 XtFree(text);
   } else {
      XmTextSetString(fw->w, "no set specified");
   }
}

/* ARGSUSED */
static void xtd_output_set_style (widget, our_info, call_info)
   Widget widget;
   caddr_t our_info;
   caddr_t call_info;
{
   fnord_widget *output = CAST(fnord_widget *, our_info);
   XmRowColumnCallbackStruct *rccs =
      CAST(XmRowColumnCallbackStruct *, call_info);
   XmToggleButtonCallbackStruct *tbcs =
      CAST(XmToggleButtonCallbackStruct *, rccs->callbackstruct);

   if (tbcs->set == False)
      return;

   if (rccs->widget == output->info.output.tb1)
      output->info.output.draw_style = ENV_BW_POSTSCRIPT_MODE;
   else if (rccs->widget == output->info.output.tb2)
      output->info.output.draw_style = ENV_GRAY_POSTSCRIPT_MODE;
   else if (rccs->widget == output->info.output.tb3)
      output->info.output.draw_style = ENV_COLOR_POSTSCRIPT_MODE;
}

/* ARGSUSED */
static void xtd_output_print(widget, our_info, call_info)
   Widget widget;
   caddr_t our_info;
   caddr_t call_info;
{
   fnord_widget *output = CAST(fnord_widget *, our_info);
   fnord_widget *view;
   char *text, name[80];
   int i, j;
   Sym sym;
   METset *set;
   METsym *view_def;

   text = XmTextGetString(output->info.output.file_editor);
   EnvSetOutputFile (widget__top->info.top.env, text);
   XtFree(text);
   
   text = XmTextGetString (output->info.output.view_editor);
   
   i = 0;

   do {
      while (text[i] == ',' || text[i] == ' ' || text[i] == '\t')
	 i++;
      
      if (text[i] == 0)
	 break;

      j = 0;
      while ((text[i] != ' ' && text[i] != ',' && 
	      text[i] != '\t' && text[i] != 0))
	 name[j++] = text[i++];

      name[j] = 0;
      sym = sym_from_string(name);
      set = METenv_lookup(output->info.output.env, sym);
      if (set == MET_NULL_SET)
      {
	 fprintf(stderr, "Printing: unknown widget %s.\n", name);
	 continue;
      }
      
      view_def = set->definition;
      if (MET_SYM_TYPE_CONTROL != view_def->type)
      {
	 fprintf(stderr, "Printing: unknown widget %s.\n", name);
	 MET_SET_FREE(set);
	 continue;
      }
      
      view = CAST(fnord_widget *, view_def->info.control.data);
      MET_SET_FREE(set);
      if (WIDGET_TYPE_VIEW != view->type)
      {
	 fprintf(stderr, "Printing: can only output from view widgets.\n");
	 continue;
      }
      
      EnvDrawAdd (widget__top->info.top.env, 
		  ViewGetPict(view->w),
		  output->info.output.draw_style);

   } while (1);

   EnvFlush (widget__top->info.top.env);
   XtFree(text);
}

static void xtd_create_output_widget(parent, output)
   Widget parent;
   fnord_widget *output;

{
   Arg arg[10];
   char s[100];
   int i = 0;
   XtCallbackRec callbacks[2];
   ME(xtd_create_output_widget);
   
   output->top = XmCreateFrame(parent, output->name, NULL, 0);

   i = 0;
   XtSetArg(arg[i], XmNfractionBase, 20); i++;
   output->w = XmCreateForm (output->top, output->name, arg, i);

   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   
   XtSetArg(arg[i], XmNeditMode, XmSINGLE_LINE_EDIT); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;

   XtSetArg(arg[i], XmNtopPosition, 11); i++;
   XtSetArg(arg[i], XmNbottomPosition, 14); i++;
   XtSetArg(arg[i], XmNleftPosition, 7); i++;
   XtSetArg(arg[i], XmNrightPosition, 20); i++;

   output->info.output.view_editor = XmCreateText(output->w, output->name,
						  arg, i);
   s[0] = 0;
   XmTextSetString(output->info.output.view_editor, s);
   XtOverrideTranslations(output->info.output.view_editor, customizer);
   
   i = 6;
   XtSetArg(arg[i], XmNtopPosition, 14); i++;
   XtSetArg(arg[i], XmNbottomPosition, 17); i++;
   XtSetArg(arg[i], XmNleftPosition, 7); i++;
   XtSetArg(arg[i], XmNrightPosition, 20); i++;

   output->info.output.file_editor = XmCreateText(output->w, output->name,
						  arg, i);
   
   XmTextSetString(output->info.output.file_editor, "fnord.ps");
   XtOverrideTranslations(output->info.output.file_editor, customizer);

   i = 4;
   XtSetArg(arg[i], XmNtopPosition, 11); i++;
   XtSetArg(arg[i], XmNbottomPosition, 14); i++;
   XtSetArg(arg[i], XmNleftPosition, 0); i++;
   XtSetArg(arg[i], XmNrightPosition, 7); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("include views:", XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   output->info.output.view_label = XmCreateLabel(output->w, output->name,
						  arg, i);

   i = 4;
   XtSetArg(arg[i], XmNtopPosition, 14); i++;
   XtSetArg(arg[i], XmNbottomPosition, 17); i++;
   XtSetArg(arg[i], XmNleftPosition, 0); i++;
   XtSetArg(arg[i], XmNrightPosition, 7); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("to file:", XmSTRING_DEFAULT_CHARSET)); i++;
   XtSetArg(arg[i], XmNorientation, XmHORIZONTAL); i++;
   output->info.output.file_label = XmCreateLabel(output->w, output->name,
						  arg, i);

   i = 0;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopPosition, 17); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNleftPosition, 13); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate("Print", XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.button =
      XmCreatePushButton(output->w, "print", arg, i);
   XtAddCallback(output->info.output.button, XmNactivateCallback,
		 xtd_output_print, output);

   i = 0;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNbottomPosition, 3); i++;
   XtSetArg(arg[i], XmNlabelString,
	    XmStringCreate(output->name, XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.label = XmCreateLabel(output->w, output->name, 
					     arg, i);

   i = 0;
   callbacks[0].callback = xtd_output_set_style;
   callbacks[0].closure = (caddr_t) output;
   callbacks[1].callback = NULL;
   callbacks[1].closure = NULL;
   XtSetArg(arg[i], XmNentryCallback, callbacks); i++;
   XtSetArg(arg[i], XmNtopAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNtopPosition, 3); i++;
   XtSetArg(arg[i], XmNbottomAttachment, XmATTACH_POSITION); i++;
   XtSetArg(arg[i], XmNbottomPosition, 10); i++;
   XtSetArg(arg[i], XmNleftAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNrightAttachment, XmATTACH_FORM); i++;
   XtSetArg(arg[i], XmNentryClass, xmToggleButtonGadgetClass); i++;
   output->info.output.radio =
      XmCreateRadioBox (output->w, output->name, arg, i);

   i = 0;
   XtSetArg (arg[i], XmNset, True); i++;
   XtSetArg (arg[i], XmNlabelString, 
	     XmStringCreateLtoR("black and white postscript",
				XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.tb1 = 
      XmCreateToggleButtonGadget (output->info.output.radio,
				  output->name, arg, i);
   XtManageChild (output->info.output.tb1);

   i = 0;
   XtSetArg (arg[i], XmNset, False); i++;
   XtSetArg (arg[i], XmNlabelString, 
	     XmStringCreateLtoR("gray scale postscript",
				XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.tb2 =
      XmCreateToggleButtonGadget (output->info.output.radio,
				  output->name, arg, i);
   XtManageChild (output->info.output.tb2);

   i = 0;
   XtSetArg (arg[i], XmNset, False); i++;
   XtSetArg (arg[i], XmNlabelString, 
	     XmStringCreateLtoR("full color postscript",
				XmSTRING_DEFAULT_CHARSET)); i++;
   output->info.output.tb3 =
      XmCreateToggleButtonGadget (output->info.output.radio,
				  output->name, arg, i);
   XtManageChild (output->info.output.tb3);

}

METsym *xtd_new_output(parent, name, alist, env)
   fnord_widget *parent;
   char *name;
   METset *alist;
   METenv *env;
{
   fnord_widget	*output;
   widget_list  *list;
   ME(xtd_new_output);
   
   xtd__text_init();
   
   ALLOCM(output, fnord_widget);
   output->type = WIDGET_TYPE_OUTPUT;
   (void) strcpy (output->name, name);
   output->w = output->top = None;
   output->kids = NULL;
   output->dft_kid_type = WIDGET_TYPE_NONE;
   output->fixed = NULL;
   output->info.output.draw_style = ENV_BW_POSTSCRIPT_MODE;

   xtd_link_in(&parent, name, WIDGET_TYPE_OUTPUT);
   
   if (parent == NULL)
   {
      FREE(output, sizeof(fnord_widget));
      return MET_NULL_SYM;
   }

   MET_ENV_COPY(output->info.output.env, env);

   xtd_create_output_widget (parent->w, output);
   xtd_winkid_geom(parent, output, alist);

   XtManageChild (output->top);
   XtManageChild (output->w);
   XtManageChild (output->info.output.button);
   XtManageChild (output->info.output.label);
   XtManageChild (output->info.output.file_editor);
   XtManageChild (output->info.output.file_label);
   XtManageChild (output->info.output.view_editor);   
   XtManageChild (output->info.output.view_label);
   XtManageChild (output->info.output.radio);

   ALLOCM(list, widget_list);
   list->w = output;
   list->next = parent->kids;
   parent->kids = list;

   return METsym_new_control((char *) output, (FILOCloc *) NULL);
}

#else

METsym *xtd_new_editor(parent, name, alist)
   fnord_widget *parent;
   char *name;
   METset *alist;

{
   ME(xtd_new_editor);
   fprintf( stderr, "Editor widgets can only be created if");
   fprintf( stderr, " fnord is linked with OSF/Motif libraries.\n");
   return MET_NULL_SYM;
}

METsym *xtd_new_printer(parent, name, alist)
   fnord_widget *parent;
   char *name;
   METset *alist;

{
   ME(xtd_new_editor);
   fprintf( stderr, "Printer widgets can only be created if");
   fprintf( stderr, " fnord is linked with OSF/Motif libraries.\n");
   return MET_NULL_SYM;
}

METsym *xtd_new_output(parent, name, alist, env)
   fnord_widget *parent;
   char *name;
   METset *alist;
   METenv *env;

{
   ME(xtd_new_editor);
   fprintf( stderr, "Ouput control widgets can only be created if");
   fprintf( stderr, " fnord is linked with OSF/Motif libraries.\n");
   return MET_NULL_SYM;
}


#endif /* MOTIF_WIDGETS */


#endif /* X_TWO_D */
