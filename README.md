User Manual - NetNinny
======================

### Building

To build the software, simply unpack the archive and issue “make” in the HTTPProxy directory. This will produce a NetNinny binary in the same catalogue. 

### Configuration

It is possible to configure yourself what port the proxy server should use. This can be done by giving the port number as a argument to the executable. For example:

	./NetNinny 8081

### Features

NetNinny has the following features: 

* Support for both HTTP version 1.0 and 1.1
* Can handle any size of data received from the web server
* Blocks URLs containing forbidden words
* Blocks content containing forbidden words
* Compatible with all major browsers, i.e follows the specification

### Lacks

We have not implemented any other request than GET currently. It also lacks complete error checking, so currently it is not hard, probably, to get the server to crash with a bad request.

### Proof

We have tested the proxy server with sites like Aftonbladet, Expressen, Wikipedia, Facebook and YouTube and it seems to work fine. 

   

