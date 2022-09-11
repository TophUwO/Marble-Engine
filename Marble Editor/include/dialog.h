#pragma once

#include <base.h>


/*
 * control type
 * 
 * Declares what member of the settings union
 * in the control-info structure is valid.
 */
enum mbe_dlg_ctrltype {
	MBE_DLGCTRLTYPE_UNKNOWN = 0, /* unknown/unspecified */

	/*
	 * Button types: 
	 *  - pushbutton
	 *  - radiobutton
	 *  - checkbox
	 */
	MBE_DLGCTRLTYPE_BUTTON,
	MBE_DLGCTRLTYPE_SPINBOX, /* spinbox control */
	MBE_DLGCTRLTYPE_EDIT     /* edit control */
};

/*
 * button flags; used to specify the
 * role of a button control
 */
enum mbe_dlg_btnflags {
	MBE_DLGBTNFLAG_UNKNOWN = 0, /* unknown/unspecified */

	MBE_DLGBTNFLAG_OK      = 1 << 0, /* OK button */
	MBE_DLGBTNFLAG_CANCEL  = 2 << 0, /* Cancel button */
	MBE_DLGBTNFLAG_RESET   = 3 << 0, /* Reset button */
	MBE_DLGBTNFLAG_MISC    = 0       /* regular button with no special role */
};

/*
 * common control flags 
 */
enum mbe_dlg_ctrlflags {
	MBE_DLGCTRLFLAG_UNKNOWN = 0,           /* unknown/unspecified */

	MBE_DLGCTRLFLAG_DISABLED     = 1 << 0, /* disabled by default */
	MBE_DLGCTRLFLAG_INITIALFOCUS = 1 << 1  /* has initial focus */
};


/*
 * Structure representing dialog control information.
 * 
 * Each dialog should define a structure array of this type,
 * specifying arttributes of each dialog control.
 * The dialog handler will use this data to automate
 * certain aspects of dialog handling.
 */
struct mbe_dlg_ctrlinfo {
	int    m_item;                /* dialog item ID */
	enum mbe_dlg_ctrltype m_type; /* control class */
	/*
	 * offset of writeback buffer, relative to
	 * the beginning of the structure pointed to
	 * by the **mp_udata** field of "mbe_dlginfo",
	 * in bytes.
	 */
	off_t  m_wboff;
	size_t m_wbsize; /* size of writeback buffer, in bytes */
	int    m_flags;  /* common control flags */
	
	/*
	 * The member **m_type** declares what field
	 * of this union is valid.
	 */
	union {
		/*
		 * This structure is used when **m_type** is
		 * set to 'MBE_DLGCTRLTYPE_BUTTON'.
		 */
		struct {
			int m_defstate; /* default state */
			int m_flags;    /* button flags */

			/*
			 * Handler function executed when the button
			 * is clicked. Note that this callback is run
			 * AFTER the system has run any predefined
			 * callbacks.
			 */
			BOOL (MB_CALLBACK *mpfn_onselect)(_In_ HWND, _In_opt_ void *);
		} _button;

		/*
		 * This structure is used when **m_type** is
		 * 'MBE_DLGCTRLTYPE_SPINBOX'.
		 */
		struct {
			/*
			 * Every spinbox control can have a "buddy" associated
			 * with it. This is normally an edit control displaying
			 * the current value of the spinbox control.
			 * Associating a buddy with a spinbox automates changing
			 * of the edit text upon encountering a change in the
			 * spinbox value. 
			 */
			int m_buddy;

			int m_def; /* default value */
			int m_min; /* minimum value */
			int m_max; /* maximum value */
		} _spin;

		/*
		 * This structure is used when **m_type** is
		 * 'MBE_DLGCTRLTYPE_EDIT'.
		 */
		struct {
			/*
			 * Default string value; NULL results in
			 * an empty string.
			 */
			TCHAR *mpz_defstr;
			int m_maxlen;
		} _edit;
	};
};

/*
 * Structure describing the nature of the dialog
 * that is about to be opened.
 */
struct mbe_dlginfo {
	HWND    mp_hparent; /* parent window */
	void   *mp_udata;   /* userdata (dialog results) */
	size_t  m_udatasz;  /* size of userdata, in bytes */
	int     m_templ;    /* dialog resource template */

	/*
	 * Structure representing detailed information
	 * about a dialog control.
	 * It controls all behavior associated with a
	 * control.
	 */
	struct mbe_dlg_ctrlinfo const *map_ctrlinfo;
	size_t m_nctrlinfo; /* number of entries in the info array */
};


/*
 * Opens a dialog window using a resource template
 * provided by the **m_templ** member of the
 * **ps_dlginfo** structure.
 * 
 * Returns the dialog result; TRUE if the dialog ended successfully,
 * or FALSE if there was an error or the dialog was ended "unsuccessfully".
 * Note that "unsuccessfully" may or may not indicate an actual error. That is,
 * closing the dialog by means other than designated controls also causes
 * an "unsuccessful" termination.
 */
extern BOOL mbe_dialog_dodialog(
	_In_ struct mbe_dlginfo const *ps_dlginfo /* dialog info */
);


