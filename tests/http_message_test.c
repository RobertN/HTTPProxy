#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../http_message.h"

int test_parse_valid_method()
{
  printf("test_parse_valid_method - running\n");
  http_request* req;
  http_request_init(&req);
  char* data = "GET http://www.redd.it/hej/ HTTP/1.1\r\n";
  http_parse_method(req, data);
  assert(strcmp(http_methods[req->method], "GET") == 0);
  assert(req->version == HTTP_VERSION_1_1);
  assert(strcmp(req->search_path, "http://www.redd.it/hej/") == 0);
  http_request_destroy(req);
  printf("test_parse_valid_method - ok\n");
  return 0;
}

int test_parse_invalid_method()
{
  printf("test_parse_invalid_method - running\n");
  http_request* req;
  http_request_init(&req);
  char* data = "FAKE http://www.redd.it/hej/ HTTP/1.1\r\n";
  http_parse_method(req, data);
  assert(strcmp(http_methods[req->method], "INVALID") == 0);
  printf("test_parse_invalid_method - ok\n");
  return 0;
}

int test_parse_invalid_http_version()
{
  printf("test_parse_invalid_http_version - running\n");
  http_request* req;
  http_request_init(&req);
  char* data = "GET http://www.redd.it/hej/ HTTP/2.0\r\n";
  http_parse_method(req, data);
  assert(req->version == HTTP_VERSION_INVALID);
  printf("test_parse_invalid_http_version - ok\n");
  return 0;
}

int main()
{
  test_parse_valid_method();
  test_parse_invalid_method();
  test_parse_invalid_http_version();
}
