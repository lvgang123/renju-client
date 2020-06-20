#ifndef __RENJU_SCENE_H__
#define __RENJU_SCENE_H__

#include "cocos2d.h"
USING_NS_CC;

#include "network/SocketIO.h"
using namespace cocos2d::network;

#include "Bead.h"
#include "public.h"
#include <vector>
#include <queue>
using namespace std;

// ��ʷ��¼
struct History {
	vector<Bead*> vec;
};

class RenjuScene : public Layer
{
public:
	CREATE_FUNC(RenjuScene);
	bool init();

	void CreatePlate();
	void CreatBead();

	virtual bool onTouchBegan(Touch * touch, Event * unused_event) override;
	virtual void onTouchEnded(Touch * touch, Event * unused_event) override;

	virtual void SelectLocation(Touch * touch);
	virtual void DropChess(Touch * touch);

	void SetSelectState(const PlateIndex & index);

	bool Screen2Plate(const Point & ptScreen, PlateIndex & index);
	Point Plate2Screen(const PlateIndex & index);

	void RecordHistory(Bead * bead);

private:
	// ������̵�����״̬				
	PlateState m_szState[15][15];
	
	// �ڰ�����
	queue<Bead*> m_quBead;
	
	// ����Ƿ��Ƿ�ѡ�д����ӵ�λ�ã�ѡ��Ϊtrue��δѡ��Ϊfalse				
	bool m_bSelectd;
	
	// ��¼ѡ�еĴ����ӵ�λ��				
	PlateIndex m_selectdLoc;	

	// ��ѡ��λ�ý��б�ǵľ���
	Sprite * m_selectSprite;

	// ÿ����ʷ��¼
	History m_history;

	// �����׽���
	SIOClient *m_client;

	// �Ƿ����ӷ����
	bool m_bConnect;
};

#endif // __RENJU_SCENE_H__