/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include "DictionaryWidget.h"

#include "PersistentSettings.h"
#include "BusyCursor.h"

#include <QTextCursor>
#include <QTextCharFormat>
#include <QMenu>
#include <QAction>

#define DICTIONARY_COMPLETER_BUTTON_SIZE_Y 24

// ============================================================================

CDictionaryLineEdit::CDictionaryLineEdit(QWidget *pParent)
	:	CSingleLineTextEdit(DICTIONARY_COMPLETER_BUTTON_SIZE_Y, pParent),
		m_pCompleter(NULL),
		m_bUpdateInProgress(false)
{

}

CDictionaryLineEdit::~CDictionaryLineEdit()
{

}

void CDictionaryLineEdit::initialize(CDictionaryDatabasePtr pDictionary)
{
	assert(pDictionary != NULL);
	m_pDictionaryDatabase = pDictionary;

	setAcceptRichText(false);
	setUndoRedoEnabled(false);		// TODO : If we ever address what to do with undo/redo, then re-enable this

	setTabChangesFocus(true);
	setWordWrapMode(QTextOption::NoWrap);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(sizeHint().height());
	setLineWrapMode(QTextEdit::NoWrap);

	m_dlyUpdateCompleter.setMinimumDelay(10);				// Arbitrary time, but I think it must be less than our textChanged delay or we may have issues
	connect(&m_dlyUpdateCompleter, SIGNAL(triggered()), this, SLOT(delayed_UpdatedCompleter()));

	m_pCompleter = new SearchCompleter_t(m_pDictionaryDatabase, *this, this);
//	m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
	// TODO : ??? Add AccentSensitivity to completer ???

	m_pCompleter->setCompletionFilterMode(CPersistentSettings::instance()->dictionaryCompleterFilterMode());

	connect(m_pCompleter, SIGNAL(activated(const QModelIndex &)), this, SLOT(insertCompletion(const QModelIndex &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)), this, SLOT(en_changedDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)));
}

void CDictionaryLineEdit::en_changedDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode)
{
	m_pCompleter->setCompletionFilterMode(nMode);
}

void CDictionaryLineEdit::en_cursorPositionChanged()
{
	// Not calling base class because we don't need to update the completer on the
	//		cursor move (unlike the Search Phrases do)
}

void CDictionaryLineEdit::setupCompleter(const QString &strText, bool bForce)
{
	QString strWord = toPlainText();

	bool bCompleterOpen = m_pCompleter->popup()->isVisible();
	if ((bForce) || (!strText.isEmpty()) || (bCompleterOpen && (strWord.length() > 2) && (textCursor().atEnd()))) {
		delayed_UpdatedCompleter();			// Do an immediate update of the completer so we have values below (it speeds up the initial completer calculations!)
		m_pCompleter->popup()->close();
		if ((bCompleterOpen) && (strWord.length() > 2) && (textCursor().atEnd())) bForce = true;				// Reshow completer if it was open already and we're changing words
		m_pCompleter->selectFirstMatchString();
	}

	if (bForce || (!strText.isEmpty() && (strWord.length() > 2))) {
		CBusyCursor iAmBusy(NULL);
		m_pCompleter->complete();
	}
}

void CDictionaryLineEdit::processPendingUpdateCompleter()
{
	if (m_dlyUpdateCompleter.isTriggered()) delayed_UpdatedCompleter();
}

void CDictionaryLineEdit::UpdateCompleter()
{
	if (CPersistentSettings::instance()->dictionaryActivationDelay() == -1) {
		// Immediate activation if the dictionary activation delay is disabled:
		delayed_UpdatedCompleter();
	} else {
		m_dlyUpdateCompleter.trigger();
	}
}

void CDictionaryLineEdit::delayed_UpdatedCompleter()
{
	m_dlyUpdateCompleter.untrigger();

	m_pCompleter->setFilterMatchString();
	m_pCompleter->setWordsFromPhrase();

	if (updateInProgress()) return;
	CDoUpdate doUpdate(this);

	QTextCursor cursor(textCursor());
	cursor.beginEditBlock();

	QTextCharFormat fmt = cursor.charFormat();
	fmt.setFontStrikeOut(false);
	fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.setCharFormat(fmt);

	if (!m_pDictionaryDatabase->wordExists(toPlainText().trimmed())) {
		fmt.setFontStrikeOut(true);
		fmt.setUnderlineColor(QColor(255,0,0));
		fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
		cursor.setCharFormat(fmt);
	}

	cursor.endEditBlock();
}

void CDictionaryLineEdit::insertCompletion(const QModelIndex &index)
{
	insertCompletion(m_pCompleter->completionModel()->data(index, Qt::DisplayRole).toString());
}

void CDictionaryLineEdit::insertCompletion(const QString &strWord)
{
	QTextCursor cursor(textCursor());
	cursor.beginEditBlock();
	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.insertText(strWord);
	cursor.endEditBlock();
}

// ============================================================================

CDictionaryWidget::CDictionaryWidget(CDictionaryDatabasePtr pDictionary, QWidget *parent)
	:	QWidget(parent),
		m_pDictionaryDatabase(pDictionary),
		m_bDoingPopup(false),
		m_bDoingUpdate(false)
{
	assert(m_pDictionaryDatabase != NULL);

	ui.setupUi(this);

	ui.editDictionaryWord->initialize(m_pDictionaryDatabase);

	ui.definitionBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.definitionBrowser, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_definitionBrowserContextMenuRequested(const QPoint &)));

	setDictionaryActivationDelay(CPersistentSettings::instance()->dictionaryActivationDelay());
	connect(ui.editDictionaryWord, SIGNAL(textChanged()), &m_dlyTextChanged, SLOT(trigger()));
	connect(&m_dlyTextChanged, SIGNAL(triggered()), this, SLOT(en_wordChanged()));
	connect(CPersistentSettings::instance(), SIGNAL(changedDictionaryActivationDelay(int)), this, SLOT(setDictionaryActivationDelay(int)));

	setFont(CPersistentSettings::instance()->fontDictionary());
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedDictionary(const QFont &)), this, SLOT(setFont(const QFont &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));

	connect(ui.buttonClearWord, SIGNAL(clicked()), ui.editDictionaryWord, SLOT(clear()));

	connect(ui.definitionBrowser, SIGNAL(anchorClicked(const QUrl &)), this, SLOT(en_anchorClicked(const QUrl &)));
	connect(ui.buttonHistoryBack, SIGNAL(clicked()), ui.definitionBrowser, SLOT(backward()));
	connect(ui.definitionBrowser, SIGNAL(backwardAvailable(bool)), ui.buttonHistoryBack, SLOT(setEnabled(bool)));
	connect(ui.buttonHistoryForward, SIGNAL(clicked()), ui.definitionBrowser, SLOT(forward()));
	connect(ui.definitionBrowser, SIGNAL(forwardAvailable(bool)), ui.buttonHistoryForward, SLOT(setEnabled(bool)));
	connect(ui.definitionBrowser, SIGNAL(sourceChanged(const QUrl &)), this, SLOT(en_sourceChanged(const QUrl &)));
}

CDictionaryWidget::~CDictionaryWidget()
{

}

void CDictionaryWidget::setWord(const QString &strWord)
{
	ui.editDictionaryWord->processPendingUpdateCompleter();
	ui.editDictionaryWord->insertCompletion(strWord);
}

void CDictionaryWidget::en_wordChanged()
{
	QString strWord = ui.editDictionaryWord->toPlainText().trimmed();
	ui.definitionBrowser->setHtml(m_pDictionaryDatabase->definition(strWord));
	if (m_pDictionaryDatabase->wordExists(strWord)) {
		m_bDoingUpdate = true;
		ui.definitionBrowser->setSource(QUrl(QString("#%1").arg(strWord)));
		m_bDoingUpdate = false;
	}
}

void CDictionaryWidget::en_sourceChanged(const QUrl &src)
{
	if (m_bDoingUpdate) return;

	QString strURL = src.toString();		// Internal URLs are in the form of "#nnnnnnnn" as anchors
	int nPos = strURL.indexOf('#');
	if (nPos > -1) setWord(strURL.mid(nPos+1));
}

void CDictionaryWidget::en_anchorClicked(const QUrl &link)
{
	// Incoming URL Format:  dict/Web-1828://Word

	QString strAnchor = link.toString();

	// Convert to:  dict://Word
	int ndxColon = strAnchor.indexOf(':');
	if (ndxColon == -1) return;
	int ndxSlash = strAnchor.left(ndxColon).indexOf('/');
	if (ndxSlash != -1) strAnchor.remove(ndxSlash, ndxColon-ndxSlash);

	QUrl urlResolved(strAnchor);

	// Scheme = "dict"
	// Host = Word

	if (urlResolved.scheme().compare("dict", Qt::CaseInsensitive) == 0) {
		setWord(urlResolved.host());
	}
}

void CDictionaryWidget::en_definitionBrowserContextMenuRequested(const QPoint &pos)
{
	bool bPopupSave = m_bDoingPopup;
	m_bDoingPopup = true;

	QMenu menu;
	QAction *pAction;

	pAction = menu.addAction(tr("&Copy"), ui.definitionBrowser, SLOT(copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	pAction->setEnabled(ui.definitionBrowser->textCursor().hasSelection());
	menu.addSeparator();
	pAction = menu.addAction(tr("Select All"), ui.definitionBrowser, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	pAction->setEnabled(!ui.definitionBrowser->document()->isEmpty());
	menu.exec(ui.definitionBrowser->viewport()->mapToGlobal(pos));

	m_bDoingPopup = bPopupSave;
}

void CDictionaryWidget::setFont(const QFont& aFont)
{
	ui.definitionBrowser->document()->setDefaultFont(aFont);
}

void CDictionaryWidget::setTextBrightness(bool bInvert, int nBrightness)
{
	setStyleSheet(QString("CDictionaryLineEdit, QTextBrowser { background-color:%1; color:%2; }")
								   .arg(CPersistentSettings::textBackgroundColor(bInvert, nBrightness).name())
								   .arg(CPersistentSettings::textForegroundColor(bInvert, nBrightness).name()));
}

// ============================================================================
