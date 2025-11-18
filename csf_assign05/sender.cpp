#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // TODO: connect to server
  Connection conn;
 
  try { // try chat server connect
    conn.connect(server_hostname, server_port);
  } catch (const std::exception &e) { // exception bc connection fails
    std::cerr << "There is unfortunately an error when we try connecting to server which is " << e.what() << std::endl;
    return 1;
  }

  // TODO: send slogin message
  Message msg(TAG_SLOGIN, username); // supposed ot identify us as sender client to server

  // send the slogin username and then wait for an ok or err
  if (!conn.send(msg) || !conn.receive(msg) || msg.tag == TAG_ERR) {
    std::cerr << msg.data << std::endl;
    return 1;
  }

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate
  std::string line;
    while (std::getline(std::cin, line)) {
      line = trim(line);
      if (line.empty()) { // ignore any empty lines
        continue;
      }

      Message out;
      if (line.rfind("/join ", 0) == 0) {
        out = Message(TAG_JOIN, line.substr(6)); // to join new room, extract the room name
      } else if (line == "/leave") {
        out = Message(TAG_LEAVE, ""); // to leave the current room you're in
      } else if (line == "/quit") {
        out = Message(TAG_QUIT, ""); // will quit the sesh and notify server
      } else if (line[0] == '/') {
        std::cerr << "The command isn't known to us\n"; // happens when command not known
        continue;
      } else { // normal chat msg
        out = Message(TAG_SENDALL, line); // anything not starting w / is broadcast to room
      }

      // these are for sending message and wait for response
      // client to wait for ok or err before input accepting
      if (!conn.send(out)) {
        break;
      }
      if (!conn.receive(msg)) {
        break;
      }

      if (msg.tag == TAG_ERR) { // given server rej request print an error
        std::cerr << msg.data << std::endl;
      } else if (out.tag == TAG_QUIT) { // quit tag here for break
        break;
      }
    }

  return 0;
}
