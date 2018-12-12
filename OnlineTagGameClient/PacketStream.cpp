#include "pch.h"
#include "PacketStream.h"

void  PacketStream::writeInt(int  dataIn)
{
	stringStream << dataIn << " ";
}

void  PacketStream::readInt(int  &dataOut)
{
	stringStream >> dataOut;
}

void  PacketStream::toCharArray(char*  arrayIn)
{
	std::string  s = stringStream.str();
	memcpy(arrayIn,s.c_str(), s.length());
	stringStream.str(""	);
}

void  PacketStream::fromCharArray(char*  arrayIn)
{
	stringStream.str("");  //  clear  the  old  stream    
	stringStream.str(arrayIn);//populate inputStream    
}
