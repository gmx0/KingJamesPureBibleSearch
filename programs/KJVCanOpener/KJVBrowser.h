#ifndef KJVBROWSER_H
#define KJVBROWSER_H

#include "dbstruct.h"
#include "Highlighter.h"
#include "PhraseEdit.h"
#include "ScriptureEdit.h"

#include <QWidget>
#include <QTextBrowser>
#include <QColor>
#include <QTimer>

// ============================================================================

namespace Ui {
class CKJVBrowser;
}

class CKJVBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVBrowser(QWidget *parent = 0);
	virtual ~CKJVBrowser();

	void Initialize(const TPhraseTag &nInitialIndex = TPhraseTag(CRelIndex(1,1,0,0)));		// Default initial location is Genesis 1

	CScriptureBrowser *browser();

public slots:
	void gotoIndex(const TPhraseTag &tag);
	void focusBrowser();
	void setHighlightTags(const TPhraseTagList &lstPhraseTags);

	// Navigation Shortcut Processing:
	void on_Bible_Beginning();
	void on_Bible_Ending();
	void on_Book_Backward();
	void on_Book_Forward();
	void on_ChapterBackward();
	void on_ChapterForward();

signals:
	void IndexChanged(const TPhraseTag &tag);

private slots:
	void on_sourceChanged(const QUrl &src);

	void BkComboIndexChanged(int index);
	void BkChpComboIndexChanged(int index);
	void TstBkComboIndexChanged(int index);
	void TstChpComboIndexChanged(int index);
	void BibleBkComboIndexChanged(int index);
	void BibleChpComboIndexChanged(int index);

private:
	void gotoIndex2(const TPhraseTag &tag);
	void doHighlighting(bool bClear = false);		// Highlight the areas marked in the PhraseTags.  If bClear=True, removes the highlighting, which is used to swapout the current tag list for a new one without redrawing everything

	// These should be used in order:
	void setBook(const CRelIndex &ndx);		// Updates BkChp list, sets lblTestament, updates TstBk and TstChp lists
	void setChapter(const CRelIndex &ndx);	// Fills in the main browser text for the desired chapter
	void setVerse(const CRelIndex &ndx);	// Scrolls browser to the specified verse for the current Bk/Tst/Chp, etc.
	void setWord(const TPhraseTag &tag);	// Scrolls browser to the specified word for the current Bk/Tst/Chp/Vrs, etc.  And selects the number of words specified

// Data Private:
private:
	CRelIndex m_ndxCurrent;
	CSearchResultHighlighter m_Highlighter;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()							\
			bool bUpdateSave = m_bDoingUpdate;	\
			m_bDoingUpdate = true;
#define end_update()							\
			m_bDoingUpdate = bUpdateSave;


	Ui::CKJVBrowser *ui;
};

#endif // KJVBROWSER_H
