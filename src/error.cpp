#include "error.h"

namespace tabor
{
	Error::Error(const QString& _reason) throw() : reason(_reason)
	{
	}

	Error::~Error() throw()
	{
	}
	
	const char* Error::what() const throw()
	{
		return reason.toStdString().c_str();
	}

	QDebug operator<<(QDebug dbg, const Error &error)
	{
		dbg << "Exception: " << error.getReason();
		return dbg;
	}
}