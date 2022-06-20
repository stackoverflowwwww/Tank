#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "ui/CocosGUI.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "cocos-ext.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace cocos2d;
using namespace cocos2d::ui;
using namespace CocosDenshion;
using namespace std;

#define WINDOWHEIGHT Director::getInstance()->getVisibleSize().height
#define WINDOWWIDTH  Director::getInstance()->getVisibleSize().width

#define max_range 5

#define UserDefault CCUserDefault::sharedUserDefault()

//class Player
//{
//public:
//	std::string name;
//	int score = 0;
//	Player(){}
//	~Player(){}
//	Player(std::string s, int t) {
//		name = s;
//		score = t;
//	}
//	
//};

struct Player
{
	std::string name;
	int score;
};

#endif
