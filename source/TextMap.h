#pragma once

struct BMuv;

uint8_t* CascadiaData();
int32_t CascadiaWidth();
int32_t CascadiaHeight();
void DeleteTextData();
void GetCascadiaMap(std::map<char, BMuv> &bmuv);
