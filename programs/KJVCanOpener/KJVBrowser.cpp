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

#include "KJVBrowser.h"
#include "ui_KJVBrowser.h"

#include <assert.h>

#include <QComboBox>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>


// ============================================================================

CKJVBrowser::CKJVBrowser(CBibleDatabasePtr pBibleDatabase, QWidget *parent) :
	QWidget(parent),
	m_pBibleDatabase(pBibleDatabase),
	m_ndxCurrent(0),
	m_bDoingUpdate(false),
	m_pScriptureBrowser(NULL),
	ui(new Ui::CKJVBrowser)
{
	assert(m_pBibleDatabase.data() != NULL);

	ui->setupUi(this);

	initialize();

	assert(m_pScriptureBrowser != NULL);

// UI Connections:
	connect(m_pScriptureBrowser, SIGNAL(gotoIndex(const TPhraseTag &)), this, SLOT(gotoIndex(const TPhraseTag &)));
	connect(m_pScriptureBrowser, SIGNAL(sourceChanged(const QUrl &)), this, SLOT(on_sourceChanged(const QUrl &)));

	connect(ui->comboBk, SIGNAL(currentIndexChanged(int)), this, SLOT(BkComboIndexChanged(int)));
	connect(ui->comboBkChp, SIGNAL(currentIndexChanged(int)), this, SLOT(BkChpComboIndexChanged(int)));
	connect(ui->comboTstBk, SIGNAL(currentIndexChanged(int)), this, SLOT(TstBkComboIndexChanged(int)));
	connect(ui->comboTstChp, SIGNAL(currentIndexChanged(int)), this, SLOT(TstChpComboIndexChanged(int)));
	connect(ui->comboBibleBk, SIGNAL(currentIndexChanged(int)), this, SLOT(BibleBkComboIndexChanged(int)));
	connect(ui->comboBibleChp, SIGNAL(currentIndexChanged(int)), this, SLOT(BibleChpComboIndexChanged(int)));

	// Set Outgoing Pass-Through Signals:
	connect(m_pScriptureBrowser, SIGNAL(activatedScriptureText()), this, SIGNAL(activatedScriptureText()));
	connect(m_pScriptureBrowser, SIGNAL(backwardAvailable(bool)), this, SIGNAL(backwardAvailable(bool)));
	connect(m_pScriptureBrowser, SIGNAL(forwardAvailable(bool)), this, SIGNAL(forwardAvailable(bool)));
	connect(m_pScriptureBrowser, SIGNAL(historyChanged()), this, SIGNAL(historyChanged()));

	// Set Incoming Pass-Through Signals:
	connect(this, SIGNAL(backward()), m_pScriptureBrowser, SLOT(backward()));
	connect(this, SIGNAL(forward()), m_pScriptureBrowser, SLOT(forward()));
	connect(this, SIGNAL(home()), m_pScriptureBrowser, SLOT(home()));
	connect(this, SIGNAL(reload()), m_pScriptureBrowser, SLOT(reload()));
}

CKJVBrowser::~CKJVBrowser()
{
	delete ui;
}

// ----------------------------------------------------------------------------

void CKJVBrowser::initialize()
{
	// --------------------------------------------------------------

	//	Swapout the widgetKJVPassageNavigator from the layout with
	//		one that we can set the database on:

	int ndx = ui->gridLayout->indexOf(ui->textBrowserMainText);
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;
	ui->gridLayout->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pScriptureBrowser = new CScriptureBrowser(m_pBibleDatabase, this);
	m_pScriptureBrowser->setObjectName(QString::fromUtf8("textBrowserMainText"));
	m_pScriptureBrowser->setMouseTracking(true);
	m_pScriptureBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_pScriptureBrowser->setTabChangesFocus(false);
	m_pScriptureBrowser->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
	m_pScriptureBrowser->setOpenLinks(false);

	delete ui->textBrowserMainText;
	ui->textBrowserMainText = NULL;
	ui->gridLayout->addWidget(m_pScriptureBrowser, nRow, nCol, nRowSpan, nColSpan);

	// Reinsert it in the correct TabOrder:
	QWidget::setTabOrder(ui->comboBkChp, m_pScriptureBrowser);
	QWidget::setTabOrder(m_pScriptureBrowser, ui->comboTstBk);

	// --------------------------------------------------------------

	begin_update();

	unsigned int nBibleChp = 0;
	ui->comboBk->clear();
	ui->comboBibleBk->clear();
	for (unsigned int ndxBk=1; ndxBk<=m_pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxBk);
		ui->comboBk->addItem(pBook->m_strBkName, ndxBk);
		ui->comboBibleBk->addItem(QString("%1").arg(ndxBk), ndxBk);
		nBibleChp += pBook->m_nNumChp;
	}
	ui->comboBibleChp->clear();
	for (unsigned int ndxBibleChp=1; ndxBibleChp<=nBibleChp; ++ndxBibleChp) {
		ui->comboBibleChp->addItem(QString("%1").arg(ndxBibleChp), ndxBibleChp);
	}

	end_update();
}

void CKJVBrowser::gotoIndex(const TPhraseTag &tag)
{
	TPhraseTag tagActual = tag;

	begin_update();

	// If branching to a "book only", goto chapter 1 of that book:
	if ((tagActual.first.book() != 0) &&
		(tagActual.first.chapter() == 0)) tagActual.first.setChapter(1);

	m_pScriptureBrowser->setSource(QString("#%1").arg(tagActual.first.asAnchor()));

	end_update();

	gotoIndex2(tagActual);
}

void CKJVBrowser::gotoIndex2(const TPhraseTag &tag)
{
	setBook(tag.first);
	setChapter(tag.first);
	setVerse(tag.first);
	setWord(tag);

	doHighlighting();

	emit on_gotoIndex(tag);
}

void CKJVBrowser::on_sourceChanged(const QUrl &src)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	QString strURL = src.toString();		// Internal URLs are in the form of "#nnnnnnnn" as anchors
	int nPos = strURL.indexOf('#');
	if (nPos > -1) {
		CRelIndex ndxRel(strURL.mid(nPos+1));
		if (ndxRel.isSet()) gotoIndex2(TPhraseTag(ndxRel));
	}
}

void CKJVBrowser::setFocusBrowser()
{
	m_pScriptureBrowser->setFocus();
}

bool CKJVBrowser::hasFocusBrowser() const
{
	return m_pScriptureBrowser->hasFocus();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setHighlightTags(const TPhraseTagList &lstPhraseTags)
{
	doHighlighting(true);				// Remove existing highlighting
	m_Highlighter.setHighlightTags(lstPhraseTags);
	doHighlighting();					// Highlight using new tags
}

void CKJVBrowser::doHighlighting(bool bClear)
{
	m_pScriptureBrowser->navigator().doHighlighting(m_Highlighter, bClear, m_ndxCurrent);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setFontScriptureBrowser(const QFont& aFont)
{
	m_pScriptureBrowser->setFont(aFont);
}

void CKJVBrowser::showDetails()
{
	m_pScriptureBrowser->showDetails();
}

void CKJVBrowser::showPassageNavigator()
{
	m_pScriptureBrowser->showPassageNavigator();
}

// ----------------------------------------------------------------------------

void CKJVBrowser::on_Bible_Beginning()
{
	assert(m_pBibleDatabase.data() != NULL);

	gotoIndex(TPhraseTag(CRelIndex(1,1,1,1)));
}

void CKJVBrowser::on_Bible_Ending()
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndx;
	ndx.setBook(m_pBibleDatabase->bibleEntry().m_nNumBk);
	ndx.setChapter(m_pBibleDatabase->bookEntry(ndx.book())->m_nNumChp);
	ndx.setVerse(m_pBibleDatabase->chapterEntry(ndx)->m_nNumVrs);
	ndx.setWord(m_pBibleDatabase->verseEntry(ndx)->m_nNumWrd);
	gotoIndex(TPhraseTag(ndx));
}

void CKJVBrowser::on_Book_Backward()
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_ndxCurrent.book() < 2) return;

	gotoIndex(TPhraseTag(CRelIndex(m_ndxCurrent.book()-1, 1, 1, 1)));
}

void CKJVBrowser::on_Book_Forward()
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_ndxCurrent.book() >= m_pBibleDatabase->bibleEntry().m_nNumBk) return;

	gotoIndex(TPhraseTag(CRelIndex(m_ndxCurrent.book()+1, 1, 1, 1)));
}

void CKJVBrowser::on_ChapterBackward()
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndx = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1), true);
	if (ndx.isSet()) gotoIndex(TPhraseTag(ndx));
}

void CKJVBrowser::on_ChapterForward()
{
	assert(m_pBibleDatabase.data() != NULL);

	CRelIndex ndx = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 1), false);
	if (ndx.isSet()) gotoIndex(TPhraseTag(ndx));
}

// ----------------------------------------------------------------------------

void CKJVBrowser::setBook(const CRelIndex &ndx)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (ndx.book() == 0) return;
	if (ndx.book() == m_ndxCurrent.book()) return;

	begin_update();

	m_ndxCurrent.setIndex(ndx.book(), 0, 0, 0);

	if (m_ndxCurrent.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		end_update();
		return;
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(m_ndxCurrent.book());

	ui->comboBk->setCurrentIndex(ui->comboBk->findData(m_ndxCurrent.book()));
	ui->comboBibleBk->setCurrentIndex(ui->comboBibleBk->findData(m_ndxCurrent.book()));

	unsigned int nTst = book.m_nTstNdx;
	ui->lblTestament->setText(m_pBibleDatabase->testamentEntry(nTst)->m_strTstName + ":");
	ui->comboTstBk->clear();
	for (unsigned int ndxTstBk=1; ndxTstBk<=m_pBibleDatabase->testamentEntry(nTst)->m_nNumBk; ++ndxTstBk) {
		ui->comboTstBk->addItem(QString("%1").arg(ndxTstBk), ndxTstBk);
	}
	ui->comboTstBk->setCurrentIndex(ui->comboTstBk->findData(book.m_nTstBkNdx));

	ui->comboBkChp->clear();
	for (unsigned int ndxBkChp=1; ndxBkChp<=book.m_nNumChp; ++ndxBkChp) {
		ui->comboBkChp->addItem(QString("%1").arg(ndxBkChp), ndxBkChp);
	}
	ui->comboTstChp->clear();
	for (unsigned int ndxTstChp=1; ndxTstChp<=m_pBibleDatabase->testamentEntry(nTst)->m_nNumChp; ++ndxTstChp) {
		ui->comboTstChp->addItem(QString("%1").arg(ndxTstChp), ndxTstChp);
	}

	end_update();
}

void CKJVBrowser::setChapter(const CRelIndex &ndx)
{
	assert(m_pBibleDatabase.data() != NULL);

	begin_update();

	m_ndxCurrent.setIndex(m_ndxCurrent.book(), ndx.chapter(), 0, 0);

	ui->comboBkChp->setCurrentIndex(ui->comboBkChp->findData(ndx.chapter()));

	if ((m_ndxCurrent.book() == 0) || (m_ndxCurrent.chapter() == 0)) {
		m_pScriptureBrowser->clear();
		end_update();
		return;
	}

	if (m_ndxCurrent.book() > m_pBibleDatabase->bibleEntry().m_nNumBk) {
		assert(false);
		end_update();
		return;
	}

	const CBookEntry &book = *m_pBibleDatabase->bookEntry(m_ndxCurrent.book());
	unsigned int nTstChp = 0;
	unsigned int nBibleChp = 0;
	for (unsigned int ndxBk=1; ndxBk<m_ndxCurrent.book(); ++ndxBk) {
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxBk);
		if (pBook->m_nTstNdx == book.m_nTstNdx)
			nTstChp += pBook->m_nNumChp;
		nBibleChp += pBook->m_nNumChp;
	}
	nTstChp += m_ndxCurrent.chapter();
	nBibleChp += m_ndxCurrent.chapter();

	ui->comboTstChp->setCurrentIndex(ui->comboTstChp->findData(nTstChp));
	ui->comboBibleChp->setCurrentIndex(ui->comboBibleChp->findData(nBibleChp));

	end_update();

	m_pScriptureBrowser->navigator().setDocumentToChapter(ndx);
}

void CKJVBrowser::setVerse(const CRelIndex &ndx)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), ndx.verse(), 0);

}

void CKJVBrowser::setWord(const TPhraseTag &tag)
{
	m_ndxCurrent.setIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), tag.first.word());
	m_pScriptureBrowser->navigator().selectWords(tag);
}

// ----------------------------------------------------------------------------

void CKJVBrowser::BkComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui->comboBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::BkChpComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	ndxTarget.setBook(m_ndxCurrent.book());
	if (index != -1) {
		ndxTarget.setChapter(ui->comboBkChp->itemData(index).toUInt());
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::TstBkComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get BookEntry for current book so we know what testament we're currently in:
		const CBookEntry &book = *m_pBibleDatabase->bookEntry(m_ndxCurrent.book());
		ndxTarget = m_pBibleDatabase->calcRelIndex(0, 0, 0, ui->comboTstBk->itemData(index).toUInt(), book.m_nTstNdx);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::TstChpComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if ((index != -1) && (m_ndxCurrent.book() > 0)) {
		// Get BookEntry for current book so we know what testament we're currently in:
		const CBookEntry &book = *m_pBibleDatabase->bookEntry(m_ndxCurrent.book());
		ndxTarget = m_pBibleDatabase->calcRelIndex(0, 0, ui->comboTstChp->itemData(index).toUInt(), 0, book.m_nTstNdx);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::BibleBkComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget.setBook(ui->comboBibleBk->itemData(index).toUInt());
		ndxTarget.setChapter(1);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

void CKJVBrowser::BibleChpComboIndexChanged(int index)
{
	assert(m_pBibleDatabase.data() != NULL);

	if (m_bDoingUpdate) return;

	CRelIndex ndxTarget;
	if (index != -1) {
		ndxTarget = m_pBibleDatabase->calcRelIndex(0, 0, ui->comboBibleChp->itemData(index).toUInt(), 0, 0);
		ndxTarget.setVerse(0);
		ndxTarget.setWord(0);
	}
	gotoIndex(TPhraseTag(ndxTarget));
}

// ----------------------------------------------------------------------------

