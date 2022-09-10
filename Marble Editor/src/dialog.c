#include <editor.h>


/*
 * Gets the button control ID of the button
 * that has the role denoted by **role**.
 * 
 * Returns the control ID if the control could
 * be found, or -1 if there is no button control
 * with the specified role.
 * If there are multiple controls with the
 * same role, the first control in the control
 * info is returned.
 */
static int mbe_dialog_int_getbtnbyrole(
	_In_ struct mbe_dlginfo *ps_dlginfo, /* dialog info */
	     int role                        /* role to search for */
) {
	if (ps_dlginfo == NULL)
		return -1;

	for (size_t index = 0; index < ps_dlginfo->m_nctrlinfo; index++) {
		struct mbe_dlg_ctrlinfo const *ps_tmp = &ps_dlginfo->map_ctrlinfo[index];

		if (ps_tmp->m_type == MBE_DLGCTRLTYPE_BUTTON)
			if ((ps_tmp->_button.m_flags & 0xFF) == role)
				return ps_tmp->m_item;
	}

	return -1;
}

/*
 * Retrieves a pointer to the control info entry in the info
 * struct array of the dialog information.
 * 
 * Returns the entry for a given control ID, or NULL if no
 * entry could be found. If there are multiple entries for
 * the same control ID, the first is returned.
 */
static struct mbe_dlg_ctrlinfo const *mbe_dialog_int_getctrlinfo(
	_In_ struct mbe_dlginfo *ps_dlginfo, /* dialog info */
	     int ctrl                        /* control ID */
) {
	if (ps_dlginfo == NULL)
		return NULL;

	for (size_t index = 0; index < ps_dlginfo->m_nctrlinfo; index++) {
		struct mbe_dlg_ctrlinfo const *ps_tmp = &ps_dlginfo->map_ctrlinfo[index];

		if (ps_tmp->m_item == ctrl)
			return ps_tmp;
	}

	return NULL;
}

/*
 * Resets the controls in the dialog to the values specified
 * by the control info entries of the dialog info structure.
 * 
 * Returns nothing.
 */
static void mbe_dialog_int_resetctrls(
	_In_ HWND p_hwnd,                   /* dialog window */
	_In_ struct mbe_dlginfo *ps_dlginfo /* dialog info */
) {
	if (p_hwnd == NULL || ps_dlginfo == NULL)
		return;

	HWND p_hctrl;
	struct mbe_dlg_ctrlinfo const *ps_tmp;
	
	for (size_t index = 0; index < ps_dlginfo->m_nctrlinfo; index++) {
		ps_tmp  = &ps_dlginfo->map_ctrlinfo[index];
		p_hctrl = MBE_DLGWND(ps_tmp->m_item);
		if (p_hctrl == NULL)
			continue;

		switch (ps_tmp->m_type) {
			case MBE_DLGCTRLTYPE_BUTTON:
				Button_SetCheck(p_hctrl, ps_tmp->_button.m_defstate);

				break;
			case MBE_DLGCTRLTYPE_SPINBOX:
				if (ps_tmp->_spin.m_buddy != 0)
					SendMessage(p_hctrl, UDM_SETBUDDY, (WPARAM)MBE_DLGWND(ps_tmp->_spin.m_buddy), 0);

				SendMessage(p_hctrl, UDM_SETRANGE32, (WPARAM)ps_tmp->_spin.m_min, (LPARAM)ps_tmp->_spin.m_max);
				SendMessage(p_hctrl, UDM_SETPOS32, 0, (LPARAM)ps_tmp->_spin.m_def);
				break;
			case MBE_DLGCTRLTYPE_EDIT:
				SetWindowText(p_hctrl, ps_tmp->_edit.mpz_defstr);
				
				if (ps_tmp->_edit.m_maxlen > 0)
					SendMessage(p_hctrl, EM_SETLIMITTEXT, ps_tmp->_edit.m_maxlen, 0);

				break;
		}

		EnableWindow(p_hctrl, ps_tmp->m_isenabled);
	}
}

/*
 * Executes a button callback function.
 * If the button has no callback associated with it,
 * the function does nothing.
 * 
 * Returns TRUE if there is no callback, or the result
 * of the callback function which can be either TRUE or
 * FALSE.
 */
static BOOL mbe_dialog_int_btncallback(
	_In_     HWND p_hwnd,                                /* dialog window */
	_In_     struct mbe_dlg_ctrlinfo const *ps_ctrlinfo, /* button control info */
	_In_opt_ void *p_udata                               /* dialog userdata */
) {
	/*
	 * The default behavior is to return TRUE if no callback is
	 * associated with the button.
	 */
	if (ps_ctrlinfo->_button.mpfn_onselect == NULL)
		return TRUE;

	return (*ps_ctrlinfo->_button.mpfn_onselect)(p_hwnd, p_udata);
}

/*
 * Update the write-back structure (member **mp_udata** of the **ps_dlginfo** struct).
 * This function may allocate memory dynamically. The caller is responsible for deallocating
 * this memory.
 * In case there is an error with writing-back the contents of a specific control, the
 * function will not return but continue until all known controls have been "visited".
 * 
 * Returns nothing.
 */
static void mbe_dialog_int_writeback(
	_In_ HWND p_hwnd,                   /* dialog window */
	_In_ struct mbe_dlginfo *ps_dlginfo /* dialog info */
) {
	if (p_hwnd == NULL || ps_dlginfo == NULL || ps_dlginfo->mp_udata == NULL)
		return;

	HWND p_hctrl;                          /* handle to the current control window */
	struct mbe_dlg_ctrlinfo const *ps_tmp; /* pointer to the control info entry of the current control */
	int ntmp;                              /* temp. integer variable */
	UINT msg;                              /* temp. message */
	void *p_src, *p_dest;                  /* source and dest for memcpy() operation */
	size_t cpysize;                        /* size for memcpy() operation, in bytes */
	TCHAR *pz_str;                         /* pointer to a dynamically-allocated string buffer */

	/*
	 * Loop through the control info array, writing-back
	 * the control contents in the order specified by the
	 * struct layout. 
	 */
	for (size_t index = 0; index < ps_dlginfo->m_nctrlinfo; index++) {
		ps_tmp  = &ps_dlginfo->map_ctrlinfo[index];
		p_hctrl = MBE_DLGWND(ps_dlginfo->map_ctrlinfo[index].m_item);
		if (p_hctrl == NULL || (ps_tmp->m_wbsize == 0 && ps_tmp->m_type != MBE_DLGCTRLTYPE_EDIT))
			continue;
		
		/* Calculate location of writeback buffer. */
		p_dest = (BYTE *)ps_dlginfo->mp_udata + ps_tmp->m_wboff;

		/*
		 * Write-back the result, depending on the
		 * control type.
		 */
		switch (ps_tmp->m_type) {
			case MBE_DLGCTRLTYPE_BUTTON:
			case MBE_DLGCTRLTYPE_SPINBOX:
				msg = ps_tmp->m_type == MBE_DLGCTRLTYPE_BUTTON
					? BM_GETCHECK
					: UDM_GETPOS32
				;

				ntmp = (int)SendMessage(p_hctrl, msg, 0, 0);

				p_src   = &ntmp;
				cpysize = ps_tmp->m_wbsize;
				break;
			case MBE_DLGCTRLTYPE_EDIT:
				if (ps_tmp->m_wbsize == 0) {
					size_t size = (size_t)GetWindowTextLength(p_hctrl);

					/*
					 * If the size to copy is 0, meaning an empty textbox,
					 * we just copy a NULL pointer. The caller has to be
					 * able to handle this.
					 */
					pz_str = NULL;
					if (size == 0)
						goto lbl_COPYPTR;

					/* Dynamically allocate memory to hold the string. */
					if (marble_system_alloc(
						MB_CALLER_INFO,
						(size + 1) * sizeof *pz_str,
						true,
						false,
						&pz_str
					) != MARBLE_EC_OK) {
						/*
						 * Just copy a NULL pointer in case allocation fails.
						 * This way, the writeback buffer will always be in
						 * a determinate state when the function returns.
						 * 
						 * "marble_system_alloc()" will always set *pz_str to NULL when
						 * it fails.
						 */
						size = 0;

						goto lbl_COPYPTR;
					}

					/* Query window text. */
					GetWindowText(p_hctrl, pz_str, (int)size + 1);

				lbl_COPYPTR:
					p_src   = (TCHAR *)&pz_str;
					cpysize = sizeof pz_str;
					break;
				}

				GetWindowText(p_hctrl, p_dest, (int)(ps_tmp->m_wbsize / sizeof *pz_str));
				continue;
			default: continue;
		}

		/*
		 * Copy the memory into the writeback structure provided
		 * by the caller of the dialog.
		 */
		memcpy(p_dest, p_src, cpysize);
	}
}

/*
 * Dialog window procedure 
 */
static INT_PTR CALLBACK mbe_dialog_int_wndproc(
	_In_ HWND p_hwnd,
	     UINT msg,
	     WPARAM wparam,
	     LPARAM lparam
) {
	MBE_WNDUSERDATA(ps_udata, struct mbe_dlginfo *);

	BOOL ret;
	int inttmp;
	struct mbe_dlg_ctrlinfo const *ps_ctrlinfo;

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(p_hwnd, GWLP_USERDATA, (LONG_PTR)lparam);

			SendMessage(p_hwnd, MBE_WM_INITDLGDATA, 0, 0);
			break;
		case MBE_WM_INITDLGDATA:
			mbe_dialog_int_resetctrls(p_hwnd, ps_udata);

			break;
		case WM_COMMAND:
			/*
			 * Get info of selected control. If it is
			 * unknown, ignore the message.
			 */
			ps_ctrlinfo = mbe_dialog_int_getctrlinfo(ps_udata, LOWORD(wparam));
			if (ps_ctrlinfo == NULL)
				break;

			if (ps_ctrlinfo->m_type == MBE_DLGCTRLTYPE_BUTTON) {
				/*
				 * If it's just a miscellaneous button control with no special role,
				 * execute its handler function and return.
				 */
				if (ps_ctrlinfo->_button.m_flags & MBE_DLGBTNFLAG_MISC) {
					mbe_dialog_int_btncallback(p_hwnd, ps_ctrlinfo, ps_udata->mp_udata);

					break;
				}

				/*
				 * Some buttons may have special roles associated with them. Currently,
				 * these roles are:
				 *	- OK button, signifying a successful dialog result
				 *  - Cancel button, signifying a negative dialog result
				 *  - Reset button, the only non-dialog-ending button that
				 *    reverts all control states the the default values provided
				 *    by the **map_ctrlinfo** member.
				 */
				switch (ps_ctrlinfo->_button.m_flags & 0xFF) {
					case MBE_DLGBTNFLAG_OK:
						/* Update userdata structure. */
						mbe_dialog_int_writeback(p_hwnd, ps_udata);

						ret = TRUE;
						break;
					case MBE_DLGBTNFLAG_CANCEL: ret = FALSE; break;
					case MBE_DLGBTNFLAG_RESET:
						mbe_dialog_int_resetctrls(p_hwnd, ps_udata);

						/* fallthru */
					default: goto lbl_RET;
				}

				/*
				 * If the callback of the role button returns FALSE, set the dialog
				 * result to FALSE.
				 * This can be used to validate the dialog result by checking all fields
				 * or invalid values, etc.
				 */
				if (mbe_dialog_int_btncallback(p_hwnd, ps_ctrlinfo, ps_udata->mp_udata) == FALSE)
					break;

				/* Return the dialog result. */
				EndDialog(p_hwnd, ret);
			}
		
			break;
		case WM_CLOSE:
			/* Get ID of Cancel button. */
			inttmp = mbe_dialog_int_getbtnbyrole(ps_udata, MBE_DLGBTNFLAG_CANCEL);

			/*
			 * Pretend this button was pressed, ending
			 * the dialog uunsuccessfully.
			 */
			SendMessage(
				p_hwnd,
				WM_COMMAND,
				MAKELONG(inttmp, 0),
				0
			);

			break;
	}

lbl_RET:
	return FALSE;
}


BOOL mbe_dialog_dodialog(
	_In_ struct mbe_dlginfo const *ps_dlginfo
) {
	if (ps_dlginfo == NULL)
		return FALSE;

	return (BOOL)DialogBoxParam(
		gls_editorapp.mp_hinst,
		MAKEINTRESOURCE(ps_dlginfo->m_templ),
		ps_dlginfo->mp_hparent,
		(DLGPROC)&mbe_dialog_int_wndproc,
		(LPARAM)ps_dlginfo
	);
}


