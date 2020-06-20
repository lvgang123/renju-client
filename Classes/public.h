#pragma once

#define BEADNUM 15 * 15

// ����λ������
struct PlateIndex {
	int i;
	int j;
};

// ��������״̬
enum PlateState {
	Empty = 0,
	Black = 1,
	White = 2,
};

// ���Ӵ�С
enum BeadSize {
	Width = 60, 
	Height = 60,
};

// ���̿ɵ����Χ
enum PlateTouchRange {
	Left = 30,
	Right = 930,
	Bottom = 30,
	Top = 930,
};