#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include "Tank.h"
#include "Brick.h"
#include "aStar.h"
#include "Global.h"

//地形ID
#define OCEAN_ID 7 //海洋
#define BRICK_ID 1	//红墙
#define BLOCK_ID 3	//白墙
#define FOREST_ID 5	//森林
#define ENEMY_POINT_ID 18 //敌方出生点
#define MY_POINT_ID 20	//我方出生点
#define BASE_ID 80  //基地

#define NONE 0

#define MY_TANK_ID 110 //玩家坦克ID
#define ENEMY_TANK_ID 220	//我方坦克ID

USING_NS_CC;
using namespace cocos2d;


class GameClient : public Scene
{
private:
	Vector<Brick*>  m_bgList;     // 背景块列表
	Vector<Tank*>   m_tankList;   // 坦克列表
	Vector<Tank*> action_doneList;
	Tank* m_tank;       // 主坦克

	Vector<Tank*>   m_shouldFireList;     // 记录需要开火的坦克 - 处理接收到开火消息的坦克

	Vector<Bullet*> m_deleteBulletList;   // 删除子弹列表
	Vector<Brick*>  m_deleteBrickList;    // 删除砖块列表
	Vector<Tank*>   m_deleteTankList;     // 删除坦克列表
	Size tileSize, visibleSize;
	TMXLayer* map_layer;

	//寻路起点指针
	mapNode* m_origin;
	//寻路终点指针
	mapNode* m_destination;
	//地图数组指针
	mapNode** m_map;

	int set_convey = 0, can_convey = 0;
	Vec2 convey_p;

	int attend_enemy = 1, all_enemy = 5;//在场敌人数,所有敌人数
	int max_num = 5;//玩家生命
	int play_rank = 1;//玩家坦克等级

	Tank** enemy = new Tank * [attend_enemy];

	Vec2 enemy_point[3];//敌方出生点
	Vec2 my_point;//我方出生点
	Vec2 my_base;//基地
	bool pau = false;


	//绘制路径的绘图节点对象
	DrawNode* m_draw;

	//计分板
	Label* scoreboard = nullptr;
	ImageView* imageView;
public:
	GameClient();
	~GameClient();
	CREATE_FUNC(GameClient);
	virtual bool init();
	static Scene* createScene();
	void createBackGround();
	void update(float delta);
	void drawBigBG(Vec2 position);

	void addEnemy(int k);
	void addFire(float t);//敌人坦克发射炮弹

	// 实现键盘回调
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

	// get
	Tank* getTank() { return m_tank; };
	Vector<Tank*> getTankList() { return m_tankList; };

	//鼠标瞬移
	void onMouseUp(Event* event);
	//初始化地图
	void initMap();
	//a*寻路
	int aStar(mapNode** map, mapNode* origin, mapNode* destination, int tag_id,Tank* tank);
	void moveOnPath(mapNode* tempNode, int tag_id,Tank* tank);
	void updatePath(float dt);

	//我方坦克全部死亡后可以进攻基地
	void attackBase();
	void gameOver(bool is_success=true);
	void success() {
		Director::getInstance()->pause();
		auto tips = Label::createWithBMFont("fonts/futura-48.fnt", "Success!");
		tips->setColor(Color3B(0, 0, 1));
		Size visibleSize = Director::getInstance()->getVisibleSize();
		tips->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
		this->addChild(tips, 10);
	}
	
	void onClick(Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
	void callback() {
		for (int i = 0; i < action_doneList.size(); i++) {
			Tank* tank=action_doneList.at(i);
			tank->action_done = 1;
		}
		action_doneList.clear();
	}
	float init_x, init_y;
	void updatePath2(float dt) {
		for (int i = 0; i < m_tankList.size(); i++) {
			auto nowTank = m_tankList.at(i);
			if (nowTank->tank_kind == 1) {
				int x = nowTank->getPositionX() / tileSize.width;
				int y = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
				int dir = rand() % 4;
				switch (dir) {
				case 0:
					nowTank->MoveDown();
					break;
				case 1:
					nowTank->MoveLeft();
					break;
				case 2:
					nowTank->MoveUP();
					break;
				case 3:
					nowTank->MoveRight();
					break;
				}
			}
		}
	}
};
static Player player = { "name",0 };
static std::vector<Player> rankings;

#endif