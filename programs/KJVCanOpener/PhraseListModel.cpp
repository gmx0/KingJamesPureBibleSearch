#include "PhraseListModel.h"

#include <QVector>

CPhraseListModel::CPhraseListModel(QObject *parent) :
	QAbstractListModel(parent)
{
}

CPhraseListModel::CPhraseListModel(const CPhraseList &phrases, QObject *parent)
	:	QAbstractListModel(parent),
		m_lstPhrases(phrases)
{
}

int CPhraseListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_lstPhrases.count();
}

QVariant CPhraseListModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_lstPhrases.size())
		return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
		return (m_lstPhrases.at(index.row()).m_bCaseSensitive ? "�" : "") + m_lstPhrases.at(index.row()).m_strPhrase;
	}

	if (role == Qt::ToolTipRole) {
		return QString();				// TODO : Decide what we want for a tooltip if anything
	}

	if (role == PHRASE_ENTRY_ROLE) {
		return QVariant::fromValue(m_lstPhrases.at(index.row()));
	}

	return QVariant();
}

bool CPhraseListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.row() < 0 || index.row() >= m_lstPhrases.size()) return false;

	if ((role == Qt::EditRole) || (role == Qt::DisplayRole)) {
		m_lstPhrases[index.row()].m_strPhrase = value.toString();
		emit dataChanged(index, index);
		return true;
	}

	if (role == Qt::ToolTipRole) {
		// TODO : Allow setting of a tooltip?
		return false;
	}

	if (role == PHRASE_ENTRY_ROLE) {
		m_lstPhrases.replace(index.row(),value.value<CPhraseEntry>());
		emit dataChanged(index, index);
		return true;
	}

	return false;
}

Qt::ItemFlags CPhraseListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool CPhraseListModel::insertRows(int row, int count, const QModelIndex &parent)
{
	if (count < 1 || row < 0 || row > rowCount(parent))
		return false;

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstPhrases.insert(row, CPhraseEntry());

	endInsertRows();

	return true;
}

bool CPhraseListModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
		return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstPhrases.removeAt(row);

	endRemoveRows();

	return true;
}

static bool ascendingLessThan(const QPair<CPhraseEntry, int> &s1, const QPair<CPhraseEntry, int> &s2)
{
	return (s1.first.m_strPhrase.compare(s2.first.m_strPhrase, Qt::CaseInsensitive) < 0);
}

static bool decendingLessThan(const QPair<CPhraseEntry, int> &s1, const QPair<CPhraseEntry, int> &s2)
{
	return (s1.first.m_strPhrase.compare(s2.first.m_strPhrase, Qt::CaseInsensitive) > 0);
}

void CPhraseListModel::sort(int /* column */, Qt::SortOrder order)
{
	emit layoutAboutToBeChanged();

	QList<QPair<CPhraseEntry, int> > list;
	for (int i = 0; i < m_lstPhrases.count(); ++i)
		list.append(QPair<CPhraseEntry, int>(m_lstPhrases.at(i), i));

	if (order == Qt::AscendingOrder)
		qSort(list.begin(), list.end(), ascendingLessThan);
	else
		qSort(list.begin(), list.end(), decendingLessThan);

	m_lstPhrases.clear();
	QVector<int> forwarding(list.count());
	for (int i = 0; i < list.count(); ++i) {
		m_lstPhrases.append(list.at(i).first);
		forwarding[list.at(i).second] = i;
	}

	QModelIndexList oldList = persistentIndexList();
	QModelIndexList newList;
	for (int i = 0; i < oldList.count(); ++i)
		newList.append(index(forwarding.at(oldList.at(i).row()), 0));
	changePersistentIndexList(oldList, newList);

	emit layoutChanged();
}

Qt::DropActions CPhraseListModel::supportedDropActions() const
{
	return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

CPhraseList CPhraseListModel::phraseList() const
{
	return m_lstPhrases;
}

void CPhraseListModel::setPhraseList(const CPhraseList &phrases)
{
	emit beginResetModel();
	m_lstPhrases = phrases;
	emit endResetModel();
}

