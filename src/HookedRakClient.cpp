/*

	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, BlastHack

	mod_sa is available from https://github.com/BlastH1ckNet/mod_s0b1it_sa/

	mod_sa is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	mod_sa is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with mod_sa.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "main.h"
#include "protect.h"


HANDLE file = CreateFile(TEXT("HWID"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
	NULL, CREATE_NEW,
	FILE_FLAG_BACKUP_SEMANTICS, NULL);

void writefilea(const char* buf, ...)
{
	DWORD dwBytesToWrite = (DWORD)strlen(buf);
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = FALSE;
	bErrorFlag = WriteFile(
		file,
		(LPCVOID)buf,
		dwBytesToWrite,
		&dwBytesWritten,
		NULL);
}

static void macaddress(static void*)
{
	IP_ADAPTER_INFO AdapterInfo[32];
	DWORD dwBufLen = sizeof(AdapterInfo);
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) != ERROR_SUCCESS)
	{
		printf("hata");
	}

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	char szBuffer[512];
	while (pAdapterInfo)
	{
		if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
		{
			sprintf_s(szBuffer, sizeof(szBuffer), "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X" //00-00-00-00-00 be like that use uppercase x instead of lowercase
				, pAdapterInfo->Address[0]
				, pAdapterInfo->Address[1]
				, pAdapterInfo->Address[2]
				, pAdapterInfo->Address[3]
				, pAdapterInfo->Address[4]
				, pAdapterInfo->Address[5]
			);
			//printf("Mac Address = %s", szBuffer);
			writefilea(szBuffer);
			writefilea("_");
		}
		pAdapterInfo = pAdapterInfo->Next;
	}
}

static void system_uuid(const BYTE* p, short ver)
{
	int only0xFF = 1, only0x00 = 1;
	int i;
	for (i = 0; i < 16 && (only0x00 || only0xFF); i++)
	{
		if (p[i] != 0x00) only0x00 = 0;
		if (p[i] != 0xFF) only0xFF = 0;
	}
	if (only0xFF)
	{
		addMessageToChatWindow("CANNOT FOUND");
		return;
	}
	if (only0x00)
	{
		addMessageToChatWindow("CANNOT CONVERTABLE");
		return;
	}
	if (ver >= 0x0206)
	{
		//addMessageToChatWindow("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
			//p[3], p[2], p[1], p[0], p[5], p[4], p[7], p[6],
			//p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		std::string uuid_;
		static const int max_uuid_size{ 50 };
		char uuid[max_uuid_size] = {};
		_snprintf_s(uuid, max_uuid_size, max_uuid_size - 1, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			p[3], p[2], p[1], p[0], p[5], p[4], p[7], p[6],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);

		uuid_ = uuid;
		writefilea(uuid_.c_str());
	}
	else
	{
		//addMessageToChatWindow("-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
		//p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
		//p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		std::string uuid_;
		static const int max_uuid_size{ 50 };
		char uuid[max_uuid_size] = {};
		_snprintf_s(uuid, max_uuid_size, max_uuid_size - 1, "-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		uuid_ = uuid;
		writefilea(uuid_.c_str());
	}
}

SYSTEMINFORMATION* find_system_information(SMBIOSData* bios_data)
{
	uint8_t* data = bios_data->SMBIOSTableData;
	while (data < bios_data->SMBIOSTableData + bios_data->Length)
	{
		uint8_t* next;
		SMBIOSHEADER* header = (SMBIOSHEADER*)data;
		if (header->length < 4)
			break;
		if (header->type == 0x01 && header->length >= 0x19)
		{
			return (SYSTEMINFORMATION*)header;
		}

		next = data + header->length;

		while (next < bios_data->SMBIOSTableData + bios_data->Length && (next[0] != 0 || next[1] != 0)) {
			next++;
		}
		next += 2;

		data = next;
	}
	return nullptr;
}

const char* get_string_by_index(const char* str, int index, const char* null_string_text = "")
{
	if (0 == index || 0 == *str) {
		return null_string_text;
	}

	while (--index) {
		str += strlen(str) + 1;
	}
	return str;
}

const char* convert_string_with_header_byte(const SMBIOSHEADER* dm, BYTE s)
{
	char* bp = (char*)dm;
	size_t i, len;
	if (s == 0)
	{
		return "ASYMPTOMATIC";
	}

	bp += dm->length;

	while (s > 1 && *bp)
	{
		bp += strlen(bp);
		bp++;
		s--;
	}
	if (!*bp)
	{
		return "INDEX_ERROR hatasi nullptr yuksek ihtimal";
	}
	len = strlen(bp);
	for (i = 0; i < len; i++)
		if (bp[i] < 32 || bp[i] == 127)
			bp[i] = '.';
	return bp;
}

static void bios(void)
{
	std::string serialnumber_;
	DWORD bufsize = 0;
	BYTE buf[65536] = { 0 };
	int ret = 0;
	SMBIOSData* Smbios;
	SMBIOSHEADER* header = NULL;
	int flag = 1;

	ret = GetSystemFirmwareTable('RSMB', 0, 0, 0);
	if (!ret)
	{
		addMessageToChatWindow("FIRST SYSTEMFRIMWARETABLE FUNCTION FAILED!\n");
	}
	bufsize = ret;
	ret = GetSystemFirmwareTable('RSMB', 0, buf, bufsize);
	if (!ret)
	{
		addMessageToChatWindow("SECOND SYSTEMFRIMWARETABLE FUNCTION FAILED!\n");
	}
	Smbios = (SMBIOSData*)buf;
	BYTE* p = Smbios->SMBIOSTableData;
	if (Smbios->Length != bufsize - 8)
	{
		addMessageToChatWindow("Smbios length error\n");
	}

	for (int s = 0; s < Smbios->Length; s++)
	{
		header = (SMBIOSHEADER*)p;
		if (header->type == 0 && flag)
		{
			//addMessageToChatWindow("BIOS Version = %s\n", convert_string_with_header_byte(header, p[0x5]));
			writefilea(convert_string_with_header_byte(header, p[0x5]));
			writefilea("_");
			//addMessageToChatWindow("BIOS Release Date = %s\n", convert_string_with_header_byte(header, p[0x8]));
			writefilea(convert_string_with_header_byte(header, p[0x8]));
			writefilea("_");
			flag = 0;
		}

		else if (header->type == 1)
		{
			writefilea(convert_string_with_header_byte(header, p[0x6]));
			writefilea("_");
			writefilea(convert_string_with_header_byte(header, p[0x7]));
			writefilea("_");
			system_uuid(p + 0x8, Smbios->SMBIOSMajorVersion * 0x100 + Smbios->SMBIOSMinorVersion);
			//addMessageToChatWindow("Version = %s\n", convert_string_with_header_byte(header, p[0x6]));
			addMessageToChatWindow("Serial Number = %s\n", convert_string_with_header_byte(header, p[0x7]));
			//addMessageToChatWindow("UUID = "); system_uuid(p + 0x8, Smbios->SMBIOSMajorVersion * 0x100 + Smbios->SMBIOSMinorVersion);
			writefilea("_");
			writefilea(convert_string_with_header_byte(header, p[0x19]));
			//addMessageToChatWindow("SKU Number = %s\n", convert_string_with_header_byte(header, p[0x19]));
			writefilea("_");
		}
		p += header->length;
		while ((*(WORD*)p) != 0) p++;
		p += 2;
	}
	SYSTEMINFORMATION* sysinfo = find_system_information(Smbios);
	const char* str = (const char*)sysinfo + sysinfo->Header.length;
	if (sysinfo)
	{
		serialnumber_ = get_string_by_index(str, sysinfo->SerialNumber);
	}
}

static void profileguid(void)
{
	HW_PROFILE_INFO   HwProfInfo;
	if (!GetCurrentHwProfile(&HwProfInfo))
	{
		addMessageToChatWindow("error");
	}
	//_tprintf(TEXT("ProfileGuid = %s\n"), HwProfInfo.szHwProfileGuid);
	//addMessageToChatWindow("Profile Guid = %s\n", HwProfInfo.szHwProfileGuid);
	writefilea(HwProfInfo.szHwProfileGuid);
	writefilea("_");
}

static void diskserialnumber()
{
	HANDLE device = CreateFileW(L"//./PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (device == INVALID_HANDLE_VALUE)
	{
		addMessageToChatWindow("HARDDISK DRIVE CANNOT FOUND");
	}
	STORAGE_PROPERTY_QUERY query{};
	query.PropertyId = StorageDeviceProperty;
	query.QueryType = PropertyStandardQuery;

	STORAGE_DESCRIPTOR_HEADER storgeDescriptorHeader = { 0 };
	DWORD dwbytesreturned;

	if (!DeviceIoControl(device, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), &storgeDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwbytesreturned, NULL))
	{
		addMessageToChatWindow("DEVICEIOCONTROL CANNOT ACCESS");
	}

	const DWORD dwbuffersize = storgeDescriptorHeader.Size;
	std::vector<BYTE> pOutBuffer(dwbuffersize, 0);
	if (!DeviceIoControl(device, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), pOutBuffer.data(), dwbuffersize, &dwbytesreturned, NULL))
	{
		addMessageToChatWindow("DEVICEIOCONTROL CANNOT ACCESS 2");
	}
	STORAGE_DEVICE_DESCRIPTOR* devicedescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer.data();
	DWORD serialnumberoffset = devicedescriptor->SerialNumberOffset;
	if (serialnumberoffset != 0)
	{
		std::string serialnumber;
		serialnumber = (char*)(pOutBuffer.data() + serialnumberoffset);
		std::string serialnumberwithoutblank;
		for (int i = 0; i < serialnumber.length(); ) {
			if (serialnumber[i] == ' ') {
				if (i == 0 || i == serialnumber.length() - 1) {
					i++;
					continue;
				}
				while (serialnumber[i + 1] == ' ')
					i++;
			}
			serialnumberwithoutblank += serialnumber[i++];
		}
		addMessageToChatWindow("Harddisk Serial Number =", serialnumberwithoutblank);
		CloseHandle(device);
	}
}

static void cdiskvolumeserialnumber(void)
{
	WCHAR volumename[MAX_PATH + 1] = { 0 };
	WCHAR filesystemname[MAX_PATH + 1] = { 0 };
	DWORD serialnumber = 0;
	DWORD maxcompenentlenght = 0;
	DWORD filesystemflagsbuffer = 0;
	//HATA NERDE AMK 
	if (GetVolumeInformation(TEXT("C:\\"), (LPSTR)volumename, sizeof(volumename), &serialnumber, &maxcompenentlenght, &filesystemflagsbuffer, (LPSTR)filesystemname, sizeof(filesystemname)) == TRUE)
	{
		//wprintf(L"%lu", serialnumber);
		std::string nondwordserialnumber = std::to_string(serialnumber);
		writefilea(nondwordserialnumber.c_str());
		writefilea("|");
	}
	else
	{
		addMessageToChatWindow("hata");
	}
}

static void computerusername(void)
{
	union pc
	{
		char username[MAX_COMPUTERNAME_LENGTH + 1];
	};
	DWORD usernametickcount = MAX_COMPUTERNAME_LENGTH + 1;
	pc computer = { 0 };

	if (!GetComputerNameA(&computer.username[0], &usernametickcount))
	{
		_exit(1);
	}
	else
	{
		std::string buffer = computer.username;
		writefilea("|");
		writefilea(computer.username);
		writefilea("_");
	}
}

static void cpuserial(void)
{
	std::array<int, 4> cpuInfo;
	__cpuid(cpuInfo.data(), 1);
	std::stringstream buffer;
	buffer << std::hex << cpuInfo.at(3) << cpuInfo.at(0);
	std::string newbuffer;
	newbuffer = buffer.str();
	std::transform(newbuffer.begin(), newbuffer.end(), newbuffer.begin(), ::toupper);
	//addMessageToChatWindow(newbuffer.c_str());
}

void replacetextfile(std::string infile, std::string outfile, std::string whatreplace, std::string whatreplacewith)
{
	std::ifstream in(infile);
	std::ofstream out(outfile);
	std::string replaceword(whatreplace);
	std::string replacewordwith(whatreplacewith);

	if (!in)
	{
		addMessageToChatWindow("dosya edit hatasi");
	}

	if (!out)
	{
		addMessageToChatWindow("dosya edit hatasi");
	}
	std::string line;
	size_t len = replaceword.length();
	while (getline(in, line))
	{
		while (true)
		{
			size_t pos = line.find(replaceword);
			if (pos != std::string::npos)
			{
				line.replace(pos, len, replacewordwith);
			}
			else
			{
				break;
			}
		}
		out << line << '\n';
	}
}

void replace(std::string& input, const std::string& from, const std::string& to)
{
	auto pos = 0;
	while (true)
	{
		size_t startPosition = input.find(from, pos);
		if (startPosition == std::string::npos)
			return;
		input.replace(startPosition, from.length(), to);
		pos += to.length();
	}
}

BYTE GetPacketID(Packet* p)
{
	if (p == 0) return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char)p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
	{
		return (unsigned char)p->data[0];
	}
}

static std::string cpuId(void)
{
	unsigned long s1 = 0;
	unsigned long s2 = 0;
	unsigned long s3 = 0;
	unsigned long s4 = 0;
	__asm
	{
		mov eax, 00h
		xor edx, edx
		cpuid
		mov s1, edx
		mov s2, eax
	}
	__asm
	{
		mov eax, 01h
		xor ecx, ecx
		xor edx, edx
		cpuid
		mov s3, edx
		mov s4, ecx
	}

	static char buf[100];
	sprintf_s(buf, "%08X%08X%08X%08X", s1, s2, s3, s4);
	return buf;
}
bool tespit = false;
void BIG_NUM_MUL(unsigned long in[5], unsigned long out[6], unsigned long factor)
{
	unsigned long src[5] = { 0 };
	for (int i = 0; i < 5; i++)
		src[i] = ((in[4 - i] >> 24) | ((in[4 - i] << 8) & 0x00FF0000) | ((in[4 - i] >> 8) & 0x0000FF00) | (in[4 - i] << 24));

	unsigned long long tmp = 0;

	tmp = unsigned long long(src[0]) * unsigned long long(factor);
	out[0] = tmp & 0xFFFFFFFF;
	out[1] = tmp >> 32;
	tmp = unsigned long long(src[1]) * unsigned long long(factor) + unsigned long long(out[1]);
	out[1] = tmp & 0xFFFFFFFF;
	out[2] = tmp >> 32;
	tmp = unsigned long long(src[2]) * unsigned long long(factor) + unsigned long long(out[2]);
	out[2] = tmp & 0xFFFFFFFF;
	out[3] = tmp >> 32;
	tmp = unsigned long long(src[3]) * unsigned long long(factor) + unsigned long long(out[3]);
	out[3] = tmp & 0xFFFFFFFF;
	out[4] = tmp >> 32;
	tmp = unsigned long long(src[4]) * unsigned long long(factor) + unsigned long long(out[4]);
	out[4] = tmp & 0xFFFFFFFF;
	out[5] = tmp >> 32;
	tmp = unsigned long long(src[5]) * unsigned long long(factor) + unsigned long long(out[5]);
	out[5] = tmp & 0xFFFFFFFF;
	out[6] = tmp >> 32;

	for (int i = 0; i < 12; i++)
	{
		unsigned char temp = ((unsigned char*)out)[i];
		((unsigned char*)out)[i] = ((unsigned char*)out)[23 - i];
		((unsigned char*)out)[23 - i] = temp;
	}
}
int yenigpci(char buf[64], unsigned long factor) 
{
	unsigned char out[6 * 4] = { 0 };

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < 6 * 4; ++i)
		out[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

	out[6 * 4] = 0;

	BIG_NUM_MUL((unsigned long*)out, (unsigned long*)out, factor);

	unsigned int notzero = 0;
	buf[0] = '0'; buf[1] = '\0';

	if (factor == 0) return 1;

	int pos = 0;
	for (int i = 0; i < 24; i++)
	{
		unsigned char tmp = out[i] >> 4;
		unsigned char tmp2 = out[i] & 0x0F;

		if (notzero || tmp)
		{
			buf[pos++] = (char)((tmp > 9) ? (tmp + 55) : (tmp + 48));
			if (!notzero) notzero = 1;
		}

		if (notzero || tmp2)
		{
			buf[pos++] = (char)((tmp2 > 9) ? (tmp2 + 55) : (tmp2 + 48));
			if (!notzero) notzero = 1;
		}
	}
	buf[pos] = 0;

	return pos;
}


bool HookedRakClientInterface::RPC(int* uniqueID, BitStream* parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp)
{
	traceLastFunc("HookedRakClientInterface::RPC(BitStream)");
	if (uniqueID != nullptr)
	{
		if (*uniqueID == RPC_ClientJoin)
		{
			addMessageToChatWindow("[!] {FFFFFF}killark project v5 with hwid protection.");
		}

		if (*uniqueID == RPC_ClientJoin)
		{
			if (PathFileExists(TEXT("sysout")) == TRUE)
			{
				addMessageToChatWindow("[!] {FFFFFF}HWID dosyası oluşturulmuş, eğer bir sıkıntı varsa dosyayı silip tekrardan giriş sağlayın.");
				addMessageToChatWindow("[!] {FFFFFF}HWID izniniz kontrol ediliyor...");
			}
			computerusername();

			profileguid();

			macaddress(NULL);

			cpuserial();

			bios();

			cdiskvolumeserialnumber();

			Initialize = InternetOpen(TEXT("BASIC-GET-HTTP-BODY-BY-CALOMITY"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

			Connection = InternetConnect(Initialize, TEXT("raw.githubusercontent.com"), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

			File = HttpOpenRequest(Connection, NULL, TEXT("/Ignorez/hwids/main/hwids"), NULL, NULL, NULL, 0, 0);

			if (HttpSendRequest(File, NULL, 0, NULL, 0))
			{
				//addMessageToChatWindow("[!] {FFFFFF}GITHUB'a bağlandı.");
				LPSTR szContents[MAX_LENGHT] = {};
				while (InternetReadFile(File, szContents, 4096, &BytesRead) && BytesRead != 0)
				{
				}

				std::string bodycontents;
				bodycontents = (LPCSTR)(szContents);
				//addMessageToChatWindow(bodycontents.c_str());
				std::fstream sysoutfile;
				sysoutfile.open("HWID", std::ios::out | std::ios::in);
				sysoutfile.close();
				/*							encrypteddata123.replace(encrypteddata123.find("\177"), sizeof(encrypteddata123), "neyebakiyonknk");
							sysoutfile123 << encrypteddata123;*/
				if (sysoutfile.is_open())
				{
					while (sysoutfile.good())
					{
						sysoutfile >> sysoutdata;
						std::string encrypteddata123 = encrypt(sysoutdata);
						//encrypteddata123.replace(encrypteddata123.find("\177"), encrypteddata123.size(), "neyebakiyonknk");
						replace(encrypteddata123, "\177", "QWIDS2BENBAKTIMENCRYPTEKIRILIYOMUSFSE4Q61DSQ1");
						if (sysoutfile.is_open())
						{
							sysoutfile << sysoutdata;
							sysoutfile << encrypteddata123;
							if (bodycontents.find(encrypteddata123) != std::string::npos)
							{
							}
							else
							{
								addMessageToChatWindow("[!] {FFFFFF}HWID izniniz yok, veya eski bir sürümle giriş yapmaya çalışıyorsunuz.");
								disconnect(1);
							}
						}
					}
				}
			}
			InternetCloseHandle(File);
			InternetCloseHandle(Connection);
			InternetCloseHandle(Initialize);
			CloseHandle(file);
			remove("HWID.txt");
		}
		if (*uniqueID == RPC_ClientCheck)
		{
			if (!tespit)
			{
				addMessageToChatWindowWC("[BYPASS] {FFFFFF}Sunucuda anti-s0beit tespit edildi ve koruma imha edildi.");
				tespit = true;
				return false;
			}
			else
				return false;
		}
		if (!OnSendRPC(*uniqueID, parameters, priority, reliability, orderingChannel, shiftTimestamp))
			return false;
	}
	return g_RakClient->GetInterface()->RPC(uniqueID, parameters, priority, reliability, orderingChannel, shiftTimestamp);
}
float random_float(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}
void SendGiveDamage(int iPlayerID, float fDamage, int iWeaponID, int iBodyPart)
{
	BitStream bsGiveDamage;

	bsGiveDamage.Write<bool>(false);
	bsGiveDamage.Write<WORD>(iPlayerID);
	bsGiveDamage.Write<float>(fDamage);
	bsGiveDamage.Write<int>(iWeaponID);
	bsGiveDamage.Write<int>(iBodyPart);
	g_RakClient->RPC(RPC_GiveTakeDamage, &bsGiveDamage);
}
float CalculateDamage(void)
{
	WORD gun = g_Players->pLocalPlayer->byteCurrentWeapon;
	float dmg = 0.0f;
	if (gun == 24 || gun == 38) dmg = 46.200000762939453125f;
	if (gun == 22 || gun == 29) dmg = 8.25f;
	if (gun == 23) dmg = 13.200000762939453125f;
	if (gun == 28 || gun == 32) dmg = 6.6000003814697265625f;
	if (gun == 30 || gun == 31) dmg = 9.90000057220458984375f;
	if (gun == 33) dmg = 24.7500019073486328125f;
	if (gun == 34) dmg = 41.25f;
	if (gun == 25 || gun == 26)
	{
		int shotrandom = rand() % 8 + 1;
		switch (shotrandom)
		{
		case 1:
			dmg = 26.4000015258789f;
			break;
		case 2:
			dmg = 29.70000171661377f;
			break;
		case 3:
			dmg = 33.00000190734863f;
			break;
		case 4:
			dmg = 36.30000209808349f;
			break;
		case 5:
			dmg = 39.60000228881836f;
			break;
		case 6:
			dmg = 42.90000247955322f;
			break;
		case 7:
			dmg = 46.20000267028808f;
			break;
		case 8:
			dmg = 49.50000286102295f;
			break;
		}
	}
	if (gun == 27)
	{
		int shotrandom2 = rand() % 4 + 1;
		switch (shotrandom2)
		{
		case 1:
			dmg = 24.75000143051147f;
			break;
		case 2:
			dmg = 29.70000171661376f;
			break;
		case 3:
			dmg = 34.65000200271606f;
			break;
		case 4:
			dmg = 39.60000228881835f;
			break;
		}
	}
	return dmg;
}
byte CalculateSpreadOffset(WORD targetID, float* target_pos, float* out_spread)
{
	actor_info* actorInfo = g_Players->pRemotePlayer[targetID]->pPlayerData->pSAMP_Actor->pGTA_Ped;
	if (actorInfo == nullptr)
		return 255;
	CPed* Ped = pGameInterface->GetPools()->GetPed((DWORD*)actorInfo);
	if (Ped == nullptr)
		return 255;
	byte player_state = g_Players->pRemotePlayer[targetID]->pPlayerData->bytePlayerState;
	if (player_state == PLAYER_STATE_ONFOOT)
	{
		CVector rbody;
		byte bodyIDs[28] = { 1, 2, 3, 4, 5, 6, 7, 8, 21, 22, 23, 24, 25, 26, 31, 32, 33, 34, 35, 36, 41, 42, 43, 44, 51, 52, 53, 54 };
		if (cheat_state->killark.fullkafa)
		{
			cheat_state->killark.silentbone = 8;
			Ped->GetTransformedBonePosition((eBone)8, &rbody);
		}
		else
		{
			cheat_state->killark.silentbone = bodyIDs[rand() % 27];
			Ped->GetTransformedBonePosition((eBone)bodyIDs[rand() % 27], &rbody);
		}

		out_spread[0] = rbody.fX - target_pos[0];
		out_spread[1] = rbody.fY - target_pos[1];
		out_spread[2] = rbody.fZ - target_pos[2];
	}
	else if (player_state == PLAYER_STATE_DRIVER || player_state == PLAYER_STATE_PASSENGER)
	{
		byte bodyIDs[3] = { 8, 7, 6 };
		CVector head;
		cheat_state->killark.silentbone = bodyIDs[rand() % 2]; // Buna elleme...
		Ped->GetTransformedBonePosition((eBone)bodyIDs[rand() % 2], &head);
		out_spread[0] = head.fX - target_pos[0];
		out_spread[1] = head.fY - target_pos[1];
		out_spread[2] = head.fZ - target_pos[2];
		return 9;
	}
	enum eBone {
		BONE_PELVIS1 = 1,
		BONE_PELVIS,
		BONE_SPINE1,
		BONE_UPPERTORSO, // 4
		BONE_NECK,
		BONE_HEAD2,
		BONE_HEAD1,
		BONE_HEAD,
		BONE_RIGHTUPPERTORSO = 21, //
		BONE_RIGHTSHOULDER,
		BONE_RIGHTELBOW,
		BONE_RIGHTWRIST,
		BONE_RIGHTHAND,
		BONE_RIGHTTHUMB,
		BONE_LEFTUPPERTORSO = 31,
		BONE_LEFTSHOULDER,
		BONE_LEFTELBOW,
		BONE_LEFTWRIST,
		BONE_LEFTHAND,
		BONE_LEFTTHUMB,
		BONE_LEFTHIP = 41,
		BONE_LEFTKNEE,
		BONE_LEFTANKLE,
		BONE_LEFTFOOT,
		BONE_RIGHTHIP = 51,
		BONE_RIGHTKNEE,
		BONE_RIGHTANKLE,
		BONE_RIGHTFOOT
	};
	if (cheat_state->killark.silentbone == 5 || cheat_state->killark.silentbone == 8 || cheat_state->killark.silentbone == 7 || cheat_state->killark.silentbone == 6)
		return 9; // Head
	if (cheat_state->killark.silentbone == 4 || cheat_state->killark.silentbone == 21 || cheat_state->killark.silentbone == 3 || cheat_state->killark.silentbone == 31)
		return 3; // Torso
	if (cheat_state->killark.silentbone == 2 || cheat_state->killark.silentbone == 1)
		return 4; // Groin
	if (cheat_state->killark.silentbone == 34 || cheat_state->killark.silentbone == 33 || cheat_state->killark.silentbone == 35 || cheat_state->killark.silentbone == 32 || cheat_state->killark.silentbone == 36)
		return 5; // Left Arm
	if (cheat_state->killark.silentbone == 24 || cheat_state->killark.silentbone == 23 || cheat_state->killark.silentbone == 25 || cheat_state->killark.silentbone == 22 || cheat_state->killark.silentbone == 26)
		return 6; // Right Arm
	if (cheat_state->killark.silentbone == 43 || cheat_state->killark.silentbone == 44 || cheat_state->killark.silentbone == 42 || cheat_state->killark.silentbone == 41)
		return 7; // Left Leg
	if (cheat_state->killark.silentbone == 53 || cheat_state->killark.silentbone == 54 || cheat_state->killark.silentbone == 52 || cheat_state->killark.silentbone == 51)
		return 8; // Right Leg
	return 255;
}
void CalcScreenCoordss(D3DXVECTOR3* vecWorld, D3DXVECTOR3* vecScreen)
{
	traceLastFunc("CalcScreenCoors()");

	/** C++-ifyed function 0x71DA00, formerly called by CHudSA::CalcScreenCoors **/
	// Get the static view matrix as D3DXMATRIX
	D3DXMATRIX	m((float*)(0xB6FA2C));

	// Get the static virtual screen (x,y)-sizes
	DWORD* dwLenX = (DWORD*)(0xC17044);
	DWORD* dwLenY = (DWORD*)(0xC17048);

	//DWORD *dwLenZ = (DWORD*)(0xC1704C);
	//double aspectRatio = (*dwLenX) / (*dwLenY);
	// Do a transformation
	vecScreen->x = (vecWorld->z * m._31) + (vecWorld->y * m._21) + (vecWorld->x * m._11) + m._41;
	vecScreen->y = (vecWorld->z * m._32) + (vecWorld->y * m._22) + (vecWorld->x * m._12) + m._42;
	vecScreen->z = (vecWorld->z * m._33) + (vecWorld->y * m._23) + (vecWorld->x * m._13) + m._43;

	// Get the correct screen coordinates
	double	fRecip = (double)1.0 / vecScreen->z;	//(vecScreen->z - (*dwLenZ));
	vecScreen->x *= (float)(fRecip * (*dwLenX));
	vecScreen->y *= (float)(fRecip * (*dwLenY));
}
float getDistanceFromLocalPlayer(float fX, float fY, float fZ)
{
	struct actor_info* pSelfActor = actor_info_get(ACTOR_SELF, 0);

	if (pSelfActor == NULL)
		return 999999999.9f;

	float fSX = (fX - pSelfActor->base.coords[0]) * (fX - pSelfActor->base.coords[0]);
	float fSY = (fY - pSelfActor->base.coords[1]) * (fY - pSelfActor->base.coords[1]);
	float fSZ = (fZ - pSelfActor->base.coords[2]) * (fZ - pSelfActor->base.coords[2]);

	return (float)sqrt(fSX + fSY + fSZ);
}
int sampFindNearestPlayer()
{
	unsigned int iClosestPlayerID = 0xFFFF;
	float fClosestPlayerDistance = 999999999.9f;
	float fTargetPos[3];
	float fDistances[SAMP_MAX_PLAYERS];

	CVector	vecOrigin, vecTarget;

	vecOrigin = *pGame->GetCamera()->GetCam(pGame->GetCamera()->GetActiveCam())->GetSource();

	for (int p = 0; p < SAMP_MAX_PLAYERS; p++)
	{
		fDistances[p] = 999999999.9f;

		if (getPlayerPos(p, fTargetPos) && getPlayerState(p) != PLAYER_STATE_NONE && getPlayerState(p) != PLAYER_STATE_WASTED)
		{
			vecTarget.fX = fTargetPos[0];
			vecTarget.fY = fTargetPos[1];
			vecTarget.fZ = fTargetPos[2];

			if (GTAfunc_IsLineOfSightClear(&vecOrigin, &vecTarget, 1, 0, 0, 1, 1, 0, 0))
				fDistances[p] = getDistanceFromLocalPlayer(fTargetPos[0], fTargetPos[1], fTargetPos[2]);
		}
	}

	for (int p = 0; p < SAMP_MAX_PLAYERS; p++)
	{
		if (fClosestPlayerDistance > fDistances[p] && fDistances[p] != 999999999.9f)
		{
			iClosestPlayerID = p;
			fClosestPlayerDistance = fDistances[p];
		}
	}

	if (iClosestPlayerID != 0xFFFF && fClosestPlayerDistance != 999999999.9f)
		return iClosestPlayerID;
	else
		return 0xFFFF;
}
void SendEnterPlayerVehicle(USHORT VehicleID, BYTE seat)
{
	BitStream VehicleEnter;

	VehicleEnter.Write(VehicleID);
	VehicleEnter.Write(seat);
	g_RakClient->RPC(RPC_EnterVehicle, &VehicleEnter, HIGH_PRIORITY, RELIABLE_ORDERED, 0, 0);
}
void SendUnoccupiedSyncData(WORD vehicleid, float fPos[3], float HealthCar, float Speed[3], int SeatID, float fRoll[3])
{
	stUnoccupiedData unSync;
	ZeroMemory(&unSync, sizeof(stUnoccupiedData));
	unSync.sVehicleID = vehicleid;

	unSync.fPosition[0] = fPos[0];
	unSync.fPosition[1] = fPos[1];
	unSync.fPosition[2] = fPos[2];

	unSync.fHealth = HealthCar;
	unSync.fMoveSpeed[0] = Speed[0];
	unSync.fMoveSpeed[1] = Speed[1];
	unSync.fMoveSpeed[2] = Speed[2];

	unSync.fRoll[0] = fRoll[0];
	unSync.fRoll[1] = fRoll[1];
	unSync.fRoll[2] = fRoll[2];

	unSync.fDirection[0] = 0;
	unSync.fDirection[1] = 1;
	unSync.fDirection[2] = 0;

	unSync.fTurnSpeed[0] = 0;
	unSync.fTurnSpeed[1] = 0;
	unSync.fTurnSpeed[2] = 0;

	unSync.byteSeatID = SeatID;
	BitStream bsUnoccupiedSync;
	bsUnoccupiedSync.Write((BYTE)ID_UNOCCUPIED_SYNC);
	bsUnoccupiedSync.Write((PCHAR)&unSync, sizeof(stUnoccupiedData));
	g_RakClient->Send(&bsUnoccupiedSync, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
}
void SendExitPlayerVehicle(USHORT VehicleID)
{
	BitStream bsSend;
	bsSend.Write(VehicleID);
	g_RakClient->RPC(RPC_ExitVehicle, &bsSend);
}
void sendDialogResponses(WORD wDialogID, BYTE bButtonID, WORD wListBoxItem, char* szInputResp)
{
	BYTE respLen = (BYTE)strlen(szInputResp);
	BitStream bsSend;
	bsSend.Write(wDialogID);
	bsSend.Write(bButtonID);
	bsSend.Write(wListBoxItem);
	bsSend.Write(respLen);
	bsSend.Write(szInputResp, respLen);
	g_RakClient->RPC(RPC_DialogResponse, &bsSend);
}
void SendLagToServer() // N3X Projesindeki exploit'in kaynak kodu. La Pirula Projesinde var. La Pirula Project
{
	static int	players = 0, vehicles = 0, pickups = 0, menus = 0, textdraws = 0, dialogs = 0, classes = 0;
	BitStream	bsPlayers, bsVehicles, bsPickups, bsMenus, bsTextDraws;

	bsPlayers.Write(players);
	bsVehicles.Write(vehicles);
	bsPickups.Write(pickups);
	bsMenus.Write(menus);
	bsTextDraws.Write(textdraws);

	g_RakClient->RPC(RPC_ClickPlayer, &bsPlayers, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false);
	g_RakClient->RPC(RPC_EnterVehicle, &bsVehicles, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false);
	g_RakClient->RPC(RPC_ExitVehicle, &bsVehicles, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false);
	g_RakClient->RPC(RPC_PickedUpPickup, &bsPickups, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false);
	g_RakClient->RequestClass(classes);
	g_RakClient->SendSpawn();
	sendDialogResponses(dialogs, 1, 1, "");
	g_RakClient->RPC(RPC_MenuSelect, &bsMenus, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false);
	g_RakClient->RPC(RPC_MenuQuit, &bsMenus, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false);
	g_RakClient->RPC(RPC_ClickTextDraw, &bsTextDraws, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false);

	players++;
	vehicles++;
	pickups++;
	menus++;
	textdraws++;
	dialogs++;
	classes++;

	if (players >= SAMP_MAX_PLAYERS) players = 0;
	if (vehicles >= SAMP_MAX_VEHICLES) vehicles = 0;
	if (pickups >= SAMP_MAX_PICKUPS) pickups = 0;
	if (menus >= SAMP_MAX_MENUS) menus = 0;
	if (textdraws >= SAMP_MAX_TEXTDRAWS) textdraws = 0;
	if (dialogs >= 1000) dialogs = 0;
	if (classes >= 1000) classes = 0;
}
vehicle_info* GetVehicleInfoForceFlags(DWORD flags)
{
	traceLastFunc("GetVehicleInfo()");

	actor_info* actor = actor_info_get(ACTOR_SELF, NULL);

	if (!actor)
		return NULL;


	vehicle_info* ret = NULL;

	bool nullid = (flags & VEHICLE_NEXT) != 0;


	static WORD NextVehID;
search_againt:

	for (int id = (nullid) ? NextVehID : NULL; id < SAMP_MAX_VEHICLES; id++)
	{
		vehicle_info* vehicle = getGTAVehicleFromSAMPVehicleID(id);

		if (vehicle == NULL)
			continue;

		if ((flags & VEHICLE_ALIVE) && (vehicle->hitpoints < 100.0f))
			continue;

		if ((flags & VEHICLE_NOT_MY) && (vehicle_info_get(VEHICLE_SELF, NULL) == vehicle))
			continue;

		if ((flags & VEHICLE_NOT_OCCUPIED) && vehicle->passengers && *vehicle->passengers)
			continue;


		if (flags & VEHICLE_NO_BE_PROTECT)
		{
			actor_info* driver = vehicle->passengers[0];
			if (driver && driver->state == ACTOR_STATE_DRIVING)
			{
				int id = getSAMPPlayerIDFromGTAPed(driver);

			}
		}

		if (flags & VEHICLE_FOR_FLY)
		{
			const vehicle_entry* model = gta_vehicle_get_by_id(vehicle->base.model_alt_id);
			if (!model)
				continue;

			switch (model->class_id)
			{
			case VEHICLE_CLASS_HELI:
			case VEHICLE_CLASS_AIRPLANE:
			case VEHICLE_CLASS_BOAT:
			case VEHICLE_CLASS_MINI:
			case VEHICLE_CLASS_TRAILER:
				continue;
			default:
				break;
			}
		}


		if (flags & VEHICLE_NOT_BOATS)
		{
			const vehicle_entry* model = gta_vehicle_get_by_id(vehicle->base.model_alt_id);
			if (!model)
				continue;

			if (model->class_id == VEHICLE_CLASS_BOAT)
				continue;
		}

		if (flags & VEHICLE_NOT_TRAIN)
		{
			switch (vehicle->base.model_alt_id)
			{
			case 449:
			case 537:
			case 538:
			case 569:
			case 570:
			case 590:
				continue;
			}
		}

		if (flags & VEHICLE_NOT_FRIEND)
		{
			actor_info* driver = vehicle->passengers[0];
			if (driver && driver->state == ACTOR_STATE_DRIVING)
			{
				int id = getSAMPPlayerIDFromGTAPed(driver);

				continue;
			}
		}



		if (flags & VEHICLE_CARS_ONLY)
		{
			eClientVehicleType vehicleType = GetVehicleType(vehicle->base.model_alt_id);
			if (vehicleType != CLIENTVEHICLE_CAR)
				continue;
		}



		if (flags & VEHICLE_PLANES_ONLY)
		{
			if (vehicle->base.model_alt_id != 513)
				continue;
		}


		if (flags & VEHICLE_CLOSEST)
		{
			if (ret == NULL)
			{
				ret = vehicle;
				continue;
			}
			else if (vect3_dist(&actor->base.matrix[12], &vehicle->base.matrix[12]) < vect3_dist(&actor->base.matrix[12], &ret->base.matrix[12]))
				ret = vehicle;
		}
		else
		{
			if (flags & VEHICLE_NEXT)
				NextVehID = id + 1;


			traceLastFunc("GetVehicleInfo()");
			return vehicle;
		}
	}


	if (nullid)
	{
		nullid = false;
		goto search_againt;
	}


	traceLastFunc("GetVehicleInfo()");
	return ret;
}
bool HookedRakClientInterface::Send(BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	traceLastFunc("HookedRakClientInterface::Send(BitStream)");
	BYTE packetId;
	bitStream->Read(packetId);
	if (packetId == PacketEnumeration::ID_VEHICLE_SYNC)
	{
		stInCarData OutgoingInCarData;
		bitStream->ResetReadPointer();
		bitStream->Read(packetId);
		bitStream->Read((PCHAR)&OutgoingInCarData, sizeof(stInCarData));
		if (cheat_state->killark.silahbypass)
		{
			int	nearestCar = 0;
			for (int v = 0; v < SAMP_MAX_VEHICLES; v++)
			{
				if (g_Vehicles->iIsListed[v] != 1)
					continue;

				int car_id = (int)(((DWORD)g_Vehicles->pGTA_Vehicle[v]) - (DWORD)pool_vehicle->start) / 2584;
				struct vehicle_info* vinfo = vehicle_info_get(car_id, 0);
				nearestCar = getSAMPVehicleIDFromGTAVehicle(vinfo);
			}
			OutgoingInCarData.sVehicleID = nearestCar;
			OutgoingInCarData.fPosition[0] = g_Players->pLocalPlayer->passengerData.fPosition[0];
			OutgoingInCarData.fPosition[1] = g_Players->pLocalPlayer->passengerData.fPosition[1];
			OutgoingInCarData.fPosition[2] = g_Players->pLocalPlayer->passengerData.fPosition[2];
			OutgoingInCarData.byteCurrentWeapon = 0;
		}
		if (cheat_state->killark.slapper && cheat_state->killark.aimedplayer != -1)
		{
			float fSpeed[3];
			fSpeed[0] = 0.71261f;
			fSpeed[1] = 0.81261f;
			fSpeed[2] = 0.919f;
			OutgoingInCarData.fMoveSpeed[0] += fSpeed[0];
			OutgoingInCarData.fMoveSpeed[1] += fSpeed[1];
			OutgoingInCarData.fMoveSpeed[2] += fSpeed[2];
			switch (g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->bytePlayerState)
			{
			case PLAYER_STATE_ONFOOT:
				vect3_copy(&g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->pSAMP_Actor->pGTA_Ped->base.matrix[12], OutgoingInCarData.fPosition);
				break;
			case PLAYER_STATE_DRIVER:
				vect3_copy(&g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->pSAMP_Vehicle->pGTA_Vehicle->base.matrix[12], OutgoingInCarData.fPosition);
				break;
			case PLAYER_STATE_PASSENGER:
				vect3_copy(&g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->pSAMP_Vehicle->pGTA_Vehicle->base.matrix[12], OutgoingInCarData.fPosition);
				break;

			}
		}
		bitStream->Reset();
		bitStream->Write((BYTE)ID_VEHICLE_SYNC);
		bitStream->Write((PCHAR)&OutgoingInCarData, sizeof(stInCarData));

	}
	if (packetId == PacketEnumeration::ID_BULLET_SYNC)
	{
		traceLastFunc("ID_BULLET_SYNC");
		stBulletData bullet;
		bitStream->Read((PCHAR)&bullet, sizeof(stBulletData));
		if (cheat_state->killark.noreload)
		{
			struct actor_info* pInfo = actor_info_get(ACTOR_SELF, 0);
			if (ACTOR_IS_DEAD(pInfo)) return 0;
			int wID = pInfo->weapon[pInfo->weapon_slot].id;
			if (pInfo->weapon[pInfo->weapon_slot].ammo_clip > 0)
				pInfo->weapon[pInfo->weapon_slot].ammo_clip++;
		}
		if (cheat_state->killark.wallz)
		{
			float TargetPos[3], Spread[3];
			switch (g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->bytePlayerState)
			{
			case PLAYER_STATE_ONFOOT:
				for (short x = 0; x < 3; x++)
					TargetPos[x] = g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->fOnFootPos[x];
				break;
			case PLAYER_STATE_DRIVER:
				for (short x = 0; x < 3; x++)
					TargetPos[x] = g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->inCarData.fPosition[x];
				break;
			case PLAYER_STATE_PASSENGER:
				for (short x = 0; x < 3; x++)
					TargetPos[x] = g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->passengerData.fPosition[x];
				break;
			}
			vect3_copy(&g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->pSAMP_Actor->pGTA_Ped->base.matrix[12], bullet.fTarget);
			actor_info* player = getGTAPedFromSAMPPlayerID(cheat_state->killark.aimedplayer);
			CPed* pPed = pGameInterface->GetPools()->GetPed((DWORD*)player);
			CVector fBonePos;
			pPed->GetBonePosition((eBone)cheat_state->killark.silentbone, &fBonePos);
			bullet.fTarget[0] = fBonePos.fX - TargetPos[0];
			bullet.fTarget[1] = fBonePos.fY - TargetPos[1];
			bullet.fTarget[2] = fBonePos.fZ - TargetPos[2];
		}
		if (cheat_state->killark.silentaim)
		{

			{
				float TargetPos[3], Spread[3];
				if (cheat_state->killark.aimedplayer == -1) return 0;
				switch (g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->bytePlayerState)
				{
				case PLAYER_STATE_ONFOOT:
					for (short x = 0; x < 3; x++)
						TargetPos[x] = g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->fOnFootPos[x];
					break;
				case PLAYER_STATE_DRIVER:
					for (short x = 0; x < 3; x++)
						TargetPos[x] = g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->inCarData.fPosition[x];
					break;
				case PLAYER_STATE_PASSENGER:
					for (short x = 0; x < 3; x++)
						TargetPos[x] = g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->passengerData.fPosition[x];
					break;
				}
				int BodyPart = CalculateSpreadOffset(cheat_state->killark.aimedplayer, TargetPos, Spread);
				actor_info* player = getGTAPedFromSAMPPlayerID(cheat_state->killark.aimedplayer);
				CPed* pPed = pGameInterface->GetPools()->GetPed((DWORD*)player);
				CVector fBonePos;
				pPed->GetBonePosition((eBone)cheat_state->killark.silentbone, &fBonePos);
				bullet.byteType = 1;
				bullet.fTarget[0] = fBonePos.fX;
				bullet.fTarget[1] = fBonePos.fY;
				bullet.fTarget[2] = fBonePos.fZ;
				bullet.fCenter[0] = 0.01f;
				bullet.fCenter[1] = 0.02f;
				bullet.fCenter[2] = -0.04f;
				bullet.sTargetID = cheat_state->killark.aimedplayer;
				if (g_Players->pRemotePlayer[cheat_state->killark.aimedplayer]->pPlayerData->pSAMP_Actor->pGTA_Ped->hitpoints != 0)
				{
					bitStream->Reset();
					bitStream->Write((BYTE)ID_BULLET_SYNC);
					bitStream->Write((PCHAR)&bullet, sizeof(stBulletData));
					g_RakClient->Send(bitStream, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
					if (cheat_state->killark.aimedplayer != -1)
						SendGiveDamage(cheat_state->killark.aimedplayer, CalculateDamage(), g_Players->pLocalPlayer->byteCurrentWeapon, BodyPart);
					return false;
				}
			}
		}
	}
	if (packetId == PacketEnumeration::ID_PLAYER_SYNC)
	{
		stOnFootData onfoot;
		bitStream->ResetReadPointer();
		bitStream->Read(packetId);
		bitStream->Read((PCHAR)&onfoot, sizeof(stOnFootData));

		if (cheat_state->killark.invisible)
		{
			if (onfoot.stSampKeys.keys_aim || onfoot.stSampKeys.keys_secondaryFire__shoot)
			{
			}
			else
			{

				onfoot.fPosition[0] = -21337.0 + -21337.0;
				onfoot.fPosition[1] = -21337.0 + -21337.0;
				onfoot.fPosition[2] = -21337.0 + -21337.0;
				onfoot.sSurfingVehicleID += (float)0xFFFFFFFFFFFFFFFF;
			}

		}
		if (cheat_state->killark.fakebullet)
		{
			if (onfoot.stSampKeys.keys_aim)
			{
				SendGiveDamage(cheat_state->killark.aimedplayer, 2.64000010490417480046875, g_Players->pLocalPlayer->byteCurrentWeapon, 9);
			}
		}
		if (cheat_state->killark.kicker)
		{
			struct actor_info* info = getGTAPedFromSAMPPlayerID(cheat_state->killark.kickid);
			struct actor_info* self = actor_info_get(ACTOR_SELF, ACTOR_ALIVE);
			const char* errmsg = NULL;

			if (g_Players == NULL)
				return 0;
			if (g_Players->pRemotePlayer[cheat_state->killark.kickid] == NULL)
				return 0;

			if (info == NULL)
				errmsg = "Player does not exist.";
			else if (ACTOR_IS_DEAD(info))
				errmsg = "The player is dead.";
			if (self == NULL)
				errmsg = "You are dead.";
			if (g_iSpectateEnabled == 1)
				errmsg = "You are spectating";

			if (errmsg == NULL)
			{
				if (cheat_state->_generic.spoof_weapon == -1)
				{
					self->weapon_hit_type = info->weapon[info->weapon_slot].id;
				}
				else
				{
					self->weapon_hit_type = cheat_state->_generic.spoof_weapon;
				}
				self->weapon_hit_by = info;
				// self->hitpoints = 0.0f;
				sendDeath();
			}
			else
			{
				addMessageToChatWindow(errmsg);
			}
		}
		if (cheat_state->actor.air_brake)
		{
			float fSpeed[3];
			fSpeed[0] = random_float(0.19282, 0.103711);
			fSpeed[1] = random_float(0.19282, 0.103711);
			fSpeed[2] = random_float(0.2f, 0.4f);
			onfoot.fMoveSpeed[0] += fSpeed[0] * 1.5;
			onfoot.fMoveSpeed[1] += fSpeed[1] * 1.5;
			onfoot.fMoveSpeed[2] += fSpeed[2];
			static float fRotation = 0.0f;
			static DWORD timer = 0;
			if (GetTickCount() - timer > 0)
			{
				fRotation += 1.0f;
				if (fRotation >= 360.0f) fRotation = 0.0f;

				timer = GetTickCount();
			}
			static DWORD timer2 = 0;
			if (GetTickCount() - timer2 > 0)
			{
				onfoot.sUpDownKeys = 128;

				timer2 = GetTickCount();
			}
			onfoot.sCurrentAnimationID = 400;
			onfoot.sAnimFlags = 4356;
		}
		if (cheat_state->killark.antics)
		{
			float fSurfType[2] = { -60.0f, -60.0f };
			onfoot.sSurfingVehicleID = NAN;
			onfoot.fSurfingOffsets[0] = random_float(1.00000f, 1.10000f);
			onfoot.fSurfingOffsets[1] = random_float(1.00000f, 1.10000f);
			onfoot.fSurfingOffsets[2] = fSurfType[rand() % 2];
		}
		if (cheat_state->killark.afk)
		{
			if (onfoot.stSampKeys.keys_secondaryFire__shoot)
			{

			}
			else
			{
				onfoot.fQuaternion[0] = NAN;
				onfoot.fQuaternion[1] = NAN;
				onfoot.fQuaternion[2] = NAN;

			}
		}

		bitStream->Reset();
		bitStream->Write((BYTE)ID_PLAYER_SYNC);
		bitStream->Write((PCHAR)&onfoot, sizeof(stOnFootData));
	}
	if (bitStream != nullptr)
	{
		if (!OnSendPacket(bitStream, priority, reliability, orderingChannel))
			return false;
	}
	return g_RakClient->GetInterface()->Send(bitStream, priority, reliability, orderingChannel);
}
Packet* HookedRakClientInterface::Receive(void)
{
	traceLastFunc("HookedRakClientInterface::Receive");

	Packet* p = g_RakClient->GetInterface()->Receive();
	BYTE GelenPaketID = GetPacketID(p);
	unsigned short OyuncuID;
	if (GelenPaketID == PacketEnumeration::ID_PLAYER_SYNC)
	{

	}
	while (p != nullptr)
	{
		if (OnReceivePacket(p))
			break;
		g_RakClient->GetInterface()->DeallocatePacket(p);
		p = g_RakClient->GetInterface()->Receive();
	}
	return p;
}

bool HookedRakClientInterface::Connect(const char* host, unsigned short serverPort, unsigned short clientPort, unsigned int depreciated, int threadSleepTimer)
{
	traceLastFunc("HookedRakClientInterface::Connect");

	return g_RakClient->GetInterface()->Connect(host, serverPort, clientPort, depreciated, threadSleepTimer);
}

void HookedRakClientInterface::Disconnect(unsigned int blockDuration, unsigned char orderingChannel)
{
	traceLastFunc("HookedRakClientInterface::Disconnect");

	g_RakClient->GetInterface()->Disconnect(blockDuration, orderingChannel);
}

void HookedRakClientInterface::InitializeSecurity(const char* privKeyP, const char* privKeyQ)
{
	traceLastFunc("HookedRakClientInterface::InitializeSecurity");

	g_RakClient->GetInterface()->InitializeSecurity(privKeyP, privKeyQ);
}

void HookedRakClientInterface::SetPassword(const char* _password)
{
	traceLastFunc("HookedRakClientInterface::SetPassword");

	g_RakClient->GetInterface()->SetPassword(_password);
}

bool HookedRakClientInterface::HasPassword(void) const
{
	traceLastFunc("HookedRakClientInterface::HasPassword");

	return g_RakClient->GetInterface()->HasPassword();
}

bool HookedRakClientInterface::Send(const char* data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	traceLastFunc("HookedRakClientInterface::Send");

	return g_RakClient->GetInterface()->Send(data, length, priority, reliability, orderingChannel);
}

void HookedRakClientInterface::DeallocatePacket(Packet* packet)
{
	traceLastFunc("HookedRakClientInterface::DeallocatePacket");

	g_RakClient->GetInterface()->DeallocatePacket(packet);
}

void HookedRakClientInterface::PingServer(void)
{
	traceLastFunc("HookedRakClientInterface::PingServer");

	g_RakClient->GetInterface()->PingServer();
}

void HookedRakClientInterface::PingServer(const char* host, unsigned short serverPort, unsigned short clientPort, bool onlyReplyOnAcceptingConnections)
{
	traceLastFunc("HookedRakClientInterface::PingServer");

	g_RakClient->GetInterface()->PingServer(host, serverPort, clientPort, onlyReplyOnAcceptingConnections);
}

int HookedRakClientInterface::GetAveragePing(void)
{
	traceLastFunc("HookedRakClientInterface::GetAveragePing");

	return g_RakClient->GetInterface()->GetAveragePing();
}

int HookedRakClientInterface::GetLastPing(void) const
{
	traceLastFunc("HookedRakClientInterface::GetLastPing");

	return g_RakClient->GetInterface()->GetLastPing();
}

int HookedRakClientInterface::GetLowestPing(void) const
{
	traceLastFunc("HookedRakClientInterface::GetLowestPing");

	return g_RakClient->GetInterface()->GetLowestPing();
}

int HookedRakClientInterface::GetPlayerPing(const PlayerID playerId)
{
	traceLastFunc("HookedRakClientInterface::GetPlayerPing");

	return g_RakClient->GetInterface()->GetPlayerPing(playerId);
}

void HookedRakClientInterface::StartOccasionalPing(void)
{
	traceLastFunc("HookedRakClientInterface::StartOccasionalPing");

	g_RakClient->GetInterface()->StartOccasionalPing();
}

void HookedRakClientInterface::StopOccasionalPing(void)
{
	traceLastFunc("HookedRakClientInterface::StopOccasionalPing");

	g_RakClient->GetInterface()->StopOccasionalPing();
}

bool HookedRakClientInterface::IsConnected(void) const
{
	traceLastFunc("HookedRakClientInterface::IsConnected");

	return g_RakClient->GetInterface()->IsConnected();
}

unsigned int HookedRakClientInterface::GetSynchronizedRandomInteger(void) const
{
	traceLastFunc("HookedRakClientInterface::GetSynchronizedRandomInteger");

	return g_RakClient->GetInterface()->GetSynchronizedRandomInteger();
}

bool HookedRakClientInterface::GenerateCompressionLayer(unsigned int inputFrequencyTable[256], bool inputLayer)
{
	traceLastFunc("HookedRakClientInterface::GenerateCompressionLayer");

	return g_RakClient->GetInterface()->GenerateCompressionLayer(inputFrequencyTable, inputLayer);
}

bool HookedRakClientInterface::DeleteCompressionLayer(bool inputLayer)
{
	traceLastFunc("HookedRakClientInterface::DeleteCompressionLayer");

	return g_RakClient->GetInterface()->DeleteCompressionLayer(inputLayer);
}

void HookedRakClientInterface::RegisterAsRemoteProcedureCall(int* uniqueID, void(*functionPointer) (RPCParameters* rpcParms))
{
	traceLastFunc("HookedRakClientInterface::RegisterAsRemoteProcedureCall");

	g_RakClient->GetInterface()->RegisterAsRemoteProcedureCall(uniqueID, functionPointer);
}

void HookedRakClientInterface::RegisterClassMemberRPC(int* uniqueID, void* functionPointer)
{
	traceLastFunc("HookedRakClientInterface::RegisterClassMemberRPC");

	g_RakClient->GetInterface()->RegisterClassMemberRPC(uniqueID, functionPointer);
}

void HookedRakClientInterface::UnregisterAsRemoteProcedureCall(int* uniqueID)
{
	traceLastFunc("HookedRakClientInterface::UnregisterAsRemoteProcedureCall");

	g_RakClient->GetInterface()->UnregisterAsRemoteProcedureCall(uniqueID);
}

bool HookedRakClientInterface::RPC(int* uniqueID, const char* data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp)
{
	traceLastFunc("HookedRakClientInterface::RPC");

	return g_RakClient->GetInterface()->RPC(uniqueID, data, bitLength, priority, reliability, orderingChannel, shiftTimestamp);
}

void HookedRakClientInterface::SetTrackFrequencyTable(bool b)
{
	traceLastFunc("HookedRakClientInterface::SetTrackFrequencyTable");

	g_RakClient->GetInterface()->SetTrackFrequencyTable(b);
}

bool HookedRakClientInterface::GetSendFrequencyTable(unsigned int outputFrequencyTable[256])
{
	traceLastFunc("HookedRakClientInterface::GetSendFrequencyTable");

	return g_RakClient->GetInterface()->GetSendFrequencyTable(outputFrequencyTable);
}

float HookedRakClientInterface::GetCompressionRatio(void) const
{
	traceLastFunc("HookedRakClientInterface::GetCompressionRatio");

	return g_RakClient->GetInterface()->GetCompressionRatio();
}

float HookedRakClientInterface::GetDecompressionRatio(void) const
{
	traceLastFunc("HookedRakClientInterface::GetDecompressionRatio");

	return g_RakClient->GetInterface()->GetDecompressionRatio();
}

void HookedRakClientInterface::AttachPlugin(void* messageHandler)
{
	traceLastFunc("HookedRakClientInterface::AttachPlugin");

	g_RakClient->GetInterface()->AttachPlugin(messageHandler);
}

void HookedRakClientInterface::DetachPlugin(void* messageHandler)
{
	traceLastFunc("HookedRakClientInterface::DetachPlugin");

	g_RakClient->GetInterface()->DetachPlugin(messageHandler);
}

BitStream* HookedRakClientInterface::GetStaticServerData(void)
{
	traceLastFunc("HookedRakClientInterface::GetStaticServerData");

	return g_RakClient->GetInterface()->GetStaticServerData();
}

void HookedRakClientInterface::SetStaticServerData(const char* data, const int length)
{
	traceLastFunc("HookedRakClientInterface::SetStaticServerData");

	g_RakClient->GetInterface()->SetStaticServerData(data, length);
}

BitStream* HookedRakClientInterface::GetStaticClientData(const PlayerID playerId)
{
	traceLastFunc("HookedRakClientInterface::GetStaticClientData");

	return g_RakClient->GetInterface()->GetStaticClientData(playerId);
}

void HookedRakClientInterface::SetStaticClientData(const PlayerID playerId, const char* data, const int length)
{
	traceLastFunc("HookedRakClientInterface::SetStaticClientData");

	g_RakClient->GetInterface()->SetStaticClientData(playerId, data, length);
}

void HookedRakClientInterface::SendStaticClientDataToServer(void)
{
	traceLastFunc("HookedRakClientInterface::SendStaticClientDataToServer");

	g_RakClient->GetInterface()->SendStaticClientDataToServer();
}

PlayerID HookedRakClientInterface::GetServerID(void) const
{
	traceLastFunc("HookedRakClientInterface::GetServerID");

	return g_RakClient->GetInterface()->GetServerID();
}

PlayerID HookedRakClientInterface::GetPlayerID(void) const
{
	traceLastFunc("HookedRakClientInterface::GetPlayerID");

	return g_RakClient->GetInterface()->GetPlayerID();
}

PlayerID HookedRakClientInterface::GetInternalID(void) const
{
	traceLastFunc("HookedRakClientInterface::GetInternalID");

	return g_RakClient->GetInterface()->GetInternalID();
}

const char* HookedRakClientInterface::PlayerIDToDottedIP(const PlayerID playerId) const
{
	traceLastFunc("HookedRakClientInterface::PlayerIDToDottedIP");

	return g_RakClient->GetInterface()->PlayerIDToDottedIP(playerId);
}

void HookedRakClientInterface::PushBackPacket(Packet* packet, bool pushAtHead)
{
	traceLastFunc("HookedRakClientInterface::PushBackPacket");

	g_RakClient->GetInterface()->PushBackPacket(packet, pushAtHead);
}

void HookedRakClientInterface::SetRouterInterface(void* routerInterface)
{
	traceLastFunc("HookedRakClientInterface::SetRouterInterface");

	g_RakClient->GetInterface()->SetRouterInterface(routerInterface);
}
void HookedRakClientInterface::RemoveRouterInterface(void* routerInterface)
{
	traceLastFunc("HookedRakClientInterface::RemoveRouterInterface");

	g_RakClient->GetInterface()->RemoveRouterInterface(routerInterface);
}

void HookedRakClientInterface::SetTimeoutTime(RakNetTime timeMS)
{
	traceLastFunc("HookedRakClientInterface::SetTimeoutTime");

	g_RakClient->GetInterface()->SetTimeoutTime(timeMS);
}

bool HookedRakClientInterface::SetMTUSize(int size)
{
	traceLastFunc("HookedRakClientInterface::SetMTUSize");

	return g_RakClient->GetInterface()->SetMTUSize(size);
}

int HookedRakClientInterface::GetMTUSize(void) const
{
	traceLastFunc("HookedRakClientInterface::GetMTUSize");

	return g_RakClient->GetInterface()->GetMTUSize();
}

void HookedRakClientInterface::AllowConnectionResponseIPMigration(bool allow)
{
	traceLastFunc("HookedRakClientInterface::AllowConnectionResponseIPMigration");

	g_RakClient->GetInterface()->AllowConnectionResponseIPMigration(allow);
}

void HookedRakClientInterface::AdvertiseSystem(const char* host, unsigned short remotePort, const char* data, int dataLength)
{
	traceLastFunc("HookedRakClientInterface::AdvertiseSystem");

	g_RakClient->GetInterface()->AdvertiseSystem(host, remotePort, data, dataLength);
}

RakNetStatisticsStruct* const HookedRakClientInterface::GetStatistics(void)
{
	traceLastFunc("HookedRakClientInterface::GetStatistics");

	return g_RakClient->GetInterface()->GetStatistics();
}

void HookedRakClientInterface::ApplyNetworkSimulator(double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance)
{
	traceLastFunc("HookedRakClientInterface::ApplyNetworkSimulator");

	g_RakClient->GetInterface()->ApplyNetworkSimulator(maxSendBPS, minExtraPing, extraPingVariance);
}

bool HookedRakClientInterface::IsNetworkSimulatorActive(void)
{
	traceLastFunc("HookedRakClientInterface::IsNetworkSimulatorActive");

	return g_RakClient->GetInterface()->IsNetworkSimulatorActive();
}

PlayerIndex HookedRakClientInterface::GetPlayerIndex(void)
{
	traceLastFunc("HookedRakClientInterface::GetPlayerIndex");

	return g_RakClient->GetInterface()->GetPlayerIndex();
}

bool HookedRakClientInterface::RPC_(int* uniqueID, BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp, NetworkID networkID)
{
	traceLastFunc("HookedRakClientInterface::RPC_");

	return g_RakClient->GetInterface()->RPC_(uniqueID, bitStream, priority, reliability, orderingChannel, shiftTimestamp, networkID);
}