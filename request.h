#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <Arduino.h>
#include "config.h"

/*
 * Class for parsing request strings
 */
class Request {
	private:
		String request;
		
		String protocol;
		String host;
		String port;
		String path;
		String query;
		String file_path;
		String file_ext;

	public:
		Request(String req);

		String getRequest();
		String getProtocol();
		String getHost();
		String getPort();
		String getPath();
		String getQuery();
		String getFile_path();
		String getFile_ext();
};

#endif // _REQUEST_H_
