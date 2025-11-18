#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // TODO: connect to server

  try {  // try chat server connect
    conn.connect(server_hostname, server_port);
  } catch (const std::exception &e) { // exception bc connection fails
    std::cerr << "There is unfortunately an error when we try connecting to server which is " << e.what() << std::endl;
    return 1;
  }



  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  Message msg(TAG_RLOGIN, username); // help for sending rlogin username to server

  // send it and wait for server to respond
  if (!conn.send(msg) || !conn.receive(msg) || msg.tag == TAG_ERR) { // if server gives error msg print stderr and leave
    std::cerr << msg.data << std::endl;
    return 1;
  }

  msg = Message(TAG_JOIN, room_name); // message for join room name to server

  // again wait for server to confirm this
  if (!conn.send(msg) || !conn.receive(msg) || msg.tag == TAG_ERR) {
    std::cerr << msg.data << std::endl;
    return 1;
  }


  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  // receiver shouldnt send again but only listen for messages now
  while (true) { // look to receive next server message
    if (!conn.receive(msg)) { // if connection clsosed or error break
      break;
    }

    // if our message is broadcast delivery you should print
    if (msg.tag == TAG_DELIVERY) {
      // specific format for server send defined
      size_t first = msg.data.find(':');
      size_t second = msg.data.find(':', first + 1);
      
      if (first != std::string::npos && second != std::string::npos) {
        // extract sender's name and message text
        std::string sender = msg.data.substr(first + 1, second - first - 1);
        std::string text = msg.data.substr(second + 1);

        // you need to display ts in form of sender: message
        std::cout << sender << ": " << text << std::endl;
      }
    // if server send error msg then print to stderr
    } else if (msg.tag == TAG_ERR) {
      std::cerr << msg.data << std::endl;
    }
  }


  return 0;
}
