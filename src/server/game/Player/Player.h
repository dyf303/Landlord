#ifndef _PLAYER_H
#define _PLAYER_H

#define PROPS_COUNT      16
#define NAME_LENGTH      12

#define CARD_TERMINATE   100
#define CARD_NUMBER      17
#define BASIC_CARD        3

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
	GAME_STATUS_WAIT_START         = 0x00,
	GAME_STATUS_STARTING           = 0x01,
	GAME_STATUS_STARTED            = 0x02,
	GAME_STATUS_DEALING_CARD       = 0x03,
	GAME_STATUS_DEALED_CARD        = 0x04,
	GAME_STATUS_GRABING_LANDLORD   = 0x05,
	GAME_STATUS_GRABED_LAND_LORD   = 0x06,
	GAME_STATUS_START_OUT_CARD     = 0x07,
	GAME_STATUS_OUT_CARDING        = 0x08,
	GAME_STATUS_OUT_CARDED         = 0x09,
	GAME_STATUS_GETING_LEFTCARD    = 0x0a,
	GAME_STATUS_GETED_LEFTCARD     = 0x0b,
	GAME_STATUS_ROUNDOVERING       = 0x0c,
	GAME_STATUS_ROUNDOVERED        = 0x0d,
	GAME_STATUS_LOG_OUTING         = 0x10,
	GAME_STATUS_LOG_OUTED          = 0x20
};

enum AiGameStatus :uint8
{
	AI_GAME_STATUS_NULL          = 0x00,
	AI_GAME_STATUS_GRAD_LANDLORD = 0x01,
	AI_GAME_STATUS_OUT_CARD      = 0x02

};

enum PlayerType
{
	PLAYER_TYPE_USER         = 0x01,
	PLAYER_TYPE_REPLACE_AI   = 0x11,
	PLAYER_TYPE_AI           = 0x10
};

enum PlayerGameType
{
	PLAYER_GAME_TYPE_LANDLORD = 0x01,
	PLAYER_GAME_TYPE_FARMER   = 0x02
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

enum DeskFlag
{
	LEFT = 0,
	RITHT = 1
};

class Player
{
public:
	explicit Player(WorldSession* session);
	~Player();
	friend class WorldSession;
	friend class OutCardAi;

	void initPlayer();
	WorldSession* GetSession() const { return _session; }
	void setSession(WorldSession * session){ _session = session; }
	void loadData(PlayerInfo &pInfo);
	uint32 getid(){ return _playerInfo.id; }
	char const * GetName() { return _playerInfo.nick_name; }

	void Update(const uint32 diff);
	void UpdateExpiration(const uint32 diff);

	uint32 getDefaultLandlordUserId();
	void setDefaultLandlordUserId(uint32 id){ _defaultGrabLandlordPlayerId = id; }

	void UpdatePlayerData();
	void UpdatePlayerLevel();
	void sendTwoDesk();
	void sendThreeDesk();
	void logOutPlayer();
	bool expiration(){ return  _expiration < 0; }
	void addPlayer(Player *player);
	void setLeftPlayer(Player * left){  _left = left; }
	void setRightPlayer(Player * right){ _right = right; }
	bool LogOut(){ return _gameStatus == GAME_STATUS_LOG_OUTED; }
	bool idle(){ return _queueFlags == QUEUE_FLAGS_NULL; }
	bool inTheGame(){ return (_gameStatus & 0x0f) > GAME_STATUS_DEALED_CARD && (_gameStatus & 0x0f) < GAME_STATUS_ROUNDOVERED; }
	bool started(){ return _playerInfo.start == 1; }
	void setStart(){ _gameStatus = GAME_STATUS_STARTING; _playerInfo.start = 1; }
	void dealCards(uint8 * cards, uint8 * baseCards);
	bool roundOver(){ return _gameStatus == GAME_STATUS_ROUNDOVERED; };
	void setRoomId(uint32 roomid){ _roomid = roomid; }
	uint32 getRoomId(){ return _roomid; }
	void setPlayerType(PlayerType type){ _playerType = type; }
	PlayerType getPlayerType(){ return _playerType; }
	PlayerInfo * getPlayerInfo(){ return &_playerInfo; };
	AtQueueFlags getQueueFlags(){ return _queueFlags; }
	void setQueueFlags(AtQueueFlags flags){ _queueFlags = flags; }
	GameStatus getGameStatus(){ return _gameStatus; }
	void setGameStatus(GameStatus status){ _gameStatus = status; }
	int32 getGrabLandlordScore(){ return _grabLandlordScore; }
	int32 getLandlordId();
	void setLandlordId(uint32 id){ _landlordPlayerId = id; }
	uint32 aiGrabLandlord(uint32 score);
	uint32 calcDoubleScore();
	void resetGame();
	PlayerGameType getPlayerGameType();

	void UpdateQueueStatus();
	void UpdateAiDelay(const uint32 diff);
	void UpdateGameStatus();
	void UpdateCurOutCardsInfo(CardType cardType, uint8 * outCards,Player *outCardsPlayer,bool updateOther = false);
	void setCurOutCardPlayer(Player * player){ _curOutCardsPlayer = player; }

	void handleWaitStart();
	void handleDealCard();
	void handleGrabLandlord();
	void handleOutCard();
	void handleGetLeftPlayerCards();
	void handleRoundOver();
	void handLogOut();
	void notifyOther();
	void arraggeCard();

	void senToAll(WorldPacket* packet,bool bSelf = false);
	void sendPacket(WorldPacket* packet);
	void aiRecvPacket(WorldPacket* packet);

	void aiHandleDealCard(WorldPacket* packet);
	void aiHandleGrabLandlord(WorldPacket* packet);
	void aiHandleOutCards(WorldPacket* packet);
	void aiHandlLogout(WorldPacket* packet);
	void aiHandGame();
	bool RoundOver(uint32 outCardPlayerId);
private:
	WorldSession* _session;
	int32 _expiration;
	int32 _aiDelay;
	uint8 _cards[24];	
	uint8 _baseCards[7];
	uint32 _roomid;
	Player *_left, *_right,*_curOutCardsPlayer;
	AtQueueFlags _queueFlags;
	PlayerType _playerType;
	GameStatus _gameStatus;
	bool _start;
	uint32 _defaultGrabLandlordPlayerId;
	int32 _grabLandlordScore;
	int32 _landlordPlayerId;
	CardType _cardType;
	uint8  _outCards[24];
	uint8 _curOutCards[24];
	CardType _curOutCardType;
	int32 _winGold;

	AiGameStatus _aiGameStatus;

private:
	///// player data
	PlayerInfo _playerInfo;

};

#endif