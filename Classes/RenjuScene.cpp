#include "RenjuScene.h"
#include "DataPack.h"

bool RenjuScene::init()
{
	if (!Layer::init()) {
		return false;
	}

	// ��������״̬��ʼ��
	int i, j;
	for (i = 0; i < 15; i++) {
		for (j = 0; j < 15; j++) {
			m_szState[i][j] = PlateState::Empty;
		}
	}

	// �ڷ�����
	CreatePlate();

	// �������
	CreatBead();

	// ����
	setTouchEnabled(true);
	setTouchMode(kCCTouchesOneByOne);

	// ѡ�о���
	m_selectSprite = Sprite::create("selected.png");
	m_selectSprite->setVisible(false);
	m_selectSprite->setAnchorPoint(ccp(0.5, 0.5));
	addChild(m_selectSprite);

	// ��������
	

	return true;
}

// ��������
void RenjuScene::CreatePlate()
{
	Sprite * plate = Sprite::create("background.png");
	addChild(plate);
	plate->setPosition(ccp(0, 0));
	plate->setAnchorPoint(ccp(0, 0));
	plate->setScale(Director::sharedDirector()->getWinSize().height / plate->getContentSize().height);
}

// ��������
void RenjuScene::CreatBead()
{
	int i = 0;
	while (i < BEADNUM) {
		Bead * bead = Bead::create();
		if (i % 2 == 0) bead->InitBead(BeadType::BLACK);
		else bead->InitBead(BeadType::WHITE);
		bead->setVisible(false);
		addChild(bead);
		m_quBead.push(bead);

		CCLOG("%p", bead);

		i++;
	}
}

bool RenjuScene::onTouchBegan(Touch *touch, Event *unused_event)
{
	return true;
}

// ������β����ƶ���һ��ѡ��һ���ƶ���
void RenjuScene::onTouchEnded(Touch *touch, Event *unused_event)
{
	if (!m_bSelectd) {
		SelectLocation(touch);
	}
	else {
		DropChess(touch);
	}
}

// ������λ��
void RenjuScene::SelectLocation(Touch * touch)
{
	PlateIndex index;
	if (!Screen2Plate(touch->getLocation(), index)) {
		return;
	}

	SetSelectState(index);
}

// ����
void RenjuScene::DropChess(Touch * touch)
{
	PlateIndex index;
	if (!Screen2Plate(touch->getLocation(), index)) {
		return;
	}

	if (index.i != m_selectdLoc.i || index.j != m_selectdLoc.j){
		// ���ó��µĴ�����״̬
		SetSelectState(index);
		return;
	}

	// ����Ϊδѡ�д�����λ��
	m_bSelectd = false; 
	m_selectSprite->setVisible(false);

	// ���ø�λ��Ϊ��ѡ��״̬ TODO:
	m_szState[index.i][index.j] = PlateState::Black;

	// ����
	Bead * bead = m_quBead.front();
	bead->setPosition(Plate2Screen(index));
	bead->setVisible(true);
	m_quBead.pop();

	// ������ʷ��¼
	RecordHistory(bead);
}

void RenjuScene::SetSelectState(const PlateIndex & index)
{
	m_bSelectd = true;
	m_selectdLoc = index;
	m_selectSprite->setPosition(Plate2Screen(index));
	m_selectSprite->setVisible(true);
}

/*
 * ��Ļ����ת������������
 *
 * ����ֵ
 *  �����λ�������ӣ�����Ϊ�����⣬�򷵻�false; ���򷵻�true
 */
bool RenjuScene::Screen2Plate(const Point & ptScreen, PlateIndex & index)
{
	int i, j;	// ����λ������
	int iX, iY; // 
	iX = static_cast<int>(ptScreen.x);
	iY = static_cast<int>(ptScreen.y);

	if (iX < PlateTouchRange::Left || iX > PlateTouchRange::Right
		|| iY < PlateTouchRange::Bottom || iY > PlateTouchRange::Right) {
		return false;
	}

	i = (iX - PlateTouchRange::Left) / BeadSize::Width;
	j = (iY - PlateTouchRange::Bottom) / BeadSize::Height;
	CCLOG("Touch: [x=%d,y=%d] [i=%d,j=%d]", iX, iY, i, j);
	if (m_szState[i][j] != PlateState::Empty){
		return false;
	}

	index.i = i;
	index.j = j;
	return true;
}

/*
 * ��������ת��Ϊ��Ļ����
 */
Point RenjuScene::Plate2Screen(const PlateIndex & index)
{
	float x = (index.i + 1) * BeadSize::Width;
	float y = (index.j + 1) * BeadSize::Height;
	return ccp(x, y);
}

/*
 * ������ʷ��¼
 */
void RenjuScene::RecordHistory(Bead * bead)
{
	m_history.vec.push_back(bead);
}