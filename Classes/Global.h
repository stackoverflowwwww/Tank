#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define WINDOWHEIGHT Director::getInstance()->getVisibleSize().height
#define WINDOWWIDTH  Director::getInstance()->getVisibleSize().width

#define max_range 5

#define UD_getInt CCUserDefault::sharedUserDefault()->getIntegerForKey 
#define UD_getBool CCUserDefault::sharedUserDefault()->getBoolForKey 
#define UD_getFloat CCUserDefault::sharedUserDefault()->getFloatForKey 
#define UD_getDouble CCUserDefault::sharedUserDefault()->getDoubleForKey 
#define UD_getString CCUserDefault::sharedUserDefault()->getStringForKey 

#define UD_setInt CCUserDefault::sharedUserDefault()->setIntegerForKey 
#define UD_setBool CCUserDefault::sharedUserDefault()->setBoolForKey 
#define UD_setFloat CCUserDefault::sharedUserDefault()->setFloatForKey 
#define UD_setDouble CCUserDefault::sharedUserDefault()->setDoubleForKey 
#define UD_setString CCUserDefault::sharedUserDefault()->setStringForKey

struct Player
{
	std::string name;
	int score=0;
};

#endif
