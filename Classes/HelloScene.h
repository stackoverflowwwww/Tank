#ifndef __HelloScene_H__
#define __HelloScene_H__

#include "Tank.h"
#include "Brick.h"
#include "aStar.h"
#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "GameClient.h"
#include "Global.h"
USING_NS_CC;

using namespace cocos2d;
using namespace cocos2d::ui;
using namespace CocosDenshion;

class HelloScene : public cocos2d::Layer
{
private:
	CCSprite* loadCircle;
	CCSprite* loadSprite;
public:
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	void loadsuccess();
	// implement the "static create()" method manually
	CREATE_FUNC(HelloScene);

};

#endif // __HELLOWORLD_SCENE_H__
