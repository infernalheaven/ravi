#include "protocol.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_initreq() {
  char testdata[200] =
    "{\"type\":\"request\",\"seq\":1,\"command\":\"initialize\","
    "\"arguments\":{\"adapterID\":\"lua\",\"pathFormat\":\"path\","
    "\"linesStartAt1\":true,\"columnsStartAt1\":true}}";
  ProtocolMessage msg;
  int msgtype = vscode_parse_message(testdata, strlen(testdata), &msg, stderr);
  if (msgtype != VSCODE_INITIALIZE_REQUEST || msg.seq != 1 ||
    msg.type != VSCODE_REQUEST ||
    strcmp(msg.u.Request.command, "initialize") != 0 ||
    msg.u.Request.request_type != VSCODE_INITIALIZE_REQUEST ||
    strcmp(msg.u.Request.u.InitializeRequest.adapterID, "lua") != 0 ||
    msg.u.Request.u.InitializeRequest.columnsStartAt1 != 1 ||
    msg.u.Request.u.InitializeRequest.linesStartAt1 != 1 ||
    strcmp(msg.u.Request.u.InitializeRequest.pathFormat, "path") != 0) {
    return 1;
  }
  ProtocolMessage res;
  vscode_make_error_response(&msg, &res, VSCODE_INITIALIZE_RESPONSE,
                             "unable to initialize");
  //char buf[1024];
  membuff_t mb;
  membuff_init(&mb, 0);
  vscode_serialize_response_new(&mb, &res);
  //printf(buf);
  const char *expected =
      "{\"type\":\"response\",\"seq\":1,\"command\":\"initialize\",\"request_seq\":1,"
      "\"success\":false,\"message\":\"unable to initialize\"}";
  if (strcmp(expected, mb.buf) != 0) {
	  membuff_free(&mb);  
	  return 1;
  }
  vscode_make_success_response(&msg, &res, VSCODE_INITIALIZE_RESPONSE);
  membuff_rewindpos(&mb);
  vscode_serialize_response_new(&mb, &res);
  const char *expected2 =
      "{\"type\":\"response\",\"seq\":2,\"command\":\"initialize\",\"request_seq\":1,"
      "\"success\":true,\"body\":{\"supportsConfigurationDoneRequest\":false,"
      "\"supportsFunctionBreakpoints\":false,"
      "\"supportsConditionalBreakpoints\":false,\"supportsEvaluateForHovers\":"
      "false}}";
  if (strcmp(expected2, mb.buf) != 0) {
	  membuff_free(&mb);  
	  return 1;
  }
  char testdata2[] = "{\"type\":\"request\",\"seq\":2,\"command\":\"setBreakpoints\",\"arguments\":{\"source\":{\"path\":\"c:\\\\github\\\\ravi\\\\ravi-tests\\\\simple.lua\"},\"lines\" : [6],\"breakpoints\" : [{\"line\":6}]} }";
  msgtype = vscode_parse_message(testdata2, strlen(testdata2), &msg, stderr);
  if (msgtype != VSCODE_SET_BREAKPOINTS_REQUEST || msg.seq != 2 ||
    msg.type != VSCODE_REQUEST ||
    strcmp(msg.u.Request.command, "setBreakpoints") != 0 ||
    msg.u.Request.request_type != VSCODE_SET_BREAKPOINTS_REQUEST ||
    strcmp(msg.u.Request.u.SetBreakpointsRequest.source.path, "c:/github/ravi/ravi-tests/simple.lua") != 0 ||
    msg.u.Request.u.SetBreakpointsRequest.breakpoints[0].line != 6) {
	  membuff_free(&mb);  
	  return 1;
  }
  membuff_free(&mb);
  return 0;
}

int test_json_stringify() {
  char src[] = "string\n";
  char small_buf[8] = { 0 };
  char bigger_buf[9] = { 0 };
  vscode_json_stringify(src, small_buf, sizeof small_buf);
  vscode_json_stringify(src, bigger_buf, sizeof bigger_buf);
  if (strcmp(small_buf, "string") != 0) return 1;
  if (strcmp(bigger_buf, "string\\n") != 0) return 1;
  return 0;
}

int test_membuff() {
	membuff_t mb;
	membuff_init(&mb, 0);
	if (mb.allocated_size != 0 || mb.pos != 0 || mb.buf != NULL)
		return 1;
	membuff_resize(&mb, 5);
	if (mb.allocated_size != 5 || mb.pos != 0 || mb.buf == NULL)
		return 1;
	const char *string1 = "hello world!";
	size_t len = strlen(string1);
	membuff_add_string(&mb, string1);
	if (mb.allocated_size != len + 1 || mb.pos != len || strcmp(mb.buf, string1) != 0)
		return 1;
	const char *string2 = "hello world!true42";
	len = strlen(string2);
	membuff_add_bool(&mb, true);
	membuff_add_int(&mb, 42);
	if (mb.allocated_size != len + 1 || mb.pos != len || strcmp(mb.buf, string2) != 0)
		return 1;
	membuff_rewindpos(&mb);
	if (mb.allocated_size != len + 1 || mb.pos != 0 || strcmp(mb.buf, string2) != 0)
		return 1;
	membuff_free(&mb);
	return 0;
}

int main(void) {
  setbuf(stdout, NULL);
  int rc = 0;
  rc += test_initreq();
  rc += test_json_stringify();
  rc += test_membuff();
  if (rc == 0) printf("OK\n");
  return rc == 0 ? 0 : 1;
}
