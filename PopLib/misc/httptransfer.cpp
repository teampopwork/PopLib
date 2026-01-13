#include "httptransfer.hpp"
#include "debug/log.hpp"
#include "appbase.hpp"

using namespace PopLib;

static int gCurTransferId = 1;

// curl needs this
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t totalSize = size * nmemb;
	std::string *output = static_cast<std::string *>(userp);
	output->append(static_cast<char *>(contents), totalSize);
	return totalSize;
}

HTTPTransfer::HTTPTransfer()
{
	mTransferId = gCurTransferId++;
	mResult = RESULT_NOT_STARTED;
	mContentLength = 0;
	mThreadRunning = false;
	mAborted = false;
	mExiting = false;
	mPort = 0;
	mSocket = 0;
	mTransferPending = false;
	
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

HTTPTransfer::~HTTPTransfer()
{
	if (mThread.joinable())
		mThread.join();

	curl_global_cleanup();
}

std::string HTTPTransfer::GetAbsURL(const std::string &theBaseURL, const std::string &theRelURL)
{
	if (theRelURL.find("://") != std::string::npos)
		return theRelURL; // already absolute

	if (theBaseURL.empty())
		return theRelURL;

	std::string base = theBaseURL;
	if (base.back() != '/' && theRelURL.front() != '/')
		base += "/";

	return base + theRelURL;
}

void HTTPTransfer::Fail(int theResult)
{
	mResult = theResult;
	mContent.clear();
	mContentLength = 0;
}

bool HTTPTransfer::SocketWait(bool checkRead, bool checkWrite)
{
	// curl already handles this
	return false;
}

void HTTPTransfer::GetThreadProc()
{
	if (mSendStr.empty())
		GetHelper(mURL);
	else
		PostHelper(mURL, mSendStr);

	mThreadRunning = false;
}

void HTTPTransfer::GetThreadProcStub(void *theArg)
{
	static_cast<HTTPTransfer *>(theArg)->GetThreadProc();
}

void HTTPTransfer::PrepareTransfer(const std::string &theURL)
{
	mURL = theURL;
	mTransferPending = true;
}

void HTTPTransfer::StartTransfer()
{
	if (mThreadRunning)
		return;
	mThreadRunning = true;
	mThread = std::thread(&HTTPTransfer::GetThreadProc, this);
}

void HTTPTransfer::GetHelper(const std::string &theURL)
{
	mContent.clear();
	mResult = RESULT_NOT_STARTED;

	CURL *curl = curl_easy_init();
	if (!curl)
	{
		Fail(RESULT_HTTP_ERROR);
		return;
	}

	curl_easy_setopt(curl, CURLOPT_URL, theURL.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mContent);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		LOG_ERROR("CURL error: %s (%d)", curl_easy_strerror(res), (int)res);
		Fail(RESULT_HTTP_ERROR);
	}
	else
	{
		long response_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

		mResult = (response_code == 200) ? RESULT_DONE : RESULT_HTTP_ERROR;
		mContentLength = static_cast<int>(mContent.size());
	}

	curl_easy_cleanup(curl);
}

void HTTPTransfer::PostHelper(const std::string &theURL, const std::string &theParams)
{
	mContent.clear();
	mResult = RESULT_NOT_STARTED;

	CURL *curl = curl_easy_init();
	if (!curl)
	{
		Fail(RESULT_HTTP_ERROR);
		return;
	}

	curl_easy_setopt(curl, CURLOPT_URL, theURL.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, theParams.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mContent);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		Fail(RESULT_HTTP_ERROR);
	}
	else
	{
		long response_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		mResult = (response_code == 200) ? RESULT_DONE : RESULT_HTTP_ERROR;
		mContentLength = static_cast<int>(mContent.size());
	}

	curl_easy_cleanup(curl);
}

void HTTPTransfer::Get(const std::string &theURL)
{
	Reset();
	PrepareTransfer(theURL);
	GetHelper(theURL);
}

void HTTPTransfer::Post(const std::string &theURL, const std::string &theParams)
{
	Reset();
	PrepareTransfer(theURL);
	PostHelper(theURL, theParams);
}

void HTTPTransfer::Get(const std::string &theBaseURL, const std::string &theRelURL)
{
	Get(GetAbsURL(theBaseURL, theRelURL));
}

void HTTPTransfer::Post(const std::string &theBaseURL, const std::string &theRelURL, const std::string &theParams)
{
	Post(GetAbsURL(theBaseURL, theRelURL), theParams);
}

void HTTPTransfer::SendRequestString(const std::string &theHost, const std::string &theSendString)
{
	// none
}

void HTTPTransfer::Abort()
{
	mAborted = true;
	mTransferPending = false;
	mResult = RESULT_ABORTED;
}

void HTTPTransfer::Reset()
{
	mResult = RESULT_NOT_STARTED;
	mContent.clear();
	mContentLength = 0;
	mTransferPending = false;
	mAborted = false;
}

void HTTPTransfer::UpdateStatus()
{
	// none
}

void HTTPTransfer::WaitFor()
{
	if (mThread.joinable())
		mThread.join();
}

int HTTPTransfer::GetResultCode()
{
	return mResult;
}

std::string HTTPTransfer::GetContent()
{
	return mContent;
}
