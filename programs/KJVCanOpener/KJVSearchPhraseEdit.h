#ifndef KJVSEARCHPHRASEEDIT_H
#define KJVSEARCHPHRASEEDIT_H

#include "dbstruct.h"

#include <QWidget>
#include <QTextEdit>
#include <QTextCursor>
#include <QKeyEvent>
#include <QCompleter>
#include <QStringList>

#include <QStatusBar>
#include <utility>

class CParsedPhrase
{
public:
	CParsedPhrase()
		:	m_nLevel(0),
			m_nCursorLevel(0),
			m_nCursorWord(-1),
			m_nLastMatchWord(-1)
	{ }
	~CParsedPhrase()
	{ }

	TIndexList GetNormalizedSearchResults() const;
	uint32_t GetMatchLevel() const;
	uint32_t GetCursorMatchLevel() const;

	virtual void ParsePhrase(const QTextCursor &curInsert);		// Parses the phrase in the editor.  Sets m_lstWords and m_nCursorWord
	QString GetCursorWord() const;
	int GetCursorWordPos() const;

protected:
	void UpdateCompleter(const QTextCursor &curInsert, QCompleter &aCompleter);
	QTextCursor insertCompletion(const QTextCursor &curInsert, const QString& completion);

private:
	void FindWords();			// Uses m_lstWords and m_nCursorWord to populate m_lstNextWords, m_lstMapping, and m_nLevel

protected:
	uint32_t m_nLevel;			// Level of the search (Number of words matched).  This is the offset value for entries in m_lstMatchMapping (at 0 mapping is ALL words) (Set by FindWords())
	TIndexList m_lstMatchMapping;	// Mapping for entire search -- This is the search result, but with each entry offset by the search level (Set by FindWords())
	uint32_t m_nCursorLevel;	// Matching level at cursor
	TIndexList m_lstMapping;	// Mapping for search through current cursor (Set by FindWords())
	QStringList m_lstNextWords;	// List of words mapping next for this phrase (Set by FindWords())

	QStringList m_lstWords;		// Fully Parsed Word list.  Blank entries only at first or last entry to indicate an insertion point. (Filled by ParsePhrase())
	int m_nCursorWord;			// Index in m_lstWords where the cursor is at -- If insertion point is in the middle of two words, Cursor will be at the left word (Set by ParsePhrase())
	int m_nLastMatchWord;		// Index in m_lstWords where the last match was found (Set by FindWords())

	QStringList m_lstLeftWords;		// Raw Left-hand Words list from extraction.  Punctionation appears clustered in separate entities (Set by ParsePhrase())
	QStringList m_lstRightWords;	// Raw Right-hand Words list from extraction.  Punctionation appears clustered in separate entities (Set by ParsePhrase())
	QString m_strCursorWord;	// Word at the cursor point between the left and right hand halves (Set by ParsePhrase())
};

// ============================================================================

class CPhraseCursor : public QTextCursor
{
public:
	CPhraseCursor(const QTextCursor &aCursor);
	virtual ~CPhraseCursor();

	bool moveCursorCharLeft(MoveMode mode = MoveAnchor);
	bool moveCursorCharRight(MoveMode mode = MoveAnchor);
	QChar charUnderCursor();

	bool moveCursorWordLeft(MoveMode mode = MoveAnchor);
	bool moveCursorWordRight(MoveMode mode = MoveAnchor);
	bool moveCursorWordStart(MoveMode mode = MoveAnchor);
	bool moveCursorWordEnd(MoveMode mode = MoveAnchor);
	QString wordUnderCursor();

	void selectWordUnderCursor();
	void selectCursorToLineStart();
	void selectCursorToLineEnd();
};


// ============================================================================

class CPhraseLineEdit : public QTextEdit, CParsedPhrase
{
	Q_OBJECT

public:
	CPhraseLineEdit(QWidget *pParent = 0);



private slots:
	void insertCompletion(const QString &completion);
	void on_textChanged();
	void on_cursorPositionChanged();

protected:
//	bool eventFilter(QObject *obj, QEvent *event);

	void UpdateCompleter();

	// TODO : Remove this and set parent to non-virtual after done debugging!
	virtual void ParsePhrase(const QTextCursor &curInsert);

private:
	void keyPressEvent(QKeyEvent* event);
	QString textUnderCursor() const;

// Data Private:
private:
	QCompleter *m_pCompleter;
	int m_nLastCursorWord;		// Used to dismiss and redisplay the popup for resizing
	bool m_bUpdateInProgress;	// Completer update in progress (to guard against re-entrance)
};

// ============================================================================

namespace Ui {
class CKJVSearchPhraseEdit;
}

class CKJVSearchPhraseEdit : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVSearchPhraseEdit(QWidget *parent = 0);
	~CKJVSearchPhraseEdit();

QStatusBar *pStatusBar;

/*
private slots:
	void on_textEdited(const QString &text);
	void insertCompletion(const QString &completion);

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private:
//	void keyPressEvent(QKeyEvent* event);

	QString textUnderCursor() const;

// Data Private:
private:
	QCompleter *m_pCompleter;
*/

// UI Private:
private:
	Ui::CKJVSearchPhraseEdit *ui;
};

#endif // KJVSEARCHPHRASEEDIT_H