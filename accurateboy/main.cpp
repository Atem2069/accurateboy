#include<iostream>
#include"Logger.h"
#include"Gameboy.h"

int main()
{
	Logger::getInstance()->msg(LoggerSeverity::Info, "Hello world!");
	GameBoy m_gameBoy;
	m_gameBoy.run();
	return 0;
}