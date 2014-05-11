/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#include "ScriptureEdit.h"

#include "dbstruct.h"
#include "KJVPassageNavigatorDlg.h"
#include "QtFindReplaceDialog/dialogs/finddialog.h"
#include "MimeHelper.h"
#include "PersistentSettings.h"
#include "UserNotesDatabase.h"
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
#include "KJVNoteEditDlg.h"
#include "KJVCrossRefEditDlg.h"
#endif
#include "ToolTipEdit.h"
#include "BusyCursor.h"
#include "myApplication.h"
#include "KJVCanOpener.h"

#include <assert.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QEvent>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QColor>

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------
	// Find Dialog:
	const QString constrFindDialogGroup("FindDialog");
}

// ============================================================================

template <class T, class U>
CScriptureText<T,U>::CScriptureText(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	T(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_pFindDialog(NULL),
		m_bDoingPopup(false),
		m_bDoingSelectionChange(false),
		m_navigator(pBibleDatabase, *this, T::useToolTipEdit()),
		m_bDoPlainCopyOnly(false),
		m_pEditMenu(NULL),
		m_pActionCopy(NULL),
		m_pActionCopyPlain(NULL),
		m_pActionCopyRaw(NULL),
		m_pActionCopyVeryRaw(NULL),
		m_pActionCopyVerses(NULL),
		m_pActionCopyVersesPlain(NULL),
		m_pActionCopyReferenceDetails(NULL),
		m_pActionCopyPassageStatistics(NULL),
		m_pActionCopyEntirePassageDetails(NULL),
		m_pActionSelectAll(NULL),
		m_pActionFind(NULL),
		m_pActionFindNext(NULL),
		m_pActionFindPrev(NULL),
		m_pActionShowAllNotes(NULL),
		m_pActionHideAllNotes(NULL),
		m_pStatusAction(NULL),
		m_pParentCanOpener(NULL),
		m_dlyDetailUpdate(-1, 500),
		m_dlyRerenderCompressor(-1, 10)
{
	assert(m_pBibleDatabase.data() != NULL);

	T::setMouseTracking(true);
	T::installEventFilter(this);

	T::viewport()->setCursor(QCursor(Qt::ArrowCursor));

	m_HighlightTimer.stop();

	// Setup Default Font and TextBrightness:
	setFont(CPersistentSettings::instance()->fontScriptureBrowser());
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());

	U::connect(CPersistentSettings::instance(), SIGNAL(fontChangedScriptureBrowser(const QFont &)), this, SLOT(setFont(const QFont &)));
	U::connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));
	U::connect(CPersistentSettings::instance(), SIGNAL(changedVerseRenderingMode(CPhraseNavigator::VERSE_RENDERING_MODE_ENUM)), &m_dlyRerenderCompressor, SLOT(trigger()));
	U::connect(CPersistentSettings::instance(), SIGNAL(changedShowPilcrowMarkers(bool)), &m_dlyRerenderCompressor, SLOT(trigger()));
	U::connect(CPersistentSettings::instance(), SIGNAL(changedBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &)), this, SLOT(en_changedBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &)));

	U::connect(&m_dlyRerenderCompressor, SIGNAL(triggered()), this, SLOT(rerender()));

	// FindDialog:
	if (T::useFindDialog()) {
		m_pFindDialog = new FindDialog(this);
		m_pFindDialog->setModal(false);
		m_pFindDialog->setTextEdit(this);
	}

	T::connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(en_cursorPositionChanged()));
	T::connect(this, SIGNAL(selectionChanged()), this, SLOT(en_selectionChanged()));
	T::connect(&m_navigator, SIGNAL(changedDocumentText()), this, SLOT(clearHighlighting()));
	T::connect(&m_HighlightTimer, SIGNAL(timeout()), this, SLOT(clearHighlighting()));
	T::connect(&m_dlyDetailUpdate, SIGNAL(triggered()), this, SLOT(en_detailUpdate()));

	m_pEditMenu = new QMenu(QObject::tr("&Edit", "MainMenu"), this);
	m_pEditMenu->setStatusTip(QObject::tr("Scripture Text Edit Operations", "MainMenu"));
	m_pActionCopy = m_pEditMenu->addAction(QObject::tr("&Copy as shown", "MainMenu"), this, SLOT(en_copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	m_pActionCopy->setStatusTip(QObject::tr("Copy selected passage browser text, as shown, to the clipboard", "MainMenu"));
	m_pActionCopy->setEnabled(false);
	T::connect(this, SIGNAL(copyAvailable(bool)), m_pActionCopy, SLOT(setEnabled(bool)));
	m_pActionCopyPlain = m_pEditMenu->addAction(QObject::tr("Copy as shown (&plain)", "MainMenu"), this, SLOT(en_copyPlain()));
	m_pActionCopyPlain->setStatusTip(QObject::tr("Copy selected passage browser text, as shown but without colors and fonts, to the clipboard", "MainMenu"));
	m_pActionCopyPlain->setEnabled(false);
	T::connect(this, SIGNAL(copyAvailable(bool)), m_pActionCopyPlain, SLOT(setEnabled(bool)));
	m_pEditMenu->addSeparator();
	m_pActionCopyRaw = m_pEditMenu->addAction(QObject::tr("Copy Raw Verse &Text (No headings)", "MainMenu"), this, SLOT(en_copyRaw()), QKeySequence(Qt::CTRL + Qt::Key_T));
	m_pActionCopyRaw->setStatusTip(QObject::tr("Copy selected passage browser text as raw phrase words to the clipboard", "MainMenu"));
	m_pActionCopyRaw->setEnabled(false);
	T::connect(this, SIGNAL(copyRawAvailable(bool)), m_pActionCopyRaw, SLOT(setEnabled(bool)));
	m_pActionCopyVeryRaw = m_pEditMenu->addAction(QObject::tr("Copy Very Ra&w Verse Text (No punctuation)", "MainMenu"), this, SLOT(en_copyVeryRaw()));
	m_pActionCopyVeryRaw->setStatusTip(QObject::tr("Copy selected passage browser text as very raw (no punctuation) phrase words to the clipboard", "MainMenu"));
	m_pActionCopyVeryRaw->setEnabled(false);
	T::connect(this, SIGNAL(copyRawAvailable(bool)), m_pActionCopyVeryRaw, SLOT(setEnabled(bool)));
	m_pEditMenu->addSeparator();
	m_pActionCopyVerses = m_pEditMenu->addAction(QObject::tr("Copy as &Verses", "MainMenu"), this, SLOT(en_copyVerses()), QKeySequence(Qt::CTRL + Qt::Key_V));
	m_pActionCopyVerses->setStatusTip(QObject::tr("Copy selected passage browser text as Formatted Verses to the clipboard", "MainMenu"));
	m_pActionCopyVerses->setEnabled(false);
	T::connect(this, SIGNAL(copyVersesAvailable(bool)), m_pActionCopyVerses, SLOT(setEnabled(bool)));
	m_pActionCopyVersesPlain = m_pEditMenu->addAction(QObject::tr("Copy as Verses (plai&n)", "MainMenu"), this, SLOT(en_copyVersesPlain()));
	m_pActionCopyVersesPlain->setStatusTip(QObject::tr("Copy selected passage browser text as Formatted Verses, but without colors and fonts, to the clipboard", "MainMenu"));
	m_pActionCopyVersesPlain->setEnabled(false);
	T::connect(this, SIGNAL(copyVersesAvailable(bool)), m_pActionCopyVersesPlain, SLOT(setEnabled(bool)));
	m_pEditMenu->addSeparator();
	m_pActionCopyReferenceDetails = m_pEditMenu->addAction(QObject::tr("Copy &Reference Details (Word/Phrase)", "MainMenu"), this, SLOT(en_copyReferenceDetails()));
	m_pActionCopyReferenceDetails->setStatusTip(QObject::tr("Copy the Word/Phrase Reference Details in the passage browser to the clipboard", "MainMenu"));
	m_pActionCopyPassageStatistics = m_pEditMenu->addAction(QObject::tr("Copy Passage Stat&istics (Book/Chapter/Verse)", "MainMenu"), this, SLOT(en_copyPassageStatistics()));
	m_pActionCopyPassageStatistics->setStatusTip(QObject::tr("Copy the Book/Chapter/Verse Passage Statistics in the passage browser to the clipboard", "MainMenu"));
	m_pActionCopyEntirePassageDetails = m_pEditMenu->addAction(QObject::tr("Copy Entire Passage Detai&ls", "MainMenu"), this, SLOT(en_copyEntirePassageDetails()), QKeySequence(Qt::CTRL + Qt::Key_B));
	m_pActionCopyEntirePassageDetails->setStatusTip(QObject::tr("Copy both the Word/Phrase Reference Detail and Book/Chapter/Verse Statistics in the passage browser to the clipboard", "MainMenu"));
	m_pEditMenu->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction(QObject::tr("Select &All", "MainMenu"), this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip(QObject::tr("Select all current passage browser text", "MainMenu"));
	m_pEditMenu->addSeparator();
	if (m_pFindDialog != NULL) {
		m_pActionFind = m_pEditMenu->addAction(QObject::tr("&Find...", "MainMenu"), this, SLOT(en_findDialog()), QKeySequence(Qt::CTRL + Qt::Key_F));
		m_pActionFind->setStatusTip(QObject::tr("Find text within the passage browser", "MainMenu"));
		m_pActionFind->setEnabled(T::useFindDialog());
		m_pActionFindNext = m_pEditMenu->addAction(QObject::tr("Find &Next", "MainMenu"), m_pFindDialog, SLOT(findNext()), QKeySequence(Qt::Key_F3));
		m_pActionFindNext->setStatusTip(QObject::tr("Find next occurrence of text within the passage browser", "MainMenu"));
		m_pActionFindNext->setEnabled(T::useFindDialog());
		m_pActionFindPrev = m_pEditMenu->addAction(QObject::tr("Find &Previous", "MainMenu"), m_pFindDialog, SLOT(findPrev()), QKeySequence(Qt::SHIFT + Qt::Key_F3));
		m_pActionFindPrev->setStatusTip(QObject::tr("Find previous occurrence of text within the passage browser", "MainMenu"));
		m_pActionFindPrev->setEnabled(T::useFindDialog());
	}

	if (qobject_cast<const QTextBrowser *>(this) != NULL) {
		T::connect(this, SIGNAL(anchorClicked(const QUrl &)), this, SLOT(en_anchorClicked(const QUrl &)));

		// Trigger adding our higlighters and things are we've discovered our CKJVCanOpener parent:
		QTimer::singleShot(1, this, SLOT(en_findParentCanOpener()));
	}

	T::setContextMenuPolicy(Qt::CustomContextMenu);
	U::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_customContextMenuRequested(const QPoint &)));

//	T::connect(ui->actionReplace, SIGNAL(triggered()), this, SLOT(findReplaceDialog()));

	U::setToolTip(QString(QObject::tr("Press %1 to see Passage Details", "MainMenu")).arg(QKeySequence(Qt::CTRL + Qt::Key_D).toString(QKeySequence::NativeText)));

	m_pStatusAction = new QAction(this);
}

template<class T, class U>
CScriptureText<T,U>::~CScriptureText()
{

}

// ----------------------------------------------------------------------------

template<class T, class U>
CKJVCanOpener *CScriptureText<T,U>::parentCanOpener() const
{
	if (m_pParentCanOpener == NULL) {
		assert(g_pMyApplication.data() != NULL);
		m_pParentCanOpener = g_pMyApplication->findCanOpenerFromChild<T>(this);
		// Note: It's possible for the parentCanOpener to be NULL if this function is called during
		//		the construction process before the parent actually exists.  In that case, we'll
		//		return NULL (callers will have to deal with that) and lock in our parent in a future
		//		call when it becomes available...
	}
	return m_pParentCanOpener;
}

template<class T, class U>
void CScriptureText<T,U>::en_findParentCanOpener()
{
	CKJVCanOpener *pCanOpener = parentCanOpener();
	assert(pCanOpener != NULL);

	if ((pCanOpener != NULL) && (qobject_cast<const QTextBrowser *>(this) != NULL)) {
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		m_pEditMenu->addSeparator();
		m_pEditMenu->addActions(pCanOpener->highlighterButtons()->actions());
		T::connect(pCanOpener->highlighterButtons(), SIGNAL(highlighterToolTriggered(QAction *)), this, SLOT(en_highlightPassage(QAction *)));
		m_pEditMenu->addSeparator();
		m_pEditMenu->addAction(pCanOpener->actionUserNoteEditor());
		m_pActionShowAllNotes = m_pEditMenu->addAction(QObject::tr("Show All Notes", "MainMenu"), this, SLOT(en_showAllNotes()));
		m_pActionShowAllNotes->setStatusTip(QObject::tr("Expand all notes in the Scripture Browser, making them visible", "MainMenu"));
		m_pActionHideAllNotes = m_pEditMenu->addAction(QObject::tr("Hide All Notes", "MainMenu"), this, SLOT(en_hideAllNotes()));
		m_pActionHideAllNotes->setStatusTip(QObject::tr("Collapse all notes in the Scripture Browser, making them hidden", "MainMenu"));

		m_pEditMenu->addSeparator();
		m_pEditMenu->addAction(pCanOpener->actionCrossRefsEditor());
#endif
	}
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::setFont(const QFont& aFont)
{
	U::document()->setDefaultFont(aFont);
}

template<class T, class U>
void CScriptureText<T,U>::setTextBrightness(bool bInvert, int nBrightness)
{
	U::setStyleSheet(QString("i_CScriptureBrowser, i_CScriptureEdit { background-color:%1; color:%2; }")
								   .arg(CPersistentSettings::textBackgroundColor(bInvert, nBrightness).name())
								   .arg(CPersistentSettings::textForegroundColor(bInvert, nBrightness).name()));
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::savePersistentSettings(const QString &strGroup)
{
	if (CPersistentSettings::instance()->settings() != NULL) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		if (m_pFindDialog != NULL) m_pFindDialog->writeSettings(settings, groupCombine(strGroup, constrFindDialogGroup));
	}
}

template<class T, class U>
void CScriptureText<T,U>::restorePersistentSettings(const QString &strGroup)
{
	if (CPersistentSettings::instance()->settings() != NULL) {
		QSettings &settings(*CPersistentSettings::instance()->settings());
		if (m_pFindDialog != NULL) m_pFindDialog->readSettings(settings, groupCombine(strGroup, constrFindDialogGroup));
	}
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::en_findDialog()
{
	if (m_pFindDialog != NULL) {
		if (haveSelection()) {
			m_pFindDialog->setTextToFind(m_lstSelectedPhrases.primarySelectionPhrase().phrase().phraseRaw());
		}
		if (m_pFindDialog->isVisible()) {
			m_pFindDialog->activateWindow();
		} else {
			m_pFindDialog->show();
		}
	}
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::clearHighlighting()
{
	if (!m_bDoingPopup) {
		m_navigator.doHighlighting(m_CursorFollowHighlighter, true);
		m_CursorFollowHighlighter.clearPhraseTags();
		m_HighlightTimer.stop();
	}
}

template<class T, class U>
bool CScriptureText<T,U>::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == this) {
		switch (ev->type()) {
			case QEvent::Wheel:
			case QEvent::ActivationChange:
			case QEvent::KeyPress:
			case QEvent::KeyRelease:
			case QEvent::FocusOut:
			case QEvent::FocusIn:
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonDblClick:
			case QEvent::Leave:
			case QEvent::MouseMove:
				return false;
			default:
				break;
		}
	}

	return U::eventFilter(obj, ev);
}

template<class T, class U>
bool CScriptureText<T,U>::event(QEvent *ev)
{
	if (ev->type() == QEvent::FocusIn) {
		emit T::activatedScriptureText();
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		bool bEditEnable = false;
		if (qobject_cast<const QTextBrowser *>(this) != NULL) {
			bEditEnable = true;
		}

		if (parentCanOpener() != NULL) {
			parentCanOpener()->actionUserNoteEditor()->setEnabled(bEditEnable);
			parentCanOpener()->actionCrossRefsEditor()->setEnabled(bEditEnable);
			const QList<QAction *> lstHighlightActions = parentCanOpener()->highlighterButtons()->actions();
			for (int ndxHighlight = 0; ndxHighlight < lstHighlightActions.size(); ++ndxHighlight) {
				lstHighlightActions.at(ndxHighlight)->setEnabled(bEditEnable);
			}
		}
#endif
	} else if (ev->type() == QEvent::FocusOut) {
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
		QFocusEvent *pFocusEvent = static_cast<QFocusEvent *>(ev);
		if ((parentCanOpener() != NULL) &&
			(pFocusEvent->reason() != Qt::MenuBarFocusReason) &&
			(pFocusEvent->reason() != Qt::PopupFocusReason)) {
			parentCanOpener()->actionUserNoteEditor()->setEnabled(false);
			parentCanOpener()->actionCrossRefsEditor()->setEnabled(false);
			const QList<QAction *> lstHighlightActions = parentCanOpener()->highlighterButtons()->actions();
			for (int ndxHighlight = 0; ndxHighlight < lstHighlightActions.size(); ++ndxHighlight) {
				lstHighlightActions.at(ndxHighlight)->setEnabled(false);
			}
		}
#endif
	}

	switch (ev->type()) {
		case QEvent::ToolTip:
			{
				if ((!U::hasFocus()) || (!haveDetails()) || (CTipEdit::tipEditIsPinned(parentCanOpener()))) {
					ev->ignore();
					return true;
				}

//				QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(ev);
//				if (m_navigator.handleToolTipEvent(parentCanOpener(), pHelpEvent, m_CursorFollowHighlighter, m_selectedPhrase.second)) {
//					m_HighlightTimer.stop();
//				} else {
//					pHelpEvent->ignore();
//				}
//				return true;
			}
			break;

		// User input and window activation makes tooltips sleep
		case QEvent::Wheel:
		case QEvent::ActivationChange:
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		case QEvent::FocusOut:
		case QEvent::FocusIn:
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
			if (ev->type() == QEvent::KeyPress) {
				QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
				if (keyEvent->modifiers() & Qt::ControlModifier) {
					if ((keyEvent->key() == Qt::Key_Plus) ||	// This one handles the on the keypad
						((keyEvent->modifiers() & Qt::ShiftModifier) &&
						 (keyEvent->key() == Qt::Key_Equal))) {	// On the main keyboard, Ctrl-+ is on the Equal Key with a Shift (Ctrl-Shift-+)
						U::zoomIn();
						ev->accept();
						return true;
					} else if (keyEvent->key() == Qt::Key_Minus) {
						U::zoomOut();
						ev->accept();
						return true;
					}
				}
			}
			// Unfortunately, there doesn't seem to be any event we can hook to to determine
			//		when the ToolTip disappears.  Looking at the Qt code, it looks to be on
			//		a 2 second timeout.  So, we'll do a similar timeout here for the highlight:
			if ((!m_bDoingPopup) && (!m_CursorFollowHighlighter.isEmpty()) && (!m_HighlightTimer.isActive()))
				m_HighlightTimer.start(2000);
			break;
		case QEvent::Leave:
			if ((!m_bDoingPopup) && (!m_CursorFollowHighlighter.isEmpty())) {
				m_HighlightTimer.start(20);
			}
			break;
		default:
			break;
	}

	return U::event(ev);
}

template<class T, class U>
bool CScriptureText<T,U>::haveDetails() const
{
	QString strToolTip = m_navigator.getToolTip(m_tagLast, selection());
	return (!strToolTip.isEmpty());
}

template<class T, class U>
void CScriptureText<T,U>::showDetails()
{
	U::ensureCursorVisible();
	if (m_navigator.handleToolTipEvent(parentCanOpener(), m_CursorFollowHighlighter, m_tagLast, selection()))
		m_HighlightTimer.stop();
}

template<>
void CScriptureText<i_CScriptureEdit, QTextEdit>::mouseDoubleClickEvent(QMouseEvent *ev)
{
	assert(m_pBibleDatabase.data() != NULL);

	begin_popup();

	CRelIndex ndxLast = m_navigator.getSelection(cursorForPosition(ev->pos())).primarySelection().relIndex();
	m_tagLast = TPhraseTag(ndxLast, (ndxLast.isSet() ? 1 : 0));
	setLastActiveTag();
	m_navigator.highlightCursorFollowTag(m_CursorFollowHighlighter, m_tagLast);
	if (ndxLast.isSet()) emit gotoIndex(m_tagLast);

	end_popup();
}

template<>
void CScriptureText<i_CScriptureBrowser, QTextBrowser>::mouseDoubleClickEvent(QMouseEvent *ev)
{
	QTextBrowser::mouseDoubleClickEvent(ev);
}

template<class T, class U>
void CScriptureText<T,U>::mouseMoveEvent(QMouseEvent *ev)
{
	m_ptLastTrackPosition = ev->pos();
	U::mouseMoveEvent(ev);
}

template<class T, class U>
void CScriptureText<T,U>::showPassageNavigator()
{
	assert(m_pBibleDatabase.data() != NULL);

	begin_popup();

	// This now works exclusively by edit cursor position, not the mouse position from
	//		hovering as it used to when there was no selection.  This is so the menu
	//		Ctrl-G shortcut to activate this will make sense and be consistent across
	//		the entire app.

	TPhraseTag tagSel = m_lstSelectedPhrases.selection().primarySelection();
	if (!tagSel.relIndex().isSet()) tagSel.relIndex() = m_tagLast.relIndex();
	if (tagSel.count() == 0) tagSel.count() = ((tagSel.relIndex().word() != 0) ? 1 : 0);			// Simulate single word selection if nothing actually selected, but only if there is a word

	// Cap the number of words to those remaining in this verse so
	//		we don't spend all day highlighting junk:
	TPhraseTag tagHighlight = tagSel;
	CRefCountCalc Wrd(m_pBibleDatabase.data(), CRefCountCalc::RTE_WORD, tagHighlight.relIndex());
	tagHighlight.count() = qMin(Wrd.ofVerse().second - Wrd.ofVerse().first + 1, tagHighlight.count());

	m_CursorFollowHighlighter.setEnabled(true);
	m_navigator.highlightCursorFollowTag(m_CursorFollowHighlighter, tagHighlight);
#ifndef USE_ASYNC_DIALOGS
	CKJVCanOpener::CKJVCanOpenerCloseGuard closeGuard(parentCanOpener());
	CKJVPassageNavigatorDlgPtr pDlg(m_pBibleDatabase, T::parentWidget());
//	pDlg->navigator().startRelativeMode(tagSel, false, TPhraseTag(m_pBibleDatabase, CRelIndex(), 1));
	pDlg->navigator().startAbsoluteMode(tagSel);
	if (pDlg->exec() == QDialog::Accepted) {
		if (pDlg != NULL) emit T::gotoIndex(pDlg->passage());		// Could get deleted during execution
	}
#else
	CKJVPassageNavigatorDlg *pDlg = new CKJVPassageNavigatorDlg(m_pBibleDatabase, T::parentWidget());
	T::connect(pDlg, SIGNAL(gotoIndex(const TPhraseTag &)), this, SIGNAL(gotoIndex(const TPhraseTag &)));
	pDlg->navigator().startAbsoluteMode(tagSel);
	pDlg->show();
#endif

	end_popup();
}

template<class T, class U>
void CScriptureText<T,U>::en_customContextMenuRequested(const QPoint &pos)
{
	assert(m_pBibleDatabase.data() != NULL);

	begin_popup();

	CRelIndex ndxLast = m_navigator.getSelection(T::cursorForPosition(pos)).primarySelection().relIndex();
	m_tagLast = TPhraseTag(ndxLast, (ndxLast.isSet() ? 1 : 0));
	setLastActiveTag();
	m_navigator.highlightCursorFollowTag(m_CursorFollowHighlighter, m_tagLast);
	QMenu *menu = new QMenu(this);
	menu->addAction(m_pActionCopy);
	menu->addAction(m_pActionCopyPlain);
	menu->addSeparator();
	menu->addAction(m_pActionCopyRaw);
	menu->addAction(m_pActionCopyVeryRaw);
	menu->addSeparator();
	menu->addAction(m_pActionCopyVerses);
	menu->addAction(m_pActionCopyVersesPlain);
	menu->addSeparator();
	menu->addAction(m_pActionCopyReferenceDetails);
	menu->addAction(m_pActionCopyPassageStatistics);
	menu->addAction(m_pActionCopyEntirePassageDetails);
	menu->addSeparator();
	menu->addAction(m_pActionSelectAll);
	if (T::useFindDialog()) {
		menu->addSeparator();
		menu->addAction(m_pActionFind);
		menu->addAction(m_pActionFindNext);
		menu->addAction(m_pActionFindPrev);
	}
	if (qobject_cast<const QTextBrowser *>(this) != NULL) {
		if (parentCanOpener()) {
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
			menu->addSeparator();
			menu->addActions(parentCanOpener()->highlighterButtons()->actions());
			menu->addSeparator();
			menu->addAction(parentCanOpener()->actionUserNoteEditor());
			if (m_pActionShowAllNotes) menu->addAction(m_pActionShowAllNotes);
			if (m_pActionHideAllNotes) menu->addAction(m_pActionHideAllNotes);
			menu->addSeparator();
			menu->addAction(parentCanOpener()->actionCrossRefsEditor());
#endif
		}
		menu->addSeparator();
		QAction *pActionNavigator = menu->addAction(QIcon(":/res/green_arrow.png"), QObject::tr("Passage &Navigator...", "MainMenu"));
		T::connect(pActionNavigator, SIGNAL(triggered()), this, SLOT(showPassageNavigator()));
		pActionNavigator->setEnabled(true);
		pActionNavigator->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	}
	menu->addSeparator();
	QAction *pActionDetails = menu->addAction(QIcon(":/res/Windows-View-Detail-icon-48.png"), QObject::tr("View &Details...", "MainMenu"));
	pActionDetails->setEnabled(haveDetails());
	pActionDetails->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
	T::connect(pActionDetails, SIGNAL(triggered()), this, SLOT(showDetails()));
#ifndef USE_ASYNC_DIALOGS
	menu->exec(T::viewport()->mapToGlobal(pos));
	delete menu;
#else
	menu->setAttribute(Qt::WA_DeleteOnClose);
	menu->popup(T::viewport()->mapToGlobal(pos));
#endif
	m_ptLastTrackPosition = pos;

	end_popup();
}

// ----------------------------------------------------------------------------

template<class T, class U>
QMimeData *CScriptureText<T,U>::createMimeDataFromSelection() const
{
	assert(m_pBibleDatabase.data() != NULL);

	QMimeData *mime = U::createMimeDataFromSelection();
	if (m_bDoPlainCopyOnly) {
		// Let the base class do the copy, but snag the plaintext
		//	version and render only that:
		QString strTemp = mime->text();
		mime->clear();
		mime->setText(strTemp);
	} else {
		if (mime->hasHtml()) {
			QTextDocument docCopy;
			docCopy.setHtml(mime->html());
			CPhraseNavigator navigator(m_pBibleDatabase, docCopy);
			navigator.removeAnchors();
			mime->setHtml(docCopy.toHtml());
		}
	}
	// TODO : Copy list of tags for multi-selection?
	if (haveSelection()) CMimeHelper::addPhraseTagToMimeData(mime, selection().primarySelection());
	return mime;
}

template<class T, class U>
void CScriptureText<T,U>::en_cursorPositionChanged()
{
	CPhraseCursor cursor(T::textCursor());
	m_tagLast.relIndex() = m_navigator.getSelection(cursor).primarySelection().relIndex();
	if (!m_tagLast.relIndex().isSet()) m_tagLast.count() = 0;
	setLastActiveTag();

	// Move start of selection tag so we can later simulate pseudo-selection of
	//		single word when nothing is really selected:
	updateSelection();
}

template<class T, class U>
void CScriptureText<T,U>::en_selectionChanged()
{
	updateSelection();
}

template<class T, class U>
void CScriptureText<T,U>::updateSelection()
{
	assert(m_pBibleDatabase.data() != NULL);
	assert(g_pUserNotesDatabase.data() != NULL);

	if (m_bDoingSelectionChange) return;
	m_bDoingSelectionChange = true;

	bool bOldSel = haveSelection();
	CSelectedPhraseList prevSelection = m_lstSelectedPhrases;
	m_lstSelectedPhrases = m_navigator.getSelectedPhrases();
	if (haveSelection() != bOldSel) emit T::copyRawAvailable(haveSelection());
	emit T::copyVersesAvailable(haveSelection() ||
								(m_tagLast.relIndex().isSet() &&
								 ((m_tagLast.relIndex().verse() != 0) ||
								  ((m_tagLast.relIndex().verse() == 0) && (m_tagLast.relIndex().word() != 0)))));
	QString strStatusText;
	CSelectionPhraseTagList lstSelection = m_lstSelectedPhrases.selection();
	if (haveSelection()) {
		strStatusText = lstSelection.primarySelection().PassageReferenceRangeText(m_pBibleDatabase.data());
	} else if (m_tagLast.relIndex().isSet()) {
		strStatusText = m_pBibleDatabase->PassageReferenceText(m_tagLast.relIndex());
	}

	if (lstSelection.primarySelection().count() > 0) {
		if (!strStatusText.isEmpty()) strStatusText += " : ";
		strStatusText += QObject::tr("%n Word(s) Selected", "Statistics", lstSelection.primarySelection().count());
	}
	T::setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();

	if (!haveSelection()) {
		const TPhraseTagList &lstTags(m_CursorFollowHighlighter.phraseTags());
		TPhraseTag nNewSel = TPhraseTag(m_tagLast.relIndex(), 1);
		if  ((lstTags.size() == 0) || (lstTags.value(0) != nNewSel))
			m_navigator.highlightCursorFollowTag(m_CursorFollowHighlighter, nNewSel);
	}
	m_CursorFollowHighlighter.setEnabled(!haveSelection());

	if ((CTipEdit::tipEditIsPinned(parentCanOpener())) && (prevSelection != m_lstSelectedPhrases))
		m_dlyDetailUpdate.trigger();

	m_bDoingSelectionChange = false;
}

template<class T, class U>
void CScriptureText<T,U>::en_detailUpdate()
{
	m_navigator.handleToolTipEvent(parentCanOpener(), m_CursorFollowHighlighter, m_tagLast, selection());
}

template<class T, class U>
void CScriptureText<T,U>::en_changedBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings)
{
	Q_UNUSED(aSettings);
	if (m_pBibleDatabase->compatibilityUUID().compare(strUUID, Qt::CaseInsensitive) == 0) {
		m_dlyRerenderCompressor.trigger();
	}
}

template<class T, class U>
void CScriptureText<T,U>::setLastActiveTag()
{
	if (m_tagLast.isSet()) {
		// Note: Special case chapter != 0, verse == 0 for special top-of-book/chapter scroll
		if ((m_tagLast.relIndex().verse() != 0) || (m_tagLast.relIndex().word() != 0) ||
			((m_tagLast.relIndex().chapter() != 0) && (m_tagLast.relIndex().verse() == 0))) {
			m_tagLastActive = m_tagLast;
			if (m_tagLastActive.relIndex().verse() == 0) m_tagLastActive.relIndex().setVerse(1);
			if (m_tagLastActive.relIndex().word() == 0) m_tagLastActive.relIndex().setWord(1);
		} else if (m_tagLast.relIndex().chapter() == 0) {
			if ((m_tagLast.relIndex().book() != m_ndxCurrent.book()) &&
				(m_ndxCurrent.chapter() == 1)) {
				m_tagLastActive = m_tagLast;
				m_tagLastActive.relIndex().setChapter(m_pBibleDatabase->bookEntry(m_tagLastActive.relIndex().book())->m_nNumChp);
				m_tagLastActive.relIndex().setVerse(m_pBibleDatabase->chapterEntry(m_tagLastActive.relIndex())->m_nNumVrs);
				m_tagLastActive.relIndex().setWord(m_pBibleDatabase->verseEntry(m_tagLastActive.relIndex())->m_nNumWrd);
			} else if ((m_tagLast.relIndex().book() == m_ndxCurrent.book()) &&
					   (m_ndxCurrent.chapter() == 1)) {
				m_tagLastActive = TPhraseTag(CRelIndex(m_ndxCurrent.book(), 1, 0, 0));
			}
		}
	}
}

template<class T, class U>
void CScriptureText<T,U>::en_gotoIndex(const TPhraseTag &tag)
{
	m_ndxCurrent = tag.relIndex();
	m_tagLastActive = tag;
}

template<class T, class U>
void CScriptureText<T,U>::rerender()
{
	if ((selection().primarySelection().relIndex().chapter() == 0) &&
		(selection().primarySelection().relIndex().verse() == 0) &&
		(selection().primarySelection().relIndex().word() == 0)) {
		// Special case if it's an entire book, use our last active tag:
		if (m_tagLastActive.isSet()) emit T::gotoIndex(m_tagLastActive);
	} else if (selection().isSet()) {
		emit T::gotoIndex(selection().primarySelection());
	} else {
		emit T::gotoIndex(TPhraseTag(m_ndxCurrent));
	}
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::en_copy()
{
	// Clear highlighting before copy so we don't have the cursor follow highlighter
	//		copied in the middle of our copied text
	m_bDoingPopup = false;
	clearHighlighting();
	T::copy();
	displayCopyCompleteToolTip();
}

template<class T, class U>
void CScriptureText<T,U>::en_copyPlain()
{
	// Clear highlighting before copy so we don't have the cursor follow highlighter
	//		copied in the middle of our copied text
	m_bDoingPopup = false;
	clearHighlighting();
	m_bDoPlainCopyOnly = true;		// Do plaintext only so user can paste into Word without changing its format, for example
	T::copy();
	m_bDoPlainCopyOnly = false;
	displayCopyCompleteToolTip();
}

template<class T, class U>
void CScriptureText<T,U>::en_copyRaw()
{
	if (!haveSelection()) return;
	QMimeData *mime = new QMimeData();
	QString strText;
	for (int ndx = 0; ndx < m_lstSelectedPhrases.size(); ++ndx) {
		if (!m_lstSelectedPhrases.at(ndx).tag().haveSelection()) continue;
		strText += m_lstSelectedPhrases.at(ndx).phrase().phrase() + "\n";
		if (CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses()) strText += "\n";
	}
	mime->setText(strText);
	// TODO : Copy list of tags for multi-selection?
	CMimeHelper::addPhraseTagToMimeData(mime, selection().primarySelection());
	QApplication::clipboard()->setMimeData(mime);
	displayCopyCompleteToolTip();
}

template<class T, class U>
void CScriptureText<T,U>::en_copyVeryRaw()
{
	if (!haveSelection()) return;
	QMimeData *mime = new QMimeData();
	QString strText;
	for (int ndx = 0; ndx < m_lstSelectedPhrases.size(); ++ndx) {
		if (!m_lstSelectedPhrases.at(ndx).tag().haveSelection()) continue;
		strText += m_lstSelectedPhrases.at(ndx).phrase().phraseRaw() + "\n";
		if (CPersistentSettings::instance()->searchResultsAddBlankLineBetweenVerses()) strText += "\n";
	}
	mime->setText(strText);
	// TODO : Copy list of tags for multi-selection?
	CMimeHelper::addPhraseTagToMimeData(mime, selection().primarySelection());
	QApplication::clipboard()->setMimeData(mime);
	displayCopyCompleteToolTip();
}

template<class T, class U>
void CScriptureText<T,U>::en_copyVerses()
{
	if (haveSelection() || (m_tagLast.relIndex().isSet() && m_tagLast.relIndex().verse() != 0)) copyVersesCommon(false);
}

template<class T, class U>
void CScriptureText<T,U>::en_copyVersesPlain()
{
	if (haveSelection() || (m_tagLast.relIndex().isSet() && m_tagLast.relIndex().verse() != 0)) copyVersesCommon(true);
}

template<class T, class U>
void CScriptureText<T,U>::en_copyReferenceDetails()
{
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, selection(), CPhraseEditNavigator::TTE_REFERENCE_ONLY, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, selection(), CPhraseEditNavigator::TTE_REFERENCE_ONLY, false));
	// TODO : Copy list of tags for multi-selection?
	CMimeHelper::addPhraseTagToMimeData(mime, selection().primarySelection());
	QApplication::clipboard()->setMimeData(mime);
	displayCopyCompleteToolTip();
}

template<class T, class U>
void CScriptureText<T,U>::en_copyPassageStatistics()
{
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, selection(), CPhraseEditNavigator::TTE_STATISTICS_ONLY, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, selection(), CPhraseEditNavigator::TTE_STATISTICS_ONLY, false));
	// TODO : Copy list of tags for multi-selection?
	CMimeHelper::addPhraseTagToMimeData(mime, selection().primarySelection());
	QApplication::clipboard()->setMimeData(mime);
	displayCopyCompleteToolTip();
}

template<class T, class U>
void CScriptureText<T,U>::en_copyEntirePassageDetails()
{
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, selection(), CPhraseEditNavigator::TTE_COMPLETE, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, selection(), CPhraseEditNavigator::TTE_COMPLETE, false));
	// TODO : Copy list of tags for multi-selection?
	CMimeHelper::addPhraseTagToMimeData(mime, selection().primarySelection());
	QApplication::clipboard()->setMimeData(mime);
	displayCopyCompleteToolTip();
}

template<class T, class U>
void CScriptureText<T,U>::copyVersesCommon(bool bPlainOnly)
{
	assert(m_pBibleDatabase.data() != NULL);

	QTextDocument docFormattedVerses;
	CPhraseNavigator navigator(m_pBibleDatabase, docFormattedVerses);
	if (haveSelection()) {
		// TODO : Fix this for range of selections
		navigator.setDocumentToFormattedVerses(selection().primarySelection());
	} else {
		TPhraseTag tagVerse = m_tagLast;
		if (tagVerse.relIndex().word() == 0) tagVerse.relIndex().setWord(1);
		navigator.setDocumentToFormattedVerses(tagVerse);
	}

	QMimeData *mime = new QMimeData();
	mime->setText(docFormattedVerses.toPlainText());
	if (!bPlainOnly) mime->setHtml(docFormattedVerses.toHtml());
	QApplication::clipboard()->setMimeData(mime);
	displayCopyCompleteToolTip();
}

template<class T, class U>
void CScriptureText<T,U>::displayCopyCompleteToolTip() const
{
	QPoint ptPos = T::mapToGlobal(m_ptLastTrackPosition);
	new CNotificationToolTip(1000, ptPos, QObject::tr("Text Copied to Clipboard", "MainMenu"), T::viewport());
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::en_highlightPassage(QAction *pAction)
{
	if (!U::hasFocus()) return;
	assert(parentCanOpener() != NULL);			// We should have a parentCanOpener or else we shouldn't have connected this slot yet
	assert(pAction != NULL);
	assert(g_pUserNotesDatabase.data() != NULL);

	// TODO : Enhance this to work with multiselection
	TPhraseTag tagSel = selection().primarySelection();
	CRelIndex relNdx = tagSel.relIndex();
	if (!relNdx.isSet()) return;

	if ((relNdx.chapter() == 0) &&
		(relNdx.verse() == 0) &&
		(relNdx.word() == 0)) {
		return;					// Don't allow highlighting entire book
	} else if ((relNdx.verse() == 0) &&
				(relNdx.word() == 0)) {
		// Allow highlighting entire chapter:
		tagSel = TPhraseTag(CRelIndex(relNdx.book(), relNdx.chapter(), 1, 1), m_pBibleDatabase->chapterEntry(relNdx)->m_nNumWrd);
	} else if ((relNdx.word() == 0) &&
				(relNdx.chapter() != 0)) {
		// Allow highlighting entire verse:
		tagSel = TPhraseTag(CRelIndex(relNdx.book(), relNdx.chapter(), relNdx.verse(), 1), m_pBibleDatabase->verseEntry(relNdx)->m_nNumWrd);
	}

	QString strHighlighterName = parentCanOpener()->highlighterButtons()->highlighter(pAction->data().toInt());
	if (strHighlighterName.isEmpty()) return;

	CBusyCursor iAmBusy(NULL);

	const TPhraseTagList *plstHighlighterTags = g_pUserNotesDatabase->highlighterTagsFor(m_pBibleDatabase, strHighlighterName);
	if ((plstHighlighterTags != NULL) && (plstHighlighterTags->completelyContains(m_pBibleDatabase.data(), tagSel))) {
		g_pUserNotesDatabase->removeHighlighterTagFor(m_pBibleDatabase, strHighlighterName, tagSel);
	} else {
		if (tagSel.haveSelection()) {
			g_pUserNotesDatabase->appendHighlighterTagFor(m_pBibleDatabase, strHighlighterName, tagSel);
		} else {
			// If we don't have a word selected, and there's no phrase to remove for it (above), go ahead and insert this word:
			g_pUserNotesDatabase->appendHighlighterTagFor(m_pBibleDatabase, strHighlighterName, TPhraseTag(tagSel.relIndex(), 1));
		}
	}
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::en_anchorClicked(const QUrl &link)
{
	QString strAnchor = link.toString();
	if (strAnchor.startsWith(QChar('N'))) {
		CRelIndex ndxLink(strAnchor.mid(1));
		assert(ndxLink.isSet());
		if (!ndxLink.isSet()) return;

		assert(g_pUserNotesDatabase.data() != NULL);
		assert(g_pUserNotesDatabase->existsNoteFor(ndxLink));
		if (!g_pUserNotesDatabase->existsNoteFor(ndxLink)) return;

		if ((ndxLink.chapter() == 0) &&
			(ndxLink.book() == m_ndxCurrent.book())) {
			if (m_ndxCurrent.chapter() == 1) {
				m_tagLastActive = TPhraseTag(CRelIndex(ndxLink.book(), 1, 0, 0));
			} else if (m_ndxCurrent.chapter() == m_pBibleDatabase->bookEntry(ndxLink.book())->m_nNumChp) {
				m_tagLastActive = TPhraseTag(ndxLink);
				m_tagLastActive.relIndex().setChapter(m_pBibleDatabase->bookEntry(ndxLink.book())->m_nNumChp);
				m_tagLastActive.relIndex().setVerse(m_pBibleDatabase->chapterEntry(m_tagLastActive.relIndex())->m_nNumVrs);
				m_tagLastActive.relIndex().setWord(m_pBibleDatabase->verseEntry(m_tagLastActive.relIndex())->m_nNumWrd);
			}
		}

		CUserNoteEntry userNote = g_pUserNotesDatabase->noteFor(ndxLink);
		userNote.setIsVisible(!userNote.isVisible());
		g_pUserNotesDatabase->setNoteFor(ndxLink, userNote);

		// Note: The Note change above will automatically trigger a rerender()
	} else if (strAnchor.startsWith(QChar('R'))) {
		CRelIndex ndxLink(strAnchor.mid(1));
		assert(ndxLink.isSet());
		if (!ndxLink.isSet()) return;

		emit T::gotoIndex(TPhraseTag(ndxLink));
	}
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::en_showAllNotes()
{
	assert(g_pUserNotesDatabase.data() != NULL);
	const CUserNoteEntryMap &mapNotes = g_pUserNotesDatabase->notesMap();
	for (CUserNoteEntryMap::const_iterator itrNotes = mapNotes.begin(); itrNotes != mapNotes.end(); ++itrNotes) {
		if (!itrNotes->second.isVisible()) {
			CUserNoteEntry userNote = g_pUserNotesDatabase->noteFor(itrNotes->first);
			userNote.setIsVisible(true);
			g_pUserNotesDatabase->setNoteFor(itrNotes->first, userNote);
		}
	}

	// Note: The changes above automatically triggers a rerender
}

template<class T, class U>
void CScriptureText<T,U>::en_hideAllNotes()
{
	assert(g_pUserNotesDatabase.data() != NULL);
	const CUserNoteEntryMap &mapNotes = g_pUserNotesDatabase->notesMap();
	for (CUserNoteEntryMap::const_iterator itrNotes = mapNotes.begin(); itrNotes != mapNotes.end(); ++itrNotes) {
		if (itrNotes->second.isVisible()) {
			CUserNoteEntry userNote = g_pUserNotesDatabase->noteFor(itrNotes->first);
			userNote.setIsVisible(false);
			g_pUserNotesDatabase->setNoteFor(itrNotes->first, userNote);
		}
	}

	// Note: The changes above automatically triggers a rerender
}

// ============================================================================

template class CScriptureText<i_CScriptureEdit, QTextEdit>;
template class CScriptureText<i_CScriptureBrowser, QTextBrowser>;

