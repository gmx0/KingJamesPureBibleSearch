#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include "dbstruct.h"
#include "KJVSearchPhraseEdit.h"
#include "SearchPhraseListModel.h"
#include "KJVSearchCriteria.h"
#include "VerseListModel.h"

#include <QMainWindow>
#include <QModelIndex>
#include <QScrollArea>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QFocusEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QString>
#include <QTreeView>
#include <QVBoxLayout>
#include <QSettings>

#include <assert.h>

// ============================================================================

class CSearchPhraseScrollArea : public QScrollArea
{
public:
	CSearchPhraseScrollArea( QWidget *parent=NULL)
		: QScrollArea(parent)
	{ }
	virtual ~CSearchPhraseScrollArea() { }

	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
};

// ============================================================================

class CSearchResultsTreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit CSearchResultsTreeView(QWidget *parent = 0);
	virtual ~CSearchResultsTreeView();

	QMenu *getEditMenu() { return m_pEditMenu; }
	QMenu *getLocalEditMenu() { return m_pEditMenuLocal; }

public slots:
	void on_copyVerseText();
	void on_copyRaw();
	void on_copyVeryRaw();
	void on_copyVerseHeadings();
	void on_copyReferenceDetails();
	void on_copyComplete();
	void on_passageNavigator();

	void on_listChanged();

signals:
	void activatedSearchResults();
	void gotoIndex(const TPhraseTag &tag);
	void canExpandAll(bool bEnable);
	void canCollapseAll(bool bEnable);

protected:
	virtual void focusInEvent(QFocusEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);

	void copyRawCommon(bool bVeryRaw) const;
	void handle_selectionChanged();

	virtual void resizeEvent(QResizeEvent *event);

private:
	bool m_bDoingPopup;				// True if popping up a menu or dialog and we don't want the highlight to disable
	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	QMenu *m_pEditMenuLocal;		// Edit menu for local popup when user right-clicks -- like above but includes view toggles
	// ----
	QAction *m_pActionCopyVerseText;			// Edit menu copy text
	QAction *m_pActionCopyRaw;		// Edit menu copy raw phrase text
	QAction *m_pActionCopyVeryRaw;	// Edit menu copy very (no punctuation) raw phrase text
	// ----
	QAction *m_pActionCopyVerseHeadings;		// Edit menu copy headings
	QAction *m_pActionCopyReferenceDetails;		// Edit menu Reference ToolTip Copy
	QAction *m_pActionCopyComplete;				// Edit menu copy everything
	// ----
	QAction *m_pActionSelectAll;	// Edit menu select all
	QAction *m_pActionClearSelection;	// Edit menu clear selection
	// ----
	QAction *m_pActionNavigator;	// Launch Passage Navigator for Search Result
	// ----
	QAction *m_pStatusAction;		// Used to update the status bar without an enter/leave sequence
};


// ============================================================================

namespace Ui {
class CKJVCanOpener;
}

class CKJVCanOpener : public QMainWindow
{
	Q_OBJECT

public:
	explicit CKJVCanOpener(const QString &strUserDatabase = QString(), QWidget *parent = 0);
	~CKJVCanOpener();

	void Initialize();

protected:
	void savePersistentSettings();
	void restorePersistentSettings();
	virtual void closeEvent(QCloseEvent * event);
	bool haveUserDatabase() const { return !m_strUserDatabase.isEmpty(); }

signals:
	void changedSearchResults();

public slots:
	bool openKJVSearchFile(const QString &strFilePathName);
	bool saveKJVSearchFile(const QString &strFilePathName) const;

	void setDisplayMode(CVerseListModel::VERSE_DISPLAY_MODE_ENUM nDisplayMode);
	void setTreeMode(CVerseListModel::VERSE_TREE_MODE_ENUM nTreeMode);
	void setShowMissingLeafs(bool bShowMissing);

protected:
	void readKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup = QString());
	void writeKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup = QString()) const;
	QString groupCombine(const QString &strSubgroup, const QString &strGroup) const;

protected slots:
	void on_NewSearch();
	void on_OpenSearch();
	void on_SaveSearch();

	void closeAllSearchPhrases();

	CKJVSearchPhraseEdit *addSearchPhrase();
	void on_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase);
	void on_changedSearchCriteria();
	void on_copySearchPhraseSummary();

	void on_addPassageBrowserEditMenu(bool bAdd);
	void on_addSearchResultsEditMenu(bool bAdd);
	void on_addSearchPhraseEditMenu(bool bAdd, const CPhraseLineEdit *pEditor = NULL);
	void on_activatedBrowser();
	void on_activatedSearchResults();
	void on_activatedPhraseEditor(const CPhraseLineEdit *pEditor);

	void on_viewVerseHeading();
	void on_viewVerseRichText();

	void on_viewAsList();
	void on_viewAsTreeBooks();
	void on_viewAsTreeChapters();
	void on_viewShowMissingsLeafs();

	bool setCurrentIndex(const CRelIndex &ndxCurrent, bool bFocusTreeView = true);

	void on_indexChanged(const TPhraseTag &tag);

	void on_browserHistoryChanged();
	void on_clearBrowserHistory();
	void on_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase);
	void on_SearchResultActivated(const QModelIndex &index);		// Enter or double-click activated

	void on_PassageNavigatorTriggered();

	void on_HelpManual();
	void on_HelpAbout();

// Data Private:
private:
	QString m_strUserDatabase;

// UI Private:
private:
	bool m_bDoingUpdate;
	QAction *m_pActionPassageBrowserEditMenu;		// Edit Menu from Passage Browser when active
	QAction *m_pActionSearchResultsEditMenu;		// Edit Menu from Search Results when active
	QAction *m_pActionSearchPhraseEditMenu;			// Edit Menu from Search Phrase when active
	QMenu *m_pViewMenu;						// View Menu, used for insertion reference for edit menu
	QAction *m_pActionShowVerseHeading;		// Toggle action to show verse heading only
	QAction *m_pActionShowVerseRichText;	// Toggle action to show verse richtext
	QAction *m_pActionShowAsList;			// Toggle action to show in list mode
	QAction *m_pActionShowAsTreeBooks;		// Toggle action to show in tree books mode
	QAction *m_pActionShowAsTreeChapters;	// Toggle action to show in tree chapters mode
	QAction *m_pActionShowMissingLeafs;		// Toggle action for tree modes to show missing leafs
	QAction *m_pActionExpandAll;	// View menu Expand All
	QAction *m_pActionCollapseAll;	// View menu Collapse All
	QAction *m_pActionBookBackward;	// Navigate Book Backward
	QAction *m_pActionBookForward;	// Navigate Book Forward
	QAction *m_pActionChapterBackward;	// Navigate Chapter Backward
	QAction *m_pActionChapterForward;	// Navigate Chapter Forward
	QAction *m_pActionNavBackward;	// Browser Navigate Backward
	QAction *m_pActionNavForward;	// Browser Navigate Forward
	QAction *m_pActionNavHome;		// Browser Navigate to History Home
	QAction *m_pActionNavClear;		// Clear Navigation History
	QAction *m_pActionJump;			// Jump to passage via Passage Navigator
	QAction *m_pActionAbout;		// About Application

	bool m_bBrowserActive;
	bool m_bSearchResultsActive;
	bool m_bPhraseEditorActive;

	QVBoxLayout *m_pLayoutPhrases;
//	CSearchPhraseListModel m_modelSearchPhraseEditors;
	CSearchPhraseEditList m_lstSearchPhraseEditors;

	int m_nLastSearchOccurrences;		// Last search summary of 'n' occurrences in 'x' verses in 'y' chapters in 'z' books
	int m_nLastSearchVerses;
	int m_nLastSearchChapters;
	int m_nLastSearchBooks;
	bool m_bLastCalcSuccess;

	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
