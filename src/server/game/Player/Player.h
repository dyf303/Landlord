#ifndef _PLAYER_H
#define _PLAYER_H

#define PROPS_COUNT      16
#define NAME_LENGTH      12

class WorldSession;

struct PlayerInfo
{
	PlayerInfo() :id(0), icon_id(0), sex(0), gold(0), level(0), score(0), all_Chess(0), win_chess(0), 
	win_Rate(0), offline_count(0), start(0), type(0), desk_id(0)
	{
		for (uint8 i = 0; i < PROPS_COUNT; ++i)
		{
			props_count[i] = 0;
		}
		for (uint8 i = 0; i < NAME_LENGTH; ++i)
		{
			account[i] = 0;
			name[i] = 0;
			nick_name[i] = 0;
		}
	}
	uint32 							  id;				           /// 用户Id
	uint32                            icon_id;                     /// 用户头像id
	uint32                            sex;                         /// 性别 (0--男，1--女)
	uint32                            gold;                        /// 用户拥有金币数
	uint32                            level;                       /// 用户等级
	uint32 							  score;			           /// 用户积分
	uint32                            all_Chess;                   /// 总局数
	uint32                            win_chess;                   /// 胜局数
	uint32                            win_Rate;                    /// 胜率
	uint32                            offline_count;               /// 掉线次数
	uint32 							  start;			           /// 标记是否开始
	uint32							  type;	                       /// 每个人的类型(初始值是1， 0 - landlord, 1 - farmer)
	uint32					          desk_id;                     /// 桌子标识
	uint32                            props_count[PROPS_COUNT];    /// 用户道具数目
	uint8                             account[NAME_LENGTH];        /// 用户账号
	uint8                             name[NAME_LENGTH];           /// 用户姓名
	uint8                             nick_name[NAME_LENGTH];      /// 用户昵称
};

enum PlayerGameStatus
{

};

enum AtQueueFlags
{

};

class Player
{
public:
	explicit Player(WorldSession* session);
	~Player();

	void loadData(PlayerInfo &pInfo);
	uint32 getid(){ return _id; }
	void Update(const uint32 diff);
	bool expiration(){ return  _expiration < 0; }
	void addPlayer(Player *player);
	bool LogOut(){ return false; }
	bool idle(){ return false; }
	bool started(){ return true; }
	void dealCards(uint8 * cards, uint8 * baseCards);
	bool endGame(){ return false; };

private:
	WorldSession* _session;
	uint32 _expiration;
	uint8 _cards[17];
	uint8 _baseCards[3];

	Player *left, *right;
	///// player data
	uint32 							  _id;				            /// 用户Id
	uint32                            _icon_id;                     /// 用户头像id
	uint32                            _sex;                         /// 性别 (0--男，1--女)
	uint32                            _gold;                        /// 用户拥有金币数
	uint32                            _level;                       /// 用户等级
	uint32 							  _score;			            /// 用户积分
	uint32                            _all_Chess;                   /// 总局数
	uint32                            _win_chess;                   /// 胜局数
	uint32                            _win_Rate;                    /// 胜率
	uint32                            _offline_count;               /// 掉线次数
	uint32 							  _start;			            /// 标记是否开始
	uint32							  _type;	                    /// 每个人的类型(初始值是1， 0 - landlord, 1 - farmer)
	uint32					          _desk_id;                     /// 桌子标识
	uint32                            _props_count[PROPS_COUNT];    /// 用户道具数目
	uint8                             _account[NAME_LENGTH];        /// 用户账号
	uint8                             _name[NAME_LENGTH];           /// 用户姓名
	uint8                             _nick_name[NAME_LENGTH];      /// 用户昵称
};

#endif