#include "stdafx.h"
#include "VertexN.h"

void GetMap(std::map<char, BMuv> &bmuv)
{
	assert(bmuv);
	bmuv.clear();
	int asciiValue = 32;
	int tableWidth = 32;
	int tableHeight = 3;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			char c = char(asciiValue);
			BMuv tempbmuv;
			tempbmuv.topLeftUV = { static_cast<float>(j) / tableWidth , static_cast<float>(i) / tableHeight };
			tempbmuv.topRightUV = { static_cast<float>(j + 1) / tableWidth , static_cast<float>(i) / tableHeight };
			tempbmuv.bottomLeftUV = { static_cast<float>(j) / tableWidth , static_cast<float>(i + 1) / tableHeight };
			tempbmuv.bottomRightUV = { static_cast<float>(j + 1) / tableWidth , static_cast<float>(i + 1) / tableHeight };

			bmuv.insert({ c, tempbmuv });
			
			asciiValue++;
		}
	}
	std::cout << "TextMap Generated!" << std::endl;
}
