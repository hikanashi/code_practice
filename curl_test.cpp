// curl_test.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

//#include "pch.h"
#include <stdio.h>
//#include <sys/eventfd.h>
#include <cstring>
#ifdef _WIN32
#include <pcreposix.h>
#else
#include <regex.h>
#endif
#include <curl/curl.h>

#include "ServerAcceptHandler.h"
#include "ResponseRuleGeneral.h"

#include "logout.h"
#include <thread>


#ifdef _WIN32
#define strcat(str1,str2)		\
	do {						\
		strcat_s(str1, sizeof(str1), str2);	\
	} while (0);
#endif


//#define LOGOUT(format, ...)		\
//	do {										\
//		char logbuf[1024] = {0};				\
//		snprintf(logbuf, sizeof(logbuf), format, __VA_ARGS__);	\
//		logout(logbuf);							\
//	} while (0);

#define LOGOUT_S(stream, ...)	LOGOUT(__VA_ARGS__)

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

	LOGOUT_S(stream, "%s, %10.10lu bytes (0x%8.8lx)\n",
		text, (unsigned long)size, (unsigned long)size);

	for (i = 0; i < size; i += width) {

		LOGOUT_S(stream, "%4.4lx: ", (unsigned long)i);

		if (!nohex) {
			/* hex not disabled, show it */
			for (c = 0; c < width; c++)
				if (i + c < size)
				{
					LOGOUT_S(stream, "%02x ", ptr[i + c]);
				}
				else
				{
					LOGOUT_S(stream, "   ");
				}
		}

		for (c = 0; (c < width) && (i + c < size); c++) {
			/* check for 0D0A; if found, skip past and start a new line of output */
			if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
				ptr[i + c + 1] == 0x0A) {
				i += (c + 2 - width);
				break;
			}
			LOGOUT_S(stream, "%c",
				(ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
			/* check again for 0D0A, to avoid an extra \n if it's at width */
			if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
				ptr[i + c + 2] == 0x0A) {
				i += (c + 3 - width);
				break;
			}
		}
		LOGOUT_S(stream,"\n"); /* newline */
	}
	fflush(stream);
}

static
int my_trace(CURL *handle, curl_infotype type,
	char *data, size_t size,
	void *userp)
{
	LOGOUT_APIIN("handle=%p type=%d", handle, type);
	const char *text;
	(void)handle; /* prevent compiler warning */

	switch (type) {
	case CURLINFO_TEXT:
		LOGOUT_S(stderr, "== Info: %s", data);
		/* FALLTHROUGH */
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		dump(text, stderr, (unsigned char *)data, size, 1);
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		dump(text, stderr, (unsigned char *)data, size, 1);
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		dump(text, stderr, (unsigned char *)data, size, 1);
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		dump(text, stderr, (unsigned char *)data, size, 1);
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	LOGOUT_APIOUT("");
	return 0;
}


static const char *urls[] = {
  "https://127.0.0.1",
  "https://httpstat.us/301",
  "http://www.yahoo.com",
  "http://www.wikipedia.org",
  "http://slashdot.org"
};
#define CNT 1

typedef struct _HttpRespHeader
{
	std::string data;
	regex_t pattern_statusline;
	bool receiving;

} HttpRespHeader;

typedef struct _HttpResp
{
	HttpRespHeader header;
	std::string body;
} HttpResp;

static size_t write_header_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	HttpRespHeader* data = (HttpRespHeader*)userdata;
	size_t realsize = size * nmemb;

	std::string recv(ptr , realsize);
	LOGOUT("HEADER:%s", recv.c_str());

	if(data->receiving == false)
	{
		if( regexec(&data->pattern_statusline, recv.c_str() , 0, NULL, 0) == 0)
		{
			data->data.clear();
			data->receiving = true;
			return realsize;
		}
	}
	else
	{
		if(recv == "\r\n")
		{
			data->receiving = false;
		}
	}
	

	data->data.append( recv );

	return realsize;
}

static size_t write_body_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::string* data = (std::string*)userdata;
	size_t realsize = size * nmemb;

	std::string recv(ptr , realsize);
//	LOGOUT("BODY:%s", recv.c_str());
	data->append( recv );

	return realsize;
}

struct curl_slist * connect_to = NULL;

static void init(CURLM *cm, int i)
{
	LOGOUT_APIIN("cm=%p i=%d", cm, i);
	CURL *eh = curl_easy_init();

	HttpResp* res = new HttpResp();
	res->header.receiving = false;
	int cmp = regcomp(&res->header.pattern_statusline, "HTTP/[0-9].[0-9] [0-9]+ .*", REG_EXTENDED | REG_ICASE | REG_NOSUB);
	if( cmp != 0 )
	{
		char    errorbuffuer[1024] = {0};
		regerror(cmp, &res->header.pattern_statusline, errorbuffuer, sizeof(errorbuffuer));
		LOGOUT("### regcmp error(%d) %s\n", cmp, errorbuffuer);
	}

	curl_easy_setopt(eh, CURLOPT_PRIVATE, res);
	curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_body_cb);
	curl_easy_setopt(eh, CURLOPT_WRITEDATA, &res->body);
	curl_easy_setopt(eh, CURLOPT_HEADERFUNCTION, write_header_cb);
	curl_easy_setopt(eh, CURLOPT_HEADERDATA, &res->header);
	curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(eh, CURLOPT_MAXREDIRS, 10L);

	curl_easy_setopt(eh, CURLOPT_URL, urls[i]);
	curl_easy_setopt(eh, CURLOPT_DEBUGFUNCTION, my_trace);
	curl_easy_setopt(eh, CURLOPT_VERBOSE, 1L);
	
	// test TLS
//	curl_easy_setopt(eh, CURLOPT_CONNECT_TO, connect_to);
	curl_easy_setopt(eh, CURLOPT_SSLENGINE, "pkcs11");
	curl_easy_setopt(eh, CURLOPT_SSLCERT, "pkcs11://test obj;type=cert");
	curl_easy_setopt(eh, CURLOPT_SSLCERTTYPE, "ENG");
	curl_easy_setopt(eh, CURLOPT_SSLKEY, "pkcs11://test obj;type=private");
	curl_easy_setopt(eh, CURLOPT_SSLKEYTYPE, "ENG");
//	curl_easy_setopt(eh, CURLOPT_SSL_VERIFYSTATUS, 1L);
	curl_easy_setopt(eh, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);

	char capath[1024 + 1] = { 0 };
	//getcwd(capath, sizeof(capath) - 1);
#ifdef _WIN32
	strcat(capath, "/opt/local/SSL/rootCA.pem");
#else
	strcat(capath, "/home/user/.local/ssl/rootCA.pem");
#endif

	curl_easy_setopt(eh, CURLOPT_CAINFO, capath);
//	curl_easy_setopt(eh, CURLOPT_CAPATH, "/etc/ssl/cert");
	
//curl_easy_setopt(curl, CURLOPT_URL, "https://localhost:23456/test.xml");
//	curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:23456/test.xml");

	curl_multi_add_handle(cm, eh);
	LOGOUT_APIOUT("");
}


static void* StopServer(void *p)
{
	//int efd = *(int*)p;
	//LOGOUT("thread efd =%d\n", efd);

	WAITMS(3 * 1000);

	//eventfd_write(efd, 5);
	//eventfd_write(efd, 9);

	WAITMS(3 * 1000);

	LOGOUT("complete server");

	//eventfd_write(efd, 10);

//	WAITMS(3 * 1000);

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
	LOGOUT_APIIN("");
	LOGOUT("chekctest2 %d %d\n", req.host.size(), unzip_size);

	LOGOUT_APIOUT("");
	return 0;
}


int main(void)
{
	LOGOUT_APIIN("");


	FunctionLogEvalPtr initeval = FunctionLogger::addPattern("init", "\\[API_IN\\].*cm=(.*) i=(.+)");

	FunctionLogEvalPtr stopserverlog = FunctionLogger::addPattern("StopServer", "complete server");

	

//	set_pkcs11_path("aaaaa", "bbbbbb");
#ifdef _WIN32
	putenv("PKCS11_PRIVATEKEY=/opt/local/ssl/pc1key.pem");
	putenv("PKCS11_CLIENTCRT=/opt/local/ssl/pc1CA.pem");
#else
	putenv("PKCS11_PRIVATEKEY=/home/user/.local/ssl/pc1key.pem");
	putenv("PKCS11_CLIENTCRT=/home/user/.local/ssl/pc1CA.pem");
#endif

	char curdir[1024 + 1] = { 0 };
//	getcwd(curdir, sizeof(curdir) - 1);
	strcpy(curdir, "C:\\opt\\local\\ssl");

	char confpath[1024 + 1] = { 0 };
#ifdef _WIN32
	snprintf(confpath, sizeof(confpath) - 1, "OPENSSL_CONF=%s\\openssl.cnf", curdir);
#else
	snprintf(confpath, sizeof(confpath)-1, "OPENSSL_CONF=%s/openssl.cnf", curdir);
#endif
	LOGOUT("### env %s\n", confpath);
	putenv(confpath);

	putenv("OPENSSL_ENGINES=/opt/local/lib/");


	char sslpath[1024 + 1] = { 0 };
	snprintf(sslpath, sizeof(sslpath) - 1, "OPENSSL_DIR=%s", curdir);


	const char* confenv = getenv("OPENSSL_CONF");
	LOGOUT("### get OPENSSL_CONF=%s\n", confenv ? confenv : "null");


	SettingConnection setting;
	setting.exit_time.tv_sec = 10;
	setting.enable_ocsp_stapling = true;
	setting.verify_stapling = true;
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
		LOGOUT("host:%s\n", req.host.c_str());
		if (unzip_size == NULL)
		{
			LOGOUT("unzip null\n");
		}

		return 0;
	};


	resp->setCheckCallback(checkfnc);
	acceptHandler.addResponse(ResponseRulePtr(resp));

	acceptHandler.start();

	//int efd = 0;
	//efd = eventfd(0, 0);

//	WAITMS(5 * 1000);

	curl_global_init(CURL_GLOBAL_ALL);

	//connect_to = curl_slist_append(NULL, "::localhost:442");


	//pthread_t stop_thread;
	//int ret = pthread_create(&stop_thread, NULL, StopServer, &efd);
	//if (ret != 0)
	//{
	//	LOGOUT("can not create thread : %d", ret);
	//}

	CURLM *cm = NULL;
	CURL *eh = NULL;
	CURLMsg *msg = NULL;
	CURLcode return_code = CURLE_OK;
	int still_running = 0, i = 0, msgs_left = 0;
	int http_status_code;


//	std::thread th(StopServer, (void*)NULL);
//	stopserverlog->wait();

	cm = curl_multi_init();

	for (i = 0; i < CNT; ++i) {
		init(cm, i);
	}

	curl_multi_perform(cm, &still_running);

	//struct curl_waitfd wfd;
	//wfd.events = CURL_WAIT_POLLIN;
	//wfd.fd = efd;
	//wfd.revents = 0;

	do {
		int numfds = 0;
		int res = curl_multi_wait(cm, 0, 0, 0, &numfds);
		if (res != CURLM_OK) 
		{
			LOGOUT_S(stderr, "error: curl_multi_wait() returned %d\n", res);
			return EXIT_FAILURE;
		}
		/*
		 if(!numfds) {
			LOGOUT_S(stderr, "error: curl_multi_wait() numfds=%d\n", numfds);
			return EXIT_FAILURE;
		 }
		*/
		//if (wfd.revents & CURL_WAIT_POLLIN)
		//{
		//	LOGOUT("CURL_WAIT_POLLIN event recv.\n");
		//	eventfd_t count = 0;
		//	eventfd_read(efd, &count);

		//	LOGOUT("eventfd count=%ld\n", count);

		//	close(efd);

		//	break;
		//}
		curl_multi_perform(cm, &still_running);

	} while (still_running);

	while ((msg = curl_multi_info_read(cm, &msgs_left))) {
		if (msg->msg == CURLMSG_DONE) {
			eh = msg->easy_handle;

			return_code = msg->data.result;
			if (return_code != CURLE_OK) {
				LOGOUT_S(stderr, "CURL error code: %d\n", msg->data.result);
				continue;
			}

			// Get HTTP status code
			http_status_code = 0;
			HttpResp* res = NULL;
			char *url = NULL;

			curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_status_code);
			curl_easy_getinfo(eh, CURLINFO_EFFECTIVE_URL, &url);
			curl_easy_getinfo(eh, CURLINFO_PRIVATE, &res);
			

			if (http_status_code == 200) {
				LOGOUT("200 OK for %s\n", url);
			}
			else {
				LOGOUT_S(stderr, "GET of %s returned http status code %d\n", url, http_status_code);
			}

			if( res->header.data.size() > 0)
			{
				LOGOUT("=================== HEADER START=====\n");
				LOGOUT("%s", res->header.data.c_str());
				LOGOUT("=================== HEADER END =====\n");
				/*
				const char* endmark = "\r\n\r\n";
				size_t endmarklen = strlen(endmark);
				const char* crlf = "\r\n"; 
				size_t crlflen = strlen(crlf);
				size_t headersize = res->header.data.size();

				auto startpos = res->header.data.find(crlf) + crlflen;
				auto lastline = res->header.data.find(endmark);
				auto endpos = lastline + endmarklen;
				while(endpos < headersize)
				{
					startpos = res->header.data.find(crlf, endpos) + crlflen;
					lastline = res->header.data.find(endmark, endpos);
					endpos = lastline + endmarklen;	
				}

				if(startpos != std::string::npos)
				{
					std::string tmp = res->header.data.substr(startpos);
					LOGOUT("=================== CUTHEADER START=====\n");
					LOGOUT("%s\n", tmp.c_str());
					LOGOUT("=================== CUTHEADER END=====\n");
				}
				*/
			}

			if( res->body.size() > 0)
			{
				LOGOUT("=================== BODY START=====\n");
				LOGOUT("%s", res->body.c_str());
				LOGOUT("=================== BODY END =====\n");
			}

			regfree(&res->header.pattern_statusline);
			delete res;
			curl_multi_remove_handle(cm, eh);
			curl_easy_cleanup(eh);
		}
		else {
			LOGOUT_S(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
		}
	}

	curl_multi_cleanup(cm);

	//pthread_join(stop_thread, NULL);

//	FunctionLogListPtr loglist = FunctionLogger::GetLogger().GetLogList("my_trace", "\\d+");

//	size_t trace = CountFunctionLog("my_trace");

//	FunctionLogPtr log = loglist->getLog(1);

//	std::vector<std::string> args;
//	bool ret = log->parseLog("Trying (\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)", args);

//	FunctionLogListPtr  nodelay = loglist->searchLog("TCP_NODELAY");

	size_t count = initeval->getCount();
	LOGOUT_S(stderr, "count:%d res[0]=%s res[1]=%s res[2]=%s\n", 
		count, 
		initeval->getResult(0).c_str(),
		initeval->getResult(1).c_str(),
		initeval->getResult(2).c_str() );

	curl_global_cleanup();

	LOGOUT_APIOUT("");

	return EXIT_SUCCESS;
}

