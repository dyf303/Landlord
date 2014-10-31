#ifndef _PLAYER_H
#define _PLAYER_H

#define PROPS_COUNT      16
#define NAME_LENGTH      12

#define CARD_TERMINATE   100
#define CARD_NUMBER      17
#define BASIC_CARD        7

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
	char                              account[NAME_LENGTH];        /// 用户账号
	char                              name[NAME_LENGTH];           /// 用户姓名
	char                              nick_name[NAME_LENGTH];      /// 用户昵称
};

enum GameStatus:uint8
{
	GAME_STATUS_WAIT_START         = 0,
	GAME_STATUS_STARTING           = 1,
	GAME_STATUS_STARTED            = 2,
	GAME_STATUS_WAIT_DEAL_CARD     = 3, 
	GAME_STATUS_DEALED_CARD        = 4,
	GAME_STATUS_GRAB_LAND_LORDING  = 5,
	GAME_STATUS_GRAB_LAND_LORDED   = 6,
	GAME_STATUS_WAIT_OUT_CARD      = 7,
	GAME_STATUS_OUT_CARDING        = 8,
	GAME_STATUS_OUT_CARDED         = 9
};

enum PlayerType
{
	PLAYER_TYPE_AI,
	PLAYER_TYPE_USER
};

enum AtQueueFlags
{
	QUEUE_FLAGS_NULL,
	QUEUE_FLAGS_ONE,
	QUEUE_FLAGS_TWO,
	QUEUE_FLAGS_THREE
};

enum CardType
{
	CARD_TYPE_BEG,
	CARD_TYPE_PASS =     CARD_TYPE_BEG,    /// 过牌
	CARD_TYPE_SINGLE,				       /// 单支
	CARD_TYPE_PAIR,					       /// 对子
	CARD_TYPE_ROCKET,				       /// 火箭
	CARD_TYPE_BOMB,					       /// 炸弹
	CARD_TYPE_TRPILE,				       /// 三张
	CARD_TYPE_TRIPLE_ONE,			       /// 三带一
	CARD_TYPE_TRIPLE_TWO,			       /// 三带二
	CARD_TYPE_SINGLE_PROGRESSION,	       /// 单顺
	CARD_TYPE_PAIR_PROGRESSION,		       /// 双顺
	CARD_TYPE_TRIPLE_PROGRESSION,	       /// 三顺
	CARD_TYPE_AIRPLANE,				       /// 飞机带翅膀
	CARD_TYPE_FOUR_TWO,				       /// 四带二
	CARD_TYPE_END
};

class Player
{
public:
	explicit Player(WorldSession* session);
	~Player();
	friend class WorldSession;

	WorldSession* GetSession() const { return _session; }
	void loadData(PlayerInfo &pInfo);
	uint32 getid(){ return _playerInfo.id; }
	char const * GetName() { return _playerInfo.nick_name; }

	void Update(const uint32 diff);
	void UpdateAiDelay(const uint32 diff);
	void checkOutPlayer();
	void checkQueueStatus();
	void checkStart();
	uint32 getDefaultLandlordUserId();
	void setDefaultLandlordUserId(uint32 id){ _defaultGrabLandlordPlayerId = id; }
	void checkDealCards();
	void checkGrabLandlord();
	void checkOutCard();


	void sendTwoDesk();
	void sendThreeDesk();
	void logOutPlayer(uint32 id);
	bool expiration(){ return  _expiration < 0; }
	void addPlayer(Player *player);
	void setLeftPlayer(Player * left){  _left = left; }
	void setRightPlayer(Player * right){ _right = right; }
	bool LogOut(){ return false; }
	bool idle(){ return false; }
	bool started(){ return _playerInfo.start == 1; }
	void setStart(){ _gameStatus = GAME_STATUS_STARTING; _playerInfo.start = 1; }
	void dealCards(uint8 * cards, uint8 * baseCards);
	bool endGame(){ return false; };
	void setRoomId(uint32 roomid){ _roomid = roomid; }
	uint32 getRoomId(){ return _roomid; }
	void setPlayerType(PlayerType type){ _playerType = type; }
	PlayerType getPlayerType(){ return _playerType; }
	PlayerInfo * getPlayerInfo(){ return &_playerInfo; };
	AtQueueFlags getQueueFlags(){ return _queueFlags; }
	GameStatus getGameStatus(){ return _gameStatus; }
	void setGameStatus(GameStatus status){ _gameStatus = status; }
	int32 getGrabLandlordScore(){ return _grabLandlordScore; }
	int32 getLandlordId();
	uint32 aiGrabLandlord();

private:
	WorldSession* _session;
	int32 _expiration;
	int32 _aiDelay;
	uint8 _cards[CARD_NUMBER];
	uint8 _baseCards[BASIC_CARD];
	uint32 _roomid;
	Player *_left, *_right;
	AtQueueFlags _queueFlags;
	PlayerType _playerType;
	GameStatus _gameStatus;
	bool _start;
	uint32 _defaultGrabLandlordPlayerId;
	int32 _grabLandlordScore;
	int32 _landlordPlayerId;
	CardType _cardType;
	char _outCards[24];

private:
	///// player data
	PlayerInfo _playerInfo;

};

#endif