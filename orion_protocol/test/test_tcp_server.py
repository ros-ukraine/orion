#!/usr/bin/python

import socket
import sys
import rospy

def main():
  rospy.init_node('test_tcp_server')

  sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  port = rospy.get_param('~port', 8989)
  server_address = ('0.0.0.0', port)

  rospy.loginfo('Starting up on {}'.format(server_address));

  sock.bind(server_address)
  sock.listen(1)

  rospy.loginfo('Waiting for a connection')
  connection, client_address = sock.accept()
  rospy.loginfo('connection from {}'.format(client_address))

  try:
    while not rospy.is_shutdown():
      data = connection.recv(16)
      if data:
        rospy.loginfo('sending data back to the client')
        connection.sendall(data)
  finally:
    connection.close()

if '__main__' == __name__:
  main()
