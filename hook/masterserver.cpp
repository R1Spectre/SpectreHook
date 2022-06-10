#include "pch.h"
#include "convar.h"
#include "masterserver.h"
#include "gameutils.h"
#include <thread>

ConVar* CVar_motd;
ConVar* CVar_r1s_ms_host;

void SetCommonHttpClientOptions(CURL* curl)
{
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, Cvar_ns_curl_log_enable->GetBool());
	curl_easy_setopt(curl, CURLOPT_VERBOSE, true);
	//curl_easy_setopt(curl, CURLOPT_USERAGENT, &NSUserAgent);
	// curl_easy_setopt(curl, CURLOPT_STDERR, stdout);
	if (CommandLine()->FindParm("-msinsecure")) // TODO: this check doesn't seem to work
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	}
}

size_t CurlWriteToStringBufferCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

bool isRequestingMOTD = false;
static char motd[2048] = "";

void GetMasterServerMOTD() {
	std::thread requestThread(
		[]()
		{
			if (isRequestingMOTD) {
				Log::Info("Already trying to get motd");
				return;
			}

			isRequestingMOTD = true;
			Log::Info("Trying to get motd");

			std::string url = CVar_r1s_ms_host->GetString() + std::string("/client/motd");

			CURL* curl = curl_easy_init();
			SetCommonHttpClientOptions(curl);
			std::string readBuffer;
			curl_easy_setopt(
				curl,
				CURLOPT_URL,
				url.c_str());
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteToStringBufferCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

			CURLcode result = curl_easy_perform(curl);

			if (result == CURLcode::CURLE_OK)
			{
				Log::Info("Got motd: \"%s\"", readBuffer.c_str());
				strncpy(motd, readBuffer.c_str(), 2048);
				motd[2047] = '\0';
				if(CVar_motd != nullptr)
					CVar_motd->SetValue(motd);
			}
			else {
				Log::Info("Failed to get motd");
			}
			isRequestingMOTD = false;

			curl_easy_cleanup(curl);
		});

	requestThread.detach();
}

typedef char (__fastcall *GetMOTDType)(__int64 a1);
GetMOTDType GetMOTDOriginal;

char __fastcall GetMOTDHook(__int64 a1) {
	GetMasterServerMOTD();
	return 0;
}

void InitialiseEngineMasterServer(HMODULE baseAddress) {
	CVar_r1s_ms_host = new ConVar("r1s_ms_host", "http://127.0.0.1", FCVAR_NONE, "R1Spectre master server hostname");

	// stop motd fetch
	HookEnabler hook;
	ENABLER_CREATEHOOK(hook, (char*)baseAddress + 0x1B2C0, &GetMOTDHook, reinterpret_cast<LPVOID*>(&GetMOTDOriginal));
}

void InitialiseClientMasterServer(HMODULE baseAddress) {
	CVar_motd = g_pCVar->FindVar("motd");
}