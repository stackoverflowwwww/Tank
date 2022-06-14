#include "GameClient.h"

GameClient::GameClient()
{

}

GameClient::~GameClient()
{

}

bool GameClient::init()
{
	if (!Scene::init())
	{
		return false;
	}
	visibleSize = Director::getInstance()->getVisibleSize();
	// 背景
	createBackGround();

	// 玩家
	m_tank = Tank::create(MY_TANK_ID, WINDOWWIDTH/2, 100, 1, 2);
	m_tankList.pushBack(m_tank);

	//敌人
	for (int i = 0; i < max_num; i++) {
		int x = rand() % ((int)WINDOWWIDTH), y = rand() % ((int)WINDOWHEIGHT);
		int tile_x = x / tileSize.width;
		int tile_y = (visibleSize.height - y) / tileSize.height;
		int id = map_layer->getTileGIDAt(Vec2(tile_x,tile_y));
		while (id!=0)
		{
			x = rand() % ((int)WINDOWWIDTH), y = rand() % ((int)WINDOWHEIGHT);
			tile_x = x / tileSize.width;
			tile_y = (visibleSize.height - y) / tileSize.height;
			id = map_layer->getTileGIDAt(Vec2(tile_x, tile_y));
		}
		Tank *tmp = Tank::create(220 + i, x, y, 2, 1);
		tmp->DontMove();
		m_tankList.pushBack(tmp);
		this->addChild(tmp, 1, i+2);
		tmp->tag_id = i+2;
		m_drawList.pushBack(tmp);
	}
	//添加绘图节点
	m_draw = DrawNode::create();
	this->addChild(m_draw, 2);
	//Color4F c(1, 1, 1, 1);
	//m_draw->drawCircle(Vec2(100, 300), 10, 360, 1, true,c);
	//Color4F c2(1, 0, 0, 1);
	//m_draw->drawCircle(Vec2(100, 200), 10, 360, 1, true, c2);
	initMap();
	//this->schedule(schedule_selector(GameClient::updatePath, this),0.5, kRepeatForever,0);
	// 碰撞检测
	this->scheduleUpdate();
	

	// 键盘事件
	auto key_listener = EventListenerKeyboard::create();
	key_listener->onKeyPressed = CC_CALLBACK_2(GameClient::onKeyPressed, this);
	key_listener->onKeyReleased = CC_CALLBACK_2(GameClient::onKeyReleased, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);

	auto mouse_listener = EventListenerMouse::create();
	mouse_listener->onMouseUp=CC_CALLBACK_1(GameClient::onMouseUp, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mouse_listener, this);

	this->addChild(m_tank,1,1);
	m_tank->tag_id = 1;
	m_drawList.pushBack(m_tank); // 联网后再加入，因为ID由服务器分配

	m_shouldFireList.clear(); 
	return true;
}

Scene* GameClient::createScene()
{
	auto scene = Scene::create();
	auto layer = GameClient::create();
	scene->addChild(layer);
	return scene;
}

void GameClient::update(float delta)
{
	updatePath(delta);
	// 收到传来的开火消息的坦克执行Fire
	if (m_shouldFireList.size() > 0)
	{
		auto tank = m_shouldFireList.at(0);
		tank->Fire();
		m_shouldFireList.clear();
	}

	// 维护坦克列表
	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		if (nowTank->getLife() <= 0)
		{
			m_tankList.eraseObject(nowTank);
		}
		bool notDraw = true;
		for (int j = 0; j < m_drawList.size(); j++)
		{
			auto drawTank = m_drawList.at(j);
			if (drawTank->getID() == nowTank->getID())
			{
				notDraw = false;
			}
		}

		// 绘制尚未绘制的坦克-针对后连进来的客户端
		if (notDraw)
		{
			this->addChild(nowTank);
			m_drawList.pushBack(nowTank);
		}
	}
	// 坦克与 坦克，物品的碰撞检测
	for (int i = 0; i < m_tankList.size(); i++)
	{
		auto nowTank = m_tankList.at(i);
		auto rect = nowTank->getRect();

		int min_x = (int)(rect.getMinX() / tileSize.width);
		int max_x = (int)(rect.getMaxX() / tileSize.width);
		int max_y = (int)((visibleSize.height - rect.getMinY()) / tileSize.height);
		int min_y = (int)((visibleSize.height - rect.getMaxY()) / tileSize.height);
		if (min_x < 0) {
			min_x = 0;
		}
		if (max_x >= 60) {
			max_x = 59;
		}
		if (min_y < 0) {
			min_y = 0;
		}
		if (max_y >= 40) {
			max_y = 39;
		}
		int type = NONE;
		if (nowTank->getLife()) {
			int k1, k2;
			for (k1 = min_x; k1 <= max_x; k1++) {
				for (k2 = min_y; k2 <= max_y; k2++) {
					int id = map_layer->getTileGIDAt(Vec2(k1, k2));
					if (id == OCEAN_ID || id == BRICK_ID || id == BLOCK_ID || id == FOREST_ID) {
						type = id;
						break;
					}
				}
				if (type != NONE) {
					break;
				}
			}
		}
		if (type != NONE && nowTank->tank_kind!=1) {
			if (type == BRICK_ID || type == BLOCK_ID) {
				if (nowTank->getDirection() == TANK_UP) {
					// 方法1：履带持续转动
					nowTank->setHindered(TANK_UP);
					nowTank->setPositionY(nowTank->getPositionY() - 1); // 避免检测成功后坦克持续受，无法行动造成卡住
				}
				if (nowTank->getDirection() == TANK_DOWN) {
					// 方法1：履带持续转动
					nowTank->setHindered(TANK_DOWN);
					nowTank->setPositionY(nowTank->getPositionY() + 1); // 避免检测成功后坦克持续受，无法行动造成卡住
				}
				if (nowTank->getDirection() == TANK_LEFT) {
					// 方法1：履带持续转动
					nowTank->setHindered(TANK_LEFT);
					nowTank->setPositionX(nowTank->getPositionX() + 1); // 避免检测成功后坦克持续受，无法行动造成卡住
				}
				if (nowTank->getDirection() == TANK_RIGHT) {
					// 方法1：履带持续转动
					nowTank->setHindered(TANK_RIGHT);
					nowTank->setPositionX(nowTank->getPositionX() - 1); // 避免检测成功后坦克持续受，无法行动造成卡住
				}
			}
			if (type == OCEAN_ID) {
				m_deleteTankList.pushBack(nowTank);
			}
		}
		// 坦克与坦克
		for (int j = 0; j < m_tankList.size(); j++)
		{
			auto nowTank = m_tankList.at(i);
			auto anotherTank = m_tankList.at(j);
			if ((nowTank->getLife() && anotherTank->getLife()) && (anotherTank->getID() != nowTank->getID()) && (nowTank->getRect().intersectsRect(anotherTank->getRect())))
			{
				// 正在运动的坦克才作出如下动作
				if (nowTank->getDirection() == TANK_UP && nowTank->isMoving())
				{
					nowTank->Stay(TANK_UP);
				}
				if (nowTank->getDirection() == TANK_DOWN && nowTank->isMoving())
				{
					nowTank->Stay(TANK_DOWN);
				}
				if (nowTank->getDirection() == TANK_LEFT && nowTank->isMoving())
				{
					nowTank->Stay(TANK_LEFT);
				}
				if (nowTank->getDirection() == TANK_RIGHT && nowTank->isMoving())
				{
					nowTank->Stay(TANK_RIGHT);
				}
			}
		}

		// 坦克与子弹
		auto tank = m_tankList.at(i);
		for (int j = 0; j < tank->getBulletList().size(); j++)
		{
			auto bullet = tank->getBulletList().at(j);
			for (int k = 0; k < m_tankList.size(); k++)
			{
				auto tank_another = m_tankList.at(k);
				if (tank->getID() != tank_another->getID())
				{
					if (bullet->getRect().intersectsRect(tank_another->getRect()))
					{
						bullet->hit_count += 1;
						if ((bullet->type == PENETRATE && bullet->hit_count >= 2) ||(bullet->type == NORMAL && bullet->hit_count >= 1) ) {
							// 子弹消除
							m_deleteBulletList.pushBack(bullet);
						}
						// 坦克消除
						m_deleteTankList.pushBack(tank_another);
					}
				}
			}
		}

		// 子弹和墙
		for (int j = 0; j < tank->getBulletList().size(); j++)
		{
			auto bullet = tank->getBulletList().at(j);
			auto rect = bullet->getRect();

			int min_x = (int)(rect.getMinX() / tileSize.width);
			int max_x = (int)(rect.getMaxX() / tileSize.width);
			int max_y = (int)((visibleSize.height - rect.getMinY()) / tileSize.height);
			int min_y = (int)((visibleSize.height - rect.getMaxY()) / tileSize.height);
			if (min_x < 0) {
				min_x = 0;
			}
			if (max_x >= 60) {
				max_x = 59;
			}
			if (min_y < 0) {
				min_y = 0;
			}
			if (max_y >= 40) {
				max_y = 39;
			}
			int type = NONE;

			int k1, k2;
			for (k1 = min_x; k1 <= max_x; k1++) {
				for (k2 = min_y; k2 <= max_y; k2++) {
					int id = map_layer->getTileGIDAt(Vec2(k1, k2));
					if (id == OCEAN_ID || id == BRICK_ID || id == BLOCK_ID || id == FOREST_ID) {
						type = id;
						break;
					}
				}
				if (type != NONE) {
					break;
				}
			}

			if (type != NONE) {
				if (type == BRICK_ID || type == BLOCK_ID) {
					// 子弹消除
					if (type == BRICK_ID) {
						bullet->hit_count += 1;
					}
					else {
						bullet->hit_count += 2;
					}
					if ((bullet->type == PENETRATE && bullet->hit_count >= 2) || (bullet->type == NORMAL && bullet->hit_count >= 1)) {
						// 子弹消除
						m_deleteBulletList.pushBack(bullet);
					}					
					if (type == BRICK_ID) {
						auto tile = map_layer->getTileAt(Vec2(k1, k2));
						tile->setVisible(false);
						tile->removeFromParent();
						m_map[k1][k2].status = ACCESS;
					}
				}
			}
		}
		// 清除删除子弹列表
		for (int j = 0; j < m_deleteBulletList.size(); j++)
		{
			auto bullet = m_deleteBulletList.at(j);
			m_deleteBulletList.eraseObject(bullet);
			tank->getBulletList().eraseObject(bullet);
			bullet->Blast();
		}

		// 清除删除坦克列表
		for (int j = 0; j < m_deleteTankList.size(); j++)
		{
			auto tank = m_deleteTankList.at(j);
			m_deleteTankList.eraseObject(tank);
			m_tankList.eraseObject(tank);
			tank->Blast();
		}
		m_deleteBulletList.clear();
		m_deleteBrickList.clear();
		m_deleteTankList.clear();
	}

}

// 绘制4个回字砖块
void GameClient::createBackGround()
{
	auto map = TMXTiledMap::create("Chapter12/tank/map.tmx");
	//map->setPosition(Vec2(22, 8));
	map_layer = map->getLayer("back");
	//map_layer->setPosition(Vec2(22, 8));
	tileSize=map->getTileSize();
	this->addChild(map, 10);
	
	//drawBigBG(Vec2(16 * 16, 25 * 16));
	//drawBigBG(Vec2(44 * 16, 25 * 16));
	//drawBigBG(Vec2(16 * 16, 14 * 16));
	//drawBigBG(Vec2(44 * 16, 14 * 16));
}

// 绘制单个回字砖块
void GameClient::drawBigBG(Vec2 position)
{
	for (int i = -2;i < 4;i ++)
	{
		for (int j = -2;j < 4;j ++)
		{
			if ((i == 1)&&(j == 0) || (i == 0)&&(j == 0) || (i == 1)&&(j == 1) || (i == 0)&&(j == 1))
			{
				// 中间留空形成回字
				continue;
			}
			auto brick = Brick::create(Vec2(position.x + (0.5 - i) * 16, position.y + (0.5 - j) * 16));
			m_bgList.pushBack(brick);
			this->addChild(brick, 2);
		}
	}
}

void GameClient::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		{
			m_tank->MoveLeft();
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
		// m_tank->MoveUP();
		{
			m_tank->MoveUP();
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		// m_tank->MoveDown();
		{
			m_tank->MoveDown();
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		// m_tank->MoveRight();
		{
			m_tank->MoveRight();
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F:
		if (set_convey) {
			can_convey = 1;
		}
		break;
	}
}

void GameClient::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		{
			m_tank->Stay(TANK_LEFT);
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
		{
			m_tank->Stay(TANK_UP);
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		{
			m_tank->Stay(TANK_DOWN);
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		{
			m_tank->Stay(TANK_RIGHT);
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_K:
		{
			m_tank->Fire();
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_F:
		if (can_convey) {
			can_convey = 0;
			set_convey = 0;
			Vec2 p;
			p.x = (int)(convey_p.x / tileSize.width);
			p.y = (int)((visibleSize.height - convey_p.y) / tileSize.height);
			int id = map_layer->getTileGIDAt(p);
			if (id == OCEAN_ID || id == BRICK_ID || id == BLOCK_ID) {
				return;
			}
			m_tank->setPosition(convey_p);
		}
		break;
	}

}

//////////////////////////////////////////////////////////////////////////
// 处理网络传输数据时作出的响应
void GameClient::addTank(int id, float x, float y, int dir, int kind)
{
	m_maxTank[tankcount] = Tank::create(id, x, y, dir, kind);
	m_tankList.pushBack(m_maxTank[tankcount++]);
}

void GameClient::addFire(Tank* tank)
{
	m_shouldFireList.pushBack(tank);
}
