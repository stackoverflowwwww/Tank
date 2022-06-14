#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include "cocos2d.h"
#include "Tank.h"
#include "Brick.h"
#include "aStar.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

#define OCEAN_ID 7
#define BRICK_ID 1
#define BLOCK_ID 3
#define FOREST_ID 5
#define NONE 0

#define MY_TANK_ID 110

USING_NS_CC;
using namespace cocos2d;

static int tankcount = 0;     // 记录当前坦克数
static int NET_TAG = 11111;   

class GameClient : public Scene
{
public:
	GameClient();
	~GameClient();

	CREATE_FUNC(GameClient);
	virtual bool init();
	static Scene* createScene();
	void createBackGround();
	void update(float delta);
	void drawBigBG(Vec2 position);

	// 对网络传来的消息作出响应
	void addTank(int id, float x, float y, int dir, int kind);
	void addFire(Tank* tank);

	// 实现键盘回调
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event); 

	// get
	Tank* getTank() { return m_tank; };
	Vector<Tank*> getTankList() { return m_tankList; };
	//绘制路径的绘图节点对象
	DrawNode* m_draw;
private:
	Vector<Brick*>  m_bgList;     // 背景块列表
	Vector<Tank*>   m_tankList;   // 坦克列表
	Tank*           m_tank;       // 主坦克
	Vector<Tank*>	m_drawList;   // 已绘制的坦克
	
	Tank*           m_maxTank[50];        // 允许链接客户数
	Vector<Tank*>   m_shouldFireList;     // 记录需要开火的坦克 - 处理接收到开火消息的坦克

	Vector<Bullet*> m_deleteBulletList;   // 删除子弹列表
	Vector<Brick*>  m_deleteBrickList;    // 删除砖块列表
	Vector<Tank*>   m_deleteTankList;     // 删除坦克列表
	Size tileSize,visibleSize;
	TMXLayer* map_layer;
	int set_convey=0,can_convey = 0;
	Vec2 convey_p;
	void onMouseUp(Event *event) {
		EventMouse* e = (EventMouse*)event;
		convey_p.x =e->getCursorX();
		convey_p.y = e->getCursorY();
		set_convey = 1;
	}

	int enermy_num = 20;
	int max_num = 1;

	mapNode **m_map;
	void initMap() {
		//根据地图宽、高分配数组空间
		m_map = new mapNode*[MAP_WIDTH];
		for (int n = 0; n < MAP_WIDTH; n++)
			m_map[n] = new mapNode[MAP_HEIGHT];
		vector<int> not_access_gid = {BRICK_ID,BLOCK_ID,OCEAN_ID};
		//依次扫描地图数组每一个单元
		for (int i = 0; i < MAP_WIDTH; i++)
		{
			for (int j = 0; j < MAP_HEIGHT; j++)
			{
				//若当前位置为墙体瓦片设置为不可通过
				bool flag = true;
				for (int id : not_access_gid) {
					if (map_layer->getTileGIDAt(Vec2(i, j)) == id) {
						flag = false;
						break;
					}
				}
				if (!flag)
				{
					mapNode temp = { NOT_ACCESS, i, j, 0, 0, 0, nullptr };
					m_map[i][j] = temp;
				}

				//否则设置为可以通过
				else
				{
					mapNode temp = { ACCESS, i, j, 0, 0, 0, nullptr };
					m_map[i][j] = temp;
				}
			}
		}
	}
	int aStar(mapNode** map, mapNode* origin, mapNode* destination,int tag_id)
	{
		if (origin == destination) {
			return 0;
		}
		openList* open = new openList;
		open->next = nullptr;
		open->openNode = origin;
		closedList* close = new closedList;
		close->next = nullptr;
		close->closedNode = nullptr;
		//循环检验8个方向的相邻节点
		while (checkNeighboringNodes(map, open, open->openNode, destination))
		{
			//从OPEN表中选取节点插入CLOSED表
			insertNodeToClosedList(close, open);
			//若OPEN表为空，表明寻路失败
			if (open == nullptr)
			{
				break;
			}
			//若终点在OPEN表中，表明寻路成功
			if (open->openNode->status == DESTINATION)
			{
				mapNode* tempNode = open->openNode;
				//调用moveOnPath（）函数控制精灵在路径上移动
				moveOnPath(tempNode,tag_id);
				break;
			}
		}
		return 0;
	}
	void moveOnPath(mapNode* tempNode,int tag_id)
	{
		static int a = 0;
		//声明存储路径坐标的结构体
		struct pathCoordinate { int x; int y; };
		//分配路径坐标结构体数组
		pathCoordinate* path = new pathCoordinate[MAP_WIDTH*MAP_HEIGHT];
		//利用父节点信息逆序存储路径坐标
		int loopNum = 0;
		while (tempNode != nullptr)
		{
			path[loopNum].x = tempNode->xCoordinate;
			path[loopNum].y = tempNode->yCoordinate;
			loopNum++;
			tempNode = tempNode->parent;
		}
		//将笑脸精灵的坐标存为绘制线段起点
		auto smile = this->getChildByTag(tag_id);
		smile->stopAllActions();
		int fromX = smile->getPositionX();
		int fromY = smile->getPositionY();
		//声明动作向量存储动作序列
		Vector<FiniteTimeAction*> actionVector;
		//从结构体数组尾部开始扫描
		for (int j = loopNum - 2; j >= 0; j--)
		{
			//将地图数组坐标转化为屏幕实际坐标
			int realX = (path[j].x + 0.5)*UNIT;
			int realY = visibleSize.height - (path[j].y + 0.5)*UNIT;
			//创建移动动作并存入动作向量
			auto moveAction = MoveTo::create(0.2, Vec2(realX, realY));
			actionVector.pushBack(moveAction);
			//绘制从起点到下一个地图单元的线段
			//m_draw->drawLine(Vec2(fromX, fromY), Vec2(realX, realY), Color4F(1.0, 1.0, 1.0, 1.0));
			//将当前坐标保存为下一次绘制的起点
			fromX = realX;
			fromY = realY;
		}
		//创建动作序列
		auto actionSequence = Sequence::create(actionVector);
		//笑脸精灵执行移动动作序列
		if (a < 1) {
			smile->runAction(actionSequence);
		}
		a++;
	}
	void updatePath(float dt) {
		auto nowTank = m_tank;
		int x = nowTank->getPositionX() / tileSize.width;
		int y = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
		int tmp = m_map[x][y].status;
		m_map[x][y].status = DESTINATION;
		auto m_dest = &m_map[x][y];
		for (int i = 0; i < m_tankList.size(); i++) {
			auto nowTank = m_tankList.at(i);
			if (nowTank->tank_kind == 1) {
				int x = nowTank->getPositionX() / tileSize.width;
				int y = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
				int tmp = m_map[x][y].status;
				m_map[x][y].status = ORIGIN;
				m_map[x][y].parent = nullptr;
				auto m_ori = &m_map[x][y];
				aStar(m_map, m_ori, m_dest,nowTank->tag_id);
				m_map[x][y].status = tmp;
			}
		}
		m_map[x][y].status = tmp;
	}
	void enermy_shoot(float dt) {
		auto nowTank = m_tank;
		int x_t = nowTank->getPositionX() / tileSize.width;
		int y_t = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
		for (int i = 0; i < m_tankList.size(); i++) {
			auto nowTank = m_tankList.at(i);
			if (nowTank->tank_kind == 1) {
				int x = nowTank->getPositionX() / tileSize.width;
				int y = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
			}
		}
	}
};

#endif