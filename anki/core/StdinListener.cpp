#include "anki/core/StdinListener.h"


namespace anki {


//==============================================================================
// workingFunc                                                                 =
//==============================================================================
void StdinListener::workingFunc()
{
	char buff[512];

	while(1)
	{
		int m = read(0, buff, sizeof(buff));
		buff[m] = '\0';
		//cout << "read: " << buff << endl;
		{
			boost::mutex::scoped_lock lock(mtx);
			q.push(buff);
			//cout << "size:" << q.size() << endl;
		}
	}
}


//==============================================================================
// getLine                                                                     =
//==============================================================================
std::string StdinListener::getLine()
{
	std::string ret;
	boost::mutex::scoped_lock lock(mtx);
	//cout << "_size:" << q.size() << endl;
	if(!q.empty())
	{
		ret = q.front();
		q.pop();
	}
	return ret;
}


//==============================================================================
// start                                                                       =
//==============================================================================
void StdinListener::start()
{
	thrd = boost::thread(&StdinListener::workingFunc, this);
}


} // end namespace
