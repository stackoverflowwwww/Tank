#include "HelloScene.h"

Scene* HelloScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = HelloScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool HelloScene::init()
{
	if (!Layer::init())
	{
		return false;
	}

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	//添加颜色层
	auto colorLayer = LayerColor::create(Color4B(128, 125, 200, 255), visibleSize.width, visibleSize.height);
	this->addChild(colorLayer);



	loadCircle = CCSprite::create("Chapter12/tank/loading.png");
	loadCircle->setPosition(ccp(visibleSize.width / 2, visibleSize.height / 2 + visibleSize.height / 4));
	loadCircle->setScale(0.5f);
	this->addChild(loadCircle);
	float dur = 0.1;
	CCRotateTo* rotateTo = CCRotateTo::create(dur,180);
	loadCircle->runAction(CCSequence::create(rotateTo, CCCallFunc::create(this,
		callfunc_selector(HelloScene::loadsuccess)), NULL));

	loadSprite = CCSprite::create("Chapter12/tank/tank.png");
	loadSprite->setPosition(ccp(100, visibleSize.height / 4));
	loadSprite->setScale(0.6);
	CCMoveTo* moveTo = CCMoveTo::create(dur, ccp(1000, visibleSize.height / 4));

	loadSprite->runAction(moveTo);
	this->addChild(loadSprite);

	return true;
}

void HelloScene::loadsuccess() {
	loadCircle->removeFromParent();
	loadSprite->removeFromParent();
	Director::getInstance()->replaceScene(TransitionSlideInT::create(1, GameClient::createScene()));
}

