#include "stdafx.h"
#include "VertexN.h"

void GetMap(std::map<char, BMuv>& bmuv)
{
	int asciiValue = 32;
	int tableWidth = 32;
	int tableHeight = 3;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			char c = char(asciiValue);
			BMuv tempbmuv;
			tempbmuv.topLeftU = static_cast<float>(j) / tableWidth;
			tempbmuv.topLeftV = static_cast<float>(i) / tableHeight;
			tempbmuv.topRightU = static_cast<float>(j + 1) / tableWidth;
			tempbmuv.topRightV = static_cast<float>(i) / tableHeight;
			tempbmuv.bottomLeftU = static_cast<float>(j) / tableWidth;
			tempbmuv.bottomLeftV = static_cast<float>(i + 1) / tableHeight;
			tempbmuv.bottomRightU = static_cast<float>(j + 1) / tableWidth;
			tempbmuv.bottomRightV = static_cast<float>(i + 1) / tableHeight;

			bmuv.insert({ c, tempbmuv });

			asciiValue++;
		}
	}
	std::cout << "TextMap Generated!" << std::endl;
}
