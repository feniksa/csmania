#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace tabor
{
	bool containString(const QString& pattern, const QString& where);
	QString cp1251toUTF8(const QByteArray& byteArray);
	QString html2plain(const QString& string);
	QString getRegexpValue(const QString& pattern, const QString& string);
}

#endif