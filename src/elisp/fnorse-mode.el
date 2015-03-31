;;; Copyright 1990, 1991, 1992, Brown University, Providence, RI
;;; 
;;; This program is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 2 of the License, or
;;; (at your option) any later version.
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program; if not, write to the Free Software
;;; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;;; 
;;;
;;; $Id: fnorse-mode.el,v 1.7 1992/06/22 17:03:06 mas Exp $


(defconst fnorse-fnord-command
  '("/usr/misc/.fnord/bin/xmfnord"
    "/usr/misc/.fnord/examples/std.fnorse" 
    "stdin")
  "*List arguments to use as the fnord interpreter")

(defvar fnorse-process nil
  "The fnord interpreter, or nil of one isn't running.")

(defvar fnorse-mode-map ()
  "Keymap used in Fnorse mode.")

(if fnorse-mode-map
    ()
  (setq fnorse-mode-map (make-sparse-keymap))
  (define-key fnorse-mode-map "\C-c\C-c" 'fnorse-fnord-buffer)
  (define-key fnorse-mode-map "\C-c\C-d" 'fnorse-kill-process)
  (define-key fnorse-mode-map "\C-c\C-b" 'fnorse-blast-process)
  (define-key fnorse-mode-map "\t" 'fnorse-indent-command))


(defvar fnorse-mode-syntax-table nil
  "Syntax table in use in Fnorse-mode buffers.")

;; check these...
(if fnorse-mode-syntax-table
    ()
  (setq fnorse-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?\\ "\\" fnorse-mode-syntax-table)
  (modify-syntax-entry ?/ ". 14" fnorse-mode-syntax-table)
  (modify-syntax-entry ?* ". 23" fnorse-mode-syntax-table)
  (modify-syntax-entry ?+ "." fnorse-mode-syntax-table)
  (modify-syntax-entry ?- "." fnorse-mode-syntax-table)
  (modify-syntax-entry ?= "." fnorse-mode-syntax-table)
  (modify-syntax-entry ?% "." fnorse-mode-syntax-table)
  (modify-syntax-entry ?< "." fnorse-mode-syntax-table)
  (modify-syntax-entry ?> "." fnorse-mode-syntax-table)
  (modify-syntax-entry ?& "." fnorse-mode-syntax-table)
  (modify-syntax-entry ?| "." fnorse-mode-syntax-table)
  (modify-syntax-entry ?\' "\"" fnorse-mode-syntax-table))



(defun fnorse-mode ()
  "Major mode for editing Fnorse code.
Tab indents.
Comments are delimited with /* ... */.
\\{fnorse-mode-map}
Turning on Fnorse mode calls the value of the variably fnorse-mode-hook
with no args, if that value is non-nil."
  (interactive)
  (kill-all-local-variables)
  (use-local-map fnorse-mode-map)
  (setq major-mode 'fnorse-mode)
  (setq mode-name "Fnorse")
;;  (setq local-abbrev-table fnorse-mode-abbrev-table)
;;  (set-syntax-table fnorse-mode-syntax-table)

;; set up local variables with make-local-variable

  (run-hooks 'fnorse-mode-hook))

;;; fire up a process.  should set EMACS envar or something...
(defun fnorse-start-process ()
  (setq fnorse-process
	(apply 'start-process
	       "fnord" "*fnord-output*"
	       (concat exec-directory "env")
	       "FNORD_NO_PROMPT=t"
	       "-"
	       fnorse-fnord-command))
  (process-send-string fnorse-process "\nFnordFix;\n"))


(defun fnorse-kill-process ()
  (interactive)
  (if fnorse-process
      (progn
	(process-send-string fnorse-process "\nFnordQuit;\n")
	(process-send-eof fnorse-process))))
  
(defun fnorse-blast-process ()
  (interactive)
  (if fnorse-process
      (let ((save-proc fnorse-process))
	(setq fnorse-process nil)
	(interrupt-process save-proc))))
  

(defun fnorse-fnord-buffer ()
  "Run Fnord on the contents of a buffer.  The variable FNORSE-FNORD-COMMAND
contains the command to be used to run Fnord."
  (interactive)

  (if fnorse-process
      (if (not (eq (process-status fnorse-process) 'run))
	  (progn
	    (message "fnord process not running, restarting.")
	    (fnorse-start-process)))
    (fnorse-start-process))

  (display-buffer "*fnord-output*" t)

  ;; reset the interpreter
  (process-send-string fnorse-process "\nFnordClear;\n")
  ;; send the current buffer to the fnord process
  (process-send-region fnorse-process (point-min) (point-max))
  )

(defun fnorse-is-at-top-level ()
  (save-excursion
    (if (re-search-backward "[^ \t\n]" nil t)
	(if (= ?\; (following-char))
	    0
	  nil)
      0)))

(defun fnorse-is-after-symbol ()
  (save-excursion
    (if (re-search-backward "[^ \t\n]" nil t)
	(if (re-search-backward "[^a-zA-Z0-9]" nil t)
	    (progn
	      (forward-char)
	      (if (= 0 (current-column)) 3 nil))
	  nil)
      nil)))
	    
;;; return column of last unbalenced grouper if we are in them, nil otherwise
;;; don't search past ; and deal with strings
(defun fnorse-find-unbalanced-grouper ()
  (save-excursion
    (let ((level 0)
	  quoted
	  done
	  result)
      (while (not done)
	(if (re-search-backward (if quoted "\""
				  "[][\";(){}]") nil t)
	    (let ((c (following-char)))
	      (cond ((or (= c ?\( )
			 (= c ?\[ )
			 (= c ?\{ ))
		     (setq level (1- level)))
		    ((or (= c ?\) )
			 (= c ?\] )
			 (= c ?\} ))
		     (setq level (1+ level)))
		    ((= c ?\; )
		     (setq done t))
		    ((= c ?\" )
		     (setq quoted (not quoted)))
		    (t
		     (message "fnorse-mode internal error."))))
	  (setq done t))
	(if (<= 0 level) t
	  (setq done t)
	  (setq result (1+ (current-column)))))
      result)))

(defun fnorse-previous-line ()
  (save-excursion
    (if (re-search-backward "[^ \t\n]" nil t)
	(progn
	  (beginning-of-line)
	  (re-search-forward "[^ \t\n]" nil t) ; can't fail
	  (1- (current-column)))
      0)))

;;; make current line begin at col.  must deal with empty current line
(defun fnorse-indent-to-col (col)
  (let ((beg (point)))
    (skip-chars-forward " \t")
    (delete-region beg (point))
    (indent-to col)))

(defun fnorse-indent-command ()
  (interactive)
  "Indent current line of fnorse code."

  ;; rules:
  ;;     if previous non-white chararcter is ; go to first column
  ;;     if one symbol between us and last ; go to 3
  ;;     if inside unbalanced groupers, line up under them
  ;;     otherwise, use same indent as previous line

  (beginning-of-line)
  (let ((col (or (fnorse-is-at-top-level)
		 (fnorse-is-after-symbol)
		 (fnorse-find-unbalanced-grouper)
		 (fnorse-previous-line))))
    (fnorse-indent-to-col col))
  (end-of-line))
