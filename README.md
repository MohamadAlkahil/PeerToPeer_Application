# PeerToPeer_Application
A simple multi client peer to peer application where from the Linux command line clients can connect to the index server to register their content through a UDP connection. Clients can check what content is registered with the index server. If a client is interested in any of the registered content it can send an address request to the index server through another UDP connection. Once the client has the address information of the other client it forms a TCP connection inorder to download the fle of interest. Once the download is complete the content is registred with the index server.

For further explanation and details please read the [Project Report](Report.pdf) 
