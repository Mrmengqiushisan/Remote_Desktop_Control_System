#include "pch.h"
#include "Command.h"

Command::Command() :threadId(0) {
	struct {
		int	nCmd;
		CMDFUNC func;
	}data[] = {
		{1,&Command::MakeDriverInfo},
		{2,&Command::MakeDriectoryInfo},
		{3,&Command::RunFile},
		{4,&Command::DownLoadFile},
		{5,&Command::MouseEvent},
		{6,&Command::SendScreen},
		{7,&Command::LockMachine},
		{8,&Command::UnLockMachine},
		{9,&Command::DeleteLoaclFile},
		{1981,&Command::TestConnect},
		{-1,NULL}
	};
	for (int i = 0; data[i].nCmd != -1; i++)
		m_mapFunction.emplace(data[i].nCmd, data[i].func);
}

int Command::ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPacket) {
	if (m_mapFunction.count(nCmd)) 
		return (this->*m_mapFunction[nCmd])(lstPacket, inPacket);
	return -1;
}
