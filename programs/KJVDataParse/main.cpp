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

#include "../KJVCanOpener/dbstruct.h"
#include "../KJVCanOpener/BuildDB.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QObject>
#include <QMainWindow>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtXml>
#include <QStringList>
#include <QtGlobal>

#include <iostream>
#include <set>

QMainWindow *g_pMainWindow = NULL;
QTranslator g_qtTranslator;

#define NUM_BK 66u
#define NUM_BK_OT 39u
#define NUM_BK_NT 27u
#define NUM_TST 2u

#define OUTPUT_HEBREW_PS119 1

typedef QList<QStringList> TChapterVerseCounts;

const QString g_arrChapterVerseCounts[NUM_BK] =
{
	"31,25,24,26,32,22,24,22,29,32,32,20,18,24,21,16,27,33,38,18,34,24,20,67,34,35,46,22,35,43,55,32,20,31,29,43,36,30,23,23,57,38,34,34,28,34,31,22,33,26",
	"22,25,22,31,23,30,25,32,35,29,10,51,22,31,27,36,16,27,25,26,36,31,33,18,40,37,21,43,46,38,18,35,23,35,35,38,29,31,43,38",
	"17,16,17,35,19,30,38,36,24,20,47,8,59,57,33,34,16,30,37,27,24,33,44,23,55,46,34",
	"54,34,51,49,31,27,89,26,23,36,35,16,33,45,41,50,13,32,22,29,35,41,30,25,18,65,23,31,40,16,54,42,56,29,34,13",
	"46,37,29,49,33,25,26,20,29,22,32,32,18,29,23,22,20,22,21,20,23,30,25,22,19,19,26,68,29,20,30,52,29,12",
	"18,24,17,24,15,27,26,35,27,43,23,24,33,15,63,10,18,28,51,9,45,34,16,33",
	"36,23,31,24,31,40,25,35,57,18,40,15,25,20,20,31,13,31,30,48,25",
	"22,23,18,22",
	"28,36,21,22,12,21,17,22,27,27,15,25,23,52,35,23,58,30,24,42,15,23,29,22,44,25,12,25,11,31,13",
	"27,32,39,12,25,23,29,18,13,19,27,31,39,33,37,23,29,33,43,26,22,51,39,25",
	"53,46,28,34,18,38,51,66,28,29,43,33,34,31,34,34,24,46,21,43,29,53",
	"18,25,27,44,27,33,20,29,37,36,21,21,25,29,38,20,41,37,37,21,26,20,37,20,30",
	"54,55,24,43,26,81,40,40,44,14,47,40,14,17,29,43,27,17,19,8,30,19,32,31,31,32,34,21,30",
	"17,18,17,22,14,42,22,18,31,19,23,16,22,15,19,14,19,34,11,37,20,12,21,27,28,23,9,27,36,27,21,33,25,33,27,23",
	"11,70,13,24,17,22,28,36,15,44",
	"11,20,32,23,19,19,73,18,38,39,36,47,31",
	"22,23,15,17,14,14,10,17,32,3",
	"22,13,26,21,27,30,21,22,35,22,20,25,28,22,35,22,16,21,29,29,34,30,17,25,6,14,23,28,25,31,40,22,33,37,16,33,24,41,30,24,34,17",
	"6,12,8,8,12,10,17,9,20,18,7,8,6,7,5,11,15,50,14,9,13,31,6,10,22,12,14,9,11,12,24,11,22,22,28,12,40,22,13,17,13,11,5,26,17,11,9,14,20,23,19,9,6,7,23,13,11,11,17,12,8,12,11,10,13,20,7,35,36,5,24,20,28,23,10,12,20,72,13,19,16,8,18,12,13,17,7,18,52,17,16,15,5,23,11,13,12,9,9,5,8,28,22,35,45,48,43,13,31,7,10,10,9,8,18,19,2,29,176,7,8,9,4,8,5,6,5,6,8,8,3,18,3,3,21,26,9,8,24,13,10,7,12,15,21,10,20,14,9,6",
	"33,22,35,27,23,35,27,36,18,32,31,28,25,35,33,33,28,24,29,30,31,29,35,34,28,28,27,28,27,33,31",
	"18,26,22,16,20,12,29,17,18,20,10,14",
	"17,17,11,16,16,13,13,14",
	"31,22,26,6,30,13,25,22,21,34,16,6,22,32,9,14,14,7,25,6,17,25,18,23,12,21,13,29,24,33,9,20,24,17,10,22,38,22,8,31,29,25,28,28,25,13,15,22,26,11,23,15,12,17,13,12,21,14,21,22,11,12,19,12,25,24",
	"19,37,25,31,31,30,34,22,26,25,23,17,27,22,21,21,27,23,15,18,14,30,40,10,38,24,22,17,32,24,40,44,26,22,19,32,21,28,18,16,18,22,13,30,5,28,7,47,39,46,64,34",
	"22,22,66,22,22",
	"28,10,27,17,17,14,27,18,11,22,25,28,23,23,8,63,24,32,14,49,32,31,49,27,17,21,36,26,21,26,18,32,33,31,15,38,28,23,29,49,26,20,27,31,25,24,23,35",
	"21,49,30,37,31,28,28,27,27,21,45,13",
	"11,23,5,19,15,11,16,14,17,15,12,14,16,9",
	"20,32,21",
	"15,16,15,13,27,14,17,14,15",
	"21",
	"17,10,10,11",
	"16,13,12,13,15,16,20",
	"15,13,19",
	"17,20,19",
	"18,15,20",
	"15,23",
	"21,13,10,14,11,15,14,23,17,12,17,14,9,21",
	"14,17,18,6",
	"25,23,17,25,48,34,29,34,38,42,30,50,58,36,39,28,27,35,30,34,46,46,39,51,46,75,66,20",
	"45,28,35,41,43,56,37,38,50,52,33,44,37,72,47,20",
	"80,52,38,44,39,49,50,56,62,42,54,59,35,35,32,31,37,43,48,47,38,71,56,53",
	"51,25,36,54,47,71,53,59,41,42,57,50,38,31,27,33,26,40,42,31,25",
	"26,47,26,37,42,15,60,40,43,48,30,25,52,28,41,40,34,28,41,38,40,30,35,27,27,32,44,31",
	"32,29,31,25,21,23,25,39,33,21,36,21,14,23,33,27",
	"31,16,23,21,13,20,40,13,27,33,34,31,13,40,58,24",
	"24,17,18,18,21,18,16,24,15,18,33,21,14",
	"24,21,29,31,26,18",
	"23,22,21,32,33,24",
	"30,30,21,23",
	"29,23,25,18",
	"10,20,13,18,28",
	"12,17,18",
	"20,15,16,16,25,21",
	"18,26,17,22",
	"16,15,15",
	"25",
	"14,18,19,16,14,20,28,13,28,39,40,29,25",
	"27,26,18,17,20",
	"25,25,22,19,14",
	"21,22,18",
	"10,29,24,21,21",
	"13",
	"14",
	"25",
	"20,29,22,11,14,17,17,13,21,11,19,17,18,20,8,21,18,24,21,15,27,21"
};

typedef struct {
	QString m_strName;
	QString m_strOsisAbbr;
	QString m_strTableName;
	QString m_strCategory;
	QString m_strDescription;
} TBook;

#define PSALMS_BOOK_NUM 19

TBook g_arrBooks[NUM_BK];
static void g_setBooks()
{
	const TBook arrBooks[NUM_BK] =
	{
		{ QObject::tr("Genesis"), "Gen", "GEN", QObject::tr("Law"), QObject::tr("The First Book of Moses") },
		{ QObject::tr("Exodus"), "Exod", "EXOD", QObject::tr("Law"), QObject::tr("The Second Book of Moses") },
		{ QObject::tr("Leviticus"), "Lev", "LEV", QObject::tr("Law"), QObject::tr("The Third Book of Moses") },
		{ QObject::tr("Numbers"), "Num", "NUM", QObject::tr("Law"), QObject::tr("The Fourth Book of Moses") },
		{ QObject::tr("Deuteronomy"), "Deut", "DEUT", QObject::tr("Law"), QObject::tr("The Fifth Book of Moses") },
		{ QObject::tr("Joshua"), "Josh", "JOSH", QObject::tr("OT Narative"), "" },
		{ QObject::tr("Judges"), "Judg", "JUDG", QObject::tr("OT Narative"), "" },
		{ QObject::tr("Ruth"), "Ruth", "RUTH", QObject::tr("OT Narative"), "" },
		{ QObject::tr("1 Samuel"), "1Sam", "SAM1", QObject::tr("OT Narative"), QObject::tr("The First Book of Samuel Otherwise Called, The First Book of the Kings") },
		{ QObject::tr("2 Samuel"), "2Sam", "SAM2", QObject::tr("OT Narative"), QObject::tr("The Second Book of Samuel Otherwise Called, The Second Book of the Kings") },
		{ QObject::tr("1 Kings"), "1Kgs", "KGS1", QObject::tr("OT Narative"), QObject::tr("The First Book of the Kings Commonly Called, The Third Book of the Kings") },
		{ QObject::tr("2 Kings"), "2Kgs", "KGS2", QObject::tr("OT Narative"), QObject::tr("The Second Book of the Kings Commonly Called, The Fourth Book of the Kings") },
		{ QObject::tr("1 Chronicles"), "1Chr", "CHR1", QObject::tr("OT Narative"), QObject::tr("The First Book of the Chronicles") },
		{ QObject::tr("2 Chronicles"), "2Chr", "CHR2", QObject::tr("OT Narative"), QObject::tr("The Second Book of the Chronicles") },
		{ QObject::tr("Ezra"), "Ezra", "EZRA", QObject::tr("OT Narative"), "" },
		{ QObject::tr("Nehemiah"), "Neh", "NEH", QObject::tr("OT Narative"), "" },
		{ QObject::tr("Esther"), "Esth", "ESTH", QObject::tr("OT Narative"), "" },
		{ QObject::tr("Job"), "Job", "JOB", QObject::tr("Wisdom"), "" },
		{ QObject::tr("Psalms"), "Ps", "PS", QObject::tr("Wisdom"), "" },
		{ QObject::tr("Proverbs"), "Prov", "PROV", QObject::tr("Wisdom"), "" },
		{ QObject::tr("Ecclesiastes"), "Eccl", "ECCL", QObject::tr("Wisdom"), QObject::tr("Ecclesiastes; Or, The Preacher") },
		{ QObject::tr("Song Of Solomon"), "Song", "SONG", QObject::tr("Wisdom"), "" },
		{ QObject::tr("Isaiah"), "Isa", "ISA", QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Isaiah") },
		{ QObject::tr("Jeremiah"), "Jer", "JER", QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Jeremiah") },
		{ QObject::tr("Lamentations"), "Lam", "LAM", QObject::tr("Major Prophets"), QObject::tr("The Lamentations of Jeremiah") },
		{ QObject::tr("Ezekiel"), "Ezek", "EZEK", QObject::tr("Major Prophets"), QObject::tr("The Book of the Prophet Ezekiel") },
		{ QObject::tr("Daniel"), "Dan", "DAN", QObject::tr("Major Prophets"), QObject::tr("The Book of <i>the Prophet</i> Daniel") },
		{ QObject::tr("Hosea"), "Hos", "HOS", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Joel"), "Joel", "JOEL", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Amos"), "Amos", "AMOS", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Obadiah"), "Obad", "OBAD", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Jonah"), "Jonah", "JONAH", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Micah"), "Mic", "MIC", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Nahum"), "Nah", "NAH", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Habakkuk"), "Hab", "HAB", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Zephaniah"), "Zeph", "ZEPH", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Haggai"), "Hag", "HAG", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Zechariah"), "Zech", "ZECH", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Malachi"), "Mal", "MAL", QObject::tr("Minor Prophets"), "" },
		{ QObject::tr("Matthew"), "Matt", "MATT", QObject::tr("NT Narative"), QObject::tr("The Gospel According to Saint Matthew") },
		{ QObject::tr("Mark"), "Mark", "MARK", QObject::tr("NT Narative"), QObject::tr("The Gospel According to Saint Mark") },
		{ QObject::tr("Luke"), "Luke", "LUKE", QObject::tr("NT Narative"), QObject::tr("The Gospel According to Saint Luke") },
		{ QObject::tr("John"), "John", "JOHN", QObject::tr("NT Narative"), QObject::tr("The Gospel According to Saint John") },
		{ QObject::tr("Acts"), "Acts", "ACTS", QObject::tr("NT Narative"), QObject::tr("The Acts of the Apostles") },
		{ QObject::tr("Romans"), "Rom", "ROM", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Romans") },
		{ QObject::tr("1 Corinthians"), "1Cor", "COR1", QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to the Corinthians") },
		{ QObject::tr("2 Corinthians"), "2Cor", "COR2", QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to the Corinthians") },
		{ QObject::tr("Galatians"), "Gal", "GAL", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Galatians") },
		{ QObject::tr("Ephesians"), "Eph", "EPH", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Ephesians") },
		{ QObject::tr("Philippians"), "Phil", "PHIL", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Philippians") },
		{ QObject::tr("Colossians"), "Col", "COL", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Colossians") },
		{ QObject::tr("1 Thessalonians"), "1Thess", "THESS1", QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to the Thessalonians") },
		{ QObject::tr("2 Thessalonians"), "2Thess", "THESS2", QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to the Thessalonains") },
		{ QObject::tr("1 Timothy"), "1Tim", "TIM1", QObject::tr("Pauline Epistles"), QObject::tr("The First Epistle of Paul the Apostle to Timothy") },
		{ QObject::tr("2 Timothy"), "2Tim", "TIM2", QObject::tr("Pauline Epistles"), QObject::tr("The Second Epistle of Paul the Apostle to Timothy") },
		{ QObject::tr("Titus"), "Titus", "TITUS", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul to Titus") },
		{ QObject::tr("Philemon"), "Phlm", "PHLM", QObject::tr("Pauline Epistles"), QObject::tr("The Epistle of Paul to Philemon") },
		{ QObject::tr("Hebrews"), "Heb", "HEB", QObject::tr("General Epistles"), QObject::tr("The Epistle of Paul the Apostle to the Hebrews") },
		{ QObject::tr("James"), "Jas", "JAS", QObject::tr("General Epistles"), QObject::tr("The General Epistle of James") },
		{ QObject::tr("1 Peter"), "1Pet", "PET1", QObject::tr("General Epistles"), QObject::tr("The First General Epistle of Peter") },
		{ QObject::tr("2 Peter"), "2Pet", "PET2", QObject::tr("General Epistles"), QObject::tr("The Second General Epistle of Peter") },
		{ QObject::tr("1 John"), "1John", "JOHN1", QObject::tr("General Epistles"), QObject::tr("The First General Epistle of John") },
		{ QObject::tr("2 John"), "2John", "JOHN2", QObject::tr("General Epistles"), QObject::tr("The Second General Epistle of John") },
		{ QObject::tr("3 John"), "3John", "JOHN3", QObject::tr("General Epistles"), QObject::tr("The Third General Epistle of John") },
		{ QObject::tr("Jude"), "Jude", "JUDE", QObject::tr("General Epistles"), QObject::tr("The General Epistle of Jude") },
		{ QObject::tr("Revelation"), "Rev", "REV", QObject::tr("Apocalyptic Epistle"), QObject::tr("The Revelation of Jesus Christ") }
	};

	for (unsigned int i=0; i<NUM_BK; ++i) {
		g_arrBooks[i] = arrBooks[i];
	}
}

QString g_arrstrTstNames[NUM_TST];
static void g_setTstNames()
{
	const QString arrstrTstNames[NUM_TST] =
		{	QObject::tr("Old Testament"),
			QObject::tr("New Testament")
		};

	for (unsigned int i=0; i<NUM_TST; ++i) {
		g_arrstrTstNames[i] = arrstrTstNames[i];
	}
}

// ============================================================================
// ============================================================================


static QString psalm119HebrewPrefix(const CRelIndex &ndx)
{
	if ((ndx.book() != PSALMS_BOOK_NUM) || (ndx.chapter() != 119) || (((ndx.verse()-1)%8) != 0)) return QString();

	QString strHebrewPrefix;

	// Add special Start tag so KJVBrowser can know to ignore the special Hebrew text insertion during highlighting:
	strHebrewPrefix += QString("<a id=\"\"A%1\"\"> </a>").arg(ndx.asAnchor());

#if (OUTPUT_HEBREW_PS119)
	switch ((ndx.verse()-1)/8) {
		case 0:
			// ALEPH
			strHebrewPrefix += QChar(0x005D0);
			break;
		case 1:
			// BETH
			strHebrewPrefix += QChar(0x005D1);
			break;
		case 2:
			// GIMEL
			strHebrewPrefix += QChar(0x005D2);
			break;
		case 3:
			// DALETH
			strHebrewPrefix += QChar(0x005D3);
			break;
		case 4:
			// HE
			strHebrewPrefix += QChar(0x005D4);
			break;
		case 5:
			// VAU
			strHebrewPrefix += QChar(0x005D5);
			break;
		case 6:
			// ZAIN
			strHebrewPrefix += QChar(0x005D6);
			break;
		case 7:
			// CHETH
			strHebrewPrefix += QChar(0x005D7);
			break;
		case 8:
			// TETH
			strHebrewPrefix += QChar(0x005D8);
			break;
		case 9:
			// JOD
			strHebrewPrefix += QChar(0x005D9);
			break;
		case 10:
			// CAPH
			strHebrewPrefix += QChar(0x005DB);		// Using nonfinal-CAPH
			break;
		case 11:
			// LAMED
			strHebrewPrefix += QChar(0x005DC);
			break;
		case 12:
			// MEM
			strHebrewPrefix += QChar(0x005DE);		// Using nonfinal-Mem
			break;
		case 13:
			// NUN
			strHebrewPrefix += QChar(0x005E0);		// Using nonfinal-Nun
			break;
		case 14:
			// SAMECH
			strHebrewPrefix += QChar(0x005E1);
			break;
		case 15:
			// AIN
			strHebrewPrefix += QChar(0x005E2);
			break;
		case 16:
			// PE
			strHebrewPrefix += QChar(0x005E4);		// Using nonfinal-Pe
			break;
		case 17:
			// TZADDI
			strHebrewPrefix += QChar(0x005E6);		// Using nonfinal-Tzaddi
			break;
		case 18:
			// KOPH
			strHebrewPrefix += QChar(0x005E7);
			break;
		case 19:
			// RESH
			strHebrewPrefix += QChar(0x005E8);
			break;
		case 20:
			// SCHIN
			strHebrewPrefix += QChar(0x005E9);
			break;
		case 21:
			// TAU
			strHebrewPrefix += QChar(0x005EA);
			break;
	}
	strHebrewPrefix += " ";
#endif

	switch ((ndx.verse()-1)/8) {
		case 0:
			strHebrewPrefix += "(ALEPH).";
			break;
		case 1:
			strHebrewPrefix += "(BETH).";
			break;
		case 2:
			strHebrewPrefix += "(GIMEL).";
			break;
		case 3:
			strHebrewPrefix += "(DALETH).";
			break;
		case 4:
			strHebrewPrefix += "(HE).";
			break;
		case 5:
			strHebrewPrefix += "(VAU).";
			break;
		case 6:
			strHebrewPrefix += "(ZAIN).";
			break;
		case 7:
			strHebrewPrefix += "(CHETH).";
			break;
		case 8:
			strHebrewPrefix += "(TETH).";
			break;
		case 9:
			strHebrewPrefix += "(JOD).";
			break;
		case 10:
			strHebrewPrefix += "(CAPH).";
			break;
		case 11:
			strHebrewPrefix += "(LAMED).";
			break;
		case 12:
			strHebrewPrefix += "(MEM).";
			break;
		case 13:
			strHebrewPrefix += "(NUN).";
			break;
		case 14:
			strHebrewPrefix += "(SAMECH).";
			break;
		case 15:
			strHebrewPrefix += "(AIN).";
			break;
		case 16:
			strHebrewPrefix += "(PE).";
			break;
		case 17:
			strHebrewPrefix += "(TZADDI).";
			break;
		case 18:
			strHebrewPrefix += "(KOPH).";
			break;
		case 19:
			strHebrewPrefix += "(RESH).";
			break;
		case 20:
			strHebrewPrefix += "(SCHIN).";
			break;
		case 21:
			strHebrewPrefix += "(TAU).";
			break;
	}

	// Add special End tag so KJVBrowser can know to ignore the special Hebrew text insertion during highlighting:
	strHebrewPrefix += QString("<a id=\"\"B%1\"\"> </a>").arg(ndx.asAnchor());

	return strHebrewPrefix;
}

// ============================================================================
// ============================================================================


// For processing hyphenated words, the following symbols will be treated
//	as hyphens and rolled into the "-" symbol for processing.  Words with
//	only hyphen differences will be added to the base word as a special
//	alternate form, allowing users to search them with or without hyphen
//	sensitivity:
const QString g_strHyphens =	QString(QChar(0x002D)) +		// U+002D	&#45;		hyphen-minus 	the Ascii hyphen, with multiple usage, or “ambiguous semantic value”; the width should be “average”
//								QString(QChar(0x007E)) +		// U+007E	&#126;		tilde 	the Ascii tilde, with multiple usage; “swung dash”
								QString(QChar(0x00AD)) +		// U+00AD	&#173;		soft hyphen 	“discretionary hyphen”
								QString(QChar(0x058A)) +		// U+058A	&#1418; 	armenian hyphen 	as soft hyphen, but different in shape
								QString(QChar(0x05BE)) +		// U+05BE	&#1470; 	hebrew punctuation maqaf 	word hyphen in Hebrew
//								QString(QChar(0x1400)) +		// U+1400	&#5120; 	canadian syllabics hyphen 	used in Canadian Aboriginal Syllabics
//								QString(QChar(0x1806)) +		// U+1806	&#6150; 	mongolian todo soft hyphen 	as soft hyphen, but displayed at the beginning of the second line
								QString(QChar(0x2010)) +		// U+2010	&#8208; 	hyphen 	unambiguously a hyphen character, as in “left-to-right”; narrow width
								QString(QChar(0x2011)) +		// U+2011	&#8209; 	non-breaking hyphen 	as hyphen (U+2010), but not an allowed line break point
								QString(QChar(0x2012)) +		// U+2012	&#8210; 	figure dash 	as hyphen-minus, but has the same width as digits
								QString(QChar(0x2013)) +		// U+2013	&#8211; 	en dash 	used e.g. to indicate a range of values
// >>>>>>>>>>>>					QString(QChar(0x2014)) +		// U+2014	&#8212; 	em dash 	used e.g. to make a break in the flow of a sentence
// >>>>>>>>>>>>					QString(QChar(0x2015)) +		// U+2015	&#8213; 	horizontal bar 	used to introduce quoted text in some typographic styles; “quotation dash”; often (e.g., in the representative glyph in the Unicode standard) longer than em dash
//								QString(QChar(0x2053)) +		// U+2053	&#8275; 	swung dash 	like a large tilde
//								QString(QChar(0x207B)) +		// U+207B	&#8315; 	superscript minus 	a compatibility character which is equivalent to minus sign U+2212 in superscript style
//								QString(QChar(0x208B)) +		// U+208B	&#8331; 	subscript minus 	a compatibility character which is equivalent to minus sign U+2212 in subscript style
								QString(QChar(0x2212)) +		// U+2212	&#8722; 	minus sign 	an arithmetic operator; the glyph may look the same as the glyph for a hyphen-minus, or may be longer ;
//								QString(QChar(0x2E17)) +		// U+2E17	&#11799; 	double oblique hyphen 	used in ancient Near-Eastern linguistics; not in Fraktur, but the glyph of Ascii hyphen or hyphen is similar to this character in Fraktur fonts
// >>>>>>>>>>>>					QString(QChar(0x2E3A)) +		// U+2E3A	&#11834; 	two-em dash 	omission dash<(a>, 2 em units wide
// >>>>>>>>>>>>					QString(QChar(0x2E3B)) +		// U+2E3B	&#11835; 	three-em dash 	used in bibliographies, 3 em units wide
//								QString(QChar(0x301C)) +		// U+301C	&#12316; 	wave dash 	a Chinese/Japanese/Korean character
//								QString(QChar(0x3030)) +		// U+3030	&#12336; 	wavy dash 	a Chinese/Japanese/Korean character
//								QString(QChar(0x30A0)) +		// U+30A0	&#12448;	katakana-hiragana double hyphen	in Japasene kana writing
//								QString(QChar(0xFE31)) +		// U+FE31	&#65073;	presentation form for vertical em dash	vertical variant of em dash
//								QString(QChar(0xFE32)) +		// U+FE32	&#65074;	presentation form for vertical en dash	vertical variant of en dash
								QString(QChar(0xFE58)) +		// U+FE58	&#65112;	small em dash	small variant of em dash
								QString(QChar(0xFE63)) +		// U+FE63	&#65123;	small hyphen-minus	small variant of Ascii hyphen
								QString(QChar(0xFF0D));			// U+FF0D	&#65293;	fullwidth hyphen-minus

// For processing words with apostrophes, the following symbols will be treated
//	as apostrophes and rolled into the "'" symbol for processing.  Words with
//	only apostrophe differences will be added to the base word as a special
//	alternate form, allowing users to search them with or without the apostrophe:
const QString g_strApostrophes =	QString(QChar(0x0027)) +		// U+0027	&#39;		Ascii apostrophe (single quote)
									QString(QChar(0x2018)) +		// U+2018	&#8216;		Quote left
									QString(QChar(0x2019)) +		// U+2019	&#8217;		Quote right
									QString(QChar(0x201B));			// U+201B	&#8219;		Quote reversed


// Ascii Word characters -- these will be kept in words as-is and includes
//	alphanumerics.  Hyphen and apostrophe are kept too, but by the rules
//	above, not here.  Non-Ascii (high UTF8 values) are also kept, but have
//	rules of their own:
const QString g_strAsciiWordChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


// Non-Word Non-Ascii characters -- these are non-Ascii characters the do
//	do automatically apply as a word -- things like quotes, etc:
const QString g_strNonAsciiNonWordChars =	QString(QChar(0x201C)) +		// U+201C	&#8220;		Double-Quote Left
											QString(QChar(0x201D)) +		// U+201D	&#8221;		Double-Quote Right
											QString(QChar(0x201E)) +		// U+201E	&#8222;		Double-Quote Base
											QString(QChar(0x201A)) +		// U+201A	&#8218;		Single-Quote Base
											QString(QChar(0x2039)) +		// U+2039	&#8249;		Single-Guil Left
											QString(QChar(0x203A)) +		// U+203A	&#8250;		Single-Guil Right
											QString(QChar(0x203C)) +		// U+203C	&#8252;		Double Exclamation
											QString(QChar(0x00AB)) +		// U+00AB	&#164;		Double-guillemot Left
											QString(QChar(0x00BB)) +		// U+00BB	&#187;		Double-guillemot Right
											QString(QChar(0x00BF)) +		// U+00BF	&#191;		Upside down question mark
											QString(QChar(0x00A1)) +		// U+00A1	&#161;		Upside down exclamation mark
											QString(QChar(0x00B7));			// U+00B7	&#183;		Centered period

const QChar g_chrPilcrow = QChar(0x00B6);		// Pilcrow paragraph marker

const QChar g_chrParseTag = QChar('|');			// Special tag to put into the verse text to mark parse tags -- must NOT exist in the text

// ============================================================================
// ============================================================================

static bool isSpecialWord(const QString &strWord)
{
	if (strWord.compare("abominations", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("am", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("amen", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("ancient", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("and", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("angel", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("Babylon", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("bishop", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("branch", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("cherub", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("comforter", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("creator", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("day", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("days", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("devil", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("dominion", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("duke", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("earth", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("elect", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("father", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("father's", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("fathers", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("ghost", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("God", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("gods", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("great", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("harlots", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("heaven", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("hell", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("highest", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("him", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("himself", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("his", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("holiness", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("holy", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("is", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("Jesus", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("Jews", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("judge", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("king", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("kings", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("kings'", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("lamb", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("legion", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("lion", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("lord", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("lord's", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("lords", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("lot", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("man", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("man's", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("master", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("masters", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("men", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("men's", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("mighty", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("moon", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("mother", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("mystery", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("Nazareth", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("of", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("one", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("our", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("righteousness", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("sanctuary", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("saviour", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("sceptre", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("shepherd", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("son", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("spirit", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("spirits", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("sun", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("tabernacle", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("that", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("the", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("this", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("thy", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("unknown", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("unto", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("word", Qt::CaseInsensitive) == 0) return true;
	if (strWord.compare("wormwood", Qt::CaseInsensitive) == 0) return true;

	return false;
}

// ============================================================================
// ============================================================================

// TAltWordSet will be a set containing all of the case-forms of a given word.  It's easier
//		to map them here as a set than in the list that the database itself uses.  The
//		TAltWordListMap will be indexed by the Lower-Case word key and will map to
//		the set of word forms for that key:
typedef std::set<QString, CWordEntry::SortPredicate> TAltWordSet;
typedef std::map<QString, TAltWordSet, CWordEntry::SortPredicate> TAltWordListMap;

// WordFromWordSet - Drives word toward lower-case and returns the resulting word.  The
//		theory is that proper names will always be capitalized and non-proper names will
//		have mixed case, being capital only when they start a new sentence.  Thus, if we
//		drive toward lower-case, we should have an all-lower-case word for non-proper
//		name words and mixed-case for proper names:
static QString WordFromWordSet(const TAltWordSet &setAltWords)
{
	QString strWord;

	for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
		if ((strWord.isEmpty()) ||
			(((*itrAltWrd).compare(strWord)) > 0)) strWord = *itrAltWrd;
	}

	return strWord;
}


// ============================================================================
// ============================================================================

class CVerseTextRichifierTags
{
public:
	CVerseTextRichifierTags()
		:	m_bAddRichPs119HebrewPrefix(true),
			m_strTransChangeAddedBegin("<i>"),
			m_strTransChangeAddedEnd("</i>"),
			m_strWordsOfJesusBegin("<font color=\"red\"> "),
			m_strWordsOfJesusEnd("</font> "),
//			m_strDivideNameBegin("<b>"),
//			m_strDivideNameEnd("</b>")
			m_strDivideNameBegin("<font size=\"-1\">"),
			m_strDivideNameEnd("</font>")
	{

	}

	~CVerseTextRichifierTags()
	{

	}

	bool addRichPs119HebrewPrefix() const { return m_bAddRichPs119HebrewPrefix; }
	void setAddRichPs119HebrewPrefix(bool bAddRichPs119HebrewPrefix)
	{
		m_bAddRichPs119HebrewPrefix = bAddRichPs119HebrewPrefix;
	}

	QString transChangeAddedBegin() const { return m_strTransChangeAddedBegin; }
	QString transChangeAddedEnd() const { return m_strTransChangeAddedEnd; }
	void setTransChangeAddedTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strTransChangeAddedBegin = strTagBegin;
		m_strTransChangeAddedEnd = strTagEnd;
	}

	QString wordsOfJesusBegin() const { return m_strWordsOfJesusBegin; }
	QString wordsOfJesusEnd() const { return m_strWordsOfJesusEnd; }
	void setWordsOfJesusTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strWordsOfJesusBegin = strTagBegin;
		m_strWordsOfJesusEnd = strTagEnd;
	}

	QString divineNameBegin() const { return m_strDivideNameBegin; }
	QString divineNameEnd() const { return m_strDivideNameEnd; }
	void setDivineNameTags(const QString &strTagBegin, const QString &strTagEnd)
	{
		m_strDivideNameBegin = strTagBegin;
		m_strDivideNameEnd = strTagEnd;
	}

protected:
	bool m_bAddRichPs119HebrewPrefix;
	QString m_strTransChangeAddedBegin;
	QString m_strTransChangeAddedEnd;
	QString m_strWordsOfJesusBegin;
	QString m_strWordsOfJesusEnd;
	QString m_strDivideNameBegin;
	QString m_strDivideNameEnd;
};

class CVerseTextPlainRichifierTags : public CVerseTextRichifierTags
{
public:
	CVerseTextPlainRichifierTags()
		:	CVerseTextRichifierTags()
	{
		setAddRichPs119HebrewPrefix(false);
		setTransChangeAddedTags("[", "]");
		setWordsOfJesusTags(QString(), QString());
		setDivineNameTags(QString(), QString());
	}
};

class CVerseTextRichifier
{
private:
	CVerseTextRichifier(const CRelIndex &ndxRelative, const QChar &chrMatchChar, const QString &strXlateText, const CVerseTextRichifier *pRichNext = NULL)
		:	m_pRichNext(pRichNext),
			m_chrMatchChar(chrMatchChar),
			m_pVerse(NULL),
			m_strXlateText(strXlateText),
			m_bAddAnchors(false),
			m_ndxCurrent(ndxRelative)
	{

	}

	CVerseTextRichifier(const CRelIndex &ndxRelative, const QChar &chrMatchChar, const CVerseEntry *pVerse, const CVerseTextRichifier *pRichNext = NULL, bool bAddAnchors = false)
		:	m_pRichNext(pRichNext),
			m_chrMatchChar(chrMatchChar),
			m_pVerse(pVerse),
			m_bAddAnchors(bAddAnchors),
			m_ndxCurrent(ndxRelative)
	{
		assert(pVerse != NULL);
	}

	~CVerseTextRichifier()
	{

	}

	class CRichifierBaton
	{
	public:
		CRichifierBaton()
		{

		}

		QString m_strDivineNameFirstLetterParseText;		// Special First-Letter Markup Text for Divine Name
	};

	QString parse(CRichifierBaton &parseBaton, const QString &strNodeIn = QString()) const
	{
		if (m_chrMatchChar.isNull()) return strNodeIn;

		QString strTemp;
		QStringList lstSplit;

		if (m_pVerse != NULL) {
			lstSplit = m_pVerse->m_strTemplate.split(m_chrMatchChar);
			assert(lstSplit.size() == (m_pVerse->m_lstRichWords.size() + 1));
			assert(strNodeIn.isNull());
		} else {
			lstSplit = strNodeIn.split(m_chrMatchChar);
		}
		assert(lstSplit.size() != 0);

		for (int i=0; i<lstSplit.size(); ++i) {
			if (i > 0) {
				if (m_pVerse != NULL) {
					if (m_bAddAnchors) strTemp += QString("<a id=\"%1\">").arg(CRelIndex(m_ndxCurrent.index() + i).asAnchor());
					if (!parseBaton.m_strDivineNameFirstLetterParseText.isEmpty()) {
						strTemp += m_pVerse->m_lstRichWords.at(i-1).left(1)
								+ parseBaton.m_strDivineNameFirstLetterParseText
								+ m_pVerse->m_lstRichWords.at(i-1).mid(1);
						parseBaton.m_strDivineNameFirstLetterParseText.clear();
					} else {
						strTemp += m_pVerse->m_lstRichWords.at(i-1);
					}
					if (m_bAddAnchors) strTemp += "</a>";
				} else {
					if (m_chrMatchChar == QChar('D')) {
						parseBaton.m_strDivineNameFirstLetterParseText = m_strXlateText;
					} else {
						strTemp += m_strXlateText;
					}
				}
			}
			if (m_pRichNext) {
				strTemp += m_pRichNext->parse(parseBaton, lstSplit.at(i));
			} else {
				strTemp += lstSplit.at(i);
			}
		}

		return strTemp;
	}

public:
	static QString parse(const CRelIndex &ndxRelative, const CBibleDatabase *pBibleDatabase, const CVerseEntry *pVerse, const CVerseTextRichifierTags &tags = CVerseTextRichifierTags(), bool bAddAnchors = false)
	{
		assert(pBibleDatabase != NULL);
		assert(pVerse != NULL);

		// Note: While it would be most optimum to reverse this and
		//		do the verse last so we don't have to call the entire
		//		tree for every word, we can't reverse it because doing
		//		so then creates sub-lists of 'w' tags and then we
		//		no longer know where we are in the list:
		CVerseTextRichifier rich_d(ndxRelative, 'd', tags.divineNameEnd());
		CVerseTextRichifier rich_D(ndxRelative, 'D', tags.divineNameBegin(), &rich_d);				// D/d must be last for font start/stop to work correctly with special first-letter text mode
		CVerseTextRichifier rich_t(ndxRelative, 't', tags.transChangeAddedEnd(), &rich_D);
		CVerseTextRichifier rich_T(ndxRelative, 'T', tags.transChangeAddedBegin(), &rich_t);
		CVerseTextRichifier rich_j(ndxRelative, 'j', tags.wordsOfJesusEnd(), &rich_T);
		CVerseTextRichifier rich_J(ndxRelative, 'J', tags.wordsOfJesusBegin(), &rich_j);
		CVerseTextRichifier rich_M(ndxRelative, 'M', (tags.addRichPs119HebrewPrefix() ? psalm119HebrewPrefix(ndxRelative) : ""), &rich_J);
		CVerseTextRichifier richVerseText(ndxRelative, 'w', pVerse, &rich_M, bAddAnchors);

		CRichifierBaton baton;
		QString strTemp = richVerseText.parse(baton);
		if (pVerse->m_nPilcrow) strTemp = g_chrPilcrow + strTemp;
		return strTemp;
	}

private:
	const CVerseTextRichifier *m_pRichNext;
	const QChar m_chrMatchChar;
	const CVerseEntry *m_pVerse;
	QString m_strXlateText;
	bool m_bAddAnchors;
	CRelIndex m_ndxCurrent;
};

// ============================================================================
// ============================================================================

class COSISXmlHandler : public QXmlDefaultHandler
{
public:
	COSISXmlHandler(const QString &strNamespace)
		:	m_strNamespace(strNamespace),
			m_bCaptureTitle(false),
			m_bInVerse(false),
			m_bInLemma(false),
			m_bInTransChangeAdded(false),
			m_bInNotes(false),
			m_bInColophon(false),
			m_bInSubtitle(false),
			m_bInForeignText(false),
			m_bInWordsOfJesus(false),
			m_bInDivineName(false)
	{
		g_setBooks();
		g_setTstNames();
		m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(QString(), QString()));		// Note: We'll set the name and description later in the reading of the data
		for (unsigned int i=0; i<NUM_BK; ++i) {
			m_lstOsisBookList.append(g_arrBooks[i].m_strOsisAbbr);
		}
	}

	~COSISXmlHandler()
	{

	}

	QStringList elementNames() const { return m_lstElementNames; }
	QStringList attrNames() const { return m_lstAttrNames; }

	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
	virtual bool characters(const QString &ch);
	virtual bool error(const QXmlParseException &exception);

	const CBibleDatabase *bibleDatabase() const { return m_pBibleDatabase.data(); }

	QString parsedUTF8Chars() const { return m_strParsedUTF8Chars; }

protected:
	int findAttribute(const QXmlAttributes &attr, const QString &strName) {
		for (int i = 0; i < attr.count(); ++i) {
			if (attr.localName(i).compare(strName, Qt::CaseInsensitive) == 0) return i;
		}
		return -1;
	}

	QString stringifyAttributes(const QXmlAttributes &attr) {
		QString strTemp;
		for (int i=0; i<attr.count(); ++i) {
			if (i) strTemp += ',';
			strTemp += attr.localName(i) + '=' + attr.value(i);
		}
		return strTemp;
	}

	QXmlAttributes attributesFromString(const QString &str) {
		QXmlAttributes attrs;
		QStringList lstPairs = str.split(',');
		for (int i=0; i<lstPairs.count(); ++i) {
			QStringList lstEntry = lstPairs.at(i).split('=');
			assert(lstEntry.count() == 2);
			if (lstEntry.count() != 2) {
				std::cerr << "\n*** Error: Attributes->String failure\n";
				continue;
			}
			attrs.append(lstEntry.at(0), QString(), lstEntry.at(0), lstEntry.at(1));
		}
		return attrs;
	}

private:
	QString m_strNamespace;
	QStringList m_lstElementNames;
	QStringList m_lstAttrNames;

	CRelIndex m_ndxCurrent;
	CRelIndex m_ndxColophon;
	CRelIndex m_ndxSubtitle;
	bool m_bCaptureTitle;
	bool m_bInVerse;
	bool m_bInLemma;
	bool m_bInTransChangeAdded;
	bool m_bInNotes;
	bool m_bInColophon;
	bool m_bInSubtitle;
	bool m_bInForeignText;
	bool m_bInWordsOfJesus;
	bool m_bInDivineName;
	QString m_strParsedUTF8Chars;		// UTF-8 (non-Ascii) characters encountered -- used for report

	CBibleDatabasePtr m_pBibleDatabase;
	QStringList m_lstOsisBookList;
};

bool COSISXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

/*
	{osis}[schemaLocation=http://www.bibletechnologies.net/2003/OSIS/namespace http://www.bibletechnologies.net/osisCore.2.1.1.xsd]

	{osisText}[osisIDWork=KJV,osisRefWork=defaultReferenceScheme,lang=en]

		{header}[]
			{work}[osisWork=KJV]
				{title}[]King James Version (1769) with Strongs Numbers and Morphology{/title}

				{identifier}[type=OSIS]Bible.KJV{/identifier}

				{refSystem}[]Bible.KJV{/refSystem}

			{/work}

			{work}[osisWork=defaultReferenceScheme]
				{refSystem}[]Bible.KJV{/refSystem}

			{/work}

		{/header}
*/

	int ndx = -1;
	unsigned int nTst = m_pBibleDatabase->m_lstTestaments.size();
	int nBk = -1;

	if (localName.compare("osisText", Qt::CaseInsensitive) == 0)  {
		ndx = findAttribute(atts, "osisIDWork");
		if (ndx != -1) m_pBibleDatabase->m_strName = atts.value(ndx);
		std::cerr << "Work: " << atts.value(ndx).toStdString() << "\n";
		ndx = findAttribute(atts, "lang");
		if  (ndx != -1) {
			std::cerr << "Language: " << atts.value(ndx).toStdString();
			if (g_qtTranslator.load("kjvdataparse_" + atts.value(ndx))) {
				QCoreApplication::installTranslator(&g_qtTranslator);
				g_setBooks();
				g_setTstNames();
				std::cerr << " (Loaded Translations)\n";
			} else {
				std::cerr << " (NO Translations Found!)\n";
			}
		}
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		if (!m_ndxCurrent.isSet()) {
			m_bCaptureTitle = true;
		} else {
			// Should we check these here??: canonical="true" subType="x-preverse" type="section"
			m_bInSubtitle = true;
			m_ndxSubtitle = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0);		// Subtitles are for the chapter, not the first verse in it, even thought that's were this tag exists
		}
	} else if (localName.compare("foreign", Qt::CaseInsensitive) == 0) {
		m_bInForeignText = true;
	} else if ((!m_ndxCurrent.isSet()) && (localName.compare("div", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "type");
		if ((ndx != -1) && (atts.value(ndx).compare("x-testament", Qt::CaseInsensitive) == 0)) {
			CTestamentEntry aTestament((m_pBibleDatabase->m_lstTestaments.size() < NUM_TST) ? g_arrstrTstNames[m_pBibleDatabase->m_lstTestaments.size()] : QString());
			m_pBibleDatabase->m_EntireBible.m_nNumTst++;
			m_pBibleDatabase->m_lstTestaments.push_back(aTestament);
			nTst++;
			std::cerr << "Testament: " << ((m_pBibleDatabase->m_lstTestaments.size() <= NUM_TST) ? aTestament.m_strTstName.toStdString() : "<Unknown>") << "\n";
		} else if ((ndx != -1) && (atts.value(ndx).compare("book", Qt::CaseInsensitive) == 0)) {
			// Some OSIS files just have book tags and no x-testament tags, so we'll try to infer
			//		testament here:
			ndx = findAttribute(atts, "osisID");
			if (ndx != -1) {
				QStringList lstOsisID = atts.value(ndx).split('.');
				if ((lstOsisID.size() != 1) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
					std::cerr << "\n*** Invalid Book osisID : " << atts.value(ndx).toStdString() << "\n";
				} else {
					std::cerr << "Book: " << lstOsisID.at(0).toStdString() << "\n";
					// note: nBk is index into array, not book number:
					if (static_cast<unsigned int>(nBk) < NUM_BK_OT) {
						nTst = 1;
					} else if (static_cast<unsigned int>(nBk) < NUM_BK) {
						nTst = 2;
					} else {
						nTst = 0;
						assert(false);			// Can't happen if our NUM_BK_xx values are correct!
					}
					while (m_pBibleDatabase->m_lstTestaments.size() < nTst) {
						CTestamentEntry aTestament(g_arrstrTstNames[m_pBibleDatabase->m_lstTestaments.size()]);
						m_pBibleDatabase->m_EntireBible.m_nNumTst++;
						m_pBibleDatabase->m_lstTestaments.push_back(aTestament);
						std::cerr << "Testament: " << aTestament.m_strTstName.toStdString() << "\n";
					}
				}
			}
		}
	} else if ((localName.compare("div", Qt::CaseInsensitive) == 0) && ((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("colophon", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "osisID");
		if (ndx != -1) {
			QStringList lstOsisID = atts.value(ndx).split('.');
			if ((lstOsisID.size() < 1) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
				std::cerr << "\n*** Unknown Colophon osisID : " << atts.value(ndx).toStdString() << "\n";
				m_ndxColophon = CRelIndex();
			} else {
				bool bOK = true;
				unsigned int nChp = 0;
				unsigned int nVrs = 0;
				m_ndxColophon = CRelIndex(nBk+1, 0, 0, 0);
				if ((lstOsisID.size() >= 2) && ((nChp = lstOsisID.at(1).toUInt(&bOK)) != 0) && (bOK)) {
					m_ndxColophon.setChapter(nChp);
					if ((lstOsisID.size() >= 3) && ((nVrs = lstOsisID.at(2).toUInt(&bOK)) != 0) && (bOK)) {
						m_ndxColophon.setVerse(nVrs);
					}
				}
			}
		} else{
			m_ndxColophon = CRelIndex();
		}
		m_bInColophon = true;
	} else if ((!m_ndxCurrent.isSet()) && (localName.compare("chapter", Qt::CaseInsensitive) == 0)) {
		if (nTst == 0) {
			std::cerr << "\n*** Found book/chapter before testament marker!\n";
		} else {
			ndx = findAttribute(atts, "osisID");
			if (ndx != -1) {
				QStringList lstOsisID = atts.value(ndx).split('.');
				if ((lstOsisID.size() != 2) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
					m_ndxCurrent = CRelIndex();
					 std::cerr << "\n*** Unknown Chapter osisID : " << atts.value(ndx).toStdString() << "\n";
				} else {
					std::cerr << "Book: " << lstOsisID.at(0).toStdString() << " Chapter: " << lstOsisID.at(1).toStdString();
					m_ndxCurrent = CRelIndex(nBk+1, lstOsisID.at(1).toUInt(), 0, 0);
					m_pBibleDatabase->m_mapChapters[m_ndxCurrent];			// Make sure the chapter entry is created, even though we have nothing to put in it yet
					m_pBibleDatabase->m_EntireBible.m_nNumChp++;
					m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumChp++;
					if (lstOsisID.at(1).toUInt() == 1) {
						m_pBibleDatabase->m_EntireBible.m_nNumBk++;
						m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumBk++;
						m_pBibleDatabase->m_lstBooks.resize(qMax(static_cast<unsigned int>(nBk+1), static_cast<unsigned int>(m_pBibleDatabase->m_lstBooks.size())));
						m_pBibleDatabase->m_lstBooks[nBk].m_nTstBkNdx = m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumBk;
						m_pBibleDatabase->m_lstBooks[nBk].m_nTstNdx = nTst;
						m_pBibleDatabase->m_lstBooks[nBk].m_strBkName = g_arrBooks[nBk].m_strName;
						m_pBibleDatabase->m_lstBooks[nBk].m_strBkAbbr = g_arrBooks[nBk].m_strOsisAbbr;
						m_pBibleDatabase->m_lstBooks[nBk].m_strTblName = g_arrBooks[nBk].m_strTableName;
						m_pBibleDatabase->m_lstBooks[nBk].m_strCat = g_arrBooks[nBk].m_strCategory;
						m_pBibleDatabase->m_lstBooks[nBk].m_strDesc = g_arrBooks[nBk].m_strDescription;
						m_pBibleDatabase->m_lstBookVerses.resize(qMax(static_cast<unsigned int>(nBk+1), static_cast<unsigned int>(m_pBibleDatabase->m_lstBookVerses.size())));
					}
					assert(m_pBibleDatabase->m_lstBooks.size() > static_cast<unsigned int>(nBk));
					m_pBibleDatabase->m_lstBooks[nBk].m_nNumChp++;
				}
			} else {
				m_ndxCurrent = CRelIndex();
				std::cerr << "\n*** Chapter with no osisID : ";
				std::cerr << stringifyAttributes(atts).toStdString() << "\n";
			}
		}
	} else if ((m_ndxCurrent.isSet()) && (localName.compare("verse", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "osisID");
		if (ndx != -1) {
			QStringList lstOsisID = atts.value(ndx).split('.');
			if ((lstOsisID.size() != 3) || ((nBk = m_lstOsisBookList.indexOf(lstOsisID.at(0))) == -1)) {
				m_ndxCurrent.setVerse(0);
				m_ndxCurrent.setWord(0);
				std::cerr << "\n*** Unknown Verse osisID : " << atts.value(ndx).toStdString() << "\n";
			} else if ((m_ndxCurrent.book() != static_cast<unsigned int>(nBk+1)) || (m_ndxCurrent.chapter() != lstOsisID.at(1).toUInt())) {
				m_ndxCurrent.setVerse(0);
				m_ndxCurrent.setWord(0);
				std::cerr << "\n*** Verse osisID doesn't match Chapter osisID : " << atts.value(ndx).toStdString() << "\n";
			} else {
				m_ndxCurrent.setVerse(lstOsisID.at(2).toUInt());
				m_ndxCurrent.setWord(0);
				std::cerr << ".";
				m_bInVerse = true;
				assert(m_bInLemma == false);
				if (m_bInLemma) std::cerr << "\n*** Error: Missing end of Lemma\n";
				m_bInLemma = false;
				assert(m_bInTransChangeAdded == false);
				if (m_bInTransChangeAdded) std::cerr << "\n*** Error: Missing end of TransChange Added\n";
				m_bInTransChangeAdded = false;
				assert(m_bInNotes == false);
				if (m_bInNotes) std::cerr << "\n*** Error: Missing end of Notes\n";
				m_bInNotes = false;
				assert(m_bInColophon == false);
				if (m_bInColophon) std::cerr << "\n*** Error: Missing end of Colophon\n";
				m_bInColophon = false;
				assert(m_bInSubtitle == false);
				if (m_bInSubtitle) std::cerr << "\n*** Error: Missing end of Subtitle\n";
				m_bInSubtitle = false;
				assert(m_bInForeignText == false);
				if (m_bInForeignText) std::cerr << "\n*** Error: Missing end of Foreign text\n";
				m_bInForeignText = false;
				assert(m_bInWordsOfJesus == false);
				if (m_bInWordsOfJesus) std::cerr << "\n*** Error: Missing end of Words-of-Jesus\n";
				m_bInWordsOfJesus = false;
				assert(m_bInDivineName == false);
				if (m_bInDivineName) std::cerr << "\n*** Error: Missing end of Divine Name\n";
				m_bInDivineName = false;
				m_pBibleDatabase->m_EntireBible.m_nNumVrs++;
				assert(static_cast<unsigned int>(nTst) <= m_pBibleDatabase->m_lstTestaments.size());
				m_pBibleDatabase->m_lstTestaments[nTst-1].m_nNumVrs++;
				assert(m_pBibleDatabase->m_lstBooks.size() > static_cast<unsigned int>(nBk));
				m_pBibleDatabase->m_lstBooks[nBk].m_nNumVrs++;
				m_pBibleDatabase->m_mapChapters[CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0)].m_nNumVrs++;
				CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
				if ((m_ndxCurrent.book() == PSALMS_BOOK_NUM) && (m_ndxCurrent.chapter() == 119) && (((m_ndxCurrent.verse()-1)%8) == 0)) {
					verse.setText(verse.text() + g_chrParseTag);
					verse.m_lstParseStack.push_back("M:");
				}
			}
		}
	} else if ((m_bInVerse) && (localName.compare("note", Qt::CaseInsensitive) == 0)) {
		m_bInNotes = true;
	} else if ((m_bInVerse) && (!m_bInNotes) && (!m_bInSubtitle) && (!m_bInColophon) && (localName.compare("milestone", Qt::CaseInsensitive) == 0)) {
		//	PTE_MARKER			Example: {verse}[osisID=Gen.5.21]{milestone}[marker=¶,type=x-p]{/milestone}
		//	PTE_MARKER_ADDED	Example: {verse}[osisID=Gen.5.3]{milestone}[marker=¶,subType=x-added,type=x-p]{/milestone}
		//	PTE_EXTRA			Example: {verse}[osisID=Gen.5.6]{milestone}[type=x-extra-p]{/milestone}
		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];

		if (((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("x-p", Qt::CaseInsensitive) == 0)) {
			if (((ndx = findAttribute(atts, "subType")) != -1) && (atts.value(ndx).compare("x-added", Qt::CaseInsensitive) == 0)) {
				verse.m_nPilcrow = CVerseEntry::PTE_MARKER_ADDED;
			} else{
				verse.m_nPilcrow = CVerseEntry::PTE_MARKER;
			}
		} else if (((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("x-extra-p", Qt::CaseInsensitive) == 0)) {
			verse.m_nPilcrow = CVerseEntry::PTE_EXTRA;
		}
	} else if ((m_bInVerse) && (!m_bInNotes) && (!m_bInSubtitle) && (!m_bInColophon) && (localName.compare("w", Qt::CaseInsensitive) == 0)) {
		m_bInLemma = true;
		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
		verse.setText(verse.text() + g_chrParseTag);
		verse.m_lstParseStack.push_back("L:" + stringifyAttributes(atts));
	} else if ((m_bInVerse) && (!m_bInNotes) && (!m_bInSubtitle) && (!m_bInColophon) && (localName.compare("transChange", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "type");
		if ((ndx != -1) && (atts.value(ndx).compare("added", Qt::CaseInsensitive) == 0)) {
			m_bInTransChangeAdded = true;
			CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
			verse.setText(verse.text() + g_chrParseTag);
			verse.m_lstParseStack.push_back("T:");
		}
	} else if ((m_bInVerse) && (!m_bInNotes) && (!m_bInSubtitle) && (!m_bInColophon) && (localName.compare("q", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "who");
		if ((ndx != -1) && (atts.value(ndx).compare("Jesus", Qt::CaseInsensitive) == 0)) {
			m_bInWordsOfJesus = true;
			CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
			verse.setText(verse.text() + g_chrParseTag);
			verse.m_lstParseStack.push_back("J:");
		}
	} else if ((m_bInVerse) && (!m_bInNotes) && (!m_bInSubtitle) && (!m_bInColophon) && (localName.compare("divineName", Qt::CaseInsensitive) == 0)) {
		m_bInDivineName = true;
		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
		verse.setText(verse.text() + g_chrParseTag);
		verse.m_lstParseStack.push_back("D:");
	}

	// Note: In the m_lstParseStack, we'll push values on as follows:
	//			L:<attrs>		-- Lemma Start
	//			l:				-- Lemma End
	//			T:				-- TransChange Added Start
	//			t:				-- TransChange Added End
	//			J:				-- Words of Jesus Start
	//			j:				-- Words of Jesus End
	//			D:				-- Divine Name Start
	//			d:				-- Divine Name End
	//			M:				-- Hebrew Psalm 119 Marker




/*
	m_lstElementNames.append(localName);
	std::cout << "{" << localName.toStdString() << "}";
	std::cout << "[";
	for (int i = 0; i < atts.count(); ++i) {
		if (i) std::cout << ",";
		std::cout << atts.localName(i).toStdString() << "=" << atts.value(i).toStdString();
//		if (atts.localName(i).compare("type", Qt::CaseInsensitive) == 0) {
			m_lstAttrNames.append(atts.localName(i) + "=" + atts.value(i));
//		}
	}
	std::cout << "]";
*/



	return true;
}

bool COSISXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);


	if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		m_bCaptureTitle = false;
		m_bInSubtitle = false;
	} else if (localName.compare("foreign", Qt::CaseInsensitive) == 0) {
		m_bInForeignText = false;
	} else if ((m_bInColophon) && (localName.compare("div", Qt::CaseInsensitive) == 0)) {
		m_bInColophon = false;
	} else if ((!m_bInVerse) && (localName.compare("chapter", Qt::CaseInsensitive) == 0)) {
		m_ndxCurrent = CRelIndex();
		std::cerr << "\n";
// Technically, we shouldn't have a chapter inside verse, but some modules use it as a special inner marking (like FrePGR, for example):
//		assert(m_bInVerse == false);
//		if (m_bInVerse) {
//			std::cerr << "\n*** End-of-Chapter found before End-of-Verse\n";
//			m_bInVerse = false;
//		}
	} else if ((m_bInVerse) && (localName.compare("verse", Qt::CaseInsensitive) == 0)) {
		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];

		QString strTemp = verse.text();

		unsigned int nWordCount = 0;
		bool bInWord = false;
		QString strWord;
		QString strRichWord;
		QStringList lstWords;
		QStringList lstRichWords;
		bool bHaveDoneTemplateWord = false;				// Used to tag words crossing parse-stack boundary (i.e. half the word is inside the parse operator and half is outside, like the word "inasmuch")
		while (!strTemp.isEmpty()) {
			bool bIsHyphen = g_strHyphens.contains(strTemp.at(0));
			bool bIsApostrophe = g_strApostrophes.contains(strTemp.at(0));
			if (strTemp.at(0) == g_chrParseTag) {
				if (bInWord) {
					if (!bHaveDoneTemplateWord) verse.m_strTemplate += QString("w");
					bHaveDoneTemplateWord = true;
				}
				assert(!verse.m_lstParseStack.isEmpty());
				if (!verse.m_lstParseStack.isEmpty()) {
					QString strParse = verse.m_lstParseStack.at(0);
					verse.m_lstParseStack.pop_front();
					int nPos = strParse.indexOf(':');
					assert(nPos != -1);		// Every ParseStack entry must contain a ':'
					QString strOp = strParse.left(nPos);
					if (strOp.compare("L") == 0) {
						// TODO : Parse Lemma for Strongs/Morph
					} else if (strOp.compare("l") == 0) {
						// TODO : End Lemma
					} else if (strOp.compare("T") == 0) {
						verse.m_strTemplate += "T";
					} else if (strOp.compare("t") == 0) {
						verse.m_strTemplate += "t";
					} else if (strOp.compare("J") == 0) {
						verse.m_strTemplate += "J";
					} else if (strOp.compare("j") == 0) {
						verse.m_strTemplate += "j";
					} else if (strOp.compare("D") == 0) {
						verse.m_strTemplate += "D";
					} else if (strOp.compare("d") == 0) {
						verse.m_strTemplate += "d";
					} else if (strOp.compare("M") == 0) {
						verse.m_strTemplate += "M";
						// For special Ps 119 Hebrew markers, add x-extra-p Pilcrow to
						//		add a pseudo-paragraph break if there currently isn't
						//		one, as it makes these more readable:
						if (verse.m_nPilcrow == CVerseEntry::PTE_NONE)
							verse.m_nPilcrow = CVerseEntry::PTE_EXTRA;
					} else {
						assert(false);		// Unknown ParseStack Operator!
					}
				}
			} else if ((strTemp.at(0).unicode() < 128) ||
				(g_strNonAsciiNonWordChars.contains(strTemp.at(0))) ||
				(strTemp.at(0) == g_chrPilcrow) ||
				(strTemp.at(0) == g_chrParseTag) ||
				(bIsHyphen) ||
				(bIsApostrophe)) {
				if ((g_strAsciiWordChars.contains(strTemp.at(0))) ||
					((bIsHyphen) && (!strRichWord.isEmpty())) ||				// Don't let words start with hyphen or apostrophe
					((bIsApostrophe) && (!strRichWord.isEmpty()))) {
					bInWord = true;
					if (bIsHyphen) {
						strWord += '-';
					} else if (bIsApostrophe) {
						strWord += '\'';
					} else strWord += strTemp.at(0);
					strRichWord += strTemp.at(0);
				} else {
					if (bInWord) {
						assert(!strRichWord.isEmpty());
						assert(!strWord.isEmpty());

						if ((strRichWord.size() == 1) &&
							((g_strHyphens.contains(strRichWord.at(0))) ||
							 (g_strApostrophes.contains(strRichWord.at(0))))) {
							// Don't count words that are only a hyphen or apostrophe:
							verse.m_strTemplate += strRichWord;
						} else {
							QString strPostTemplate;		// Needed so we get the "w" marker in the correct place
							// Remove trailing hyphens from words and put them in the template.
							//		We'll keep trailing apostophes for posessive words, like: "Jesus'":
							while ((!strRichWord.isEmpty()) && (g_strHyphens.contains(strRichWord.at(strRichWord.size()-1)))) {
								assert(!strWord.isEmpty());
								strPostTemplate += strRichWord.at(strRichWord.size()-1);
								strRichWord = strRichWord.left(strRichWord.size()-1);
								strWord = strWord.left(strWord.size()-1);
							}
							if (!strRichWord.isEmpty()) {
								nWordCount++;
								m_ndxCurrent.setWord(verse.m_nNumWrd + nWordCount);
								if (!bHaveDoneTemplateWord) verse.m_strTemplate += QString("w");
								lstWords.append(strWord);
								lstRichWords.append(strRichWord);
							}
							verse.m_strTemplate += strPostTemplate;
						}
						strWord.clear();
						strRichWord.clear();
						bInWord = false;
					}
					if (strTemp.at(0) != g_chrPilcrow) {
						if (strTemp.at(0) == g_chrParseTag) {
							std::cerr << "\n*** WARNING: Text contains our special parse tag character and may cause parsing issues\nTry recompiling using a different g_chrParseTag character!\n";
						}
						verse.m_strTemplate += strTemp.at(0);
					} else {
						// If we see a pilcrow marker in the text, but the OSIS didn't declare it, go ahead and add it
						//	as a marker, but flag it of type "added":
						if (verse.m_nPilcrow == CVerseEntry::PTE_NONE) verse.m_nPilcrow = CVerseEntry::PTE_MARKER_ADDED;
					}
					bHaveDoneTemplateWord = false;
				}
			} else {
				if (!m_strParsedUTF8Chars.contains(strTemp.at(0))) m_strParsedUTF8Chars += strTemp.at(0);

				bInWord = true;
				if (strTemp.at(0) == QChar(0x00C6)) {				// U+00C6	&#198;		AE character
					strWord += "Ae";
				} else if (strTemp.at(0) == QChar(0x00E6)) {		// U+00E6	&#230;		ae character
					strWord += "ae";
				} else if (strTemp.at(0) == QChar(0x0132)) {		// U+0132	&#306;		IJ character
					strWord += "IJ";
				} else if (strTemp.at(0) == QChar(0x0133)) {		// U+0133	&#307;		ij character
					strWord += "ij";
				} else if (strTemp.at(0) == QChar(0x0152)) {		// U+0152	&#338;		OE character
					strWord += "Oe";
				} else if (strTemp.at(0) == QChar(0x0153)) {		// U+0153	&#339;		oe character
					strWord += "oe";
				} else {
					strWord += strTemp.at(0);			// All other UTF-8 leave untranslated
				}
				strRichWord += strTemp.at(0);
			}

			strTemp = strTemp.right(strTemp.size()-1);
		}

		assert(verse.m_lstParseStack.isEmpty());		// We should have exhausted the stack above!

		if (bInWord) {
			if ((strRichWord.size() == 1) &&
				((g_strHyphens.contains(strRichWord.at(0))) ||
				 (g_strApostrophes.contains(strRichWord.at(0))))) {
				// Don't count words that are only a hyphen or apostrophe:
				verse.m_strTemplate += strRichWord;
			} else {
				QString strPostTemplate;		// Needed so we get the "w" marker in the correct place
				// Remove trailing hyphens from words and put them in the template.
				//		We'll keep trailing apostophes for posessive words, like: "Jesus'":
				while ((!strRichWord.isEmpty()) && (g_strHyphens.contains(strRichWord.at(strRichWord.size()-1)))) {
					assert(!strWord.isEmpty());
					strPostTemplate += strRichWord.at(strRichWord.size()-1);
					strRichWord = strRichWord.left(strRichWord.size()-1);
					strWord = strWord.left(strWord.size()-1);
				}
				if (!strRichWord.isEmpty()) {
					nWordCount++;
					m_ndxCurrent.setWord(verse.m_nNumWrd + nWordCount);
					if (!bHaveDoneTemplateWord) verse.m_strTemplate += QString("w");
					lstWords.append(strWord);
					lstRichWords.append(strRichWord);
				}
				verse.m_strTemplate += strPostTemplate;
			}
			strWord.clear();
			strRichWord.clear();
			bInWord = false;
		}
		bHaveDoneTemplateWord = false;

		m_pBibleDatabase->m_EntireBible.m_nNumWrd += nWordCount;
		m_pBibleDatabase->m_lstTestaments[m_pBibleDatabase->m_lstBooks.at(m_ndxCurrent.book()-1).m_nTstNdx-1].m_nNumWrd += nWordCount;
		m_pBibleDatabase->m_lstBooks[m_ndxCurrent.book()-1].m_nNumWrd += nWordCount;
		m_pBibleDatabase->m_mapChapters[CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0)].m_nNumWrd += nWordCount;
		verse.m_nNumWrd += nWordCount;
		verse.m_lstWords.append(lstWords);
		verse.m_lstRichWords.append(lstRichWords);



//std::cout << m_pBibleDatabase->PassageReferenceText(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)).toStdString() << "\n";
//std::cout << verse.text().toStdString() << "\n" << verse.m_strTemplate.toStdString() << "\n" << verse.m_lstWords.join(",").toStdString() << "\n" << QString("Words: %1\n").arg(verse.m_nNumWrd).toStdString();

		assert(static_cast<unsigned int>(verse.m_strTemplate.count('w')) == verse.m_nNumWrd);
		if (static_cast<unsigned int>(verse.m_strTemplate.count('w')) != verse.m_nNumWrd)
			std::cerr << "\n*** Error: Verse word count doesn't match template word count!!!\n";

		m_ndxCurrent.setVerse(0);
		m_ndxCurrent.setWord(0);
		m_bInVerse = false;
	} else if ((m_bInNotes) && (localName.compare("note", Qt::CaseInsensitive) == 0)) {
		m_bInNotes = false;
	} else if ((m_bInLemma) && (localName.compare("w", Qt::CaseInsensitive) == 0)) {
		m_bInLemma = false;
		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
		verse.setText(verse.text() + g_chrParseTag);
		verse.m_lstParseStack.push_back("l:");
	} else if ((m_bInTransChangeAdded) && (localName.compare("transChange", Qt::CaseInsensitive) == 0)) {
		m_bInTransChangeAdded = false;
		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
		verse.setText(verse.text() + g_chrParseTag);
		verse.m_lstParseStack.push_back("t:");
	} else if ((m_bInWordsOfJesus) && (localName.compare("q", Qt::CaseInsensitive) == 0)) {
		m_bInWordsOfJesus = false;
		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
		verse.setText(verse.text() + g_chrParseTag);
		verse.m_lstParseStack.push_back("j:");
	} else if ((m_bInDivineName) && (localName.compare("divineName", Qt::CaseInsensitive) == 0)) {
		m_bInDivineName = false;
		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
		verse.setText(verse.text() + g_chrParseTag);
		verse.m_lstParseStack.push_back("d:");
	}



//	std::cout << "{/" << localName.toStdString() << "}\n";

	return true;
}

bool COSISXmlHandler::characters(const QString &ch)
{
	QString strTemp = ch;
// TODO : REMOVE
//	strTemp.replace('\n', ' ');

	if (m_bCaptureTitle) {
		m_pBibleDatabase->m_strDescription = strTemp;
		std::cerr << "Title: " << strTemp.toUtf8().data() << "\n";
	} else if (m_bInColophon) {
		if (m_ndxColophon.isSet()) {
			CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[m_ndxColophon];
			footnote.setText(footnote.text() + strTemp);
		}
	} else if ((m_bInSubtitle) && (!m_bInForeignText)) {
		CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[m_ndxSubtitle];
		footnote.setText(footnote.text() + strTemp);
	} else if ((m_bInVerse) && (!m_bInNotes) && (!m_bInForeignText)) {

		assert((m_ndxCurrent.book() != 0) && (m_ndxCurrent.chapter() != 0) && (m_ndxCurrent.verse() != 0));
//		std::cout << strTemp.toStdString();

		CVerseEntry &verse = (m_pBibleDatabase->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(0, m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
//		verse.setText(verse.text() + strTemp);
		verse.setText(verse.text() + (m_bInDivineName ? strTemp.toUpper() : strTemp));

		assert(!strTemp.contains(g_chrParseTag, Qt::CaseInsensitive));
		if (strTemp.contains(g_chrParseTag, Qt::CaseInsensitive)) {
			std::cerr << "\n*** ERROR: Text contains the special parse tag!!  Change the tag in KJVDataParse and try again!\n";
		}

	} else if ((m_bInVerse) && (m_bInNotes)) {
		CFootnoteEntry &footnote = ((!m_bInLemma) ? m_pBibleDatabase->m_mapFootnotes[CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)] :
													m_pBibleDatabase->m_mapFootnotes[m_ndxCurrent]);
		footnote.setText(footnote.text() + strTemp);
	}



//	std::cout << ch.toStdString();

	return true;
}

bool COSISXmlHandler::error(const QXmlParseException &exception)
{
	std::cerr << QString("\n\n*** %1\n").arg(exception.message()).toStdString();
	return true;
}

// ============================================================================
// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	const char *pstrFilename = NULL;

	if (argc < 3) {
		std::cerr << QString("Usage: %1 <OSIS-Database> <datafile-path>\n\n").arg(argv[0]).toStdString();
		std::cerr << QString("Reads and parses the OSIS database and outputs all of the CSV files\n").toStdString();
		std::cerr << QString("    necessary to import into KJPBS\n\n").toStdString();
		return -1;
	}

	QDir dirOutput(argv[2]);
	if (!dirOutput.exists()) {
		std::cerr << QString("\n\n*** Output path \"%1\" doesn't exist\n\n").arg(dirOutput.canonicalPath()).toStdString();
		return -2;
	}

	pstrFilename = argv[1];

	QFile fileOSIS;

	fileOSIS.setFileName(QString(pstrFilename));
	if (!fileOSIS.open(QIODevice::ReadOnly)) {
		std::cerr << QString("\n\n*** Failed to open OSIS database \"%1\"\n").arg(pstrFilename).toStdString();
		return -3;
	}

	QXmlInputSource xmlInput(&fileOSIS);
	QXmlSimpleReader xmlReader;
	COSISXmlHandler xmlHandler("http://www.bibletechnologies.net/2003/OSIS/namespace");

	xmlReader.setContentHandler(&xmlHandler);
	xmlReader.setErrorHandler(&xmlHandler);
//	xmlReader.setFeature("http://www.bibletechnologies.net/2003/OSIS/namespace", true);

	bool bOK = xmlReader.parse(xmlInput);
	if (!bOK) {
		std::cerr << QString("\n\n*** Failed to parse OSIS database \"%1\"\n%2\n").arg(pstrFilename).arg(xmlHandler.errorString()).toStdString();
		return -4;
	}

	const CBibleDatabase *pBibleDatabase = xmlHandler.bibleDatabase();

	// ------------------------------------------------------------------------

	QFile fileTestaments;	// Testaments CSV being written
	QFile fileBooks;		// Books CSV being written (Originally known as "TOC")
	QFile fileChapters;		// Chapters CSV being written (Originally known as "Layout")
	QFile fileVerses;		// Verses CSV being written (Originally known as "BOOKS")
	QFile fileWords;		// Words CSV being written
	QFile fileFootnotes;	// Footnotes CSV being written
	QFile fileWordSummary;	// Words Summary CSV being written

	fileTestaments.setFileName(dirOutput.absoluteFilePath("TESTAMENT.csv"));
	if (!fileTestaments.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Testament Output File \"%1\"\n").arg(fileTestaments.fileName()).toStdString();
		return -5;
	}

	fileTestaments.write(QString(QChar(0xFEFF)).toUtf8());			// UTF-8 BOM
	fileTestaments.write(QString("TstNdx, TstName\r\n").toUtf8());
	for (unsigned int nTst=1; nTst<=pBibleDatabase->bibleEntry().m_nNumTst; ++nTst) {
		fileTestaments.write(QString("%1,\"%2\"\r\n").arg(nTst).arg(pBibleDatabase->testamentEntry(nTst)->m_strTstName).toUtf8());
	}
	std::cerr << QFileInfo(fileTestaments).fileName().toStdString() << "\n";
	fileTestaments.close();

	fileBooks.setFileName(dirOutput.absoluteFilePath("TOC.csv"));
	if (!fileBooks.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Books Output File \"%1\"\n").arg(fileBooks.fileName()).toStdString();
		return -6;
	}

	fileBooks.write(QString(QChar(0xFEFF)).toUtf8());			// UTF-8 BOM
	fileBooks.write(QString("BkNdx,TstBkNdx,TstNdx,BkName,BkAbbr,TblName,NumChp,NumVrs,NumWrd,Cat,Desc\r\n").toUtf8());

	fileChapters.setFileName(dirOutput.absoluteFilePath("LAYOUT.csv"));
	if (!fileChapters.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Chapters Output File \"%1\"\n").arg(fileChapters.fileName()).toStdString();
		return -7;
	}

	fileChapters.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileChapters.write(QString("BkChpNdx,NumVrs,NumWrd,BkAbbr,ChNdx\r\n").toUtf8());


	TChapterVerseCounts lstChapterVerseCounts;
	// mapWordList will be for ALL forms of all words so that we can get mapping/counts
	//	for all unique forms of words.  Words in this map will NOT be indexed by the
	//	lowercase nor the base-form of the word, but by the actual word itself.
	//	Then, once we've used this to build all of the unique indexes for the different
	//	word forms, we'll consolidate them and ultimately build the database entries which will
	//	contain the wordlists by lowercase word and have the main word value and all
	//	alternates:
	TWordListMap mapWordList;				// mapWordList is indexed by the Word form as-is (no changes in case)
	TAltWordListMap mapAltWordList;			// mapAltWordList is indexed by the LowerCase form of the Word

	unsigned int nWordAccum = 0;
	for (unsigned int nBk=1; nBk<=qMax(pBibleDatabase->bibleEntry().m_nNumBk, NUM_BK); ++nBk) {
		if (nBk > NUM_BK) {
			std::cerr << QString("\n*** ERROR: Module has extra Book : %1\n").arg(nBk).toStdString();
			lstChapterVerseCounts.push_back(QStringList());
		} else {
			lstChapterVerseCounts.push_back(g_arrChapterVerseCounts[nBk-1].split(","));
		}
		const CBookEntry *pBook = pBibleDatabase->bookEntry(nBk);
		if ((pBook == NULL) || (pBook->m_strTblName.isEmpty())) {
			std::cerr << QString("\n*** ERROR: Module is missing Book : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, 0, 0, 0))).toStdString();
			continue;
		}

		// BkNdx,TstBkNdx,TstNdx,BkName,BkAbbr,TblName,NumChp,NumVrs,NumWrd,Cat,Desc
		fileBooks.write(QString("%1,%2,%3,\"%4\",%5,%6,%7,%8,%9,\"%10\",\"%11\"\r\n")
						.arg(nBk)							// 1
						.arg(pBook->m_nTstBkNdx)			// 2
						.arg(pBook->m_nTstNdx)				// 3
						.arg(pBook->m_strBkName)			// 4
						.arg(pBook->m_strBkAbbr)			// 5
						.arg(pBook->m_strTblName)			// 6
						.arg(pBook->m_nNumChp)				// 7
						.arg(pBook->m_nNumVrs)				// 8
						.arg(pBook->m_nNumWrd)				// 9
						.arg(pBook->m_strCat)				// 10
						.arg(pBook->m_strDesc)				// 11
						.toUtf8());

		fileVerses.setFileName(dirOutput.absoluteFilePath(QString("BOOK_%1_%2.csv").arg(nBk, 2, 10, QChar('0')).arg(pBook->m_strTblName)));
		if (!fileVerses.open(QIODevice::WriteOnly)) {
			std::cerr << QString("\n\n*** Failed to open Verses Output File \"%1\"\n").arg(fileVerses.fileName()).toStdString();
			return -8;
		}

		fileVerses.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
		fileVerses.write(QString("ChpVrsNdx,NumWrd,nPilcrow,PText,RText\r\n").toUtf8());

		std::cerr << QFileInfo(fileVerses).fileName().toStdString();

		unsigned int nChapterWordAccum = 0;
		unsigned int nChaptersExpected = qMax(pBook->m_nNumChp, static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size()));
		for (unsigned int nChp=1; nChp<=nChaptersExpected; ++nChp) {
			if (nChp > static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) {
				std::cerr << QString("\n*** ERROR: Module has extra Chapter : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toStdString();
			}
			const CChapterEntry *pChapter = pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0));
			if (pChapter == NULL) {
				std::cerr << QString("\n*** ERROR: Module is missing Chapter : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toStdString();
				continue;
			}

			std::cerr << ".";

			// BkChpNdx,NumVrs,NumWrd,BkAbbr,ChNdx
			fileChapters.write(QString("%1,%2,%3,%4,%5\r\n")
							   .arg(CRelIndex(0,0,nBk,nChp).index())		// 1
							   .arg(pChapter->m_nNumVrs)					// 2
							   .arg(pChapter->m_nNumWrd)					// 3
							   .arg(pBook->m_strBkAbbr)						// 4
							   .arg(nChp)									// 5
							   .toUtf8());

//			std::cout << QString("%1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toStdString();
			unsigned int nVerseWordAccum = 0;
			unsigned int nVersesExpected = qMax(pChapter->m_nNumVrs, static_cast<unsigned int>((nChp <= static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) ? lstChapterVerseCounts.at(nBk-1).at(nChp-1).toUInt() : 0));
			for (unsigned int nVrs=1; nVrs<=nVersesExpected; ++nVrs) {
				if (nVrs > static_cast<unsigned int>((nChp <= static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) ? lstChapterVerseCounts.at(nBk-1).at(nChp-1).toUInt() : 0)) {
					std::cerr << QString("\n*** ERROR: Module has extra Verse : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toStdString();
				}
				const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
				if (pVerse == NULL) {
					std::cerr << QString("\n*** ERROR: Module is missing Verse : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toStdString();
					continue;
				}
//				std::cout << QString("%1 : %2\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).arg(pVerse->m_strTemplate).toStdString();

				nVerseWordAccum += pVerse->m_nNumWrd;
				nWordAccum += pVerse->m_nNumWrd;
				(const_cast<CVerseEntry*>(pVerse))->m_nWrdAccum = nWordAccum;

				assert(pBibleDatabase->NormalizeIndexNoAccum(CRelIndex(nBk, nChp, nVrs, 1)) == (pVerse->m_nWrdAccum-pVerse->m_nNumWrd+1));
				assert(pBibleDatabase->DenormalizeIndexNoAccum(pVerse->m_nWrdAccum-pVerse->m_nNumWrd+1) == CRelIndex(nBk, nChp, nVrs, 1).index());

				QStringList lstTempRich = CVerseTextRichifier::parse(CRelIndex(nBk,nChp, nVrs, 0), pBibleDatabase, pVerse, CVerseTextRichifierTags(), false).split('\"');
				QString strBuffRich = lstTempRich.join("\"\"");
				QStringList lstTempPlain = CVerseTextRichifier::parse(CRelIndex(nBk,nChp, nVrs, 0), pBibleDatabase, pVerse, CVerseTextPlainRichifierTags(), false).split('\"');
				QString strBuffPlain = lstTempPlain.join("\"\"");

				// ChpVrsNdx,NumWrd,nPilcrow,PText,RText
				fileVerses.write(QString("%1,%2,%3,\"%4\",\"%5\"\r\n")
								 .arg(CRelIndex(0,0,nChp,nVrs).index())		// 1
								 .arg(pVerse->m_nNumWrd)					// 2
								 .arg(pVerse->m_nPilcrow)					// 3
								 .arg(strBuffPlain)							// 4
								 .arg(strBuffRich)							// 5
								 .toUtf8());


				// Needs to be after we calculate nWordAccum above so we can output anchor tags:
//				QString strTemp = CVerseTextRichifier::parse(pVerse, CVerseTextRichifierTags(), false);
//				std::cout << QString("%1 : %2\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).arg(strTemp).toUtf8().data();


/*
				std::cout << g_arrBooks[nBk-1].m_strOsisAbbr.toStdString() << QString(" %1:%2 : ").arg(nChp).arg(nVrs).toStdString();
				if (pVerse->m_nPilcrow == CVerseEntry::PTE_NONE) {
					std::cout << "false \n";
				} else {
					std::cout << "true ";
					switch (pVerse->m_nPilcrow) {
						case CVerseEntry::PTE_MARKER:
							std::cout << "(Marker)";
							break;
						case CVerseEntry::PTE_MARKER_ADDED:
							std::cout << "(Added)";
							break;
						case CVerseEntry::PTE_EXTRA:
							std::cout << "(Extra)";
							break;
						default:
							break;
					}
					std::cout << "\n";
				}
*/

				// Now use the words we've gathered from this verse to build the Word Lists and Concordance:
				assert(pVerse->m_nNumWrd == static_cast<unsigned int>(pVerse->m_lstWords.size()));
				for (unsigned int nWrd=1; nWrd<=pVerse->m_nNumWrd; ++nWrd) {
					QString strWord = pVerse->m_lstWords.at(nWrd-1);
					CWordEntry &wordEntry = mapWordList[strWord];
					TAltWordSet &wordSet = mapAltWordList[strWord.toLower()];
					wordSet.insert(strWord);
					wordEntry.m_ndxNormalizedMapping.push_back(pVerse->m_nWrdAccum-pVerse->m_nNumWrd+nWrd);
				}
			}
			if (nVerseWordAccum != pChapter->m_nNumWrd) {
				std::cerr << QString("\n*** Error: %1 Chapter Word Count (%2) doesn't match sum of Verse Word Counts (%3)!\n")
												.arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0)))
												.arg(pChapter->m_nNumWrd)
												.arg(nVerseWordAccum)
												.toStdString();
			}
			nChapterWordAccum += pChapter->m_nNumWrd;
			(const_cast<CChapterEntry*>(pChapter))->m_nWrdAccum = nWordAccum;
		}
		if (nChapterWordAccum != pBook->m_nNumWrd) {
			std::cerr << QString("\n*** Error: %1 Book Word Count (%2) doesn't match sum of Chapter Word Counts (%3)!\n")
												.arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, 0, 0 ,0)))
												.arg(pBook->m_nNumWrd)
												.arg(nChapterWordAccum)
												.toStdString();
		}
		(const_cast<CBookEntry*>(pBook))->m_nWrdAccum = nWordAccum;

		fileVerses.close();

		std::cerr << "\n";
	}

	std::cerr << QFileInfo(fileChapters).fileName().toStdString() << "\n";
	fileChapters.close();

	std::cerr << QFileInfo(fileBooks).fileName().toStdString() << "\n";
	fileBooks.close();

	// ------------------------------------------------------------------------

	fileWords.setFileName(dirOutput.absoluteFilePath("WORDS.csv"));
	if (!fileWords.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Words Output File \"%1\"\n").arg(fileWords.fileName()).toStdString();
		return -9;
	}
	std::cerr << QFileInfo(fileWords).fileName().toStdString();

	fileWords.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileWords.write(QString("WrdNdx,Word,bIndexCasePreserve,NumTotal,AltWords,AltWordCounts,NormalMap\r\n").toUtf8());

	unsigned int nWordIndex = 0;

	// We've now built a list of word indexes and alternate forms, and now we
	//	need to take this list and convert it to the form of the database:
	TWordListMap &mapDbWordList = const_cast<TWordListMap &>(pBibleDatabase->mapWordList());
	for (TAltWordListMap::const_iterator itrUniqWrd = mapAltWordList.begin(); itrUniqWrd != mapAltWordList.end(); ++itrUniqWrd) {
		const TAltWordSet &setAltWords = itrUniqWrd->second;
		CWordEntry &wordEntryDb = mapDbWordList[itrUniqWrd->first];
		wordEntryDb.m_bCasePreserve = false;
		for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
			TWordListMap::const_iterator itrWrd = mapWordList.find(*itrAltWrd);
			if (itrWrd == mapWordList.end()) {
				std::cerr << QString("\n*** Error: %1 -> %2 -- Couldn't Find it (something bad happened!)\n").arg(itrUniqWrd->first).arg(*itrAltWrd).toUtf8().data();
				continue;
			}
			wordEntryDb.m_lstAltWords.push_back(*itrAltWrd);
			wordEntryDb.m_lstAltWordCount.push_back(itrWrd->second.m_ndxNormalizedMapping.size());
			wordEntryDb.m_ndxNormalizedMapping.insert(wordEntryDb.m_ndxNormalizedMapping.end(), itrWrd->second.m_ndxNormalizedMapping.begin(), itrWrd->second.m_ndxNormalizedMapping.end());
			wordEntryDb.m_strWord = WordFromWordSet(setAltWords);
			if (isSpecialWord(*itrAltWrd)) wordEntryDb.m_bCasePreserve = true;
		}

		assert(wordEntryDb.m_lstAltWords.size() == wordEntryDb.m_lstAltWordCount.size());
		assert(wordEntryDb.m_lstAltWords.size() > 0);

		if ((nWordIndex % 100) == 0) std::cerr << ".";

		// WrdNdx,Word,bIndexCasePreserve,NumTotal,AltWords,AltWordCounts,NormalMap

		nWordIndex++;
		fileWords.write(QString("%1,\"%2\",%3,%4,").arg(nWordIndex).arg(wordEntryDb.m_strWord).arg(wordEntryDb.m_bCasePreserve ? 1 :0).arg(wordEntryDb.m_ndxNormalizedMapping.size()).toUtf8());
		for (int i=0; i<wordEntryDb.m_lstAltWords.size(); ++i) {
			fileWords.write(QString((i == 0) ? "\"" : ",").toUtf8());
			fileWords.write(wordEntryDb.m_lstAltWords.at(i).toUtf8());
		}
		fileWords.write(QString("\",").toUtf8());
		for (int i=0; i<wordEntryDb.m_lstAltWordCount.size(); ++i) {
			fileWords.write(QString((i == 0) ? "\"" : ",").toUtf8());
			fileWords.write(QString("%1").arg(wordEntryDb.m_lstAltWordCount.at(i)).toUtf8());
		}
		fileWords.write(QString("\",").toUtf8());
		for (unsigned int i=0; i<wordEntryDb.m_ndxNormalizedMapping.size(); ++i) {
			fileWords.write(QString((i == 0) ? "\"" : ",").toUtf8());
			fileWords.write(QString("%1").arg(wordEntryDb.m_ndxNormalizedMapping.at(i)).toUtf8());
		}
		fileWords.write(QString("\"\r\n").toUtf8());
	}

	fileWords.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

	fileWordSummary.setFileName(dirOutput.absoluteFilePath("WORDS_summary.csv"));
	if (!fileWordSummary.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Words Summary Output File \"%1\"\n").arg(fileWordSummary.fileName()).toStdString();
		return -9;
	}
	std::cerr << QFileInfo(fileWordSummary).fileName().toStdString();

	unsigned int nTotalWordCount = 0;
	unsigned int arrTotalTestamentWordCounts[NUM_TST];
	memset(arrTotalTestamentWordCounts, 0, sizeof(arrTotalTestamentWordCounts));
	unsigned int arrTotalBookWordCounts[NUM_BK];
	memset(arrTotalBookWordCounts, 0, sizeof(arrTotalBookWordCounts));

	fileWordSummary.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileWordSummary.write(QString("\"Word\",\"AltWords\",\"Entire\nBible\"").toUtf8());
	for (unsigned int nTst=0; nTst<NUM_TST; ++nTst) {
		QString strTemp = g_arrstrTstNames[nTst];
		strTemp.replace(' ', '\n');
		fileWordSummary.write(QString(",\"%1\"").arg(strTemp).toUtf8());
	}
	for (unsigned int nBk=0; nBk<NUM_BK; ++nBk) {
		fileWordSummary.write(QString(",\"%1\"").arg(g_arrBooks[nBk].m_strName).toUtf8());
	}
	fileWordSummary.write(QString("\r\n").toUtf8());

	nWordIndex = 0;

// Use previously defined mapDbWordList:
//	TWordListMap &mapDbWordList = const_cast<TWordListMap &>(pBibleDatabase->mapWordList());
	for (TAltWordListMap::const_iterator itrUniqWrd = mapAltWordList.begin(); itrUniqWrd != mapAltWordList.end(); ++itrUniqWrd) {
		const TAltWordSet &setAltWords = itrUniqWrd->second;
		CWordEntry &wordEntryDb = mapDbWordList[itrUniqWrd->first];
		QString strAltWords;
		for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
			if (!strAltWords.isEmpty()) strAltWords += ",";
			strAltWords += *itrAltWrd;
		}

		fileWordSummary.write(QString("\"%1\",\"%2\",%3").arg(wordEntryDb.m_strWord).arg(strAltWords).arg(wordEntryDb.m_ndxNormalizedMapping.size()).toUtf8());

		assert(wordEntryDb.m_lstAltWords.size() == wordEntryDb.m_lstAltWordCount.size());
		assert(wordEntryDb.m_lstAltWords.size() > 0);

		if ((nWordIndex % 100) == 0) std::cerr << ".";
		nWordIndex++;

		unsigned int arrTestamentWordCounts[NUM_TST];
		memset(arrTestamentWordCounts, 0, sizeof(arrTestamentWordCounts));
		unsigned int arrBookWordCounts[NUM_BK];
		memset(arrBookWordCounts, 0, sizeof(arrBookWordCounts));

		for (TIndexList::const_iterator itr = wordEntryDb.m_ndxNormalizedMapping.begin(); itr != wordEntryDb.m_ndxNormalizedMapping.end(); ++itr) {
			CRelIndex ndx(pBibleDatabase->DenormalizeIndex(*itr));
			assert(ndx.isSet());
			assert(ndx.book() != 0);
			if (ndx.book() <= NUM_BK_OT) {
				arrTestamentWordCounts[0]++;
				arrTotalTestamentWordCounts[0]++;
			} else if (ndx.book() <= NUM_BK) {
				arrTestamentWordCounts[1]++;
				arrTotalTestamentWordCounts[1]++;
			} else {
				// Word in unknown Testament -- assert here??
			}
			if (ndx.book() <= NUM_BK) {
				arrBookWordCounts[ndx.book()-1]++;
				arrTotalBookWordCounts[ndx.book()-1]++;
			} else {
				// Word in unknwon Book -- assert here??
			}
			nTotalWordCount++;
		}

		for (unsigned int nTst=0; nTst<NUM_TST; ++nTst) {
			fileWordSummary.write(QString(",%1").arg(arrTestamentWordCounts[nTst]).toUtf8());
		}
		for (unsigned int nBk=0; nBk<NUM_BK; ++nBk) {
			fileWordSummary.write(QString(",%1").arg(arrBookWordCounts[nBk]).toUtf8());
		}

		fileWordSummary.write(QString("\r\n").toUtf8());
	}
	fileWordSummary.write(QString("\"\",\"\",%1").arg(nTotalWordCount).toUtf8());
	for (unsigned int nTst=0; nTst<NUM_TST; ++nTst) {
		fileWordSummary.write(QString(",%1").arg(arrTotalTestamentWordCounts[nTst]).toUtf8());
	}
	for (unsigned int nBk=0; nBk<NUM_BK; ++nBk) {
		fileWordSummary.write(QString(",%1").arg(arrTotalBookWordCounts[nBk]).toUtf8());
	}
	fileWordSummary.write(QString("\r\n").toUtf8());

	fileWordSummary.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

	unsigned int nFootnoteIndex = 0;

	fileFootnotes.setFileName(dirOutput.absoluteFilePath("FOOTNOTES.csv"));
	if (!fileFootnotes.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Footnotes Output File \"%1\"\n").arg(fileFootnotes.fileName()).toStdString();
		return -10;
	}
	std::cerr << QFileInfo(fileFootnotes).fileName().toStdString();

	fileFootnotes.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	fileFootnotes.write(QString("BkChpVrsWrdNdx,PFootnote,RFootnote\r\n").toUtf8());

	const TFootnoteEntryMap &mapFootnotes = pBibleDatabase->footnotesMap();
	for (TFootnoteEntryMap::const_iterator itrFootnotes = mapFootnotes.begin(); itrFootnotes != mapFootnotes.end(); ++itrFootnotes) {
		QStringList lstTempFootnote = (itrFootnotes->second).text().split('\"');
		QString strTempFootnote = lstTempFootnote.join("\"\"");
		// BkChpVrsWrdNdx,PFootnote,RFootnote
		fileFootnotes.write(QString("%1,\"%2\",\"%3\"\r\n")
							.arg((itrFootnotes->first).index())			// 1
							.arg(strTempFootnote)						// 2			-- TODO : FIX
							.arg(strTempFootnote)						// 3			-- TODO : FIX
							.toUtf8());

		if ((nFootnoteIndex % 100) == 0) std::cerr << ".";
		nFootnoteIndex++;
	}

	fileFootnotes.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

/*
	std::cerr << "Checking Indexes";
	for (unsigned int nBk=1; nBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(nBk);
		assert(pBook != NULL);
		for (unsigned int nChp=1; nChp<=pBook->m_nNumChp; ++nChp) {
			const CChapterEntry *pChapter = pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0));
			assert(pChapter != NULL);
			for (unsigned int nVrs=1; nVrs<=pChapter->m_nNumVrs; ++nVrs) {
				const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
				assert(pVerse != NULL);
				for (unsigned int nWrd=1; nWrd<=pVerse->m_nNumWrd; ++nWrd) {
					assert(pBibleDatabase->NormalizeIndex(CRelIndex(nBk, nChp, nVrs, nWrd)) == pBibleDatabase->NormalizeIndexNoAccum(CRelIndex(nBk, nChp, nVrs, nWrd)));
					assert(pBibleDatabase->DenormalizeIndex(pVerse->m_nWrdAccum-pVerse->m_nNumWrd+nWrd) == pBibleDatabase->DenormalizeIndexNoAccum(pVerse->m_nWrdAccum-pVerse->m_nNumWrd+nWrd));
					assert(pBibleDatabase->DenormalizeIndex(pBibleDatabase->NormalizeIndex(CRelIndex(nBk, nChp, nVrs, nWrd))) == CRelIndex(nBk, nChp, nVrs, nWrd).index());
				}
			}
		}
		std::cerr << ".";
	}
	std::cerr << "\n";
*/

	// ------------------------------------------------------------------------

/*
	std::cout << QString("Bible:  Testaments: %1  Books: %2  Chapters: %3  Verses: %4  Words: %5\n")
						.arg(pBibleDatabase->bibleEntry().m_nNumTst)
						.arg(pBibleDatabase->bibleEntry().m_nNumBk)
						.arg(pBibleDatabase->bibleEntry().m_nNumChp)
						.arg(pBibleDatabase->bibleEntry().m_nNumVrs)
						.arg(pBibleDatabase->bibleEntry().m_nNumWrd)
						.toUtf8().data();

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumTst; ++i) {
		std::cout << QString("%1 : Books: %2  Chapters: %3  Verses: %4  Words: %5\n")
						.arg(pBibleDatabase->testamentEntry(i)->m_strTstName)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumBk)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumChp)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumVrs)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumWrd)
						.toUtf8().data();
	}

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumBk; ++i) {
		std::cout << QString("%1 : Chapters: %2  Verses: %3  Words: %4\n")
						.arg(pBibleDatabase->bookEntry(i)->m_strBkName)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumChp)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumVrs)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumWrd)
						.toUtf8().data();
	}

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumBk; ++i) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(i);
		assert(pBook != NULL);
		for (unsigned int j=1; j<=pBook->m_nNumChp; ++j) {
			std::cout << QString("%1 Chapter %2 : Verses: %3  Words: %4\n")
						.arg(pBook->m_strBkName)
						.arg(j)
						.arg(pBibleDatabase->chapterEntry(CRelIndex(i, j, 0, 0))->m_nNumVrs)
						.arg(pBibleDatabase->chapterEntry(CRelIndex(i, j, 0, 0))->m_nNumWrd)
						.toUtf8().data();
		}
	}
*/

	// ------------------------------------------------------------------------


/*
	std::cout << "\n============================ Element Names  =================================\n";
	QStringList lstElements = xmlHandler.elementNames();
	lstElements.sort();
	lstElements.removeDuplicates();
	for (int i = 0; i < lstElements.count(); ++i) {
		std::cout << lstElements.at(i).toStdString() << "\n";
	}

	std::cout << "\n\n============================ Attribute Names  =================================\n";
	QStringList lstAttrib = xmlHandler.attrNames();
	lstAttrib.sort();
	lstAttrib.removeDuplicates();
	for (int i = 0; i < lstAttrib.count(); ++i) {
		std::cout << lstAttrib.at(i).toStdString() << "\n";
	}

*/

	// ------------------------------------------------------------------------

	QString strParsedUTF8 = xmlHandler.parsedUTF8Chars();
	std::cerr << "UTF8 Characters Parsed: \"" << strParsedUTF8.toUtf8().data() << "\"\n";
	for (int i = 0; i<strParsedUTF8.size(); ++i) {
		std::cerr << "    \"" << QString(strParsedUTF8.at(i)).toUtf8().data() << "\" (" << QString("%1").arg(strParsedUTF8.at(i).unicode(), 4, 16, QChar('0')).toUtf8().data() << ")\n";
	}

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}
