#pragma once
class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		sSum = pack.sSum;
		strData = pack.strData;
	}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			sSum = pack.sSum;
			strData = pack.strData;
		}
		return *this;
	}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = (DWORD)(nSize + 4);
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else strData.clear();
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
			sSum += BYTE(pData[j]) & 0xFF;
	}
	//解包
	CPacket(const BYTE* pData, size_t& nSize) :CPacket() {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i = i + 2;
				break;
			}
		}
		//检查包长度
		if (i + 4 + 2 + 2 > nSize) {
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 4);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum{ 0 };
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}
	WORD GetCmd() { return sCmd; }
	std::string GetStrData() { return strData; }
	int Size() {
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;							pData += 2;
		*(DWORD*)(pData) = nLength;						pData += 4;
		*(WORD*)pData = sCmd;							pData += 2;
		memcpy(pData, strData.c_str(), strData.size());	pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
	~CPacket() {}
private:
	WORD		sHead;		//固定长度为 FE FF
	DWORD		nLength;	//包长度
	WORD		sCmd;		//控制命令
	WORD		sSum;		//和校验
	std::string	strData;	//包数据
	std::string strOut;		//整个包的数据
};

typedef struct file_info {
	BOOL isInvalid;         //是否有效
	BOOL isDirectory;       //是否为目录0否1是
	BOOL hasNext;           //是否还有后续文件0没有1有
	char szFileName[MAX_PATH]{}; //文件名
	file_info() {
		isInvalid = FALSE;
		isDirectory = -1;
		hasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
}FILEINFO, * PFILEINFO;

typedef struct MouseEvent {
	WORD nAction;//点击，移动，双击
	WORD nButton;//左键，右键，中键
	POINT ptXY;	 //坐标
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
}MOUSEEV, * PMOUSEEV;

