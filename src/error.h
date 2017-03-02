#ifndef ERROR_H
#define ERROR_H

#include <QString>
#include <QDebug>
#include <exception>

namespace tabor
{
	class Error : public std::exception
	{
	public:
		Error(const QString& _reason) throw();
		~Error() throw();
		
		virtual const char* what() const throw();
		
		inline const QString& getReason() const throw() { return reason; }
		
	private:
		QString reason;
	};

	QDebug operator<<(QDebug dbg, const Error &error);
}

#endif