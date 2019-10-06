// curl_test.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <stdio.h>
#include <sys/eventfd.h>

#include <curl/curl.h>
#include "engine.h"

#include "ServerAcceptHandler.h"
#include "ResponseRuleGeneral.h"



#ifdef _WIN32
#define strcat(str1,str2)		\
	do {						\
		strcat_s(str1, sizeof(str1), str2);	\
	} while (0);
#endif

static
void dump(const char *text,
	FILE *stream, unsigned char *ptr, size_t size,
	char nohex)
{
	size_t i;
	size_t c;

	unsigned int width = 0x10;


	if (nohex)
		/* without the hex output, we can fit more on screen */
		width = 0x40;

	fprintf(stream, "%s, %10.10lu bytes (0x%8.8lx)\n",
		text, (unsigned long)size, (unsigned long)size);

	for (i = 0; i < size; i += width) {

		fprintf(stream, "%4.4lx: ", (unsigned long)i);

		if (!nohex) {
			/* hex not disabled, show it */
			for (c = 0; c < width; c++)
				if (i + c < size)
					fprintf(stream, "%02x ", ptr[i + c]);
				else
					fputs("   ", stream);
		}

		for (c = 0; (c < width) && (i + c < size); c++) {
			/* check for 0D0A; if found, skip past and start a new line of output */
			if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
				ptr[i + c + 1] == 0x0A) {
				i += (c + 2 - width);
				break;
			}
			fprintf(stream, "%c",
				(ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
			/* check again for 0D0A, to avoid an extra \n if it's at width */
			if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
				ptr[i + c + 2] == 0x0A) {
				i += (c + 3 - width);
				break;
			}
		}
		fputc('\n', stream); /* newline */
	}
	fflush(stream);
}

static
int my_trace(CURL *handle, curl_infotype type,
	char *data, size_t size,
	void *userp)
{
	const char *text;
	(void)handle; /* prevent compiler warning */

	switch (type) {
	case CURLINFO_TEXT:
		fprintf(stderr, "== Info: %s", data);
		/* FALLTHROUGH */
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	dump(text, stderr, (unsigned char *)data, size, 1);
	return 0;
}


static const char *urls[] = {
  "http://localhost",
  "http://www.yahoo.com",
  "http://www.wikipedia.org",
  "http://slashdot.org"
};
#define CNT 1

static size_t cb(char *d, size_t n, size_t l, void *p)
{
	/* take care of the data here, ignored in this example */
	(void)d;
	(void)p;
	return n * l;
}

struct curl_slist * connect_to = NULL;

static void init(CURLM *cm, int i)
{
	CURL *eh = curl_easy_init();
	curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
	curl_easy_setopt(eh, CURLOPT_URL, urls[i]);
	curl_easy_setopt(eh, CURLOPT_PRIVATE, urls[i]);
	curl_easy_setopt(eh, CURLOPT_DEBUGFUNCTION, my_trace);
	curl_easy_setopt(eh, CURLOPT_VERBOSE, 1L);
	
	// test TLS
//	curl_easy_setopt(eh, CURLOPT_CONNECT_TO, connect_to);
	//curl_easy_setopt(eh, CURLOPT_SSLENGINE, "pkcs11");
	//curl_easy_setopt(eh, CURLOPT_SSLCERT, "pkcs11://test obj;type=cert");
	//curl_easy_setopt(eh, CURLOPT_SSLKEY, "pkcs11://test obj;type=private");
	//curl_easy_setopt(eh, CURLOPT_SSLCERTTYPE, "ENG");
	//curl_easy_setopt(eh, CURLOPT_SSLKEYTYPE, "ENG");

	char capath[1024 + 1] = { 0 };
	getcwd(capath, sizeof(capath) - 1);
	strcat(capath, "\\localhost_crt.pem");

	curl_easy_setopt(eh, CURLOPT_CAINFO, capath);

//curl_easy_setopt(curl, CURLOPT_URL, "https://localhost:23456/test.xml");
//	curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:23456/test.xml");

	curl_multi_add_handle(cm, eh);
}


static void* StopServer(void *p)
{
	int efd = *(int*)p;
	printf("thread efd =%d\n", efd);

	WAITMS(10 * 1000);

	eventfd_write(efd, 5);
	eventfd_write(efd, 9);

	WAITMS(3 * 1000);

	eventfd_write(efd, 10);

	WAITMS(3 * 1000);

	pthread_exit(NULL);
	return NULL;
}

/*
// own certificate create
openssl genrsa 2048 > server.key
openssl req -new -key server.key > server.csr
  // attention FDQN is set access hostname
openssl x509 -days 3650 -req -signkey server.key < server.csr > server.crt
//openssl rsa -in server.key -pubout -out server-public.key

*/

static int checktest(
	HttpRequest&	req,
	const uint8_t*	unzip_data,
	size_t			unzip_size)
{
	printf("chekctest2\n");

	return 0;
}


int main(void)
{
//	set_pkcs11_path("aaaaa", "bbbbbb");
	_putenv("PKCS11_PRIVATEKEY=C:\\opt\\SSL\\privatekey.pem");
	_putenv("PKCS11_CLIENTCRT=C:\\opt\\SSL\\cert.pem");
	
	//ENGINE_load_openssl();
	//OPENSSL_init_ssl(OPENSSL_INIT_ENGINE_ALL_BUILTIN
	//			| OPENSSL_INIT_ENGINE_OPENSSL
	//			| OPENSSL_INIT_LOAD_CONFIG, NULL);


	SettingConnection setting;
	//setting.exit_time.tv_sec = 10;
//	setting.enable_tls = true;
//	setting.certificate_chain = getcwd(NULL, 1024);
//	setting.certificate_chain += "\\ms_server_crt.pem";
//	setting.private_key = getcwd(NULL, 1024);
//	setting.private_key += "\\ms_server_privatekey.pem";

	ServerAcceptHandler acceptHandler(setting);

	// set enviroment
//	HTTP_PROXY = http://localhost:23456
//	HTTPS_PROXY = http ://localhost:23456

	char TESTBODY[] = "hello test";
	ResponseRuleGeneral* resp = new ResponseRuleGeneral(200, TESTBODY, strlen(TESTBODY));
//	resp->setPath("/test.html");
	resp->setCheckCallback(checktest);
	acceptHandler.addResponse(ResponseRulePtr(resp));

	resp = new ResponseRuleGeneral(404);
	auto checkfnc = [](HttpRequest& req, const uint8_t* unzip_data, size_t unzip_size)
	{ 
		printf("host:%s\n", req.host.c_str());
		if (unzip_size == NULL)
		{
			printf("unzip null\n");
		}

		return 0;
	};


	resp->setCheckCallback(checkfnc);
	acceptHandler.addResponse(ResponseRulePtr(resp));

	acceptHandler.start();

	int efd = 0;
	efd = eventfd(0, 0);


	connect_to = curl_slist_append(NULL, "::localhost:442");


	pthread_t stop_thread;
	int ret = pthread_create(&stop_thread, NULL, StopServer, &efd);
	if (ret != 0)
	{
		printf("can not create thread : %d", ret);
	}

	CURLM *cm = NULL;
	CURL *eh = NULL;
	CURLMsg *msg = NULL;
	CURLcode return_code = CURLE_OK;
	int still_running = 0, i = 0, msgs_left = 0;
	int http_status_code;
	const char *szUrl;

	curl_global_init(CURL_GLOBAL_ALL);


	cm = curl_multi_init();

	for (i = 0; i < CNT; ++i) {
		init(cm, i);
	}

	curl_multi_perform(cm, &still_running);

	struct curl_waitfd wfd;
	wfd.events = CURL_WAIT_POLLIN;
	wfd.fd = efd;
	wfd.revents = 0;

	do {
		int numfds = 0;
		int res = curl_multi_wait(cm, &wfd, 1, 0, &numfds);
		if (res != CURLM_OK) 
		{
			fprintf(stderr, "error: curl_multi_wait() returned %d\n", res);
			return EXIT_FAILURE;
		}
		/*
		 if(!numfds) {
			fprintf(stderr, "error: curl_multi_wait() numfds=%d\n", numfds);
			return EXIT_FAILURE;
		 }
		*/
		if (wfd.revents & CURL_WAIT_POLLIN)
		{
			printf("CURL_WAIT_POLLIN event recv.\n");
			eventfd_t count = 0;
			eventfd_read(efd, &count);

			printf("eventfd count=%ld\n", count);

			close(efd);

			break;
		}
		curl_multi_perform(cm, &still_running);

	} while (1);

	while ((msg = curl_multi_info_read(cm, &msgs_left))) {
		if (msg->msg == CURLMSG_DONE) {
			eh = msg->easy_handle;

			return_code = msg->data.result;
			if (return_code != CURLE_OK) {
				fprintf(stderr, "CURL error code: %d\n", msg->data.result);
				continue;
			}

			// Get HTTP status code
			http_status_code = 0;
			szUrl = NULL;

			curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_status_code);
			curl_easy_getinfo(eh, CURLINFO_PRIVATE, &szUrl);

			if (http_status_code == 200) {
				printf("200 OK for %s\n", szUrl);
			}
			else {
				fprintf(stderr, "GET of %s returned http status code %d\n", szUrl, http_status_code);
			}

			curl_multi_remove_handle(cm, eh);
			curl_easy_cleanup(eh);
		}
		else {
			fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
		}
	}

	curl_multi_cleanup(cm);

	return EXIT_SUCCESS;
}

