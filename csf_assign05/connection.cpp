#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, fd); // initialize buffered io for socket descriptor
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  // TODO: call rio_readinitb to initialize the rio_t object
  // here convert port # to string 
  std::ostringstream oss;
  oss << port; 
  std::string port_str = oss.str(); // helpful for openclientfd


  m_fd = open_clientfd(const_cast<char *>(hostname.c_str()), const_cast<char *>(port_str.c_str())); // try making connection to host:port

  // less than 0 means fail since openclientfd then returned -1
  if (m_fd < 0) {
    m_last_result = EOF_OR_ERROR;
    throw std::runtime_error("Failed to connect to server");
  }

  rio_readinitb(&m_fdbuf, m_fd); // initialize rio buffer for ts connected socket
 
  m_last_result = SUCCESS; // last operation mark good
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  close(); // close safely if still open
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  return m_fd >= 0; // open and valid
}

void Connection::close() {
  // TODO: close the connection if it is open
  if (m_fd >= 0) {
    ::close(m_fd); // use system close()
    m_fd = -1; // mark it as closed too
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  std::string encoded = msg.encode(); // get msg encoded to tag:data format

  // if more than protocol limit then its invalid
  if (encoded.size() > Message::MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }
  
  ssize_t n = rio_writen(m_fd, const_cast<char *>(encoded.c_str()), encoded.size()); // send with a write (handle short writes and interruptions)

  // if write not work or not complete have error
  if (n != (ssize_t)encoded.size()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  m_last_result = SUCCESS; // the result is successful
  return true; // indicate success too
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  char buf[Message::MAX_LEN + 1]; // buffer here to store a single incoming msg

  ssize_t n = rio_readlineb(&m_fdbuf, buf, Message::MAX_LEN); // one line read terminated by \n

  // if file end or error
  if (n <= 0) {
    m_last_result = EOF_OR_ERROR; // mark result and return false
    return false;
  }

  buf[n] = '\0'; // make sure null terminating
  std::string line(buf); // convert this to std::string

  // deocde the line into message of tag:data
  if (!msg.decode(line)) {
    m_last_result = INVALID_MSG;
    return false;
  }

  m_last_result = SUCCESS; // result message successfully read/decoded
  return true; // indicate this success
}
