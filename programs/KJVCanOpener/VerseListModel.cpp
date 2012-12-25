#include "VerseListModel.h"

#include <QVector>
#include <QModelIndexList>

CVerseListModel::CVerseListModel(QObject *parent)
	:	QAbstractListModel(parent),
		m_nSearchScopeMode(CKJVSearchCriteria::SSME_WHOLE_BIBLE),
		m_nDisplayMode(VDME_HEADING)
{
}

CVerseListModel::CVerseListModel(const CVerseList &verses, VERSE_DISPLAY_MODE_ENUM nDisplayMode, QObject *parent)
	:	QAbstractListModel(parent),
		m_lstVerses(verses),
		m_nSearchScopeMode(CKJVSearchCriteria::SSME_WHOLE_BIBLE),
		m_nDisplayMode(nDisplayMode)
{
}

int CVerseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return (hasExceededDisplayLimit() ? 0 : m_lstVerses.count());
}

QVariant CVerseListModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_lstVerses.size())
		return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		switch (m_nDisplayMode) {
			case VDME_HEADING:
				return m_lstVerses.at(index.row()).getHeading();
			case VDME_VERYPLAIN:
				return m_lstVerses.at(index.row()).getVerseVeryPlainText();
			case VDME_RICHTEXT:
				return m_lstVerses.at(index.row()).getVerseRichText();
			case VDME_COMPLETE:
				return m_lstVerses.at(index.row()).getVerseRichText();		// TODO : FINISH THIS ONE!!!
			default:
				return QString();
		}
	}

	if ((role == Qt::ToolTipRole) ||
		(role == TOOLTIP_PLAINTEXT_ROLE) ||
		(role == TOOLTIP_NOHEADING_ROLE) ||
		(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
		const CVerseListItem &refVerse = m_lstVerses[index.row()];
		bool bHeading = ((role != TOOLTIP_NOHEADING_ROLE) && (role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE));
		QString strToolTip;
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "<qt><pre>";
		if (bHeading) strToolTip += refVerse.getHeading() + "\n";
		QPair<int, int> nResultsIndexes = GetResultsIndexes(index.row());
		if (nResultsIndexes.first != nResultsIndexes.second) {
			strToolTip += QString("%1Search Results %2-%3 of %4 phrase occurrences\n")
									.arg(bHeading ? "    " : "")
									.arg(nResultsIndexes.first)
									.arg(nResultsIndexes.second)
									.arg(GetTotalResultsCount());
		} else {
			assert(nResultsIndexes.first != 0);		// This will assert if the row was beyond those defined in our list
			strToolTip += QString("%1Search Result %2 of %3 phrase occurrences\n")
									.arg(bHeading ? "    " : "")
									.arg(nResultsIndexes.first)
									.arg(GetTotalResultsCount());
		}
		QPair<int, int> nVerseResult = GetVerseIndexAndCount(index.row());
		strToolTip += QString("%1    Verse %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nVerseResult.first).arg(nVerseResult.second);
		QPair<int, int> nChapterResult = GetChapterIndexAndCount(index.row());
		strToolTip += QString("%1    Chapter %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nChapterResult.first).arg(nChapterResult.second);
		QPair<int, int> nBookResult = GetBookIndexAndCount(index.row());
		strToolTip += QString("%1    Book %2 of %3 in Search Scope\n").arg(bHeading ? "    " : "").arg(nBookResult.first).arg(nBookResult.second);
		strToolTip += refVerse.getToolTip(m_lstParsedPhrases);
		if ((role != TOOLTIP_PLAINTEXT_ROLE) &&
			(role != TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) strToolTip += "</pre></qt>";
		return strToolTip;
	}

	if (role == VERSE_ENTRY_ROLE) {
		return QVariant::fromValue(m_lstVerses.at(index.row()));
	}

	return QVariant();
}

bool CVerseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.row() < 0 || index.row() >= m_lstVerses.size()) return false;

	if ((role == Qt::EditRole) || (role == Qt::DisplayRole)) {
		switch (m_nDisplayMode) {
			case VDME_HEADING:
			case VDME_VERYPLAIN:
			case VDME_RICHTEXT:
			case VDME_COMPLETE:
				return false;		// read-only
		}
	}

	if ((role == Qt::ToolTipRole) ||
		(role == TOOLTIP_PLAINTEXT_ROLE) ||
		(role == TOOLTIP_NOHEADING_ROLE) ||
		(role == TOOLTIP_NOHEADING_PLAINTEXT_ROLE)) {
		return false;				// read-only
	}

	if (role == VERSE_ENTRY_ROLE) {
		m_lstVerses.replace(index.row(), value.value<CVerseListItem>());
		emit dataChanged(index, index);
		return true;
	}

	return false;
}

Qt::ItemFlags CVerseListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool CVerseListModel::insertRows(int row, int count, const QModelIndex &parent)
{
	if (count < 1 || row < 0 || row > m_lstVerses.size())
		return false;
	if (parent.isValid()) return false;

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstVerses.insert(row, CVerseListItem());

	endInsertRows();

	return true;
}

bool CVerseListModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (count <= 0 || row < 0 || (row + count) > m_lstVerses.size())
		return false;
	if (parent.isValid()) return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstVerses.removeAt(row);

	endRemoveRows();

	return true;
}

static bool ascendingLessThan(const QPair<CVerseListItem, int> &s1, const QPair<CVerseListItem, int> &s2)
{
	// Both normalized and denormalized are in order, but it's less expensive
	//	 to convert to normal when we already have relative
	return s1.first.getIndexDenormalized() < s2.first.getIndexDenormalized();
}

static bool decendingLessThan(const QPair<CVerseListItem, int> &s1, const QPair<CVerseListItem, int> &s2)
{
	// Both normalized and denormalized are in order, but it's less expensive
	//	 to convert to normal when we already have relative
	return s1.first.getIndexDenormalized() > s2.first.getIndexDenormalized();
}

void CVerseListModel::sort(int /* column */, Qt::SortOrder order)
{
	emit layoutAboutToBeChanged();

	QList<QPair<CVerseListItem, int> > list;
	for (int i = 0; i < m_lstVerses.count(); ++i)
		list.append(QPair<CVerseListItem, int>(m_lstVerses.at(i), i));

	if (order == Qt::AscendingOrder)
		qSort(list.begin(), list.end(), ascendingLessThan);
	else
		qSort(list.begin(), list.end(), decendingLessThan);

	m_lstVerses.clear();
	QVector<int> forwarding(list.count());
	for (int i = 0; i < list.count(); ++i) {
		m_lstVerses.append(list.at(i).first);
		forwarding[list.at(i).second] = i;
	}

	QModelIndexList oldList = persistentIndexList();
	QModelIndexList newList;
	for (int i = 0; i < oldList.count(); ++i)
		newList.append(index(forwarding.at(oldList.at(i).row()), 0));
	changePersistentIndexList(oldList, newList);

	emit layoutChanged();
}

Qt::DropActions CVerseListModel::supportedDropActions() const
{
	return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

CVerseList CVerseListModel::verseList() const
{
	return m_lstVerses;
}

void CVerseListModel::setVerseList(const CVerseList &verses)
{
	emit beginResetModel();
	m_lstVerses = verses;
	emit endResetModel();
}

TParsedPhrasesList CVerseListModel::parsedPhrases() const
{
	return m_lstParsedPhrases;
}

TPhraseTagList CVerseListModel::setParsedPhrases(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nSearchScopeMode, const TParsedPhrasesList &phrases)
{
	// Note: Basic setting of this list doesn't change the model, as the phrases
	//		themselves are used primarily for building of tooltips that are
	//		appropriate for the entire search scope.  However, once these are
	//		set, we'll call the buildVerseListFromParsedPhrases function that
	//		will build and set the VerseList, which will change the model.
	//		Therefore, the beginResetModel/endResetModel calls don't exist here,
	//		but down in setVerseList:
	m_lstParsedPhrases = phrases;
	m_nSearchScopeMode = nSearchScopeMode;
	buildScopedResultsInParsedPhrases();
	return buildVerseListFromParsedPhrases();
}

bool CVerseListModel::hasExceededDisplayLimit() const
{
	if (g_bEnableNoLimits) return false;
	return ((m_lstVerses.size() > g_nSearchLimit) && (m_nDisplayMode != VDME_HEADING));
}

void CVerseListModel::setDisplayMode(VERSE_DISPLAY_MODE_ENUM nDisplayMode)
{
	if (!hasExceededDisplayLimit()) {
		emit layoutAboutToBeChanged();
		m_nDisplayMode = nDisplayMode;
		emit layoutChanged();
	} else {
		emit beginResetModel();
		m_nDisplayMode = nDisplayMode;
		emit endResetModel();
	}
}

QPair<int, int> CVerseListModel::GetResultsIndexes(int nRow) const
{
	QPair<int, int> nResultsIndexes;
	nResultsIndexes.first = 0;
	nResultsIndexes.second = 0;

	for (int ndx = 0; ((ndx < nRow) && (ndx < m_lstVerses.size())); ++ndx) {
		nResultsIndexes.first += m_lstVerses.at(ndx).phraseTags().size();
	}
	nResultsIndexes.second = nResultsIndexes.first;
	if (nRow < m_lstVerses.size()) {
		nResultsIndexes.first++;
		nResultsIndexes.second += m_lstVerses.at(nRow).phraseTags().size();
	}

	return nResultsIndexes;		// Result first = first result index, second = last result index for specified row
}

int CVerseListModel::GetTotalResultsCount() const
{
	int nResultsCount = 0;

	for (int ndx = 0; ndx < m_lstVerses.size(); ++ndx) {
		nResultsCount += m_lstVerses.at(ndx).phraseTags().size();
	}

	return nResultsCount;
}

QPair<int, int> CVerseListModel::GetBookIndexAndCount(int nRow) const
{
	int ndxBook = 0;		// Index into Books
	int nBooks = 0;			// Results counts in Books

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		nBooks++;			// Count the book we are on and skip the ones that are on the same book:
		if (ndx <= nRow) ndxBook++;
		if (ndx < (m_lstVerses.size()-1)) {
			bool bNextIsSameReference=false;
			uint32_t nCurrentBook = m_lstVerses.at(ndx).getBook();
			do {
				if (nCurrentBook == m_lstVerses.at(ndx+1).getBook()) {
					bNextIsSameReference=true;
					ndx++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndx<(m_lstVerses.size()-1)));
		}
	}

	return QPair<int, int>(ndxBook, nBooks);
}

QPair<int, int> CVerseListModel::GetChapterIndexAndCount(int nRow) const
{
	int ndxChapter = 0;		// Index into Chapters
	int nChapters = 0;		// Results counts in Chapters

	for (int ndx=0; ndx<m_lstVerses.size(); ++ndx) {
		nChapters++;		// Count the chapter we are on and skip the ones that are on the same book/chapter:
		if (ndx <= nRow) ndxChapter++;
		if (ndx < (m_lstVerses.size()-1)) {
			bool bNextIsSameReference=false;
			uint32_t nCurrentBook = m_lstVerses.at(ndx).getBook();
			uint32_t nCurrentChapter = m_lstVerses.at(ndx).getChapter();
			do {
				if ((nCurrentBook == m_lstVerses.at(ndx+1).getBook()) &&
					(nCurrentChapter == m_lstVerses.at(ndx+1).getChapter())) {
					bNextIsSameReference=true;
					ndx++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndx<(m_lstVerses.size()-1)));
		}
	}

	return QPair<int, int>(ndxChapter, nChapters);
}

QPair<int, int> CVerseListModel::GetVerseIndexAndCount(int nRow) const
{
	return QPair<int, int>(nRow+1, m_lstVerses.size());
}

void CVerseListModel::buildScopedResultsInParsedPhrases()
{
	QList<int> lstNdxStart;
	QList<int> lstNdxEnd;
	QList<CRelIndex> lstScopedRefs;
	QList<bool> lstNeedScope;
	int nNumPhrases = m_lstParsedPhrases.size();

	// Fetch results from all phrases and build a list of lists, denormalizing entries, and
	//		setting the phrase size details:
	for (int ndx=0; ndx<nNumPhrases; ++ndx) {
		lstNdxStart.append(0);
		lstNdxEnd.append(0);
		lstScopedRefs.append(CRelIndex());
		lstNeedScope.append(true);
	}

	// Now, we'll go through our lists and compress the results to the scope specified
	//		for each phrase.  We'll then find the lowest valued one and see if the others
	//		match.  If they do, we'll push all of those results onto the output.  If not,
	//		we'll toss results for the lowest until we get a match.  When any list hits
	//		its end, we're done and can break out since we have no more matches

	bool bDone = (nNumPhrases == 0);		// We're done if we have no phrases (or phrases with results)
	while (!bDone) {
		uint32_t nMaxScope = 0;
		for (int ndx=0; ndx<nNumPhrases; ++ndx) {
			const CParsedPhrase *phrase = m_lstParsedPhrases.at(ndx);
			if (!lstNeedScope[ndx]) {
				nMaxScope = qMax(nMaxScope, lstScopedRefs[ndx].index());
				continue;		// Only find next scope for a phrase if we need it
			}
			lstNdxStart[ndx] = lstNdxEnd[ndx];		// Begin at the last ending position
			if (lstNdxStart[ndx] >= phrase->GetPhraseTagSearchResults().size()) {
				bDone = true;
				break;
			}
			lstScopedRefs[ndx] = ScopeIndex(phrase->GetPhraseTagSearchResults().at(lstNdxStart[ndx]).first, m_nSearchScopeMode);
			for (lstNdxEnd[ndx] = lstNdxStart[ndx]+1; lstNdxEnd[ndx] < phrase->GetPhraseTagSearchResults().size(); ++lstNdxEnd[ndx]) {
				CRelIndex ndxScopedTemp = ScopeIndex(phrase->GetPhraseTagSearchResults().at(lstNdxEnd[ndx]).first, m_nSearchScopeMode);
				if (lstScopedRefs[ndx].index() != ndxScopedTemp.index()) break;
			}
			// Here lstNdxEnd will be one more than the number of matching, either the next index
			//		off the end of the array, or the first non-matching entry.  So the scoped
			//		area is from lstNdxStart to lstNdxEnd-1.
			nMaxScope = qMax(nMaxScope, lstScopedRefs[ndx].index());
			lstNeedScope[ndx] = false;
		}
		if (bDone) continue;		// If we run out of phrase matches on any phrase, we're done
		// Now, check the scoped references.  If they match for all indexes, we'll push the
		//	results to our output and set flags to get all new scopes.  Otherwise, compare them
		//	all against our maximum scope value and tag any that's less than that as needing a
		//	new scope (they weren't matches).  Then loop back until we've either pushed all
		//	results or run out of matches.
		bool bMatch = true;
		for (int ndx=0; ndx<nNumPhrases; ++ndx) {
			if (lstScopedRefs[ndx].index() != nMaxScope) {
				lstNeedScope[ndx] = true;
				bMatch = false;
			}
		}
		if (bMatch) {
			// We got a match, so push results to output and flag for new scopes:
			for (int ndx=0; ndx<nNumPhrases; ++ndx) {
				const CParsedPhrase *phrase = m_lstParsedPhrases.at(ndx);
				phrase->SetContributingNumberOfMatches(phrase->GetContributingNumberOfMatches() + (lstNdxEnd[ndx]-lstNdxStart[ndx]));
				for ( ; lstNdxStart[ndx]<lstNdxEnd[ndx]; ++lstNdxStart[ndx]) {
					phrase->AddScopedPhraseTagSearchResult(phrase->GetPhraseTagSearchResults().at(lstNdxStart[ndx]));
				}
				lstNeedScope[ndx] = true;
			}
		}
	}
}

TPhraseTagList CVerseListModel::buildVerseListFromParsedPhrases()
{
	CVerseList lstReferences;
	TPhraseTagList lstResults;

	for (int ndx=0; ndx<m_lstParsedPhrases.size(); ++ndx) {
		const CParsedPhrase *phrase = m_lstParsedPhrases.at(ndx);
		assert(phrase != NULL);
		lstResults.append(phrase->GetScopedPhraseTagSearchResults());
	}

	qSort(lstResults.begin(), lstResults.end(), TPhraseTagListSortPredicate::ascendingLessThan);

	for (int ndxResults=0; ndxResults<lstResults.size(); ++ndxResults) {
		if (!lstResults.at(ndxResults).first.isSet()) {
			assert(false);
			lstReferences.push_back(CVerseListItem(0, 0));
			continue;
		}
		lstReferences.push_back(CVerseListItem(lstResults.at(ndxResults)));

		CVerseListItem &verseItem(lstReferences.last());

		if (ndxResults<(lstResults.size()-1)) {
			bool bNextIsSameReference=false;
			CRelIndex ndxRelative = lstResults.at(ndxResults).first;
			do {
				CRelIndex ndxNextRelative = lstResults.at(ndxResults+1).first;

				if ((ndxRelative.book() == ndxNextRelative.book()) &&
					(ndxRelative.chapter() == ndxNextRelative.chapter()) &&
					(ndxRelative.verse() == ndxNextRelative.verse())) {
					verseItem.addPhraseTag(lstResults.at(ndxResults+1));
					bNextIsSameReference=true;
					ndxResults++;
				} else {
					bNextIsSameReference=false;
				}
			} while ((bNextIsSameReference) && (ndxResults<(lstResults.size()-1)));
		}
	}

	setVerseList(lstReferences);

	return lstResults;
}

CRelIndex CVerseListModel::ScopeIndex(const CRelIndex &index, CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM nMode)
{
	CRelIndex indexScoped;

	switch (nMode) {
		case (CKJVSearchCriteria::SSME_WHOLE_BIBLE):
			// For Whole Bible, we'll set the Book to 1 so that anything in the Bible matches:
			if (index.isSet()) indexScoped = CRelIndex(1, 0, 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_TESTAMENT):
			// For Testament, set the Book to the 1st Book of the corresponding Testament:
			if (index.book()) {
				if (index.book() <= g_lstTOC.size()) {
					const CTOCEntry &toc = g_lstTOC[index.book()-1];
					unsigned int nTestament = toc.m_nTstNdx;
					unsigned int nBook = 1;
					for (unsigned int i=1; i<nTestament; ++i)
						nBook += g_lstTestaments[i-1].m_nNumBk;
					indexScoped = CRelIndex(nBook, 0, 0 ,0);
				}
			}
			break;
		case (CKJVSearchCriteria::SSME_BOOK):
			// For Book, mask off Chapter, Verse, and Word:
			indexScoped = CRelIndex(index.book(), 0, 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_CHAPTER):
			// For Chapter, mask off Verse and Word:
			indexScoped = CRelIndex(index.book(), index.chapter(), 0, 0);
			break;
		case (CKJVSearchCriteria::SSME_VERSE):
			// For Verse, mask off word:
			indexScoped = CRelIndex(index.book(), index.chapter(), index.verse(), 0);
			break;
		default:
			break;
	}

	return indexScoped;
}


