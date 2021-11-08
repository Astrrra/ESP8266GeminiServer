#include "request.h"
#include "splitstr.h"

Request::Request(String req) {

  // full request
  req.trim();
  String request = req;

  // protocol
  String protocol = splitStrStart(req, "//");
  if (protocol.equals(req))
  {
    protocol = "gemini:";
  }

  // split protocol form host/path part
  String s = splitStrEnd(req, "//");
  if (s.equals("")) s = req;

  // host and port
  String host = splitStrStart(s, "/");
  String port = "";
  if (!strHasDelimiter(host, ":"))
  {
    port = String(PORT);
  }
  else
  {
    port = splitStrEnd(host, ":");
    if (port = "") port = String(PORT);
    host = splitStrStart(host, ":");
  }

  // path and query
  s = splitStrEnd(s, "/");
  String path = splitStrStart(s, "?");
  String query = splitStrEnd(s, "?");

  // get file path and extension
  String file_path = String("/gemini/" + path);
  if (file_path.charAt(file_path.length() - 1) == '/')
  {
    file_path = String(file_path + "index.gmi");
  }
  String file_ext = splitStrEnd(file_path, ".");

  this->request = request;
  this->protocol = protocol;
  this->host = host;
  this->port = port;
  this->path = path;
  this->query = query;
  this->file_path = file_path;
  this->file_ext = file_ext;
}

String Request::getRequest() {return this->request;}

String Request::getProtocol() {return this->protocol;}

String Request::getHost() {return this->host;}

String Request::getPort() {return this->port;}

String Request::getPath() {return this->path;}

String Request::getQuery() {return this->query;}

String Request::getFile_path() {return this->file_path;}

String Request::getFile_ext() {return this->file_ext;}
