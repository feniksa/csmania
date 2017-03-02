#include "utils.h"

#include <QDebug>
#include <QRegExp>
#include <QTextCodec>
#include <QTextDocument>

namespace tabor
{
	bool containString(const QString& pattern, const QString& where)
	{
		QRegExp rx(pattern);
		return (rx.indexIn(where) == -1)?false:true;			
	}

	QString cp1251toUTF8(const QByteArray& byteArray)
	{
		QTextCodec *codec = QTextCodec::codecForName("UTF-8");
		return codec->toUnicode(byteArray);
	}

	QString html2plain(const QString& string)
	{
		QTextDocument text;
		text.setHtml(string);
		return text.toPlainText();
	}

	QString getRegexpValue(const QString& pattern, const QString& string)
	{
		QRegExp rx(pattern);

		if (rx.indexIn(string) != -1)  {
			return rx.cap(1);
		} else {
			//qWarning() << "Pattern" << pattern << "not found in string" << string;
		}
		
		return "";
	}

}