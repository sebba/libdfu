#include <dfu.h>
#include <dfu-internal.h>
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "tcp-conn-lwip-raw.h"
#include "rx-method-http-lwip.h"

#define POST_FORM_DATA "multipart/form-data"

struct flash_upload_post_session {
	char session_id[40];
	/* Contains data to be appended on next post request */
	char buf[60];
	int len;
};

static int http_flash_upload_post(const struct http_url *u,
				  struct http_connection *c,
				  struct phr_header *headers, int num_headers,
				  const char *data, int data_len)
{
	struct phr_header *h;
	int ret;
	struct tcp_conn_data *cd = c->cd;
	const char *contents;

	h = http_find_header("Content-Type", headers, num_headers);
	if (!h) {
		dfu_err("%s: missing Content-Type header\n", __func__);
		/* FIXME: IS THIS OK ? */
		return http_request_error(c, HTTP_BAD_REQUEST);
	}
	if (memcmp(h->value, POST_FORM_DATA, strlen(POST_FORM_DATA))) {
		dfu_err("%s: unsupported Content-Type\n", __func__);
		/* FIXME: IS THIS OK ? */
		return http_request_error(c, HTTP_NOT_IMPLEMENTED);
	}
	/* Actual contents start */
	contents = http_find_post_contents_start(data);
	if (!contents) {
		dfu_err("%s: missing contents in data\n", __func__);
		/* FIXME: IS THIS OK ? */
		return http_request_error(c, HTTP_BAD_REQUEST);
	}
	if (dfu_binary_file_append_buffer(c->bf,
					  contents,
					  data_len - (contents - data)) < 0) {
		dfu_err("%s: error appending data\n", __func__);
		return http_request_error(c, HTTP_INTERNAL_SERVER_ERROR);
	}
	ret = http_send_status(cd, HTTP_OK);
	c->can_close = 1;
	tcp_server_socket_lwip_raw_close(c->cd);
	return ret;
}

declare_http_url(flash_upload, NULL, http_flash_upload_post);