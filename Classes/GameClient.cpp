#include "GameClient.h"

GameClient::GameClient()
{}

GameClient::~GameClient()
{}

bool GameClient::init()
{
	if (!Scene::init())
	{
		return false;
	}
	visibleSize = Director::getInstance()->getVisibleSize();
	// 背景
	createBackGround();
	initMap();

	SimpleAudioEngine::getInstance()->playBackgroundMusic("sound/start.mp3", false);

	//data.score = 0;

	/*for (int i = 0; i < attend_enemy; i++) {
		enemy_point[i] = Vec2((int)(WINDOWWIDTH*i/attend_enemy+5), (int)WINDOWHEIGHT-10);
	}*/

	//敌人
	for (int i = 0; i < attend_enemy; i++) {
		addEnemy(i);
	}
	this->schedule(schedule_selector(GameClient::addFire), 1.0f);//设置敌人坦克每隔0.5秒射击

	//添加绘图节点
	m_draw = DrawNode::create();
	this->addChild(m_draw, 2);
	//Color4F c(1, 1, 1, 1);
	//m_draw->drawCircle(Vec2(100, 300), 10, 360, 1, true,c);
	//Color4F c2(1, 0, 0, 1);
	//m_draw->drawCircle(Vec2(100, 200), 10, 360, 1, true, c2);

	//this->schedule(schedule_selector(GameClient::updatePath, this),0.5, kRepeatForever,0);
	// 碰撞检测

	//添加关卡提示
	Label* label = Label::createWithBMFont("fonts/futura-48.fnt", "The First Pass");
	//Label* label = Label::createWithBMFont("fonts/futura-48.fnt", myWrap("Enemy:" + std::to_string(all_enemy) + "MyLife:" + std::to_string(max_num) + "Score:" + std::to_string(player.score),10));

	label->setColor(cocos2d::Color3B(255, 255, 255));
	label->setPosition(Vec2(visibleSize.width / 2 - label->getWidth(), visibleSize.height / 2));
	label->runAction(CCSequence::create(CCDelayTime::create(2), CCFadeOut::create(1), CCRemoveSelf::create(true), NULL));//2秒后消失
	this->addChild(label, 2);

	// 键盘事件
	auto key_listener = EventListenerKeyboard::create();
	key_listener->onKeyPressed = CC_CALLBACK_2(GameClient::onKeyPressed, this);
	key_listener->onKeyReleased = CC_CALLBACK_2(GameClient::onKeyReleased, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_listener, this);

	auto mouse_listener = EventListenerMouse::create();
	mouse_listener->onMouseUp = CC_CALLBACK_1(GameClient::onMouseUp, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mouse_listener, this);

	// 玩家
	m_tank = Tank::create(MY_TANK_ID, WINDOWWIDTH / 2, 100, 1, 2);

	this->addChild(m_tank, 1, 1);
	m_tank->tag_id = MY_TANK_ID;
	m_tankList.pushBack(m_tank);
	//updatePath(1.0f);

	m_shouldFireList.clear();


	string theinfo = "Enemy:" + std::to_string(all_enemy) + "     MyLife:" + std::to_string(max_num) + "     Score:" + std::to_string(player.score);
	scoreboard = Label::createWithBMFont("fonts/futura-48.fnt", theinfo);
	scoreboard->setScale(0.6);
	scoreboard->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - scoreboard->getHeight() - 20));
	this->addChild(scoreboard, 1);

	this->scheduleUpdate();
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
	//updatePath(delta);
	// 收到传来的开火消息的坦克执行Fire
	string theinfo = "Enemy:" + std::to_string(all_enemy) + "     MyLife:" + std::to_string(max_num) + "     Score:" + std::to_string(player.score);
	scoreboard->setString(theinfo);

	for (int i = 0; i < m_shouldFireList.size(); i++) {
		auto tank = m_shouldFireList.at(i);
		tank->Fire();
	}
	m_shouldFireList.clear();


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
		if (type != NONE && nowTank->tank_kind != 1) {
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
				if (nowTank->tag_id == MY_TANK_ID) {
					if (max_num > 0) {

						m_tankList.pushBack(m_tank);
						this->addChild(m_tank, 1, 1);
						m_tank->tag_id = MY_TANK_ID;
						m_tank = Tank::create(MY_TANK_ID, WINDOWWIDTH / 2, 100, 1, 2);
					}
					else
					{
						attackBase();
					}
					max_num--;
				}

			}
		}
		// 坦克与坦克
		for (int j = 0; j < m_tankList.size(); j++) {
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
			if (anotherTank->getID() == nowTank->getID()) {

			}
		}
		// 基地与子弹
		auto tank = m_tankList.at(i);
		auto base = this->getChildByTag(BASE_ID);
		for (int j = 0; j < tank->getBulletList().size(); j++)
		{
			auto bullet = tank->getBulletList().at(j);

			if (base->getBoundingBox().intersectsRect(bullet->getRect()))
			{
				base->setVisible(false);
				auto baseSprite = Sprite::create("Chapter12/tank/base1.png");
				baseSprite->setPosition(31 * tileSize.width, 1 * tileSize.height);
				this->addChild(baseSprite, 3, BASE_ID);

				bullet->hit_count += 1;
				//if ((bullet->type == PENETRATE && bullet->hit_count >= 2) || (bullet->type == NORMAL && bullet->hit_count >= 1)) {
					// 子弹消除
				bullet->setLife(0);
				m_deleteBulletList.pushBack(bullet);
				gameOver();

			}


		}
		// 坦克与子弹
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
						//if ((bullet->type == PENETRATE && bullet->hit_count >= 2) || (bullet->type == NORMAL && bullet->hit_count >= 1)) {
							// 子弹消除
						bullet->setLife(0);
						m_deleteBulletList.pushBack(bullet);

						//}
						//if (bullet->type == PENETRATE) {

						//}
						int life = tank_another->getLife() - 1;
						if (life > 0) {
							tank_another->setLife(life);
						}
						else {
							// 坦克消除并创建新的坦克
							
							int level ;
							if (tank->getLevel() < 8) {
								level = tank->getLevel() + 1;
							}
							else
								level = 8;
							tank->setLevel(level);
							tank->setLife(level);
							CCParticleSystemQuad* ps = CCParticleSystemQuad::create
							("Particle/update.plist");
							ps->setPosition(tank->getPosition());
							ps->setDuration(1);
							ps->setAutoRemoveOnFinish(true);
							ps->setScaleX(0.08);
							ps->setScaleY(0.12);
							this->addChild(ps);

							int m_textureX = ((tank->getLevel() - 1) * 4 + 1) * 14;
							int m_textureY = 1 * 14;
							tank->setTextureRect(Rect(m_textureX - 19.0, m_textureY - 19.0, 28, 28));
							//tank->setPosition(Vec2(tank->getPosition().x + 1, tank->getPosition().y + 1));
							m_deleteTankList.pushBack(tank_another);

							SimpleAudioEngine::getInstance()->playEffect("sound/explosion.wav", false);
							if (tank_another->tag_id == MY_TANK_ID) {

								if (max_num > 0) {
									m_tank = Tank::create(MY_TANK_ID, WINDOWWIDTH / 2, 100, 1, 2);

									this->addChild(m_tank, 1, 1);
									m_tank->tag_id = MY_TANK_ID;
									m_tankList.pushBack(m_tank);

								}
								else
								{
									attackBase();
								}
								max_num--;
							}
							else {
								CCParticleSystemQuad* pss = CCParticleSystemQuad::create
								("Particle/ring.plist");
								pss->setPosition(tank_another->getPosition());
								pss->setDuration(1);
								pss->setAutoRemoveOnFinish(true);
								pss->setScaleX(0.08);
								pss->setScaleY(0.08);
								this->addChild(pss);
								int thenum = tank_another->getLevel() * 50;
								auto label = Label::createWithBMFont("fonts/futura-48.fnt", StringUtils::format("%d", thenum));
								this->addChild(label, 5);
								label->setScale(0.5);
								label->setPosition(tank_another->getPosition());
								label->runAction(CCSequence::create(CCDelayTime::create(1), CCFadeOut::create(1), CCRemoveSelf::create(true), NULL));//2秒后消失
								player.score = player.score + thenum;



								if (all_enemy > attend_enemy && m_tankList.size() <= 4) {
									int en = tank_another->tag_id - ENEMY_TANK_ID;
									addEnemy(en);
									//this->schedule(schedule_selector(GameClient::updatePath, this), 0.5, kRepeatForever, 0);
									all_enemy--;
								}
								else if (all_enemy == 0) {
									gameOver();
								}
								else {
									all_enemy--;
								}

							}
						}


					}
				}
			}
		}

		// 子弹与子弹
		for (int j = 0; j < tank->getBulletList().size(); j++)
		{
			auto bullet = tank->getBulletList().at(j);
			for (int k = 0; k < m_tankList.size(); k++)
			{
				auto tank_another = m_tankList.at(k);
				if (tank->getID() != tank_another->getID())

				{
					for (int m = 0; m < tank_another->getBulletList().size(); m++) {
						auto bullet_another = tank_another->getBulletList().at(m);

						if (bullet->getRect().intersectsRect(bullet_another->getRect()))
						{
							bullet->hit_count += 1;
							bullet_another->hit_count++;
							//if ((bullet->type == PENETRATE && bullet->hit_count >= 2) || (bullet->type == NORMAL && bullet->hit_count >= 1)) {
								// 子弹消除
							m_deleteBulletList.pushBack(bullet);
							bullet->setLife(0);
							//}
							// 子弹消除
							//if ((bullet_another->type == PENETRATE && bullet_another->hit_count >= 2) || (bullet_another->type == NORMAL && bullet_another->hit_count >= 1)) {
							m_deleteBulletList.pushBack(bullet_another);
							bullet_another->setLife(0);
							//}
						}

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
					//if ((bullet->type == PENETRATE && bullet->hit_count >= 2) || (bullet->type == NORMAL && bullet->hit_count >= 1)) {
						// 子弹消除
					bullet->setLife(0);
					m_deleteBulletList.pushBack(bullet);

					//}
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
		for (int j = 0; j < m_deleteTankList.size(); j++) {

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
void GameClient::addEnemy(int k) {
	int x = rand() % ((int)WINDOWWIDTH), y = rand() % ((int)WINDOWHEIGHT);
	int tile_x = x / tileSize.width;
	int tile_y = (visibleSize.height - y) / tileSize.height;
	int id = map_layer->getTileGIDAt(Vec2(tile_x, tile_y));
	while (id != 0)
	{
		x = rand() % ((int)WINDOWWIDTH), y = rand() % ((int)WINDOWHEIGHT);
		tile_x = x / tileSize.width;
		tile_y = (visibleSize.height - y) / tileSize.height;
		id = map_layer->getTileGIDAt(Vec2(tile_x, tile_y));
	}
	enemy[k] = Tank::create(ENEMY_TANK_ID, x, y, 2, 1);

	//enemy[i] = Tank::create(ENEMY_TANK_ID, enemy_point[i].x, enemy_point[i].y, 2, 1);
	enemy[k]->DontMove();

	m_tankList.pushBack(enemy[k]);
	this->addChild(enemy[k], 1, k + ENEMY_TANK_ID);
	enemy[k]->tag_id = k + ENEMY_TANK_ID;
}



//进攻基地

void GameClient::attackBase() {

}
void GameClient::gameOver() {
	Director::getInstance()->pause();//停止坦克们的动作




	////添加颜色层
	//auto colorLayer = LayerColor::create(Color4B(128, 125, 200, 255), 480, visibleSize.height);
	//colorLayer->setPosition(Vec2(200, 0));
	//this->addChild(colorLayer);

	//ui::ListView* lv = ui::ListView::create();
	//lv->setDirection(ui::ScrollView::Direction::VERTICAL);//设置方向为垂直方向
	//lv->setBounceEnabled(true);
	//lv->setBackGroundImage("white_bg.png");
	//lv->setBackGroundImageScale9Enabled(true);
	//lv->setContentSize(Size(300, visibleSize.height));
	//lv->setAnchorPoint(Vec2(0.5, 0.5));
	//lv->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	//lv->setItemsMargin(10);
	//colorLayer->addChild(lv);




	//auto size = Size(300, 100);
	//for (int i = 0; i < 15; ++i)
	//{
	//	auto image = ui::ImageView::create("test.png");
	//	image->setPosition(Vec2(image->getContentSize().width / 2, size.height / 2));

	//	//listView的item需要用Layout对象
	//	auto layout = cocos2d::ui::Layout::create();

	//	layout->setBackGroundImageScale9Enabled(true);


	//	//或者设计背景色
	//	//layout->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
	//	//layout->setBackGroundColor(Color3B(255, 255, 255));

	//	layout->setContentSize(size);

	//	layout->addChild(image);
	//	lv->addChild(layout);

	//}




	//Director::getInstance()->replaceScene(RankList::createScene());

	auto button = Button::create("Chapter12/tank/replay.png", "Chapter12/tank/replay.png", "Chapter12/tank/replay.png");//添加restart按钮
	button->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 8));
	button->ignoreContentAdaptWithSize(false);
	button->setContentSize(visibleSize / 5);
	this->addChild(button, 12);
	button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
		switch (type)
		{
		case ui::Widget::TouchEventType::BEGAN:
			break;
		case ui::Widget::TouchEventType::ENDED:
			Director::getInstance()->replaceScene(TransitionSlideInT::create(1, GameClient::createScene()));
			Director::getInstance()->resume();
			break;
		default:
			break;
		}
		});
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
		SimpleAudioEngine::getInstance()->playEffect("sound/shot.wav", false);
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
			/*int x = convey_p.x, y = convey_p.y;
			m_map[x][y].status = DESTINATION;
			m_destination = &m_map[x][y];*/

			m_draw->clear();
			updatePath(1.0);
		}
		break;



	case cocos2d::EventKeyboard::KeyCode::KEY_P:
	{
		pau = !pau;
		if (pau) {
			Director::getInstance()->pause();
		}
		else {
			Director::getInstance()->resume();
		}

	}
	break;
	case cocos2d::EventKeyboard::KeyCode::KEY_H: {

	}
											   break;
	case cocos2d::EventKeyboard::KeyCode::KEY_R:
	{
		gameOver();

	}
	break;



	}

}

//所有敌方坦克进入待射击列表
void GameClient::addFire(float t)
{
	for (int i = 0; i < attend_enemy; i++) {
		m_shouldFireList.pushBack(enemy[i]);
	}
}

void GameClient::onMouseUp(Event* event) {
	EventMouse* e = (EventMouse*)event;
	convey_p.x = e->getCursorX();
	convey_p.y = e->getCursorY();
	set_convey = 1;


}
void GameClient::initMap() {
	//根据地图宽、高分配数组空间
	m_map = new mapNode * [MAP_WIDTH];
	for (int n = 0; n < MAP_WIDTH; n++)
		m_map[n] = new mapNode[MAP_HEIGHT];
	vector<int> not_access_gid = { BRICK_ID,BLOCK_ID,OCEAN_ID };
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

void GameClient::updatePath(float dt) {
	if (max_num > 0) {
		int x = m_tank->getPositionX() / tileSize.width;
		int y = (visibleSize.height - m_tank->getPositionY()) / tileSize.height;
		int tmp_ = m_map[x][y].status;
		m_map[x][y].status = DESTINATION;
		auto m_dest = &m_map[x][y];


		for (int i = 0; i < m_tankList.size(); i++) {
			auto nowTank = m_tankList.at(i);

			if (nowTank->tank_kind == 1) {
				nowTank->stopActionByTag(ENEMY_TANK_ID);
				int x = nowTank->getPositionX() / tileSize.width;
				int y = (visibleSize.height - nowTank->getPositionY()) / tileSize.height;
				int tmp = m_map[x][y].status;
				m_map[x][y].status = ORIGIN;
				m_map[x][y].parent = nullptr;
				auto m_ori = &m_map[x][y];
				aStar(m_map, m_ori, m_dest, nowTank->tag_id);
				m_map[x][y].status = tmp;
			}
		}
		m_map[x][y].status = tmp_;
	}

}

int GameClient::aStar(mapNode** map, mapNode* origin, mapNode* destination, int tag_id)
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
			moveOnPath(tempNode, tag_id);
			break;
		}
	}
	return 0;
}
void GameClient::moveOnPath(mapNode* tempNode, int tag_id)
{
	//声明存储路径坐标的结构体
	struct pathCoordinate { int x; int y; };
	//分配路径坐标结构体数组
	pathCoordinate* path = new pathCoordinate[MAP_WIDTH * MAP_HEIGHT];
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
		int realX = (path[j].x + 0.5) * UNIT;
		int realY = visibleSize.height - (path[j].y + 0.5) * UNIT;
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
	actionSequence->setTag(ENEMY_TANK_ID);
	//笑脸精灵执行移动动作序列

	smile->runAction(actionSequence);

}

// 绘制背景地图
void GameClient::createBackGround()
{
	auto map = TMXTiledMap::create("Chapter12/tank/map1.tmx");
	//map->setPosition(Vec2(22, 8));
	map_layer = map->getLayer("back");
	//map_layer->setPosition(Vec2(22, 8));
	tileSize = map->getTileSize();
	this->addChild(map, 10);

	m_map = nullptr;
	//寻路起点指针

	m_destination = nullptr;
	auto baseSprite = Sprite::create("Chapter12/tank/base.png");
	baseSprite->setPosition(31 * tileSize.width, 1 * tileSize.height);
	this->addChild(baseSprite, 3, BASE_ID);
}

//void GameClient::menuSubmitCallback(Ref* pSender)
//{
//	// 获取提交的成绩
//	p[max_range].name = textEdit->getString();
//	p[max_range].score = player.score;
//
//	bool isExist = false;
//	// 玩家是否已经在排行榜
//	for (int i = 0; i < max_range; i++) {
//		if (p[i].name == p[max_range].name) {
//			p[i].score = p[i].score > p[max_range].score ? p[i].score : p[max_range].score;
//			isExist = true;
//			break;
//		}
//	}
//
//	if (!isExist) {
//		// 排个序（冒泡）
//		for (int i = 0; i < max_range; i++) {
//			for (int j = max_range - i; j > 0; j--) {
//				if (p[j].score > p[j - 1].score) {
//					Player temp;
//					temp = p[j];
//					p[j] = p[j - 1];
//					p[j - 1] = temp;
//				}
//			}
//		}
//	}
//
//	// 存入XML
//	for (int i = 1; i <= max_range; i++) {
//		// 给 XML 相应内容赋值
//		UD_setString(StringUtils::format("p%d_name", i).c_str(), p[i - 1].name);
//		UD_setInt(StringUtils::format("p%d_score", i).c_str(), p[i - 1].score);
//	}
//}





