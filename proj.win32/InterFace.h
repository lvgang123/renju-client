#pragma once
#include "cocos2d.h"

/**
 * ��Ϸ�߼��������ƶ�
 */
class GameLogic
{
public:
	GameLogic();
	~GameLogic();

	//�ж����ӵ��Ƿ���ȷ
	virtual BOOL bIsDropPointRight();

	//�������Ӳ���
	virtual void MoveBean();

	//AI������ӵ�ȷ��
	virtual void BestDropPoint();

	//��������ж�	0-ƽ��	1-����ʤ	2-����ʤ	
	virtual int MatchResult();

	//��¼��ʷ����
	virtual void RecordHistoricalMatch();
private:

};

GameLogic::GameLogic()
{
}

GameLogic::~GameLogic()
{
}